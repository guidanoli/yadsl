lib: libset.so libgraph.so

tests: set.test.out graph.test.out

all: lib tests

clean:
	rm -rf *.so *.o *.out

libset.so: set.o
	$(CC) $^ -shared -o $@

libgraph.so: graph.o set.o
	$(CC) $^ -shared -o $@

graph.o: graph.c
	$(CC) -c -fPIC $< -o $@

set.o: set.c
	$(CC) -c -fPIC $< -o $@

graph.test.out: graph.test.o libgraph.so
	$(CC) $^ -L. -lgraph -Wl,-R. -o $@

graph.test.o: graph.test.c
	$(CC) -c $< -o $@

set.test.out: set.test.o libset.so
	$(CC) $^ -L. -lset -Wl,-R. -o $@

set.test.o: set.test.c
	$(CC) -c $< -o $@

.PHONY: lib tests all clean
