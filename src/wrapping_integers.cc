#include "wrapping_integers.hh"
using namespace std;

uint32_t Wrap32::getrawvalue()
{
  return this->raw_value_;
}

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // uint64_t baselen = ((uint64_t)1 << 32 );
  // (n + isn + baselen)%baselen;
  uint64_t baselen = ( (uint64_t)1 << 32 );
  uint32_t ret_raw_value_ = ( ( n + zero_point.getrawvalue() ) % baselen );
  return Wrap32 { ret_raw_value_ };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // 序列长度\*1 + （y - ISN + 序列长度) % 序列长度  或者 序列长度 \*2 + （y + ISN + 序列长度) % 序列长度
  // (checkpoint - （y - ISN + 序列长度) % 序列长度) / 序列长度 * 序列长度 + （y + ISN + 序列长度) % 序列长度
  uint64_t baselen = ( (uint64_t)1 << 32 ); // 队列长度
  uint64_t checkval = ( (uint64_t)1 << 31 );
  uint64_t modlen
    = ( this->raw_value_ - zero_point.getrawvalue() + baselen ) % baselen; // 取mod 后的长度 也是最小的真实距离
  uint64_t retval = 0;
  if ( int64_t( checkpoint ) / int64_t( baselen ) == 0 ) {
    retval = modlen;
  } else {
    retval = ( uint64_t( int64_t( checkpoint ) / int64_t( baselen ) ) - 1 ) * baselen + modlen;
  }
  while ( ( retval < checkpoint ) ) {
    if ( ( checkpoint - retval ) > checkval )
      retval += baselen;
    else
      break;
  }
  return { retval };
}
