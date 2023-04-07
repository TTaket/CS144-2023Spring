Lab 2 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about [15] hours to do. I [did not] attend the lab session.

I worked with or talked about this assignment with: nikhilvr, rodion1, stao18

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
Wrapping: mod the sum of n and isn by 2^32

Unwrapping: 
    1. find absolute valule of n with respect to isn
    2. get an anchor near checkpoint that is a multiple of 2^32
    3. make 2 guesses, a and b, such that one is above checkpoint, one below,
        both wrap to n
    4. return the guess closest to checkpoint

TCP Receiver::Segment received

If there's a SYN flag in this packet, set the syn received flag to true 
and save the seqno of this packet as the ISN.

If we still haven't received a SYN after parsing the header, do nothing, return
Otherwise, unwrap the seqno from the header into an absolute seqno 
Use this to calculate the index to pass into the reassembler:
    absolute seqno    if this is the first packet 
    absolute seqno -1 if this is not the first
        and the SYN flag has been sent in a previous packet
        so we need to account for SYN taking up 1 seqno

TCP Receiver::ackno
Return the number of bytes we've assembled and written to the stream,
+1 for the SYN flag
+1 for the FIN flag if we have assembled it


Implementation Challenges:
Understanding how the lab description translates into code was more difficult
in this lab, especially the "ballpark" around checkpoint for wrapping int, 
what to use as checkpoint in tcp receiver, and when to count the fin flag
when calculating the ackno. 

Remaining Bugs:
I removed error checkings in stream reassembler to pass the lab2 tests,
but I'm wondering why they happened in the first place, since we shouldn't
be allowed to write more after we've called end input.

- Optional: I had unexpected difficulty with: 
git reset. I modifed the test files heavily when debugging (since I had 
uncaught bugs in both my lab0 and lab1 implementation), and I had some 
trouble with version control when rolling back changes in one file. 
Some of the tests suddenly failed when I was making last minute tweaks and 
I couldn't figure out why so I had to roll back a few versions and redid 
my changes.

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by:
How such a big bug from my lab0 got through the tests (when peeking in
bytestream, I was modding by current capacity instead of max capacity).

- Optional: I'm not sure about: [describe]
