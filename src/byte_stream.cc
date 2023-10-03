#include <cstdint>
#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , size_( 0 )
  , Buffer_( 2 * capacity )
  , isclose_( false )
  , iserror_( false )
  , write_count_( 0 )
  , read_count_( 0 )
{}
void Writer::push( string data )
{
  // 计算能够放置的数量
  const size_t able_push_len = std::min( data.length(), available_capacity() );

  // 进行字符串拷贝
  for ( size_t pos = 0; pos < able_push_len; pos++ ) {
    Buffer_[( write_count_ + pos ) % capacity_] = data[pos];
    Buffer_[( write_count_ + pos ) % capacity_ + capacity_] = data[pos];
  }
  size_ += able_push_len;
  write_count_ += able_push_len;
  return;
}

string_view Reader::peek() const
{
  return string_view( Buffer_.begin(), Buffer_.end() ).substr( read_count_ % capacity_, size_ );
}
void Reader::pop( uint64_t len )
{
  // 计算能够删除的数量
  const size_t able_pop_len = std::min( len, bytes_buffered() );
  read_count_ += able_pop_len;
  size_ -= able_pop_len;
  return;
}
void Writer::close()
{
  isclose_ = true;
  return;
}

void Writer::set_error()
{
  iserror_ = true;
  return;
}

bool Writer::is_closed() const
{
  return ( isclose_ );
}

uint64_t Writer::available_capacity() const
{
  return ( this->capacity_ - this->size_ );
}

uint64_t Writer::bytes_pushed() const
{
  return ( this->write_count_ );
}

bool Reader::is_finished() const
{
  return ( this->isclose_ && ( bytes_buffered() == 0 ) );
}

bool Reader::has_error() const
{
  return ( this->iserror_ );
}

uint64_t Reader::bytes_buffered() const
{
  return ( this->size_ );
}

uint64_t Reader::bytes_popped() const
{
  return ( read_count_ );
}
