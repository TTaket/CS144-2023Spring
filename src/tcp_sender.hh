#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <map>

class TCPSender
{
protected:
  class Timer{
    protected:
      uint64_t start_time_;
      uint64_t clock_time_;
      bool Isopen_;


    public:
      Timer();
      void Changeclocktime(uint64_t clocktime);
      void Start(uint64_t starttime , uint64_t clocktime);
      void Close();
      bool CheckOvertime(uint64_t nowtime) const;
      bool IsOpen() const;
  };
protected:
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t Now_RTO_;   // 现在的RTO
  uint64_t Now_time_;  // 现在的时间戳
  uint64_t Num_Resend; // 重传次数
  uint64_t checkpoint; //检查点
  uint64_t Seqno; //绝对序列号
  uint64_t num_in_flight; //等待确认的数量
  uint16_t window_size_; //窗口大小
  enum TCPSender_status
  {
    CLOSED,
    SYN_SENT,
    ESTABLISHED,
    FIN_WAIT,
    FINISH
  } status_;
  Timer timer_;
  //设计存储结构 - 保存发送但是没确定接受的数据包 - 队列
  std::deque<TCPSenderMessage>out_Noackqueue;
  //设计存储结构 - 准备发送的包裹 - 队列
  std::deque<TCPSenderMessage>out_Readyqueue;
protected:
  /* 推送包到两个队列同时更新数据 */
  void push_msg(TCPSenderMessage &msg );
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );


  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( const uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
