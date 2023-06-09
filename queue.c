
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

cond_t c; // like in tutorial
mutex_t m; // like in tutorial

typedef struct node {
    int fd;
    struct node *next;
    // struct timeval time;

} Node;

typedef struct {
    Node* head;
    Node* tail;
    int num_of_elements;
} Queue;


// Initialize queue
void initQueue(Queue* q1) {
    q1->head = NULL;
    q1->tail = NULL;
    q1->num_of_elements = 0;
}


// insert node
void insertToQueue(Queue *q1, int new_elem) {
    Node *new_node = malloc(sizeof(Node));
    new_node->fd = new_elem;
    new_node->next = NULL;

    if (q1->num_of_elements == 0)
    {
        q1->tail = new_node;
        q1->head = new_node;
    }
    else
    {
        q1->tail->next = new_node;
        q1->tail = q1->tail->next;
    }
    q1->num_of_elements ++;

}


// Remove node
int removeFromQueue(Queue *q1) {
    int to_return;
    if (q1->head == NULL) {
        // queue is empty
        return -1;
    }
    Node *temp = q1->head;
    to_return = temp->fd;
    q1->head = q1->head->next;
    q1->num_of_elements--;
    if (q1->head == NULL) {
       // after remove queue should be empty
        q1->tail = NULL;
    }
    free(temp);
    return to_return;
}

// display
void display(Queue *q1) {
    Node *temp = q1->head;
    while (temp != NULL) {
        printf("%d ", temp->fd);
        temp = temp->next;
    }
    printf("\n num of elelm is :%d \n", q1->num_of_elements);
    printf("I love U Ohad\n");
}


void deleteQueue(Queue *q1) {
    Node *temp = q1->head;
    while (temp != NULL) {
        Node *curr = temp;
        temp = temp->next;
        free(curr);
    }
    q1->head = NULL;
    q1->tail = NULL;
    q1->num_of_elements = 0;
}


//bool removeByIndex(Queue* q1 , int index)
//{
//    if (index >= q1->num_of_elements )
//    {
//        return false;
//    }
//    if (q1->head == NULL) {
//        return false;
//    }
//
//    Node* temp = q1->head;
//    if (index == 0) {
//        //remove the head of Queue
//        if(q1->num_of_elements == 1)
//        {
//            q1->tail = NULL;
//        }
//        q1->head = temp->next;
//        free(temp);
//        q1->num_of_elements--;
//        return true;
//    }
//    for (int i= 0; i < index - 1 && temp != NULL; i++) {
//        temp = temp->next;
//    }
//
//    if (temp == NULL || temp->next == NULL) {
//        return false;
//    }
//
//    if(q1->num_of_elements - 1 == index)
//    {
//        q1->tail = temp;
//    }
//    Node* temp_next = temp->next->next;
//    free(temp->next);
//    q1->num_of_elements--;
//    temp->next = temp_next;
//    return true;
//}




void enqueue(Queue *q1, int new_elem) {
    mutex_lock(&m);
    //add x to tail
    insertToQueue(q1,new_elem)
    cond_signal(&c);
    mutex_unlock(&m);
}

int dequeue(Queue *q1) {
    int to_return;
    mutex_lock(&m);
    while (q1->num_of_elements == 0) {
        cond_wait(&c, &m);
    }
    to_return = removeFromQueue(q1);
    mutex_unlock(&m);
    return  to_return;
}



int main() {

    return 0;
}


