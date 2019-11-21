graph.so: graph.o
	$(CC) $< -shared -o $@

graph.o: graph.c
	$(CC) -c -fPIC $< -o $@
