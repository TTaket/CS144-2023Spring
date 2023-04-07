Lab 6 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about 3 hours to do. I did not attend the lab session.

Program Structure and Design of the Router:
`add_route()`
    - checks if the prefix length exists in the map, create a new entry if not.
    - inserts the masked route prefix into the corresponding prefix length map
    along with its associated next hop address and interface number

`route_one_datagram()`
    - loops through the routing table from the longest to shortest prefix 
      length (we check the most to least specific entries so we can stop at 
      the first matching prefix we find)
    - when we find a matching prefix (by comparing the masked destination 
      address of the datagram to the masked prefix in the table),
        - we drop the packet if the TTL field is 0 or will be 0 after decrement
        - otherwise, decrement the TTL and send it out through the appropriate
          interface
            - if the next hop is not an empty optional, then the datagram
              needs to pass through our router to get to another router
                - we construct an Address from the next hop ip address and 
                  send it out on the corresponsing interface number
            - if the next hop is empty, then the network the datagram is trying
              to reach is directly connected to our router, so we can reach 
              the datagram's destination address from our router
                - we construct an Address from the datagram's destination
                  ip address and send it out on the corresponding interface num
    - if we can't find a matching prefix in the routing table, the datagram is
      implicitly dropped

`get_mask()`
    - constructs a 32-bit mask that leaves out the first prefix length bits by 
        - taking the max value of the 32bit unsigned integer type (which is
          all 1s) and shifting it by 32 - prefix length
        - if the prefix length is 0 more than 32, then we are masking
          every bit, i.e. the mask is 0 

Implementation Challenges:
Just figuring out how to construct an Address and bitmask. Online C++ compiler
was helpful to test out the bitmask.

Remaining Bugs:
Not that I'm aware of

- Optional: I had unexpected difficulty with: git error: object file is empty,
lost all my commits along the way, but my progress was safe
https://stackoverflow.com/questions/11706215/how-to-fix-git-error-object-file-is-empty

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
