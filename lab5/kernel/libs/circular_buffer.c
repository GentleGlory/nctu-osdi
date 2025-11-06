#include "circular_buffer.h"



void circular_buffer_init(struct circular_buffer *buffer)
{
	buffer->read_idx = buffer->write_idx = 0;
}

int circular_buffer_empty(struct circular_buffer *buffer)
{
	return buffer->read_idx == buffer->write_idx;
}

int circular_buffer_full(struct circular_buffer *buffer)
{
	int next_idx = (buffer->write_idx + 1) % CIRCULAR_BUFFER_MAX_SIZE;

	return next_idx == buffer->read_idx;
}

unsigned char circular_buffer_read(struct circular_buffer *buffer)
{
	if (circular_buffer_empty(buffer)) {
		return 0;
	}
	unsigned char ret = buffer->buffer[buffer->read_idx];
	buffer->read_idx = (buffer->read_idx + 1) % CIRCULAR_BUFFER_MAX_SIZE;

	return ret;
}

void circular_buffer_write(struct circular_buffer *buffer,
	unsigned char c)
{
	if (circular_buffer_full(buffer)) {
		return; // write failed
	}
	buffer->buffer[buffer->write_idx] = c;
	buffer->write_idx = (buffer->write_idx + 1) % CIRCULAR_BUFFER_MAX_SIZE;
}
