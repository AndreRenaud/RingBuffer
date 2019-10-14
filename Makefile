CFLAGS=-g -Wall -Wextra -pipe
default: test_ring_buffer

test: test_ring_buffer
	./test_ring_buffer

test_ring_buffer: ring_buffer.o test_ring_buffer.o
	$(CC) -o $@ ring_buffer.o test_ring_buffer.o $(LFLAGS)

%.o: %.c ring_buffer.h
	cppcheck --quiet $<
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm *.o test_ring_buffer