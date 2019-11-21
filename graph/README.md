# graph - Graph-related problems

Here sits the graph-related problems that will access a common graph data structure.

## Testing

First, compile the libraries and the tests by running `make all` on your terminal line.

### Graph library

The `graph.test.out` executable receives one parameter which is the number of vertices in the graph. See that size = 0 will not be accepted and -1 (which is `2^32 - 1` since size_t is an unsigned data type) will not allocate the data structure. The more-or-less in between should throw no errors whatsoever.

``` bash
$ ./graph.test.out 0
Invalid graph size '0'
$ ./graph.test.out asda
Invalid graph size 'asda'
$ ./graph.test.out -1
Error #2: Could not create graph
$ ./graph.test.out 256
No errors.
```

### Set library

The `set.test.out` executable provides a very easy to use interface for testing the basic functionalities. The parameters you provide is a list of numbers you either add to the set (the number itself) or remove from the set (a `-` (minus sign) followed by the number) or check if it is contained (a `?` (question mark) followed by the number).

``` bash
$ ./set.test.out
No errors.
$ ./set.test.out 1
No errors.
$ ./set.test.out -1
Error #4 on 1-th number (-1): Could not remove number from set
$ ./set.test.out 1 1
Error #3 on 2-th number (1): Could not add number to set
$ ./set.test.out 1 -1
No errors.
$ ./set.test.out ?1 1 ?1 -1 ?1 1 ?1
[1] Does not contain 1
[3] Contains 1
[5] Does not contain 1
[7] Contains 1
No errors.
```