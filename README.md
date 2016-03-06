**A Modified Elias Code**

The Elias code is used to encode numbers when the upper bound is unknown.
It is a prefix code (or prefix-free code) that adds leading 0's and a 1 to
the encoded version to maintain prefix-freeness. For example,

```
1   ->   1
2   ->  010
3   ->  011
4   ->  ...
```

So, each code comprises some leading 0's, a single 1 bit, and then a value.
The number of leading 0's indicate the number of bits in the value.
The "marker" middle 1 bit simply indicates the end of the number of 0's.
Wikipedia has a nice page on this called "Elias Gamma Coding" at:
http://en.wikipedia.org/wiki/Elias_gamma_coding

**Sidenote**

This library, mec, demonstrates a different approach to Elias coding.
Before going any further, it's worth noting that the whole point of Elias coding
is to encode unbounded numbers. Being solely a proof-of-concept, this library
only works for encoded words up to 64-bits, and thus actually enforces an upper bound.
As a result, this library is likely not useful in any real application.

**This library**

Returning back to the Elias code and this library's modification,
the mec library adds leading 0's AND leading 1's in alternating fashion.
This increases the number of permutations available for a given
encoded bit length, thereby reducing the average bit length per encoded symbol.
(I think... that is, I don't prove any of this :) ).

```
1     010
2     100
3     011
4     101
5    00100
6    11000
7    00101
8    11001
9    00110
10   11010
11   00111
12   11011
13  0001000
14  1110000
15  0001001
16  1110001
17  0001010
18  1110010
19  0001011
20  1110011
21  0001100
22  1110100
23  0001101
24  1110101
25  0001110
26  1110110
27  0001111
28  1110111
```

So, you could use the assignment above (1 code per value), or map the same
underlying value to two codes such that the only difference is that
one code uses leading 0's and the other leading 1's (and flip the marker bit).
This library takes the latter approach, encoding values with a "hi" and
"lo" option so that the same integer value has the same encoded bit-length,
but can be toggled to signal one of two cases: "hi" and "lo".


