#include <pthread.h>

#include "acutest.h"
#include "ring_buffer.h"

static void ring_buffer_dump(const struct ring_buffer *rb)
{
	printf("rb: data_len=%u head=%u tail=%u used=%u free=%u full=%d\n",
		rb->data_len, rb->head, rb->tail, ring_buffer_used_space(rb), 
		ring_buffer_free_space(rb), ring_buffer_full(rb));
	printf("rb: bytes written: %lld bytes read: %lld\n",
		rb->bytes_written, rb->bytes_read);
}

static void simple_test(void)
{
	struct ring_buffer rb;
	uint8_t data[16];
	char output[16];

	TEST_CHECK(ring_buffer_init(&rb, data, sizeof(data)) >= 0);
	TEST_CHECK(ring_buffer_write(&rb, "foo", 4) == 4);
	TEST_CHECK(ring_buffer_used_space(&rb) == 4);
	TEST_CHECK(ring_buffer_read(&rb, output, sizeof(output)) == 4);
	TEST_CHECK(ring_buffer_used_space(&rb) == 0);
	TEST_CHECK(strcmp(output, "foo") == 0);
	ring_buffer_dump(&rb);
}

static void limits_test(void)
{
	struct ring_buffer rb;
	uint8_t data[16];
	char tmp[16];
	char result[16];

	for (unsigned i = 0; i < sizeof(tmp); i++)
		tmp[i] = i;

	TEST_CHECK(ring_buffer_init(&rb, data, sizeof(data)) >= 0);
	TEST_CHECK(ring_buffer_write(&rb, tmp, sizeof(tmp)) == sizeof(tmp) - 1);
	ring_buffer_dump(&rb);
	TEST_CHECK(ring_buffer_full(&rb));
	TEST_CHECK(ring_buffer_read(&rb, result, sizeof(result)) == sizeof(result) - 1);
	TEST_CHECK(memcmp(tmp, result, sizeof(tmp) - 1) == 0);
}

static void wrap_test(void)
{
	struct ring_buffer rb;
	uint8_t data[16];
	char tmp[16];
	char result[16];

	for (unsigned i = 0; i < sizeof(tmp); i++)
		tmp[i] = i * 5 + 3;
	TEST_CHECK(ring_buffer_init(&rb, data, sizeof(data)) >= 0);
	TEST_CHECK(ring_buffer_write(&rb, tmp, sizeof(tmp)) == sizeof(tmp) - 1);
	TEST_CHECK(ring_buffer_read(&rb, result, sizeof(result)) == sizeof(result) - 1);
	TEST_CHECK(memcmp(tmp, result, sizeof(tmp) - 1) == 0);
	TEST_CHECK(ring_buffer_write(&rb, tmp, sizeof(tmp)) == sizeof(tmp) - 1);
	TEST_CHECK(ring_buffer_read(&rb, result, sizeof(result)) == sizeof(result) - 1);
	TEST_CHECK(memcmp(tmp, result, sizeof(tmp) - 1) == 0);
}

static bool thread_done = false;
static bool thread_error = false;

static inline uint8_t pos_to_char(unsigned pos)
{
	// Just so we know we're getting the right data back
	return pos * 5 + 100;
}

static void *reader_thread(void *data)
{
	struct ring_buffer *rb = (struct ring_buffer *)data;
	unsigned int pos = 0;

	while (!thread_done && !thread_error) {
		uint8_t buffer[1024];
		unsigned len = ring_buffer_read(rb, buffer, rand() % sizeof(buffer));
		for (unsigned i = 0; i < len && !thread_error; i++) {
			if (buffer[i] != pos_to_char(pos + i)) {
				fprintf(stderr, "Mismatch at %u: got 0x%x != expected 0x%x\n", pos + i, buffer[i], pos_to_char(pos + i));
				thread_error = true;
			}
		}
		pos += len;
	}

	return NULL;
}

static void *writer_thread(void *data)
{
	struct ring_buffer *rb = (struct ring_buffer *)data;
	unsigned pos = 0;

	while (!thread_done && !thread_error) {
		uint8_t buffer[1024];
		unsigned write_len = rand() % sizeof(buffer);
		for (unsigned i = 0; i < write_len; i++)
			buffer[i] = pos_to_char(pos + i);
		pos += ring_buffer_write(rb, buffer, write_len);
	}

	return NULL;
}

static void thread_test(void)
{
	pthread_t r, w;
	uint8_t buffer[1024];
	struct ring_buffer rb;

	TEST_CHECK(ring_buffer_init(&rb, buffer, sizeof(buffer)) >= 0);

	thread_done = thread_error = false;
	TEST_CHECK(pthread_create(&r, NULL, reader_thread, &rb) >= 0);
	TEST_CHECK(pthread_create(&w, NULL, writer_thread, &rb) >= 0);

	for (unsigned i = 0; i < 10 && !thread_error; i++)
		usleep(200 * 1000);
	thread_done = true;
	pthread_join(r, NULL);
	pthread_join(w, NULL);
	TEST_CHECK(thread_error == false);
	ring_buffer_dump(&rb);
}

TEST_LIST = {
	{"simple", simple_test},
	{"limits", limits_test},
	{"wrap", wrap_test},
	{"thread", thread_test},
	{0}
};