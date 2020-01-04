# graph - Graph-related problems

Here sits the graph-related problems that will access a common graph data structure, namely `Graph`, represented by ordered adjacency sets, implemented by the `Set` module.

## Testing

First, compile the libraries and the tests by running CMake on this very directory. You can find the CMake binaries [here](https://cmake.org/download/). If you're running CMake on Windows, a Visual Studio solution will sit on the `bin` folder. And if you're on Linux, a Makefile will be created instead. That's the magic of CMake.

Every test binary executable takes its arguments as instructions to act on a single data structure. The instructions themselves differ from module but the way they are parsed are pretty much the same.

### Graph module testing

The graph will be undirected and of size 10 by default, but you can change either properties by the use of double-dashed flags `type` and `size`, respectively.  The examples below use Linux terminal syntax but the program works equivalently for Windows platforms. Be aware that the testing module is not totally safe, since it makes use of the `sscanf` C function, which overflows overly large numbers and strings. The reason for not crafting a more robust parser is not to overly complicate what isn't in the scope of this project.

``` bash
$ ./graph.test.out
$ ./graph.test.out --size=42
$ ./graph.test.out --size=42 --type=DIRECTED
```

Other than that, you're able to add and remove edges by providing also through command-line parameters the vertices that the edge connects. Add an edge by adding an argument `u,v+`, which connects the vertices u and v (indexes that go from `0` to the `graph size - 1`), and remove an edge by `u,v-`. You can also consult if an edge exists or not by `u,v?`. Observe that this is a simple graph, meaning that `u,u` (also known as an 'arc') cannot exist. Following are some examples of tests.

``` bash
$ ./graph.test.out 1,1+
Error #1 on the action #1 (1,1+): Could not add edge
$ ./graph.test.out 1,2+
No errors.
$ ./graph.test.out 1,2+ 1,2-
No errors.
$ ./graph.test.out 1,2-
Error #4 on the action #1 (1,2-): Could not remove edge
$ ./graph.test.out 1,2?
[1] Does not contain (1,2)
No errors.
$ ./graph.test.out 1,2+ 1,2? 1,2- 1,2? 1,2+ 1,2?
[2] Contains (1,2)
[4] Does not contain (1,2)
[6] Contains (1,2)
No errors.
```

You can even iterate through an adjacency list of neighbours of a certain edge `u` by adding an argument `ui`. This is sometimes crucial for certain algorithms that need to have a good time complexity. It can also be provided the number of neighbours of an edge `u` by adding an argument `un` so that, programmatically, all neighbours can be iterated through. See the following example:

```bash
$ ./graph.test.out 1n
[1] 0
No errors.
$ ./graph.test.out 1,5+ 1,7 1n
[3] 2
No errors.
$ ./graph.test.out 1,5+ 1,7 1i 1i
[3] 7
[4] 5
No errors.
```

Additional help can be obtained by parsing the `--help` flag too.

### Set module testing

Similarly, the testing module provides a very easy to use interface for testing the basic functionalities. The parameters you provide is a list of numbers you either add to the set (the number followed by a `+`) or remove from the set (the number followed by a `-`) or check if it is contained (the number followed by a `?`).

``` bash
$ ./set.test.out
No errors.
$ ./set.test.out 1+
No errors.
$ ./set.test.out 1-
Error #4 on action #1 (1-): Could not remove number from set
$ ./set.test.out 1+ 1+
Error #3 on action #2 (1+): Could not add number to set
$ ./set.test.out 1+ 1-
No errors.
$ ./set.test.out 1? 1+ 1? 1- 1? 1+ 1?
[1] Does not contain 1
[3] Contains 1
[5] Does not contain 1
[7] Contains 1
No errors.
```

Additionally, you can also iterate through the set in order (`f`: first, `l`: last, `p`: previous, `n`: next) and obtain the current number pointed by the cursor (`c`: current) and the set size (`s`: size).

```bash
$ ./set.test.out 1+ 2+ 3+ c p c p c
[4] 3
[6] 2
[8] 1
$ ./set.test.out 1+ 2+ 3+ f c l c s
[5] 1
[7] 3
[8] 3
```

**Beware** that adding and removing numbers from the set may affect the number pointed by the cursor, thus it is not safe to alter the structure state while iterating it (similar to Java List implementations contract).