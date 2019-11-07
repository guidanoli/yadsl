CFLAGS = -w

diff: diff.o main.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

debug: CFLAGS += -g -O
debug: diff

.PHONY: debug
