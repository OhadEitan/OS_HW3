#include "segel.h"
#include "request.h"
#include "queue.c"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE_OF_SCHED_ALG 7
//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//




// HW3: Parse the new arguments too
void getargs(int *port, int* threads_amount, int* queue_size, char* sched_algorithm, int* max_size, int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads_amount = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(sched_algorithm,argv[4]);
    if(argc > 4) { //only in the case of dynamic sched algorithm
        *max_size = atoi(argv[5]);
    }
}

pthread_mutex_t m;
pthread_cond_t c_;
Queue* requests_waiting_to_be_picked;


void* threadRoutine(void* thread_index){
    int index = *(int*)thread_index;
    while(1){
        pthread_mutex_lock(&m);
        while(requests_waiting_to_be_picked->num_of_elements == 0){
            pthread_cond_wait(&c_,&m);
        }
        //now there is a task to handle
        Node* request_to_handle =
    }

}

void policy_block(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                  int* queue_size, int* request){
    if (requests_waiting_to_be_picked->num_of_elements +
        requests_currently_handled->num_of_elements == *queue_size)
    {
        // Do nothing
        ;
    }
    else {
        enqueue(requests_waiting_to_be_picked,*request);
    }
}

void policy_drop_tail(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request){
    requests_currently_handled->dequeue();
    requests_waiting_to_be_picked->enqueue(request);
}

void policy_drop_head(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request){

}




int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_amount,queue_size,max_size;
    struct sockaddr_in clientaddr;
    char sched_algorithm[SIZE_OF_SCHED_ALG];
    getargs(&port,&threads_amount, &queue_size, sched_algorithm, &max_size, argc, argv);

    //create the requests queue
    //requests_waiting_to_be_picked = malloc(sizeof(Queue));
    // requests_currently_handled = malloc(sizeof(Queue));
    Queue* requests_currently_handled;

    initQueue(requests_waiting_to_be_picked);
    initQueue(requests_currently_handled);
    pthread_mutex_init(&m,NULL);


    //
    // HW3: Create some threads...
    //
    pthread_t* threads = malloc(sizeof (pthread_t) * threads_amount);
    for(int i=0;i<threads_amount;i++){
        pthread_create(&threads[i],NULL,threadRoutine,&i);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        //
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        requestHandle(connfd);

        Close(connfd);
    }

}


    


 
