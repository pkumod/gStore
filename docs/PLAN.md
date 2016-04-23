## Improve The Core

- add numeric value query function. need to answer numeric range query efficiently and space consume cannot be too large

- add a control module to heuristically select an kind of index for a SPARQL query to filter(not always vstree)

- typedef all frequently used types, to avoid inconsistence and high modify cost

- - -

## Better The Interface

- write web interface for gStore, and a web page to operate on it, just like virtuoso

- - -

## Idea Collection Box

- warnings remain in using Parser/(antlr)!(modify sparql.g 1.1 and regenerate). change name to avoid redefine problem(maybe error), or go to use executable to parse

- build compress module(such as key-value module and stream module), but the latter just needs one-pass read/write, which may causes the compress method to be used both in disk and memory. all operations of string in memory can be changed to operations after compress: provide compress/archive interface, compare function. there are many compress algorithms to be chosen, then how to choose? what about utf-8 encoding problem? this method can lower the consume of memory and disk, but consumes more CPU. However, the time is decided by isomorphism. Simple compress is not good, but too complicated method will consume too much time, how to balance?  (merge the continuous same characters, Huffman tree)

- mmap to speedup KVstore?

- the strategy for Stream:is 85% valid? consider sampling, analyse the size of result set and decide strategy? how to support order by: sort in memory if not put in file; otherwise, partial sort in memory, then put into file, then proceed external sorting

