#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <iostream>
#include <list>
#include <optional>
#include <queue>
#include <map>
#include <unordered_map>
#include <utility>

// A "network interface" that connects IP (the internet layer, or network layer)
// with Ethernet (the network access layer, or link layer).

// This module is the lowest layer of a TCP/IP stack
// (connecting IP with the lower-layer network protocol,
// e.g. Ethernet). But the same module is also used repeatedly
// as part of a router: a router generally has many network
// interfaces, and the router's job is to route Internet datagrams
// between the different interfaces.

// The network interface translates datagrams (coming from the
// "customer," e.g. a TCP/IP stack or router) into Ethernet
// frames. To fill in the Ethernet destination address, it looks up
// the Ethernet address of the next IP hop of each datagram, making
// requests with the [Address Resolution Protocol](\ref rfc::rfc826).
// In the opposite direction, the network interface accepts Ethernet
// frames, checks if they are intended for it, and if so, processes
// the the payload depending on its type. If it's an IPv4 datagram,
// the network interface passes it up the stack. If it's an ARP
// request or reply, the network interface processes the frame
// and learns or replies as necessary.
class NetworkInterface
{
private:
  //现在的时间
  uint64_t now_time_t;
  //第三层以及以下不需要维护可靠性
  // arp映射表 IP - eth
  std::unordered_map<uint32_t , EthernetAddress>arp_ip2eth_map_;

  //arp表有效时间
  std::unordered_map<uint32_t , uint64_t>arp_valid_time_map_;

  //arpReq存活 - 防止大量arp请求发送
  std::unordered_map<uint32_t, uint64_t> _arpreq_survival_map_;

  // 当时发送的时候不知道对方具体eth地址 发送arp请求 在这个时候把包放入此队列
  // 当接受arp回复确定对方eth的时候（recv）把这个等待队列对应信息发送出去
  std::unordered_map<uint32_t , std::vector<InternetDatagram> >wait_map;
  // 出站队列 - 已经准备好了的
  std::deque<EthernetFrame>out_que_;

  // Ethernet (known as hardware, network-access, or link-layer) address of the interface
  EthernetAddress ethernet_address_;

  // IP (known as Internet-layer or network-layer) address of the interface
  Address ip_address_;

  // ARPMAP时间
  const uint64_t MaxArpMapTime;
  // ARPREQ时间
  const uint64_t MaxArpReqTime;
public:
  // Construct a network interface with given Ethernet (network-access-layer) and IP (internet-layer)
  // addresses
  NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address );

  // Access queue of Ethernet frames awaiting transmission
  std::optional<EthernetFrame> maybe_send();

  // Sends an IPv4 datagram, encapsulated in an Ethernet frame (if it knows the Ethernet destination
  // address). Will need to use [ARP](\ref rfc::rfc826) to look up the Ethernet destination address
  // for the next hop.
  // ("Sending" is accomplished by making sure maybe_send() will release the frame when next called,
  // but please consider the frame sent as soon as it is generated.)
  // 转发 - 转发同样负责超时管理
  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  // Receives an Ethernet frame and responds appropriately.
  // If type is IPv4, returns the datagram.
  // If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
  // If type is ARP reply, learn a mapping from the "sender" fields.
  std::optional<InternetDatagram> recv_frame( const EthernetFrame& frame );

  // Called periodically when time elapses
  void tick( size_t ms_since_last_tick );
};
