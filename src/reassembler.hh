#pragma once

#include "byte_stream.hh"
#include <map>
#include <string>

class Reassembler
{
public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  Reassembler();
protected:
  //设计存储结构 ： 

  //  节点信息结构体 
  class Block_Node{
  public:
    uint64_t begin_index = 0;
    uint64_t end_index = 0;
    uint64_t length = 0;
    bool islast = false;
    std::string data = "";
    bool operator < (const Block_Node & that) const{  
      return begin_index < that.begin_index;
    };
    bool operator == (const Block_Node & that) const{  
      return ((begin_index == that.begin_index) && (end_index == that.end_index) && (islast == that.islast))  ;
    };

  };


  std::map<uint64_t, Block_Node> Blocks_ ;

  //  uint64_t 记录当前需要的开始索引是多少
  uint64_t byte_ready_;
  //  uint64_t 记录排队的字节有多少
  uint64_t byte_pend_;

protected:
  // update_output
  // 更新可以输出的数量到output 
  void update_output(uint64_t first_index ,Writer&output) ;


  // merge
  // 对两个block进行合并
  void merge_block (Block_Node &BN1  ,Block_Node &BN2  );

public:
  // insert  
  // 插入一个block 
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );
  void insert_SYN();
  void insert_FIN();

  // How many bytes are stored in the Reassembler itself?
  // 重组器本身存储了多少字节 - 应该是返回有序的长度个数
  uint64_t bytes_pending() const;
};
