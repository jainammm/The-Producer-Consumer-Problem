#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "buffer.h"

#define MAX_SLEEP 3

buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t full, empty;

int producedItems, insertAt, removeFrom;

int insert_item(buffer_item item){
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    if( producedItems != BUFFER_SIZE){
        buffer[insertAt++] = item;
        if (insertAt == BUFFER_SIZE)
            insertAt = 0;
        producedItems++;
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        return 0;
    }

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
    
    return -1;
}

int remove_item(buffer_item *item){    
    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    
    if( producedItems != 0){
        *item = buffer[removeFrom++];
        if (removeFrom == BUFFER_SIZE)
            removeFrom = 0;
        producedItems--;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        return 0;
    }

    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
    
    return -1;
}

void *producer(void *param){
    buffer_item item;
    
    while(1){
        sleep(rand() % MAX_SLEEP + 1); 
        
        item = rand();
        if(insert_item(item) == 0)
            printf("Producer created the product - %d!!!\n", item);
        else
            printf("Error in producing :(\n");
    }
}


void *consumer(void *param){
    buffer_item item;
    
    while(1){
        sleep(rand() % MAX_SLEEP + 1); 

        if(remove_item(&item) == 0)
            printf("Consumer consumed the product - %d!!!\n", item);
        else
            printf("Error in consuming :(\n");
    }
}

int main(int argc, char **argv){
    if (argc != 4){
        printf("Please enter three args, specifying sleep time before termination, number of producer threads and number of consumer threads.\n");
        return -1;
    }

    int sleepTime, numProducer, numConsumer;

    sscanf(argv[1], "%d", &sleepTime);
    sscanf(argv[2], "%d", &numProducer);
    sscanf(argv[3], "%d", &numConsumer);

    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    producedItems = 0;
    insertAt = 0;
    removeFrom = 0;

    pthread_t producersThreads[numProducer];
    pthread_t consumersThreads[numConsumer];
    for(int i = 0; i < numConsumer; i++)
        pthread_create(&consumersThreads[i], NULL, consumer, NULL);
    for(int i = 0; i < numProducer; i++)
        pthread_create(&producersThreads[i], NULL, producer, NULL);

    sleep(sleepTime);
    printf("Terminating the Program!\n");
    return 0;
}

