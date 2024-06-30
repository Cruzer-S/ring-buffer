#ifndef RING_BUFFER_H__
#define RING_BUFFER_H__

#include <stdlib.h>

typedef struct ring_buffer *RingBuffer;

// `bufsiz` must greater than zero.
RingBuffer ring_buffer_create(size_t bufsiz);

size_t ring_buffer_get_usage(RingBuffer rb);

// `size` must greater than zero.
int ring_buffer_enqueue(RingBuffer rb, char *data, size_t size);
int ring_buffer_dequeue(RingBuffer rb, char *data, size_t readlen);

void ring_buffer_destroy(RingBuffer rb);

void ring_buffer_print(RingBuffer rb);

#endif
