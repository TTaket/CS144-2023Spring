Lab 4 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about 16 hours to do. I did not attend the lab session.
I used 2 late days on this lab. (and 1 late day for lab2)

I worked with or talked about this assignment with:
Nikhil Raghuraman (nikhilvr)
Simon Tao (stao18)

Program Structure and Design of the TCPConnection:
In `connect()`:
  - first call to sender's `fill_window()`, which will send a SYN flag
    and any data in the bytestream
  - send those segments

In `segment_received()`:
  - give the segment to the receiver to process the ackno and window size
  - reset the time since we last received a segment
  - if the segment has the RST flag set, 
    set both streams to error and set connection state to inactive
  - if we have initiated a connection (`syn_sent()` is true if the next
    absolute seqno is > 0), we need to make sure at least one segment
    is sent in reply to the remote peer,
    - so if there is no segments ready, we need to put an empty segment into
      the outbound queue
  - if we didn't initiate a connection and we get a SYN from the remote peer,
    connect 
  - check if we still need to linger
    - try switching close mode checks if the remote peer has ended their input,
      if so, we don't need to linger and can initiate a passive close
      when conditions are met (`try_closing_connection()`)

In `write()`:
  - if the local peer has ended input or we haven't initiated connection,
    we can't write anything to the stream
  - otherwise
    - write as much as possible to the outbound bytestream
    - tell the sender to fill the window with segments containing the 
      freshly written bytes and try to send those segments

In `send_segments()`:
  - do nothing if connection is inactive
  - empty the outbounding queue of segments from the sender. for each segment,
    - check if the receiver has an ackno and window size available,
      and include it in the segment if there is
    - set the reset flag if the optional argument rst is true
    - send it out on the outbound queue of the connection

In `try_closing_connection()`:
  - if the connection is inactive, no need to close
  - determine if we need to linger
  - if the closing conditions are met, set the connection to inactive

In `tick()`:
  - exit if connection is inactive
  - notify the sender's tick
  - incremend the time since we last received a segment
  - check if we have made too many retransmissions,
    - reset the connection if necessary
  - otherwise, call `send_segments()` to resend any segments the sender 
    might've put on the outbound queue
  - try closing the connection 
    (if we're lingering, might've lingered long enough)



Implementation Challenges:
When to push segments onto the outbound queue, when to send a(n empty) segment,
etc. mostly the order in which things need to be done. 
Also handlig starting and ending conditions (before SYN, after FIN)

Remaining Bugs:
I haven't had time to further investigate, but when I manually set up a 
connection and end input on one, the other end also ends input
after one more send, which it shouldn't until I explicitly ctrl-D.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: 
Giving more instructions on how to use txrx.sh, wireshark, and the other
testing scripts. Student-contributed methods on Piazza were hit-and-miss.
I didn't have to rely a lot on debugging using wireshark and such in this
lab (everything just worked after I realized that cout statements cause
the hashes to mismatch and removed them, which would also be a great thing 
to mention in the lab!) but I'd imagine it to become useful in the future. 

It'd be really really great if we could have a lecture or lab session
dedicated to showing us how to use Wireshark, or maybe give us some 
resources to self-teach. That one Friday lecture was good for getting started
but I felt like everyone was equally confused and there isn't any 
comprehensive material we can review besides the TA's help during the session.


- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: 
I haven't been able to test anything besides `tcp_udp` manually because I'm 
haven't figured out how to correcty run them.

Why are we not supposed to acknowledge any segments until we've initiated
a connection and the SYN has been received by the other side?
