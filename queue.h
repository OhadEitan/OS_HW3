#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>

pthread_cond_t c; // like in tutorial
pthread_mutex_t m; // like in tutorial


typedef struct node {
    int fd;
    struct timeval time; // when did the request arrive in the server
    struct node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int num_of_elements;
} Queue;

void initQueue(Queue* q1);
void insertToQueue(Queue* q1, int new_elem, struct timeval clock);
int removeHeadFromQueue(Queue* q1);
int removeTailFromQueue(Queue* q1);
int removeRandom(Queue* q1);
void display(Queue* q1);
void deleteQueue(Queue* q1);
void enqueue(Queue* q1, int new_elem, struct timeval clock);
int dequeueHead(Queue* q1);
int dequeueTail(Queue* q1);
int dequeueRandom(Queue* q1);

#endif
