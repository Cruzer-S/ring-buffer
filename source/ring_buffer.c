#include "ring_buffer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

typedef struct ring_buffer_node {
	char *data;
	char *dptr;

	struct ring_buffer_node *next;

	size_t usage;
} *RingBufferNode;

struct ring_buffer {
	RingBufferNode head, tail;

	size_t bufsiz;
	size_t total_usage;
};

static RingBufferNode create_new_node(size_t bufsiz, RingBufferNode next)
{
	RingBufferNode node = malloc(sizeof(struct ring_buffer_node));
	if (node == NULL)
		return NULL;

	node->data = malloc(bufsiz);
	if (node->data == NULL) {
		free(node);
		return NULL;
	}

	node->dptr = node->data;

	node->usage = 0;
	node->next = next ? next : node;

	return node;
}

static RingBufferNode destroy_node(RingBufferNode node)
{
	RingBufferNode next = node->next == node ? NULL : node->next;

	free(node->data);
	free(node);

	return next;
}

RingBuffer ring_buffer_create(size_t bufsiz)
{
	RingBuffer rb;

	rb = malloc(sizeof(struct ring_buffer));
	if (rb == NULL)
		return NULL;

	rb->head = rb->tail = NULL;

	rb->bufsiz = bufsiz;
	rb->total_usage = 0;

	RingBufferNode new_node = create_new_node(rb->bufsiz, NULL);
	if (new_node == NULL) {
		free(rb);
		return NULL;
	}

	rb->head = rb->tail = new_node;

	return rb;
}

size_t ring_buffer_get_usage(RingBuffer rb)
{
	return rb->total_usage;
}

size_t get_remain_size(RingBufferNode node, size_t bufsiz)
{
	return (node->data + bufsiz) - (node->dptr + node->usage);
}

int ring_buffer_enqueue(RingBuffer rb, char *data, size_t size)
{
	size_t remain = get_remain_size(rb->tail, rb->bufsiz);

	if (remain < size) {
		RingBufferNode new = create_new_node(rb->bufsiz, rb->head);
		if (new == NULL)
			return -1;

		memcpy(&rb->tail->dptr[rb->tail->usage], data, remain);
		rb->tail->usage += remain;

		rb->tail->next = new;
		rb->tail = new;

		rb->total_usage += remain;

		return ring_buffer_enqueue(rb, data + remain, size - remain);
	}

	memcpy(&rb->tail->dptr[rb->tail->usage], data, size);
	rb->tail->usage += size;

	rb->total_usage += size;

	return 0;
}

int ring_buffer_enqueue_from_nbfd(RingBuffer rb, int fd)
{
	size_t remain = get_remain_size(rb->tail, rb->bufsiz);

	if (remain == 0) {
		RingBufferNode new = create_new_node(rb->bufsiz, rb->head);
		if (new == NULL)
			return -1;

		rb->tail->next = new;
		rb->tail = new;
	}

	int readlen = read(fd, &rb->tail->dptr[rb->tail->usage], remain);
	if (readlen == -1) {
		if (errno == EAGAIN)
			return 0;

		return -1;
	}

	rb->tail->usage += readlen;
	rb->total_usage += readlen;

	return ring_buffer_enqueue_from_nbfd(rb, fd);
}


int ring_buffer_dequeue(RingBuffer rb, char *data, size_t readlen)
{
	if (rb->total_usage < readlen)
		return -1;

	size_t usage = rb->head->usage;

	if (readlen > usage) {
		memcpy(data, rb->head->dptr, usage);

		rb->total_usage -= usage;

		rb->tail->next = destroy_node(rb->head);
		rb->head = rb->tail->next;

		return ring_buffer_dequeue(rb, data + usage, readlen - usage);
	}

	memcpy(data, rb->head->dptr, readlen);

	rb->head->dptr += readlen;
	rb->head->usage -= readlen;

	rb->total_usage -= readlen;

	return 0;
}

static void ring_buffer_print_internal(
		RingBufferNode head, RingBufferNode current
) {
	if (current->usage == 0)
		return ;

	printf("%*s", (int) current->usage, current->dptr);

	if (current->next != head)
		ring_buffer_print_internal(head, current->next);
}

void ring_buffer_print(RingBuffer rb)
{
	ring_buffer_print_internal(rb->head, rb->head);
}

void ring_buffer_destroy(RingBuffer rb)
{
	while ( (rb->tail->next = destroy_node(rb->head)) )
		rb->head = rb->tail->next;

	free(rb);
}
