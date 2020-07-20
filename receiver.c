#include <stddef.h> // For NULL
#include <stdio.h> // For printf
#include <pthread.h> // For pthreads
#include <stdlib.h> // For strtoul and malloc
#include <string.h> // For memset
#include <unistd.h> // For close()
#include <errno.h> // For errno
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

static bool lostMemoryReceiver = false;

static char *messageReceived = NULL;

static bool waiting = false;

static void freeItem(void *pItem) {
    free(pItem);
}

void receiverCleanup(void *unused) {
    if (waiting) {
        if (pthread_mutex_unlock(&receiverDisplayMutex) != 0){
            perror("Error");
            exit(1);
        };
    }

}

void *receiver(void *argv) {
    struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);

	pthread_cleanup_push(receiverCleanup, NULL);

    while (1) {
        messageReceived = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        lostMemoryReceiver = true;

		int bytesRx = recvfrom(socketDescriptor, messageReceived, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
		if (bytesRx == -1) {
            perror("Error");
            exit(1);
		}
		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
        messageReceived[terminateIdx] = 0;

        // Entering critical section
        if (pthread_mutex_lock(&receiverDisplayMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (List_count(receiverList) > 0) {
            waiting = true;
            if (pthread_cond_wait(&receiverSpotAvailable,&receiverDisplayMutex) != 0) {
                perror("Error");
                exit(1);
            }
            waiting = false;
        }


        // Critical Section
        if (List_append(receiverList, messageReceived) != 0) {
            perror("Error");
            exit(1);
        }
        lostMemoryReceiver = false;

        // Leaving critical section
        if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
            perror("Error");
            exit(1);
        }
        if (pthread_cond_signal(&messagesToDisplay) != 0) {
            perror("Error");
            exit(1);
        }
	}
pthread_cleanup_pop(1);
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
        perror("Error");
        exit(1);
    }
    if (bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin)) != 0) {
        perror("Error");
        exit(1);
    }

    receiverList = List_create(); // Initializing the Receiver List

    // Creating the receiver thread
    if (pthread_create(&receiverThread, NULL, receiver, argv) != 0) {
        perror("Error");
        exit(1);
    }
}

void receiverDestructor() {
    if (pthread_join(receiverThread,NULL) != 0) {
        perror("Error");
        exit(1);
    }

    List_free(receiverList, freeItem);

    if (close(socketDescriptor) != 0){
        perror("Error");
        exit(1);
    }

    if (lostMemoryReceiver) {
        free(messageReceived);
    }

    if (pthread_cond_destroy(&messagesToDisplay) != 0) {
        perror("Error");
        exit(1);
    }

    if (pthread_cond_destroy(&receiverSpotAvailable) != 0) {
        perror("Error");
        exit(1);
    }

    if (pthread_mutex_destroy(&receiverDisplayMutex) != 0) {
        perror("Error");
        exit(1);
    }
}