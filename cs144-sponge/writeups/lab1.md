Lab 1 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about [10] hours to do. I [did not] attend the lab session.

Program Structure and Design of the StreamReassembler:

I originally chose to store chunks of aggregated bytes in a standard set 
so I can iterate through them in order and able to exit early, but I switched 
to using a list thinking that the order of the packets are too random to make 
a huge difference in efficiency by exiting early, so I switched to a list for 
constant time insertion/removal, but the two seem to have comparable run time 
on tests. 

When a new chunk arrives, I  merge any existing chunks 
into the new chunk by removing them from the unassembled set and either:
  1) prepend to the new data
  2) append
  3) do nothing if the existing chunk is contained within the new chunk
or if the new chunk is contained within an existing chunk, I simply ignore it.
Then I either add the new chunk to the bytestream, or insert it back into the 
unassembled set.


Implementation Challenges:
Figuring out the aggregation logic was a bit tricky, it was a matter
of drawing out 4 diagrams and following them, but I had a bug with the
condition for dealing with partially overlapping chunks that took a while
to show up because I hadn't been thorough with drawing out all the 
possibilities to see the conditions I need to be excluding as well. 

I was initially a little confused with when to set eof 
and how to identify and handle data not in the window.
I definitely ran into more problems because I was trying to optimize 
too early before really understanding the problem. 

Remaining Bugs:


- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about:
