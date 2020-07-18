#include <pthread.h> // For pthreads
#include <stdio.h> // For printf
#include <stdlib.h> // For free
#include <unistd.h> // For Write
#include <string.h> //For strcpy

#include "receiver.h" // Need variables from this
#include "sender.h"
#include "input.h"

#define MSG_MAX_LEN 512

static char *messageFromReceiver = NULL;

pthread_t displayThread; // Defining the Display Thread

static bool lostMemoryDisplay = false;

void *display(void *unused) {

    while (1) {
        // Entering critical section
        if (pthread_mutex_lock(&receiverDisplayMutex) != 0) {
            exit(1);
        }
        if (List_count(receiverList) < 1) {
            if (pthread_cond_wait(&messagesToDisplay, &receiverDisplayMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }
        }

        // Critical section
        messageFromReceiver = (char *) List_first(receiverList);
        List_remove(receiverList);
        lostMemoryDisplay = true;

        // Leaving critical section
        if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        if (pthread_cond_signal(&receiverSpotAvailable) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }

        // Writing contents to the screen
        write(fileno(stdout), messageFromReceiver, MSG_MAX_LEN);

        // Shutdown
        if (*messageFromReceiver == '!') {
            pthread_cancel(inputThread); // Sending cancellation request to input thread
            pthread_cancel(receiverThread); // Sending cancellation request to receiver thread
            pthread_cancel(senderThread); // Sending cancellation request to sender thread
            pthread_exit(NULL);
        }

        free(messageFromReceiver);
        lostMemoryDisplay = false;


    }
}

void displayInit() {
    if (pthread_create(&displayThread, NULL, display, NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}

void displayDestructor() {
    if (lostMemoryDisplay) {
        free(messageFromReceiver);
    }
    if (pthread_join(displayThread,NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}