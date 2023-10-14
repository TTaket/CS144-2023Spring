#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : now_time_t(0)
  , arp_ip2eth_map_({})
  , arp_valid_time_map_({})
  , _arpreq_survival_map_({})
  , wait_map({})
  , out_que_({})
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
  , MaxArpMapTime(30000)
  , MaxArpReqTime(5000)
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}


// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
// 信息发送 - 把网络包发送到下一条
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{

  //下一跳ipnum
  uint32_t nxt_ipnum = next_hop.ipv4_numeric();

  //尝试找到对方的mac地址
  if(arp_ip2eth_map_.find(nxt_ipnum) == arp_ip2eth_map_.end()){
    //没有找到 - 发送arp信息

    //如果五秒内尝试过发送 则不发送arp请求
    if(_arpreq_survival_map_.find(nxt_ipnum) == _arpreq_survival_map_.end()){
      //构建包
      ARPMessage arpback({});
      arpback.hardware_type = ARPMessage::TYPE_ETHERNET;             // Type of the link-layer protocol (generally Ethernet/Wi-Fi)
      arpback.protocol_type = EthernetHeader::TYPE_IPv4; // Type of the Internet-layer protocol (generally IPv4)
      arpback.hardware_address_size = sizeof( EthernetHeader::src );
      arpback.protocol_address_size = sizeof( IPv4Header::src );
      arpback.opcode = ARPMessage::OPCODE_REQUEST;
      
      arpback.sender_ethernet_address   =this->ethernet_address_;
      arpback.sender_ip_address         =this->ip_address_.ipv4_numeric();
      
      arpback.target_ip_address         =nxt_ipnum;

      EthernetFrame Arpreq({});
      Arpreq.header.dst = ETHERNET_BROADCAST;
      Arpreq.header.src = arpback.sender_ethernet_address;
      Arpreq.header.type = EthernetHeader::TYPE_ARP;
      Arpreq.payload = serialize(arpback);
      //发送arp请求包
      out_que_.push_back(Arpreq);
      //设置arpreq保护
      _arpreq_survival_map_[nxt_ipnum] = now_time_t;
    }
    //放入到等待队列
    wait_map[nxt_ipnum].push_back(dgram);
    
  }else{
    //网络包 -> 链路层包
    EthernetFrame Ethpack({});
    Ethpack.header.dst =  arp_ip2eth_map_[nxt_ipnum];
    Ethpack.header.src = this->ethernet_address_;
    Ethpack.header.type = EthernetHeader::TYPE_IPv4;
    Ethpack.payload = serialize(dgram);
    
    //发送包
    out_que_.push_back(Ethpack);
  }
}

// frame: the incoming Ethernet frame
// 处理arp缓冲表
// 处理因eth地址未知而堆积的包
// 如果类型是 IPv4，则返回数据报。
// 如果类型是 ARP 请求，则从“sender”字段中学习映射，并发送 ARP 回复。
// 如果类型是 ARP 回复，则从“sender”字段中学习映射。
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  //不是我的包 丢弃 
   if ( frame.header.dst != this->ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) 
    return nullopt;

  //如果信息是 arp
  if(frame.header.type== EthernetHeader::TYPE_ARP){
    //在本层处理
    ARPMessage ARPpack({});
    if(parse( ARPpack, frame.payload ) == false) return nullopt;

    //不支持解析 报错 
    if(ARPpack.supported() == false) return nullopt;

    //如果是请求 - 学习发送方的eth地址和ip
    if(ARPpack.opcode == ARPMessage::OPCODE_REQUEST){
      EthernetAddress src_eth = ARPpack.sender_ethernet_address;
      uint32_t        src_ip  = ARPpack.sender_ip_address;
      //如果没有学习则学习
      if(arp_ip2eth_map_.find(src_ip) == arp_ip2eth_map_.end()){
        arp_ip2eth_map_[src_ip] = src_eth;
        arp_valid_time_map_[src_ip] = now_time_t;
      }
      //如果能回复就进行回复
      if(this->ip_address_.ipv4_numeric() == ARPpack.target_ip_address){
        //构建包
        ARPMessage arpback({});
        arpback.hardware_type = ARPMessage::TYPE_ETHERNET;             // Type of the link-layer protocol (generally Ethernet/Wi-Fi)
        arpback.protocol_type = EthernetHeader::TYPE_IPv4; // Type of the Internet-layer protocol (generally IPv4)
        arpback.hardware_address_size = sizeof( EthernetHeader::src );
        arpback.protocol_address_size = sizeof( IPv4Header::src );
        arpback.opcode = ARPMessage::OPCODE_REPLY;
        
        arpback.sender_ethernet_address   =this->ethernet_address_;
        arpback.sender_ip_address         =this->ip_address_.ipv4_numeric();
        
        arpback.target_ethernet_address   =ARPpack.sender_ethernet_address;
        arpback.target_ip_address         =ARPpack.sender_ip_address;

        EthernetFrame Ethpack({});
        Ethpack.header.dst = arpback.target_ethernet_address;
        Ethpack.header.src = arpback.sender_ethernet_address;
        Ethpack.header.type = EthernetHeader::TYPE_ARP;
        Ethpack.payload = serialize(arpback);
        //发送包
        out_que_.push_back(Ethpack);
      }
    }
    //如果是回应 - 学习两方地址
    if(ARPpack.opcode == ARPMessage::OPCODE_REPLY){
      EthernetAddress src_eth = ARPpack.sender_ethernet_address;
      uint32_t        src_ip  = ARPpack.sender_ip_address;
      EthernetAddress dst_eth = ARPpack.target_ethernet_address;
      uint32_t        dst_ip  = ARPpack.target_ip_address;
      //如果没有学习就学习
      if(arp_ip2eth_map_.find(src_ip) == arp_ip2eth_map_.end()){
        arp_ip2eth_map_[src_ip] = src_eth;
        arp_valid_time_map_[src_ip] = now_time_t;
      }
      //如果没有学习就学习
      if(arp_ip2eth_map_.find(dst_ip) == arp_ip2eth_map_.end()){
        arp_ip2eth_map_[dst_ip] = dst_eth;
        arp_valid_time_map_[dst_ip] = now_time_t;
      }

      //处理淤积的包
      if(wait_map.find(src_ip) != wait_map.end()){
        for(auto it: wait_map[src_ip]){
          send_datagram( it, Address::from_ipv4_numeric(src_ip) );
        }
        //删除掉等待队列
        wait_map.erase(wait_map.find(src_ip));
      }
    }
    return {};
  }

  //如果信息是 IPv4 
  if(frame.header.type== EthernetHeader::TYPE_IPv4){
    //进行解析
    InternetDatagram Internetpack({});
    if(parse(Internetpack ,frame.payload) == true){
      return Internetpack;
    }else{
      return nullopt;
    }
  }
  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
// 更新arp表 和 arpreq信息
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  //更新时间
  now_time_t += (uint64_t)ms_since_last_tick;
  
  //arp表 处理
  for(auto it =arp_valid_time_map_.begin() ; it!= arp_valid_time_map_.end();){
    if(it->second +MaxArpMapTime <= now_time_t){
      //处理 
      //arp 删除 
      arp_ip2eth_map_.erase(arp_ip2eth_map_.find(it->first));
      //时间表删除
      it = arp_valid_time_map_.erase(it);
    }else{
      it++;
    }
  }

  //arpreq 处理
  for(auto it =_arpreq_survival_map_.begin() ; it!= _arpreq_survival_map_.end();){
    if(it->second +MaxArpReqTime <= now_time_t){
      it = _arpreq_survival_map_.erase(it);
    }else{
      it++;
    }
  }
}

// 3层转换为2层 为上层提供最前的可用链路层包
optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(out_que_.empty()){
    return {};
  }else{
    auto pack = out_que_.front();
    out_que_.pop_front();
    return pack;
  }
  
}
