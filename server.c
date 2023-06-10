#include "segel.h"
#include "request.h"
#include "queue.c"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE_OF_SCHED_ALG 7
# define FD_IS_NOT_VALID -999
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
Queue* requests_currently_handled;


_Noreturn void* threadRoutine(void* thread_index){ //TODO: remove the _Noreturn in the beginning of this declaration
    //TODO:  understand how to work with the process' index
    //int index = *(int*)thread_index;
    while(1){
        pthread_mutex_lock(&m);
        while(requests_waiting_to_be_picked->num_of_elements == 0){ //there is no request to handle
            pthread_cond_wait(&c_,&m);
        }
        //now there is a task to handle
        Node* request_to_handle = requests_waiting_to_be_picked->head;
        dequeueHead(requests_waiting_to_be_picked);
        int fd_req_to_handle = request_to_handle->fd;
        if(fd_req_to_handle != FD_IS_NOT_VALID){
            enqueue(requests_currently_handled,fd_req_to_handle);
        }

        //handle the request
        pthread_mutex_unlock(&m);
        requestHandle(fd_req_to_handle);
        close(fd_req_to_handle);

        //request handling is finished
        pthread_mutex_lock(&m);
        dequeueHead(requests_currently_handled); //remove the finished request
        pthread_mutex_unlock(&m);
    }
}

void policy_block(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                  int* queue_size, int* request)
{
    // Do nothing
    return;
}

void policy_drop_tail(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request){
    // TODO this policy isnt good I think
    dequeueTail(requests_currently_handled);
    return;
}

void policy_drop_head(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request){
    dequeueHead(requests_currently_handled);
    return;

}

void policy_block_flush(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                       int* queue_size, int* request){

    if (requests_currently_handled->num_of_elements != 0)
    {
        return;
    }
    else {
        //// TODO the case where waiting is full and handle isn't cover here, what should we do , or maybe
        //// the implementation of this case is in handle function
    }
}

void policy_dynamic(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                        int* queue_size, int* request, int max_size)
{
    if ((*queue_size) == max_size)
    {
        policy_drop_tail(&requests_waiting_to_be_picked, &requests_currently_handled, &queue_size, &request);
        return;
    }
    else
    {
        (*queue_size)++;
        (*request) = FD_IS_NOT_VALID;
        return;

    }
}


void policy_drop_random(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                        int* queue_size, int* request)
{
    int fifty_precent = ((*queue_size) / 2);
    for (int i = 0; i < fifty_precent; ++i) {
        dequeueRandom(requests_waiting_to_be_picked);
    }
    //TODO should I should entere the new request now or just drop 50%
    return;
    }
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

        if(requests_currently_handled->num_of_elements >= queue_size){
            //need to activate the relavant overload handling policy
            if(strcmp(sched_algorithm,"block") == 0){
                policy_block(requests_waiting_to_be_picked,requests_currently_handled, &queue_size, &connfd);
            }
            else if(strcmp(sched_algorithm,"dt") == 0){
                policy_drop_tail(requests_waiting_to_be_picked,requests_currently_handled,&queue_size, &connfd);
            }
            else if(strcmp(sched_algorithm,"dh") == 0){
                policy_drop_head(requests_waiting_to_be_picked,requests_currently_handled,&queue_size, &connfd);
            }
            else if(strcmp(sched_algorithm,"bf") == 0){
                policy_block_flush(requests_waiting_to_be_picked,requests_currently_handled, &queue_size, &connfd);
            }
            else if(strcmp(sched_algorithm,"dynamic") == 0){
                policy_dynamic(requests_waiting_to_be_picked,requests_currently_handled,&queue_size, &connfd, max_size);
            }
            else if(strcmp(sched_algorithm,"random") == 0){
                policy_drop_random(requests_waiting_to_be_picked,requests_currently_handled,&queue_size, &connfd);
            }
            else{
                printf("error while choosing the overload policy\n");
            }
        }
        if(connfd != FD_IS_NOT_VALID){
            enqueue(requests_waiting_to_be_picked,connfd);
        }
        //TODO: make sure that the mutex is lock and unlock properly in the enqueue function
        requestHandle(connfd);
        Close(connfd);
    }

}


    


 
