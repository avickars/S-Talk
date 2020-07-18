#include <stddef.h> // For NULL
#include <pthread.h> // For pthreads
#include <stdlib.h> // For malloc
#include <stdio.h> // For printf
#include <unistd.h> //For read()
#include <errno.h> // For error
#include <string.h> // FOr error
#include "list.h"

#define MSG_MAX_LEN 512

List *senderList; // Defining the Sender List

pthread_t inputThread; // Defining the input thread

pthread_cond_t messagesToSend=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_cond_t inputSpotAvailable=PTHREAD_COND_INITIALIZER;
pthread_mutex_t inputSenderMutex=PTHREAD_MUTEX_INITIALIZER;

bool lostMemory = false;

bool first = true;

char *newMessage = NULL;

static void freeItem(void *pItem) {
    free(pItem);
}

void *input(void *unused) {
    int oldState;
    int oldType;
    int numBytes;

    while (1) {
        int oldCancelState;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
        newMessage = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        lostMemory = true;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldCancelState);

//        read(fileno(stdin), newMessage, MSG_MAX_LEN);
        fgets(newMessage, MSG_MAX_LEN, stdin); // Getting user input

        // Entering critical section
        if (pthread_mutex_lock(&inputSenderMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        if (List_count(senderList) > 1) {
            if(pthread_cond_wait(&inputSpotAvailable,&inputSenderMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }
        }

            // Critical Section
            List_append(senderList, newMessage);
            lostMemory = false;

        // Leaving critical section
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        if (pthread_cond_signal(&messagesToSend) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
    }


}

void inputInit() {
    senderList = List_create(); // Initializing the Receiver List

    if (pthread_create(&inputThread, NULL, input, NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}

void inputDestructor() {
    if (pthread_join(inputThread,NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    List_free(senderList, freeItem);

    if (lostMemory) {
        free(newMessage);
    }

    if (pthread_cond_destroy(&messagesToSend) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    if (pthread_cond_destroy(&inputSpotAvailable) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
    int error = pthread_mutex_destroy(&inputSenderMutex);
    if (error != 0) {
        printf("ERROR: %s (@%d): failed condition ", __func__, __LINE__);
        fprintf(stderr, "Value of errno: %d\n", error);
        perror("Error printed by perror");
        fprintf(stderr, "Error opening file: %s\n", strerror( error));
//        exit(1);
    }

}