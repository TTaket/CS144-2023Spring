Lab 7 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

My lab partner's SUNet ID: nikhilvr

I also worked with or collaborated with: [their sunetids here]

This lab took me about 3.5 hours to do. I did attend the lab session.

Solo portion:
    - Yes, my implementation did successfully start and end a conversation 
      with another copy of itself.
    - Yes
    - After lab6 and before reading lab7, I was playing around with the 
      `tcp_udp` manual test from lab4 and changing my logic for shutting down a 
      connection because when running manually, it was behaving strangely. I
      realized that it was not a bug in my code, but the end logic does
      make more sense after I fixed it: I check if the close mode should 
      be switched to stop lingering before anytime I try to check if I need
      to close the connection, instead of spliting them up like before.
      The previous version also passes all the tests, so perhaps it was 
      sufficient.

Group portion:
    - My partner was Nikhil Raghuranman (nikhilvr)
    - Yes
    - Yes
    - My partner initially got a segfault because he assumed that because he
      only sends one ARP request, he expects to receive exactly one ARP reply.
      Because he didn't account for duplication, he searches his collection
      of messages waiting to be sent for the IP address with unknown next
      ethernet address for the freshly received ARP reply's IP address, but 
      he accesses the pointer that the search function returns without 
      checking if it's valid, assuming that the entry with the IP address
      exists in the collection. After he changed it to check if the iterator
      is valid, everything works as expected.

Creative portion (optional):
[]

Other remarks:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
