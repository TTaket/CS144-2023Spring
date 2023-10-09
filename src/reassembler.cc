#include "reassembler.hh"

using namespace std;

Reassembler::Reassembler() : Blocks_( {} ), byte_ready_( 0 ), byte_pend_( 0 ) {}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // 超出重组器范围的包 - 丢弃
  if ( first_index >= byte_ready_ + output.available_capacity() ) {
    return;
  }
  if ( first_index + data.length() < byte_ready_ ) {
    return;
  }

  // 部分超出 - 截断
  if ( first_index + data.length() > byte_ready_ + output.available_capacity() ) {
    data = data.substr( 0, byte_ready_ + output.available_capacity() - first_index );
  }
  // 前截断
  if ( first_index < byte_ready_ ) {
    data = data.substr( byte_ready_ - first_index );
    first_index = byte_ready_;
  }

  // 无意义串
  if ( data.empty() && ( is_last_substring == false ) ) {
    return;
  }

  // 剩下的部分是需要处理的串
  Block_Node nowBlock( { first_index, first_index + data.length(), data.length(), is_last_substring, data } );

  // 去重
  if ( Blocks_.find( first_index ) != Blocks_.end() ) {
    if ( Blocks_[first_index] == nowBlock ) {
      return;
    }
  }

  // 数据处理
  // 查询他左侧第一个串 看看需不需要合并
  auto it = Blocks_.lower_bound( first_index );
  if ( it != Blocks_.begin() ) {
    it--;
    Block_Node beforeNode = it->second;
    if ( it->second.end_index > first_index ) {
      byte_pend_ -= beforeNode.length;
      merge_block( nowBlock, beforeNode );
      Blocks_.erase( it );
    }
  }
  // 查询大于等于它索引的串 看看需不需要合并
  it = Blocks_.lower_bound( first_index );
  while ( ( it != Blocks_.end() ) ) {
    Block_Node afterNode = it->second;
    if ( ( afterNode.begin_index < nowBlock.end_index ) ) {
      byte_pend_ -= afterNode.length;
      merge_block( nowBlock, afterNode );
      it = Blocks_.erase( it );
    } else {
      break;
    }
  }

  Blocks_[nowBlock.begin_index] = nowBlock;
  byte_pend_ += nowBlock.length;

  // 判断是否可以加入到准备好的队列;
  while ( ( Blocks_.find( byte_ready_ ) != Blocks_.end() ) && ( !output.is_closed() ) ) {
    update_output( byte_ready_, output );
  }
}

void Reassembler::update_output( uint64_t first_index, Writer& output )
{
  string_view tmpstr( Blocks_[first_index].data );
  // 取出这个串并且加到output里面
  output.push( string( tmpstr ) );

  // 扩展已经准备的长度
  byte_ready_ += tmpstr.length();
  byte_pend_ -= tmpstr.length();

  // 如果是最后一个子串 关闭流
  if ( Blocks_[first_index].islast ) {
    output.close();
  }

  // 删除这个节点
  Blocks_.erase( Blocks_.find( first_index ) );
}

// 把 bn2 合并到 bn1上
void Reassembler::merge_block( Block_Node& BN1, Block_Node& BN2 )
{
  if ( BN1 == BN2 ) {
    return;
  }
  string tmpdata = "";
  // data处理
  // 如果包含关系
  if ( BN1.begin_index <= BN2.begin_index && BN1.end_index >= BN2.end_index ) { // 如果包含关系
    tmpdata = BN1.data;
  } else if ( BN2.begin_index <= BN1.begin_index && BN2.end_index >= BN1.end_index ) { // 如果包含关系
    tmpdata = BN2.data;
  } else { // 非包含关系
    if ( BN2.begin_index <= BN1.begin_index && BN2.end_index <= BN1.end_index ) {
      swap( BN1.data, BN2.data );
      swap( BN1.length, BN2.length );
      swap( BN1.end_index, BN2.end_index );
      swap( BN1.begin_index, BN2.begin_index );
    }

    tmpdata = BN1.data;
    tmpdata += BN2.data.substr( BN1.end_index - BN2.begin_index );
  }

  BN1.begin_index = min( BN1.begin_index, BN2.begin_index );
  BN1.end_index = max( BN1.end_index, BN2.end_index );
  BN1.islast = BN1.islast || BN2.islast;
  BN1.length = BN1.end_index - BN1.begin_index;
  BN1.data = tmpdata;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return { byte_pend_ };
}

void Reassembler::insert_SYN()
{
  byte_ready_++;
};
void Reassembler::insert_FIN()
{
  byte_ready_++;
};