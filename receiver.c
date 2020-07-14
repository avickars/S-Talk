#include <stddef.h> // For NULL
#include <stdio.h> // For printf
#include <pthread.h> // For pthreads
#include <stdlib.h> // For strtoul and malloc
#include <string.h> // For memset
#include "list.h"

#include <netdb.h> // For socket
#include <sys/socket.h>

pthread_t receiverThread; // Defining the receiverThread

#define MSG_MAX_LEN 1024 // Maximum Length of a message

List *receiverList; // Defining the Receiver List

pthread_cond_t messagesToDisplay=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_mutex_t receiverDisplayMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receiverSpotAvailable=PTHREAD_COND_INITIALIZER; // Creating condition variables

int socketDescriptor;

static void freeItem(void *pItem) {
    free(pItem);
}

void receiverCleanUp(void* unused) {
    pthread_mutex_unlock(&receiverDisplayMutex);
    pthread_cond_signal(&messagesToDisplay);

    shutdown(socketDescriptor, 2);
}

void *receiver(void *argv) {
    int oldState;
    int oldType;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType);
    pthread_cleanup_push(receiverCleanUp, NULL);

    struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);

    while (1) {
        // Locking mutex
		if (pthread_mutex_lock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
		};

		if (List_count(receiverList) > 0) {
		    // Immediately waking up diplay
			if (pthread_cond_wait(&receiverSpotAvailable,&receiverDisplayMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
			}
		}

        // Disabling the cancellation state to ensure that space that is allocated makes it onto the senderList(So it can be freed later on)
        if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
		char *messageReceived = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        List_append(receiverList, messageReceived); // Putting user input on the list
        if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
        // Enabling the cancellation state now that the allocated memory is on the list

		int bytesRx = recvfrom(socketDescriptor, messageReceived, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
        messageReceived[terminateIdx] = 0;


        // Unlocking mutex
		if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
		}

        // Signaling in case the consumer did a wait earlier, it is now ready
		if (pthread_cond_signal(&messagesToDisplay) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
		}

	}
    pthread_cleanup_pop(1);
}

void receiverInit(void *argv) {
    // Socket Set up
    // CITATION: Socket Set Up Taken from Dr. Brian Fraser CMPT 300 Workshop on June 26 2020
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
    if (pthread_cond_destroy(&messagesToDisplay) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    if (pthread_cond_destroy(&receiverSpotAvailable) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
    int errnum = pthread_mutex_destroy(&receiverDisplayMutex);
    if (errnum != 0) {
        fprintf(stderr, "Error: %s\n", strerror( errnum ));
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
//        exit(1);
    }

    List_free(receiverList, freeItem);

    if (pthread_join(receiverThread,NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}
