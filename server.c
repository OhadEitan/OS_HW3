#ifndef SERVER_C
#define SERVER_C

#include "queue.h"
#include "segel.h"
#include "request.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter_statistic.h"
#include <sys/time.h>


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

// Initialize Counter_stat


//HW3: Parse the new arguments too
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
    if(argc == 6) { //only in the case of dynamic sched algorithm
        *max_size = atoi(argv[5]);
    }
    
    //TODO: what to do if the num of args is more that 6
}

extern pthread_mutex_t m;
extern pthread_cond_t c;
Queue* requests_waiting_to_be_picked;
Queue* requests_currently_handled;
Counter_statistic* counter_statistics;


void* threadRoutine(void* thread_index){ //TODO: remove the _Noreturn in the beginning of this declaration
    //TODO:  understand how to work with the process' index
    printf("inside routine\n");
    int index_of_thread = *(int*)thread_index;
    free(thread_index);
    struct timeval handle_time, clock, wait_time;
    while(1){
        pthread_mutex_lock(&m);
        while(requests_waiting_to_be_picked->num_of_elements == 0){ //there is no request to handle
            pthread_cond_wait(&c,&m);
            //printf("num of elements: %d\n" , requests_waiting_to_be_picked->num_of_elements);
        }
        //printf("outside the while - num of elements is not 0\n"); 
        //now there is a task to handle
        Node* request_to_handle = requests_waiting_to_be_picked->head;
        //printf("--------1----------\n");
        clock = request_to_handle->time;
        int fd_req_to_handle = request_to_handle->fd;
        counter_statistics->arrival_time[index_of_thread] = clock;
        //printf("--------2----------\n");
        dequeueHead(requests_waiting_to_be_picked);
        //printf("after dequeue from requests_waiting_to_be_picked:\n");
        if(fd_req_to_handle != FD_IS_NOT_VALID){
            enqueue(requests_currently_handled,fd_req_to_handle,clock);
            gettimeofday(&handle_time,NULL);
        }
        timersub(&handle_time, &clock, &wait_time);
        counter_statistics->wait_time[index_of_thread] = wait_time;
        //handle the request
        //printf("handling the request:\n");
        pthread_mutex_unlock(&m);
        requestHandle(fd_req_to_handle,index_of_thread);
        close(fd_req_to_handle);
        //printf("after handling the request:\n");

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
    while (requests_waiting_to_be_picked->num_of_elements +
           requests_currently_handled->num_of_elements == *queue_size)
    {
        pthread_cond_wait(&c, &m);
    }
}

void policy_drop_tail(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request) {
    // TODO this policy isnt good I think
    printf("entered drop tail policy\n");

    if (requests_waiting_to_be_picked->num_of_elements +
        requests_currently_handled->num_of_elements == *queue_size) {
        printf("inside drop tail if \n");
        //close(dequeueTail(requests_waiting_to_be_picked));
        close(*request);
    }
}

void policy_drop_head(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                      int* queue_size, int* request)
{
    if (requests_waiting_to_be_picked->num_of_elements +
        requests_currently_handled->num_of_elements == *queue_size) {
        close(dequeueHead(requests_currently_handled));
    }
}

void policy_block_flush(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                        int* queue_size, int* request){
    while ((requests_waiting_to_be_picked->num_of_elements +
            requests_currently_handled->num_of_elements == *queue_size)
           && (requests_currently_handled->num_of_elements != 0))
    {
        pthread_cond_wait(&c, &m);
    }
    close(*request);
    (*request) = FD_IS_NOT_VALID;
}

void policy_dynamic(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                    int* queue_size, int* request, int max_size) {
    if (requests_waiting_to_be_picked->num_of_elements +
        requests_currently_handled->num_of_elements == *queue_size) {
        if ((*queue_size) == max_size) {
            policy_drop_tail(requests_waiting_to_be_picked, requests_currently_handled, queue_size, request);
            return;
        }
        else {
            (*queue_size)++;
            close(*request);
            (*request) = FD_IS_NOT_VALID;
            return;
        }
    }
}

void policy_drop_random(Queue* requests_waiting_to_be_picked, Queue* requests_currently_handled,
                        int* queue_size, int* request)
{
    int fifty_percent;
    if (requests_waiting_to_be_picked->num_of_elements +
        requests_currently_handled->num_of_elements == *queue_size) {
        fifty_percent = ((*queue_size) / 2);
        for (int i = 0; i < fifty_percent; ++i) {
            close(dequeueRandom(requests_waiting_to_be_picked));
        }
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_amount,queue_size,max_size;
    max_size = 0;
    struct sockaddr_in clientaddr;
    char sched_algorithm[SIZE_OF_SCHED_ALG];
    getargs(&port,&threads_amount, &queue_size, sched_algorithm, &max_size, argc, argv);

    //inits
    
    counter_statistics = malloc(sizeof(Counter_statistic));
    requests_waiting_to_be_picked = malloc(sizeof(Queue));
    requests_currently_handled = malloc(sizeof(Queue));

    initCounterStatistic(counter_statistics,threads_amount);
    initQueue(requests_waiting_to_be_picked);
    initQueue(requests_currently_handled);
    pthread_mutex_init(&m,NULL);
    pthread_cond_init(&c,NULL);
    
     


    //
    // HW3: Create some threads...
    //
    
    
    pthread_t* threads = malloc(sizeof (pthread_t) * threads_amount);
    for(int i=0;i<threads_amount;i++){
		int* index = malloc(sizeof(int));
		*index = i;
        pthread_create(&threads[i],NULL,threadRoutine,index);
    }
    

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        struct timeval date_request;
        gettimeofday(&date_request,NULL);

        
        pthread_mutex_lock(&m);

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
        //printf("before entering to enqueue\n");
        if(connfd != FD_IS_NOT_VALID){
            //printf("connfd is: %d\n" , connfd);
            enqueue(requests_waiting_to_be_picked,connfd,date_request);
            //printf("after enqueue\n");
            
           /* printf("--------PRINT QUEUES---------\n");
            printf("print requests_waiting_to_be_picked:\n");
            display(requests_waiting_to_be_picked);
            printf("print requests_currently_handled:\n");
            display(requests_currently_handled);*/

        }

        pthread_cond_signal(&c);
        pthread_mutex_unlock(&m);

        
        //TODO: make sure that the mutex is lock and unlock properly in the enqueue function
        // requestHandle(connfd); //the handling is in the threads routine - make sure it works
        //Close(connfd);
    }

}


#endif
