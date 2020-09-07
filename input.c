#include <stddef.h> // For NULL
#include <pthread.h> // For pthreads
#include <stdlib.h> // For malloc
#include <stdio.h> // For printf
#include <string.h> // For error
#include <unistd.h>
#include "list.h"

#define MSG_MAX_LEN 512

List *senderList; // Defining the Sender List

pthread_t inputThread; // Defining the input thread

pthread_cond_t messagesToSend=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_cond_t inputSpotAvailable=PTHREAD_COND_INITIALIZER;
pthread_mutex_t inputSenderMutex=PTHREAD_MUTEX_INITIALIZER;

static bool lostMemory = false;

static char *newMessage = NULL;

static bool waiting = false;

static void freeItem(void *pItem) {
    free(pItem);
}

static void inputCleanup(void *unused) {
    if (waiting) {
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            perror("Error");
            exit(1);
        }
    }

}

static void *input(void *unused) {
    pthread_cleanup_push(inputCleanup, NULL);
    while (1) {
        newMessage = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        lostMemory = true;

        // Testing if the end of a file is reached.  If yes, strcpy() "!" to newMessage to signal a shutdown
        if(fgets(newMessage, MSG_MAX_LEN, stdin) == NULL) {     // Getting user input
            strcpy(newMessage, "!");
        }


        // Entering critical section
        if (pthread_mutex_lock(&inputSenderMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (List_count(senderList) > 1) {
            waiting = true;
            if(pthread_cond_wait(&inputSpotAvailable,&inputSenderMutex) != 0) {
                perror("Error");
                exit(1);
            }
            waiting = false;
        }
            // Critical Section
            if (List_append(senderList, newMessage) == -1) {
                write(fileno(stdout), "List Full, Message Not Sent", MSG_MAX_LEN);
                free(newMessage);
            }
            lostMemory = false;

        // Leaving critical section
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (pthread_cond_signal(&messagesToSend) != 0) {
            perror("Error");
            exit(1);
        }
    }
    pthread_cleanup_pop(1);


}

void inputInit() {
    senderList = List_create(); // Initializing the Receiver List

    if (pthread_create(&inputThread, NULL, input, NULL) != 0) {
        perror("Error");
        exit(1);
    }
}

void inputDestructor() {
    if (pthread_join(inputThread,NULL) != 0) {
        perror("Error");
        exit(1);
    }

    List_free(senderList, freeItem);

    if (lostMemory) {
        free(newMessage);
    }

    if (pthread_cond_destroy(&messagesToSend) != 0) {
        perror("Error");
        exit(1);
    }

    if (pthread_cond_destroy(&inputSpotAvailable) != 0) {
        perror("Error");
        exit(1);
    }

    if (pthread_mutex_destroy(&inputSenderMutex) != 0) {
        perror("Error");
        exit(1);
    }

}