#include <stddef.h> // For NULL
#include <stdio.h> // For printf
#include <pthread.h> // For pthreads
#include <stdlib.h> // For strtoul and malloc
#include <string.h> // For memset
#include "list.h"

#include "display.h"
#include "sender.h"
#include "input.h"

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

    pthread_cond_destroy(&messagesToDisplay);
    pthread_cond_destroy(&receiverSpotAvailable);
    pthread_mutex_destroy(&receiverDisplayMutex);

    List_free(receiverList, freeItem);
    shutdown(socketDescriptor, 2);
}

void *receiver(void *argv) {
    int oldState;
    int oldType;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType);
    pthread_cleanup_push(receiverCleanUp, NULL);
	// NOTE: Socket Set Up Taken from Dr. Brian Fraser CMPT 300 Workshop on June 26 2020
	struct sockaddr_in sin;
	memset(&sin, 0 ,sizeof(sin));
	sin.sin_family = AF_INET; // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network long
	// sin.sin_port = htons(PORT); // Host to network short
	sin.sin_port = htons((unsigned short)strtoul(argv, NULL, 0)); // Host to network short
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Creating the socket for UDP
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

    struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);

    while (1) {
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (List_count(receiverList) > 0) {
			// pthread_cond_signal(&messagesToDisplay);
			pthread_cond_wait(&receiverSpotAvailable,&receiverDisplayMutex); // Immediately waking up diplay
		}
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState);
		char *messageReceived = (char *) malloc(MSG_MAX_LEN * sizeof(char));
        List_append(receiverList, messageReceived); // Putting user input on the list
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);

		int bytesRx = recvfrom(socketDescriptor, messageReceived, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
        messageReceived[terminateIdx] = 0;



		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext

		pthread_cond_signal(&messagesToDisplay);  // Signaling incase the consumer did a wait earlier, it is now ready

	}
    pthread_cleanup_pop(1);
    return NULL;
}

void receiverInit(void *argv) {
    receiverList = List_create(); // Initializing the Receiver List
    pthread_create(&receiverThread, NULL, receiver, argv);
}

void receiverDestructor() {
    pthread_join(receiverThread,NULL);
}
