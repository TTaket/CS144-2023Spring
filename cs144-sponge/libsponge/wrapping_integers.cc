#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

const uint64_t MOD32 = (1ul << 32);
//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{uint32_t((n + isn.raw_value()) % (1ul << 32))};
}

uint64_t diff(uint64_t a, uint64_t b) { return (a < b) ? b - a : a - b; }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // absolute value of n wrt isn
    const uint32_t abs_n = (n.raw_value() >= isn.raw_value()) ? n - isn : (1ul << 32) - (isn - n);

    // number of wraparounds if we put checkpoint into a WrappingInt32
    uint64_t n_wraps = checkpoint / (1ul << 32);
    const uint64_t a = abs_n + n_wraps * (1ul << 32);
    // make sure a and b: one above, one below checkpoint
    n_wraps += a < checkpoint ? 1 : -1;
    const uint64_t b = abs_n + n_wraps * (1ul << 32);

    return (diff(checkpoint, a) > diff(checkpoint, b)) ? b : a;
}
