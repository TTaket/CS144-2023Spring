#include "network_interface.hh"
#include "arp_message.hh"
#include "ethernet_frame.hh"
#include <iostream>

// Ha Tran hahntrn
// CS 144 Lab 5
// 20201104 

// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address), _forwarding_table{}, _outstanding_msg{}, _unsent_datagrams{} {
    cerr << endl << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to 
//! (typically a router or default gateway, but may also be another host 
//! if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t 
//! (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    if (debug) cout << "DEBUG: sending dgram" << endl;
    const uint32_t next_hop_ip = next_hop.ipv4_numeric(); //?

    // IP addr is an entry in table, we know which ethernet addr to send to
    auto it = _forwarding_table.find(next_hop_ip);
    if (it != _forwarding_table.end()) {
        EthernetFrame frame;
        EthernetHeader hdr;

        hdr.type = EthernetHeader::TYPE_IPv4;
        hdr.dst = it->second.eth_addr;
        hdr.src = _ethernet_address;

        frame.header() = hdr;
        frame.payload() = dgram.serialize();
        _frames_out.push(frame);
        return;
    }

    // unknown IP addr, need to broadcast an ARPMessage Request
    // (unless we already did in the last 5 sec)
    // and queue datagram to be sent when we get a reply
    auto it_dg = _unsent_datagrams.find(next_hop_ip);
    if (it_dg == _unsent_datagrams.end()) {
        queue<InternetDatagram> q;
        q.push(dgram);
        _unsent_datagrams.insert(std::pair<uint32_t, queue<InternetDatagram>> (next_hop_ip, q));
    } else {
        it_dg->second.push(dgram);
    }

    auto it_msg = _outstanding_msg.find(next_hop_ip);
    if (it_msg != _outstanding_msg.end()) {
        if (debug) cout << "DEBUG: jk don't send an ARP request, last sent "<<it_msg->second<<"ms ago" << endl;
        return;
    }
    _outstanding_msg.insert(std::pair<uint32_t, size_t>(next_hop_ip, 0));

    ARPMessage msg;
    msg.opcode = msg.OPCODE_REQUEST;
    msg.sender_ethernet_address = _ethernet_address;
    msg.sender_ip_address = _ip_address.ipv4_numeric();
    msg.target_ethernet_address = ARPMSG_ETHERNET_BROADCAST;
    msg.target_ip_address = next_hop_ip;
    
    EthernetFrame frame;
    EthernetHeader hdr;

    hdr.type = EthernetHeader::TYPE_ARP;
    hdr.dst = ETHERNET_BROADCAST;
    hdr.src = _ethernet_address;
        
    frame.header() = hdr; 
    frame.payload() = msg.serialize();
    _frames_out.push(frame);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // if the ethernet address is not ours and it's not broadcasting, then
    // the frame wasn't meant to be sent to us
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) return {};

    Buffer payload_single = frame.payload().concatenate();
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {

        // DATAGRAM PAYLOAD
        InternetDatagram dgram;
        if (dgram.parse(payload_single) != ParseResult::NoError) 
            return {};
        if (debug) cout<<"DEBUG: recv'd datagram w/ frame hdr: "<<frame.header().to_string()<<endl;
        return dgram;
    } else if (frame.header().type == EthernetHeader::TYPE_ARP) {

        // ARP MESSAGE PAYLOAD
        if (debug) cout<<"DEBUG: recv'd ARP msg"<<endl;
        ARPMessage msg;

        // if the message can't be parsed or the target IP address is not ours
        if (msg.parse(payload_single) != ParseResult::NoError || msg.target_ip_address != _ip_address.ipv4_numeric()) 
            return {};

        // learn a mapping: we can reach this IP address via this ethernet address
        Entry ent = { msg.sender_ethernet_address, 0 };
        _forwarding_table.insert(std::pair<uint32_t, Entry>(msg.sender_ip_address, ent));

        // send any datagrams waiting to be sent
        if (debug) cout << "DEBUG: n unsent dgrams: " << _unsent_datagrams.size() << endl;
        auto it_dg = _unsent_datagrams.find(msg.sender_ip_address);
        if (it_dg != _unsent_datagrams.end()) {
            while (!it_dg->second.empty()) {
                send_datagram(it_dg->second.front(), Address::from_ipv4_numeric(it_dg->first));
                it_dg->second.pop();
            }
        }

        if (msg.opcode == msg.OPCODE_REQUEST) {
            // message is a request, send a reply
            ARPMessage reply;
            reply.opcode = reply.OPCODE_REPLY;
            reply.sender_ethernet_address = _ethernet_address;
            reply.sender_ip_address = _ip_address.ipv4_numeric();
            reply.target_ethernet_address = msg.sender_ethernet_address;
            reply.target_ip_address = msg.sender_ip_address; 

            EthernetFrame reply_frame;
            EthernetHeader hdr;

            hdr.type = EthernetHeader::TYPE_ARP;
            hdr.dst = msg.sender_ethernet_address;
            hdr.src = _ethernet_address;
                
            reply_frame.header() = hdr; 
            reply_frame.payload() = reply.serialize();
            if (debug) cout << "DEBUG: sending ARP reply: " << reply_frame.header().to_string() << endl;
            _frames_out.push(reply_frame);
        } else {
            // message is a reply, delete the request (if it exists)
            _outstanding_msg.erase(msg.sender_ip_address);
        }
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    if (debug) cout << "DEBUG: tick! " << ms_since_last_tick << "ms passed" << endl;
    // remove any entries older than 30s in forwarding table
    for (auto it = _forwarding_table.begin(); it != _forwarding_table.end();) {
        if (it->second.time_elapsed + ms_since_last_tick > ENTRY_EXPIRY) {
            it = _forwarding_table.erase(it);
        } else {
            it->second.time_elapsed += ms_since_last_tick;
            it++; 
        }
    }

    // update entries in the queue of arp requests
    for (auto it = _outstanding_msg.begin(); it != _outstanding_msg.end();) {
        // remove any entries older than 5s
        if (it->second + ms_since_last_tick > MSG_EXPIRY) {
            it = _outstanding_msg.erase(it);
        } else {
            it->second += ms_since_last_tick;
            it++;
        }
    }
}

