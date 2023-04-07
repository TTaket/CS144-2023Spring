#include "router.hh"
#include <bitset>
#include <iostream>

using namespace std;

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    if (debug) {
        cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";
        cerr << "DEBUG: Bitmask: " << bitset<32>(get_mask(prefix_length)) << endl; 
    }

    // if the prefix length doesn't exist yet, create an entry with empty map
    if (_routing_table.find(prefix_length) == _routing_table.end()) {
        _routing_table.insert(make_pair(prefix_length, map<uint32_t, InterfaceOut>()));
    }
    _routing_table[prefix_length].insert(make_pair(route_prefix & get_mask(prefix_length), InterfaceOut(next_hop, interface_num)));
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // look at most to least specific (longest to shortest prefix length)
    for (auto it = _routing_table.rbegin(); it != _routing_table.rend(); it++) {
        auto prefixes = it->second;
        auto prefix_found = prefixes.find(dgram.header().dst & get_mask(it->first));
        if (prefix_found != prefixes.end()) {
            // drop any packets with TTL = 0 or 1, otherwise decrement TTL
            if (dgram.header().ttl <= 1)
                break;
            dgram.header().ttl--;

            InterfaceOut io_send = prefix_found->second;
            if (io_send.next_hop.has_value()) {
                // need to send dgram through other routers
                interface(io_send.interface_num).send_datagram(dgram, Address::from_ipv4_numeric(io_send.next_hop->ipv4_numeric()));
            } else {
                // router is directly attached to network
                interface(io_send.interface_num).send_datagram(dgram, Address::from_ipv4_numeric(dgram.header().dst));
            } 
            break;
        }
    }
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
