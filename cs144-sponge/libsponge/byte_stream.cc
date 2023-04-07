#include "byte_stream.hh"

#include <iostream>

/** Ha Tran
 * 2020-09-21
 * Ring buffer queue implementation from CS106B Section 5
 *
 */
template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : done_writing(false)
    , buffer(capacity)
    , data_begin(0)
    , n_bytes_read(0)
    , n_bytes_written(0)
    , cur_size(0)
    , max_capacity(capacity)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    const size_t n_bytes_to_write = std::min(data.length(), remaining_capacity());

    for (size_t i = 0; i < n_bytes_to_write; i++) {
        buffer[(data_begin + cur_size + i) % max_capacity] = data[i];
    }
    n_bytes_written += n_bytes_to_write;
    cur_size += n_bytes_to_write;
    return n_bytes_to_write;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // if requesting more bytes than what's in the buffer,
    // return as many as possible (i.e. all bytes written in buffer)

    // num bytes to return, max is num bytes in buffer
    size_t len_ret = std::min(len, cur_size);

    // data is in one contiguous block in buffer
    if (data_begin + len_ret <= max_capacity) {
        std::string s(buffer.begin() + data_begin, buffer.begin() + data_begin + len_ret);
        return s;
    }

    // data is wrapped around the buffer, broken up in the middle
    std::string s1(buffer.begin() + data_begin, buffer.end());
    std::string s2(buffer.begin(), buffer.begin() + (data_begin + len_ret) % max_capacity);
    s1.append(s2);
    return s1;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    data_begin = (data_begin + len) % max_capacity;
    n_bytes_read += std::min(len, cur_size);
    cur_size = std::max(size_t(0), cur_size - len);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string ret = peek_output(len);
    pop_output(ret.length());
    return ret;
}

void ByteStream::end_input() { done_writing = true; }

bool ByteStream::input_ended() const { return done_writing; }

size_t ByteStream::buffer_size() const { return cur_size; }

bool ByteStream::buffer_empty() const { return cur_size == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return n_bytes_written; }

size_t ByteStream::bytes_read() const { return n_bytes_read; }

size_t ByteStream::remaining_capacity() const { return max_capacity - cur_size; }
