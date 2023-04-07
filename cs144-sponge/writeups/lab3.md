Lab 3 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about 16.5 hours to do. I did not attend the lab session.

I worked with or talked about this assignment with: 
Nikhil Raguraman (nikhilvr)
Simon Tao (stao18)

Program Structure and Design of the TCPSender:

In `fill_window()`:
We try to  push segments out until the next seqno no longer fits 
in the window (delimited by the biggest ackno we've received 
and the last advertised window size). 

    If we realize a fin bit has been sent, exit.

    We set the syn and fin bit, making sure that they fit in the window,
    but the max payload size only limits the size of the data we read from 
    the stream.

    If we realize the segment is empty, because we have no more data and flags
    to send, exit. Otherwise, we push the segments to the outbound queue to get
    sent, and increment the next seqno by the length in sequence space of the 
    segment.
    
    If the timer isn't running, start it for the original retransmission timeout.

In `ack_received()`:
  - update the window size
  - check if the ackno is actionable
    - if the ackno is smaller than the ackno we have saved, we already know 
      that segment has been acknowledged, we don't need to do anything
    - if the ackno is bigger than the next seqno, it'd be for a segment
      we have not sent, which doesn't make sense, we can also ignore
  - update the ackno if it's in range
  - remove all outstanding segments that have been completely acknowledged
  - start the timer if we were able to confirm at least one segment
  - reset the count of number of consecutive retransmission
  - call `fill_window()` to try to send segments in case the updated 
    window size frees up room for more segments

In `tick()`:
We update the timer's time elapsed, check if it's expired. If it did,
retransmit the oldest outstanding segment and backoff the timer
(restart the timer with a longer timeout for the next retransmission).

Implementation Challenges:
Figure out when to do what, such as:
  - restart timer
  - backoff on rtto
  - not send a segment
  - check for FIN
and how many bytes to send to fit in the window but also limit the payload
size.

Remaining Bugs:
In fill window, if bytes in flight is much much bigger than window size, 
then when calculating number of bytes to send = window size - bytes in flight
we might get an overflow value < max payload size, but something else 
is probably really really wrong if that happens.

Currently in `tcp_sender` I update the window size to be the most current,
even if the ackno is not useful (too small (ackno of a segment we've already gotten
an acknowledgement for) or too big). 

- Optional: I had unexpected difficulty with: 
Reading comprehension and following lab description. 

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: 
I'm still confused why we need to set the seqno when sending an empty
segment for acknowledgement or resetting.
