#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <iostream>
#include <random>

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _outstanding_segments()
    , _timer{0, retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _abs_ackno; }

void TCPSender::fill_window() {
    while (_next_seqno <= _abs_ackno + _window_size) {
        // this means we've already sent the segment with FIN flag
        if (_stream.eof() && _next_seqno >= _stream.bytes_written() + 2)
            return;

        uint16_t n_bytes_to_send = _window_size - bytes_in_flight();

        // if space left to fill in window is more than the max payload
        // or we sent out more bytes than the window size, n_bytes_to_send overflows

        // if the receiver reports a window size of 0 but we have stuff to send
        if (_window_size == 0 && bytes_in_flight() == 0)
            n_bytes_to_send = 1;

        TCPHeader hdr;
        hdr.seqno = wrap(_next_seqno, _isn);
        if (_next_seqno == 0 && n_bytes_to_send > 0) {
            hdr.syn = true;
            n_bytes_to_send--;
        }

        if (n_bytes_to_send > TCPConfig::MAX_PAYLOAD_SIZE)
            n_bytes_to_send = TCPConfig::MAX_PAYLOAD_SIZE;

        // fill as many bytes as we can from the stream
        TCPSegment seg;
        seg.payload() = _stream.read(n_bytes_to_send);
        n_bytes_to_send = n_bytes_to_send == TCPConfig::MAX_PAYLOAD_SIZE ? 1 : n_bytes_to_send - seg.payload().size();

        // include the FIN flag if it fits
        if (_stream.eof() && (n_bytes_to_send > 0))
            hdr.fin = true;

        seg.header() = hdr;

        // if the segment is empty (no flags or data) don't send
        if (seg.length_in_sequence_space() == 0)
            return;
        _segments_out.push(seg);
        _outstanding_segments.push_back(OrderedSegment{_next_seqno, seg});
        _next_seqno += seg.length_in_sequence_space();

        // if the timer isn't running, start it with the original rtto
        if (_timer.expired())
            _timer.start(_initial_retransmission_timeout);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _window_size = window_size;  // update window size even if we get a wacko ackno?

    uint64_t new_abs_ackno = unwrap(ackno, _isn, _abs_ackno);
    if (new_abs_ackno <= _abs_ackno || new_abs_ackno > _next_seqno)
        return;
    _abs_ackno = new_abs_ackno;

    for (auto it = _outstanding_segments.begin();
         it != _outstanding_segments.end() && it->seqno + it->segment.length_in_sequence_space() <= _abs_ackno;
         it = _outstanding_segments.erase(it))

        // only restart timer if there are new complete segments confirmed to be received
        _timer.start(_initial_retransmission_timeout);
    _n_consec_retransmissions = 0;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _timer.time_elapsed += ms_since_last_tick;
    if (debug)
        cout << ">> timer at: " << _timer.time_elapsed << " / " << _timer.timeout << endl;
    if (_timer.expired()) {
        if (debug)
            cout << ">> >> timer expired!" << endl;
        // retransmit earliest segment not fully acknowledged
        if (!_outstanding_segments.empty()) {
            if (debug)
                cout << ">> >> resending segm: " << _outstanding_segments.begin()->segment.header().summary() << endl;
            _segments_out.push(_outstanding_segments.begin()->segment);

            // only backoff if we had to resend a segment
            if (_window_size > 0) {
                _n_consec_retransmissions++;
                _timer.timeout *= 2;
            }
            _timer.start(_timer.timeout);
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _n_consec_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPHeader hdr;
    TCPSegment seg;
    hdr.seqno = wrap(_next_seqno, _isn);  // always set seqno
    seg.header() = hdr;
    _segments_out.push(seg);
}
