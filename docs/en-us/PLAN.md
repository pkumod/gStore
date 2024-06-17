## Improve The Core

- speed up the join process and postprocessing of SPARQL using GPU or FPGA

- improve the indices and support concurrent reads

- add numeric value query function. need to answer numeric range query efficiently and space consume cannot be too large

- typedef all frequently used types, to avoid inconsistence and high modify cost

- - -

## Better Interface

- the usability of ghttp(ERROR_CODE, API ...)

- improve socket interface

- docker settings

- - -

## Idea Collection Box

- warnings remain in using Parser/(antlr)!(modify sparql.g 1.1 and regenerate). change name to avoid redefine problem(maybe error), or go to use executable to parse

- mmap to speedup KVstore?

- the strategy for Stream:is 85% valid? consider sampling, analyse the size of result set and decide strategy? how to support order by: sort in memory if not put in file; otherwise, partial sort in memory, then put into file, then proceed external sorting

