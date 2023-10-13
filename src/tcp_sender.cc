#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , Now_RTO_(initial_RTO_ms)
  , Now_time_(0)
  , Num_Resend(0)
  , checkpoint(0)
  , Seqno(0)
  , num_in_flight(0)
  , window_size_(1)
  , status_(CLOSED)
  , timer_({})
  , out_Noackqueue({})
  , out_Readyqueue({})
{}

//未确认的字节数
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return (num_in_flight);
}

//重传次数
uint64_t TCPSender::consecutive_retransmissions() const
{
  return {Num_Resend};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if(out_Readyqueue.empty()){
    return {};
  }

  TCPSenderMessage pack = out_Readyqueue.front();
  out_Readyqueue.pop_front();

  return pack;
}

//真正的推送包裹 这样可以保证每次推送对Sender的状态影响 和 包信息同步
void TCPSender::push_msg(TCPSenderMessage &msg ){
  //计算包的大小
  size_t pack_size = msg.sequence_length();
  
  //增加待回答队列长度
  num_in_flight += pack_size;
  //改变序列号和检查点
  Seqno += pack_size;
  checkpoint = Seqno;

  //包放入双队列
  out_Noackqueue.push_back(msg);
  out_Readyqueue.push_back(msg);

  //打开计时器
  if(!timer_.IsOpen()){
    timer_.Start(Now_time_ , Now_RTO_);
  }
}
void TCPSender::push( Reader& outbound_stream )
{
  //按照状态分类
  //如果处于关闭阶段
  if (status_ == FINISH || status_ == FIN_WAIT)
  {
    return;
  }

  //如果处于关闭阶段
  if(status_ == CLOSED){
    //第一次握手通常不带信息
    TCPSenderMessage SYN_PACK = {Wrap32::wrap(Seqno , isn_) ,true , {}  , false};

    //切换状态
    if(!outbound_stream.is_finished()){
      status_ = SYN_SENT;
    }else{
      SYN_PACK.FIN = true;
      status_ = FIN_WAIT;
    }

    push_msg(SYN_PACK);
  }

  //如果处于第二次握手阶段
  if(status_ == SYN_SENT){
    //等待握手成功
    return;
  }

  //如果处于通讯阶段
  if(status_ == ESTABLISHED){
    //如果窗口突然缩小 小于等待确认长度 return
    if(num_in_flight > window_size_){
      return;
    }

    //计算能够传输字节数量
    uint16_t Able_Send = 0;
    if(window_size_ == 0)
      Able_Send = 1;
    else
      Able_Send =(window_size_ - num_in_flight);
    
    while(Able_Send > 0 && outbound_stream.bytes_buffered()){
      //有可以传送的空间去传送 - 封装包裹
      string_view buffer = outbound_stream.peek();    
      size_t pack_size = min((size_t)Able_Send ,min(buffer.length() , TCPConfig::MAX_PAYLOAD_SIZE));  
      TCPSenderMessage pack = {Wrap32::wrap(Seqno , isn_) ,false , Buffer(std::string(buffer.substr(0 , pack_size)))  , false};
      outbound_stream.pop(pack_size);
      Able_Send -= pack_size;
      
      //检查是否关闭 - 最后一个信息携带fin信息
      if(outbound_stream.is_finished()){
        //满足关闭条件 - 如果有空间去传送
        if(Able_Send != 0){
          //设置标志位
          pack.FIN = true;

          //切换状态 - 因为重传不由这里负责
          status_ = FIN_WAIT;
        }
      }

      //发送包裹
      push_msg(pack);
    }

    //检查是否关闭 - 单独发送fin报文
    if(status_ == ESTABLISHED && outbound_stream.is_finished()){
      //满足关闭条件 - 如果有空间去传送
      if(Able_Send){ 
        //准备关闭报文
        TCPSenderMessage FIN_PACK = {Wrap32::wrap(Seqno , isn_) ,false, {}  , true};
        push_msg(FIN_PACK);

        //切换状态 - 因为重传不由这里负责
        status_ = FIN_WAIT;
      }
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage pack({});
  pack.seqno = Wrap32::wrap(Seqno , isn_);
  pack.SYN = false;
  pack.FIN = false;
  pack.payload = {};
  return {pack};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  //丢弃错误的包
  //1. 没有ack的包 - 什么回应包没有ack? 猜测对端返回值也是可选项 所以返回是空ack 证明对端没有收到信息
  if (msg.ackno.has_value() == false){
    return;
  }
  //2. 超过Seqno的 历史连接遗留
  if(msg.ackno.value().unwrap(isn_ , checkpoint) > Seqno){
    return;
  }
  //3. 小于确认接受编号的 超时严重的包
  if(msg.ackno.value().unwrap(isn_ , checkpoint) < Seqno - num_in_flight){
    return;
  }

  //处理状态
  if(status_ == SYN_SENT && msg.ackno == Wrap32::wrap(1,isn_)){
    status_ = ESTABLISHED;
  }
  if(status_ == FIN_WAIT && msg.ackno.value().unwrap(isn_ , checkpoint) == Seqno){
    status_ = FINISH;
  }
  


  //更新窗口大小
  window_size_ = msg.window_size;

  //计算包ACK
  uint64_t backAck = msg.ackno.value().unwrap(isn_ , checkpoint);
  //状态位 是否有包被取走
  bool UsefulFlag = false;
  //处理out_Noackqueue队列
  while(!out_Noackqueue.empty()){
    TCPSenderMessage Message = out_Noackqueue.front();
    uint64_t PackNextSeq= Message.seqno.unwrap(isn_ , checkpoint) + (uint64_t)Message.sequence_length();
    if(PackNextSeq <= backAck){
      num_in_flight -= Message.sequence_length();
      out_Noackqueue.pop_front();
      UsefulFlag = true;
    }else{
      break;
    }
  }
  if(UsefulFlag){
    Now_RTO_ = initial_RTO_ms_;
    Num_Resend = 0; 
    if(out_Noackqueue.empty()){//如果被取光了 关闭计时器
      timer_.Close();
    }else{//如果没被取光 重置计时器
      timer_.Start(Now_time_ ,Now_RTO_);
    }
  }
}

void TCPSender::tick( const uint64_t ms_since_last_tick )
{
  //更新时间
  Now_time_ += ms_since_last_tick;

  //计时器是开的
  //检测超时
  //1.没超时跳过 
  if(!timer_.CheckOvertime(Now_time_)) return;
  
  /*
  2.存在超时行为
    重传 TCP 接收方尚未完全确认的最早（最低序列号）段
    RTO翻倍 重传计数器+1
    重启timer
  */
  timer_.Close();
  out_Readyqueue.push_front(out_Noackqueue.front());
  if(window_size_ !=0){
    Now_RTO_ = Now_RTO_ * 2;
    Num_Resend += 1;
  }
  timer_.Start(Now_time_ , Now_RTO_);

}
TCPSender::Timer::Timer()
  :start_time_(0)
  ,clock_time_(0)
  ,Isopen_(false){
}

void TCPSender::Timer::Changeclocktime(uint64_t clocktime){
  clock_time_ = clocktime;
}
void TCPSender::Timer::Start(uint64_t starttime, uint64_t clocktime){
  start_time_ = starttime;
  clock_time_ = clocktime;
  Isopen_ = true;
}
void TCPSender::Timer::Close(){
  Isopen_ = false; 
}
bool TCPSender::Timer::IsOpen() const{
  return {Isopen_};
}
bool TCPSender::Timer::CheckOvertime(uint64_t nowtime) const{
  if(!Isopen_) 
    return false;
  if((nowtime >= start_time_) && (nowtime - start_time_ >= clock_time_)){
    return true;
  }
  return false;
}
