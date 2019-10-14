#include <errno.h>
#include <string.h>

#include "ring_buffer.h"

int ring_buffer_init(struct ring_buffer *rb, void *buffer, unsigned size)
{
	if (size <= 0 || !buffer)
		return -EINVAL;

	if ((size & (size - 1)) != 0) // Must be a power of two size
		return -EINVAL;

	memset(rb, 0, sizeof(*rb));
	rb->data = buffer;
	rb->data_len = size;
	rb->data = (uint8_t *)buffer;
	return 0;
}

unsigned ring_buffer_write(struct ring_buffer *rb, const void *data, unsigned data_len)
{
	const uint8_t *d8 = (const uint8_t *)data;
	unsigned free_space = ring_buffer_free_space(rb);

	if (data_len >= free_space)
		data_len = free_space;

	unsigned to_end = rb->data_len - rb->head;
	if (to_end >= data_len)
		to_end = data_len;

	memcpy(&rb->data[rb->head], data, to_end);

	if (to_end < data_len)
		memcpy(rb->data, d8 + to_end, data_len - to_end);

	rb->head = (rb->head + data_len) % rb->data_len;

	rb->bytes_written += data_len;

	return data_len;
}

unsigned ring_buffer_read(struct ring_buffer *rb, void *data, unsigned max_len)
{
	unsigned used = ring_buffer_used_space(rb);
	uint8_t *d8 = (uint8_t *)data;

	if (used < max_len)
		max_len = used;
	unsigned to_end = rb->data_len - rb->tail;

	if (to_end >= max_len)
		to_end = max_len;

	memcpy(d8, &rb->data[rb->tail], to_end);

	if (to_end < max_len)
		memcpy(d8 + to_end, rb->data, max_len - to_end);

	rb->tail = (rb->tail + max_len) % rb->data_len;

	rb->bytes_read += max_len;

	return max_len;
}

bool ring_buffer_full(const struct ring_buffer *rb)
{
	return rb->head == ((rb->tail - 1) % rb->data_len);
}

bool ring_buffer_empty(const struct ring_buffer *rb)
{
	return rb->head == rb->tail;
}

unsigned ring_buffer_used_space(const struct ring_buffer *rb)
{
	return (rb->head - rb->tail) % rb->data_len;
}

unsigned ring_buffer_free_space(const struct ring_buffer *rb)
{
	return rb->data_len - ((rb->head - rb->tail) % rb->data_len) - 1;
}
