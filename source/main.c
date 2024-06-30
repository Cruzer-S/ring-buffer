#include <stdio.h>

#include "ring_buffer.h"

#define BUFSIZE 10

char text[] = "12345987650";
size_t text_size = sizeof(text) - 1;



static void enqueue(RingBuffer rb, char *data, int size)
{
	printf("enqueue text: %.*s\n", size, data);
	ring_buffer_enqueue(rb, data, size);
	printf("result: " ); ring_buffer_print(rb); putchar('\n');
	printf("total usage: %zu\n\n", ring_buffer_get_usage(rb));
}

static void dequeue(RingBuffer rb, int size)
{
	char buffer[size + 1];

	ring_buffer_dequeue(rb, buffer, size);
	buffer[size] = '\0';
	printf("dequeue text %d: %s\n", size, buffer);
	printf("result: " ); ring_buffer_print(rb); putchar('\n');
	printf("total usage: %zu\n\n", ring_buffer_get_usage(rb));
}

int main(int argc, char *argv[])
{
	RingBuffer rb = ring_buffer_create(BUFSIZE);
	if (rb == NULL)
		exit(EXIT_FAILURE);

	enqueue(rb, text, text_size);

	dequeue(rb, 3);

	enqueue(rb, text + 5, 3);
	
	dequeue(rb, 6);

	ring_buffer_destroy(rb);

	return 0;
}
