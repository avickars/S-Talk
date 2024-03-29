#include <pthread.h> // For pthread
#include <stdio.h> // For printf
#include <stdlib.h> // For free
#include <string.h> // For memset
#include <netdb.h> // For socket
#include <sys/socket.h>
#include <sys/types.h>

#include "list.h" // List implementation

#include "input.h"
#include "display.h"
#include "receiver.h"

#define MSG_MAX_LEN 512

pthread_t senderThread; // Defining the Sender thread

struct addrinfo *result;

static char *msgFromInput; // Pointer to the message to be sent

static bool waiting = false;

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};

void senderCleanup(void *unused) {
    if (waiting){
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            perror("Error");
        }
    }
}

void *sender(void *args) {
    char msgToSend[MSG_MAX_LEN];
    int messageLength;
    pthread_cleanup_push(senderCleanup, NULL);

    while(1) {

        // Entering critical section
        if (pthread_mutex_lock(&inputSenderMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (List_count(senderList) < 1) {
            waiting = true;
            if (pthread_cond_wait(&messagesToSend, &inputSenderMutex) != 0) {
                perror("Error");
                exit(1);
            }
            waiting = false;
        }

        // In critical section
        msgFromInput = (char *) List_first(senderList);
        strcpy(msgToSend, msgFromInput);
        List_remove(senderList);
        free(msgFromInput);

        // Leaving critical section
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (pthread_cond_signal(&inputSpotAvailable) != 0) {
            perror("Error");
            exit(1);
        }

        // Sending message
        messageLength = sendto(socketDescriptor, msgToSend, MSG_MAX_LEN, 0, result->ai_addr, result->ai_addrlen);
        if (messageLength == -1) {
            perror("Error");
            exit(1);
        }

        // Shutdown
        if (*msgToSend== '!') {
            // Sending cancellation request to input thread
            if (pthread_cancel(inputThread) != 0) {
                perror("Error");
                exit(1);
            }

            // Sending cancellation request to receiver thread
            if (pthread_cancel(receiverThread) != 0) {
                perror("Error");
                exit(1);
            }

            // Sending cancellation request to display thread
            if (pthread_cancel(displayThread) != 0) {
                perror("Error");
                exit(1);
            }
            pthread_exit(NULL);
        }
        memset(msgToSend, 0 ,MSG_MAX_LEN);


    }
    pthread_cleanup_pop(1);
}

void senderInit(void *argv) {
    struct senderArgs *senderArgsPtr = argv;

    // Getting Remote Machine Info:
    // CITATION: http://beej.us/guide/bgnet/html/#getaddrinfoman
    // CITATION: https://www.youtube.com/watch?v=MOrvead27B4
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo((char *) senderArgsPtr->REMOTENAME, (char *) senderArgsPtr->REMOTEPORT, &hints, &result) != 0) {
        perror("Error");
        exit(1);
    }

    if (pthread_create(&senderThread, NULL, sender, NULL) != 0) {
        perror("Error");
        exit(1);
    }
}

void senderDestructor() {
    freeaddrinfo(result);

    if (pthread_join(senderThread, NULL) != 0) {
        perror("Error");
        exit(1);
    }


}