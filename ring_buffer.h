/**
 * Simple ring buffer implementation
 * Lockless and thread safe, as long as there is guaranteed one reader and one writer
 * No dynamic memory allocation. No external dependencies (beyond memcpy & memset)
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

struct ring_buffer {
	uint8_t *data;
	unsigned data_len;
	unsigned head;
	unsigned tail;

	uint64_t bytes_written;  // Stores the total number of bytes written to the buffer
	uint64_t bytes_read;     // Stores the total number of bytes read to the buffer
};

/**
 * Initialise the ring buffer
 * @param rb Ring buffer
 * @param buffer Area to store data in
 * @param size Number of bytes in buffer (must be a power of 2)
 * @return < 0 on failure, >= 0 on success
 */
int ring_buffer_init(struct ring_buffer *rb, void *buffer, unsigned size);

/**
 * Write data into the ring buffer
 * @param rb Ring buffer to write to
 * @param data Data to write in
 * @param data_len maximum number of bytes in `data` to try and write
 * @return Number of bytes written
 */
unsigned ring_buffer_write(struct ring_buffer *rb, const void *data, unsigned data_len);

/**
 * Read data out of the ring buffer
 * @param rb Ring buffer to read from
 * @param data Area to store data in
 * @param max_len Maximum number of bytes to read into `data`
 * @return Number of bytes read into `data`
 */
unsigned ring_buffer_read(struct ring_buffer *rb, void *data, unsigned max_len);

/**
 * Determine if the ring buffer is full or not
 */
bool ring_buffer_full(const struct ring_buffer *rb);

/**
 * Determine if the ring buffer is empty or not
 */
bool ring_buffer_empty(const struct ring_buffer *rb);

/**
 * Determine how many bytes are currently stored in the ring buffer
 */
unsigned ring_buffer_used_space(const struct ring_buffer *rb);

/**
 * Determine how many bytes can currently be written into the ring
 * buffer
 */
unsigned ring_buffer_free_space(const struct ring_buffer *rb);

#endif