# graph - Graph-related problems

Here sits the graph-related problems that will access a common graph data structure.

## Testing

First, compile the libraries and the tests by running `make all` on your terminal line.

### Graph library

The `graph.test.out` executable receives the first parameter as the number of vertices in the graph. See that size = 0 will not be accepted and -1 (which is `2^64 - 1` since size_t is an unsigned data type) will not allocate the data structure. The more-or-less in between should throw no errors whatsoever.

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

Other than that, you're able to add and remove edges by providing also through command-line parameters the vertices that the edge connects. Add an edge by `u,v+`, connecting the vertices u and v (indexes that go from 0 to the graph size - 1), and remove an edge by `u,v-`. You can also consult if an edge exists or not by `u,v?`. Observe that this is a simple graph, meaning that `u,u` (also known as an 'arc') cannot exist.

``` bash
./graph.test.out 10
No errors.
$ ./graph.test.out 10 1,1+
Error #3 on 2-th number (1,1+): Could not add edge
$ ./graph.test.out 10 1,2+
No errors.
$ ./graph.test.out 10 1,2+ 1,2-
No errors.
$ ./graph.test.out 10 1,2-
Error #6 on 2-th number (1,2-): Could not remove edge
$ ./graph.test.out 10 1,2?
[2] Does not contain (1,2)
No errors.
$ ./graph.test.out 10 1,2+ 1,2? 1,2- 1,2? 1,2+ 1,2?
[3] Contains (1,2)
[5] Does not contain (1,2)
[7] Contains (1,2)
No errors.

```

### Set library

The `set.test.out` executable provides a very easy to use interface for testing the basic functionalities. The parameters you provide is a list of numbers you either add to the set (the number followed by a `+` (plus sign)) or remove from the set (the number followed by a `-` (minus sign)) or check if it is contained (the number followed by a `?` (question mark)).

``` bash
$ ./set.test.out
No errors.
$ ./set.test.out 1+
No errors.
$ ./set.test.out 1-
Error #4 on 1-th number (1-): Could not remove number from set
$ ./set.test.out 1+ 1+
Error #3 on 2-th number (1+): Could not add number to set
$ ./set.test.out 1+ 1-
No errors.
$ ./set.test.out 1? 1+ 1? 1- 1? 1+ 1?
[1] Does not contain 1
[3] Contains 1
[5] Does not contain 1
[7] Contains 1
No errors.
```