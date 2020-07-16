#include <stddef.h> // For NULL
#include <stdio.h> // For printf
#include <pthread.h> // For pthreads
#include <stdlib.h> // For strtoul and malloc
#include <string.h> // For memset
#include <unistd.h> // For close()
#include "list.h"

#include <netdb.h> // For socket
#include <sys/socket.h>

pthread_t receiverThread; // Defining the receiverThread

#define MSG_MAX_LEN 512 // Maximum Length of a message

List *receiverList; // Defining the Receiver List

pthread_cond_t messagesToDisplay=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_mutex_t receiverDisplayMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receiverSpotAvailable=PTHREAD_COND_INITIALIZER; // Creating condition variables

int socketDescriptor;

bool lostMemoryReceiver = false;

char *messageReceived = NULL;

static void freeItem(void *pItem) {
    free(pItem);
}

void *receiver(void *argv) {
    struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);

    while (1) {
        int oldCancelState;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
        messageReceived = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        lostMemoryReceiver = true;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldCancelState);

		int bytesRx = recvfrom(socketDescriptor, messageReceived, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
        messageReceived[terminateIdx] = 0;

        // Entering critical section
        if (pthread_mutex_lock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        if (List_count(receiverList) > 0) {
            if (pthread_cond_wait(&receiverSpotAvailable,&receiverDisplayMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }
        }

        // Critical Section
        List_append(receiverList, messageReceived);
        lostMemoryReceiver = false;

        // Leaving critical section
        if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        if (pthread_cond_signal(&messagesToDisplay) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }



	}
}

void receiverInit(void *argv) {

    // Socket Set up
    //// CITATION: Socket Set Up Taken from Dr. Brian Fraser CMPT 300 Workshop on June 26 2020
    struct sockaddr_in sin;
    memset(&sin, 0 ,sizeof(sin));
    sin.sin_family = AF_INET; // Connection may be from network
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network long
    sin.sin_port = htons((unsigned short)strtoul(argv, NULL, 0)); // Host to network short
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Creating the socket for UDP
    if (socketDescriptor == -1) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
    if (bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin)) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    receiverList = List_create(); // Initializing the Receiver List

    // Creating the receiver thread
    if (pthread_create(&receiverThread, NULL, receiver, argv) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}

void receiverDestructor() {
    if (pthread_join(receiverThread,NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    List_free(receiverList, freeItem);

    close(socketDescriptor);

    if (lostMemoryReceiver) {
        free(messageReceived);
    }

    if (pthread_cond_destroy(&messagesToDisplay) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    if (pthread_cond_destroy(&receiverSpotAvailable) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    if (pthread_mutex_destroy(&receiverDisplayMutex) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
//        exit(1);
    }
}
