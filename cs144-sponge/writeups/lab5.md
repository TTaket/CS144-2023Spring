Lab 5 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about 10.5 hours to do. I did attend the lab session.

Program Structure and Design of the NetworkInterface:
When sending datagrams, we first determine if we know which ethernet adress
to send the datagram so it can reach the IP address specified. If it is not
in the forwarding table, then we need to broadcast an ARP message request
and wait for a reply to come before we can send the datagram, unless we have
sent a request for that IP address in the past 5 seconds, in which case we
will do nothing.

When we receive a frame, first check if it is meant to be sent to us 
(destination/target ethernet matches ours).
If it's a datagram and we can parse it, return the datagram. (the IP address
doesn't need to match ours since it might just be passing by)
If it's an ARP message and we can parse it and it's meant for us (both the 
ethernet and ip address match ours), we need to learn the mapping from the
message's source/sender ethernet and IP address. 
If the message is a request, we need to send a reply, telling the sender
that they can reach our IP address via our ethernet address.
If the message is a reply, we need to remove the message from our list of
outstanding requests waiting for a reply.

When time passes (via `tick()`) we need to update each entry in the forwarding
table (and remove any entries that expired) and the collection of outstanding 
ARP requests (and remove any entries that expired).

Implementation Challenges:
Figuring out how to use the parse function and creating an Address struct
from a numeric IP address. Mostly just unfamiliar C++ syntax (like inserting
a pair into the map).

Remaining Bugs:
Not in this lab that I know of.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
