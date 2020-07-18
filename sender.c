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

static bool lostMemory = false;

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};

void *sender(void *args) {
    char msgToSend[MSG_MAX_LEN];
    int messageLength;

    while(1) {

        // Entering critical section
        if (pthread_mutex_lock(&inputSenderMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        }
        if (List_count(senderList) < 1) {
            if (pthread_cond_wait(&messagesToSend, &inputSenderMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }
        }

        // In critical section
        msgFromInput = (char *) List_first(senderList);
        strcpy(msgToSend, msgFromInput);
        List_remove(senderList);
        free(msgFromInput);

        // Leaving critical section
        if (pthread_mutex_unlock(&inputSenderMutex) != 0) {
            exit(1);
        }
        if (pthread_cond_signal(&inputSpotAvailable) != 0) {
            exit(1);
        }

        // Sending message
        messageLength = sendto(socketDescriptor, msgToSend, MSG_MAX_LEN, 0, result->ai_addr, result->ai_addrlen);
        if (messageLength == -1) {
            exit(1);
        }

        // Shutdown
        if (*msgToSend== '!') {
            pthread_cancel(inputThread); // Sending cancellation request to input thread
            pthread_cancel(receiverThread); // Sending cancellation request to receiver thread
            pthread_cancel(displayThread); // Sending cancellation request to display thread
            pthread_exit(NULL);

        }
        memset(msgToSend, 0 ,MSG_MAX_LEN);


    }
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
        exit(1);
    }

    if (pthread_create(&senderThread, NULL, sender, NULL) != 0) {
      exit(1);
    }
}

void senderDestructor() {
    freeaddrinfo(result);

    if (pthread_join(senderThread, NULL) != 0) {
        exit(1);
    }


}