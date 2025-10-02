#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

#define CIRCULAR_BUFFER_MAX_SIZE	2048

struct circular_buffer {
	int read_idx;
	int write_idx;
	unsigned char buffer[CIRCULAR_BUFFER_MAX_SIZE];
};

void circular_buffer_init(struct circular_buffer *buffer);

int circular_buffer_empty(struct circular_buffer *buffer);
int circular_buffer_full(struct circular_buffer *buffer);

//Make sure that buffer is not empty before you read it.
unsigned char circular_buffer_read(struct circular_buffer *buffer);

//Make sure that buffer is not full before you read it.
void circular_buffer_write(struct circular_buffer *buffer, unsigned char c);

#endif