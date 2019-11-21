lib: libgraph.so

tests: t.graph.out

libgraph.so: graph.o
	$(CC) $< -shared -o $@

graph.o: graph.c
	$(CC) -c -fPIC $< -o $@

t.graph.out: t.graph.o libgraph.so
	$(CC) $^ -L. -lgraph -Wl,-R. -o $@

clean:
	rm -rf *.so *.o *.out

.PHONY: lib tests clean
