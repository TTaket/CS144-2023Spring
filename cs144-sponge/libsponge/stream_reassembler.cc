#include "stream_reassembler.hh"

#include <iostream>
//! Ha Tran
//! 2020-09-26
//! 20aut CS144 Lab 1

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , unasmb()
    , first_unasmb(0)
    , n_unasmb_bytes(0)
    , eof_set(false)
    , last_byte(0) {}

//! \details Look through existing unassembled chunks to
//! 1) check if new chunk already contained in an existing chunk
//!      -> return false (signal caller to ignore/skip over new chunk)
//! 2) merge any existing chunks that overlaps into the new chunk
//!      -> return true (caller can add to output byte stream or unassembled set)
bool StreamReassembler::merge(std::list<Chunk> &chunks, Chunk &new_chunk) {
    // [new0, new1) new chunk's byte number range
    // [cur0, cur1) cur chunk's byte number ramge
    const size_t new0 = new_chunk.index;
    const size_t new1 = new_chunk.index + new_chunk.data.size();

    // if new chunk of data overlaps with any existing unassembled chunk, merge them
    auto it = chunks.begin();
    while (it != chunks.end()) {
        const size_t cur0 = it->index;
        const size_t cur1 = it->index + it->data.size();
        if (cur0 <= new0 && new1 <= cur1) {
            // new data already contained in an existing chunk, exit early
            return false;
        } else if (new0 <= cur0 && cur1 <= new1) {
            // new chunk encapsulates an existing chunk, remove to replace w/ new
            n_unasmb_bytes -= it->data.size();
            it = chunks.erase(it);
        } else if (cur0 <= new0 && new0 <= cur1) {
            // new chunk overlaps end of current chunk, prepend cur chunk's data to new data
            new_chunk.data = it->data.substr(0, new0 - cur0) + new_chunk.data;
            new_chunk.index = cur0;
            n_unasmb_bytes -= it->data.size();
            it = chunks.erase(it);
        } else if (new0 <= cur0 && cur0 <= new1) {
            // new chunk overlaps beginning of cur chunk, append
            new_chunk.data += it->data.substr(new1 - cur0);
            n_unasmb_bytes -= it->data.size();
            it = chunks.erase(it);
        } else {
            // no overlap
            it++;
        }
    }
    return true;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (_output.input_ended())
        return;
    if (eof) {
        eof_set = true;
        last_byte = index + data.size();
    }

    // some part of new chunk of data is within the receiver advertised window
    const size_t first_unacceptable = first_unasmb + _capacity - _output.buffer_size();
    if (index + data.size() >= first_unasmb && index < first_unacceptable) {
        Chunk new_chunk = {data, index};

        // if data too long to fit in window, chop off back bits
        if (new_chunk.index + new_chunk.data.size() > first_unacceptable) {
            new_chunk.data = new_chunk.data.substr(0, first_unacceptable - new_chunk.index);
        }
        // if data overlaps ByteStream, chop off front bits
        if (new_chunk.index < first_unasmb) {
            new_chunk.data = new_chunk.data.substr(first_unasmb - new_chunk.index);
            new_chunk.index = first_unasmb;
        }

        // if new chunk is already contained in an existing chunk, return early
        if (not merge(unasmb, new_chunk))
            return;

        // if can be assembled, write directly to ByteStream, else add to unasmb
        if (new_chunk.index == first_unasmb) {
            _output.write(new_chunk.data);
            first_unasmb += new_chunk.data.size();
        } else {
            n_unasmb_bytes += new_chunk.data.size();
            unasmb.push_back(new_chunk);
        }
    }

    // after potentially updating _output ByteStream, check if we're done writing
    if (eof_set && last_byte <= first_unasmb)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return n_unasmb_bytes; }

bool StreamReassembler::empty() const { return unasmb.empty(); }
