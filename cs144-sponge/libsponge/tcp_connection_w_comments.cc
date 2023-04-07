#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _last_segm_recv_timer; }

void TCPConnection::try_switching_close_mode() {
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended())
        _linger_after_streams_finish = false;
}

bool TCPConnection::syn_sent() { return _sender.next_seqno_absolute() > 0; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _last_segm_recv_timer = 0;
    if (seg.header().rst) {
        reset();
    }
    _receiver.segment_received(seg);

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }
    
    //cout<<">> received segm: "<<seg.header().summary()<<endl;
    //cout<<">> unasmb bytes: "<<_receiver.unassembled_bytes()<<endl;

    if (syn_sent()) {
        // if segment non-empty and we need to return an ack but no segments are ready to be sent out
        // makes sure at least one segment is sent in reply
        if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
            //cout<<">> recv'd non-empty segm but queue empty, make empty segment to ack"<<endl;
            _sender.send_empty_segment();
        }
        send_segments();
    }
    
    if (seg.header().syn && !syn_sent()) {
        connect();
    }
    // cout<<">> >> stream out not oef? "<<_sender.stream_in().eof()
    //         <<": " <<_sender.stream_in().input_ended()
    //         <<" && "<<_sender.stream_in().buffer_empty()<<endl
    //     <<">> >> stream in input ended ie. fin recv'd? "<<_receiver.stream_out().input_ended()<<endl;
    try_switching_close_mode();
}

bool TCPConnection::active() const { return _active; }

void TCPConnection::send_segments(bool rst) {
    if (!active())
        return;

    while (!_sender.segments_out().empty()) {
        //? would you ever *not* send a segment? window size?? isn't that 
        // taken care of in the sender??

        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        auto possible_ackno = _receiver.ackno();
        if (possible_ackno.has_value()) {
            seg.header().ack = true;
            seg.header().ackno = possible_ackno.value();
            seg.header().win = _receiver.window_size();
        }
        if (rst || _sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) 
            seg.header().rst = true;

        _segments_out.push(seg);

        //cout<<">> sending segment: "<<seg.header().summary()<<endl;
    }
}

size_t TCPConnection::write(const string &data) {
    if (_sender.stream_in().input_ended() || !syn_sent())
        return 0;
    DUMMY_CODE(data);  
    size_t n_bytes_written = _sender.stream_in().write(data);

    //cout<<">> writing "<<n_bytes_written<<" bytes, "<<_sender.stream_in().remaining_capacity()<<" bytes remaining."<<endl;
    _sender.fill_window(); 
    send_segments();
    return n_bytes_written;
}

void TCPConnection::try_closing_connection() { 
    send_segments();
    if (!active())
        return;

    //cout<<">> try closing connection: "<<endl;
    bool active_close = _linger_after_streams_finish 
                        && _last_segm_recv_timer >= 10 * _cfg.rt_timeout;
    bool pr1 = _receiver.stream_out().input_ended() && _receiver.unassembled_bytes() == 0;
    bool pr2 = _sender.stream_in().eof() 
        && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 
        && _sender.bytes_in_flight() == 0; //? > 0

    //cout<<">> >> linger? "<<_linger_after_streams_finish<<endl
    //    <<">> >> active close: "<<active_close<<endl
    //    <<">> >> passive close: "<<!_linger_after_streams_finish<<endl
    //    <<">> >> prereq 1: "<<pr1<<endl
    //    <<">> >> prereq 2 & 3: "<<pr2<<endl;
    if ((active_close || !_linger_after_streams_finish) && pr1 && pr2) {
        //cout << ">> closing..." << endl;
        _active = false;
    }
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!active())
        return;

    _sender.tick(ms_since_last_tick); 

    _last_segm_recv_timer += ms_since_last_tick;
    //cout<<">> tick! "<<ms_since_last_tick<<" : "
    //   <<_last_segm_recv_timer<<" / "<<10*_cfg.rt_timeout<<endl;

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS
            && _sender.segments_out().empty()) {
        //cout<<"too many consecutive retx, send RST"<<endl;
        _sender.send_empty_segment();
    }

    // might need to resend a segment if _sender.tick timer expires
    // or send a reset segment
    send_segments(); 
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS)
        reset();

    // if inbound  stream ends before tcp connection reach eof on outbound stream
    //cout<<">> >> stream out not oef? "<<_sender.stream_in().eof()
    //        <<": " <<_sender.stream_in().input_ended()
    //        <<" && "<<_sender.stream_in().buffer_empty()<<endl
    //    <<">> >> stream in input ended ie. fin recv'd? "<<_receiver.stream_out().input_ended()<<endl;
    try_switching_close_mode();
    try_closing_connection();
}

void TCPConnection::end_input_stream() { 
    //cout<<">> ending input stream"<<endl;
    _sender.stream_in().end_input(); 
    _sender.fill_window();
    try_closing_connection();
}

void TCPConnection::connect() {
    //cout<<endl<<">> connect"<<endl;
    _sender.fill_window();
    send_segments();
}


void TCPConnection::reset() {
    //cout<<">> RESET, close stream"<<endl;

    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            if (_sender.segments_out().empty())
                _sender.send_empty_segment();
            send_segments(true);
            reset();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
