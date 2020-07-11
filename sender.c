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




#define MSG_MAX_LEN 1024

pthread_t senderThread; // Defining the Sender thread

char *messageToSend; // Pointer to the message to be sent

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};

void senderCleanup(void *socketDescriptor) {
    shutdown(*(int *)socketDescriptor, 2);

    pthread_cond_signal(&inputSpotAvailable);
    pthread_mutex_unlock(&acceptingInputMutex);

}

void *sender(void *args) {
	struct senderArgs *senderArgsPtr = args;

	// Getting Remote Machine Info: 
	// CITATION: http://beej.us/guide/bgnet/html/#getaddrinfoman
	// CITATION: https://www.youtube.com/watch?v=MOrvead27B4
    struct addrinfo hints;
	struct addrinfo *result;    
	struct sockaddr_in sinRemote;
	int rv;
	int len;
	int socketDescriptor;

	pthread_cleanup_push(senderCleanup, &socketDescriptor);
    
	memset(&hints, 0, sizeof(hints));        
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo((char *) senderArgsPtr->REMOTENAME, (char *) senderArgsPtr->REMOTEPORT, &hints, &result) != 0)) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	socketDescriptor = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	while(1) {
        pthread_mutex_lock(&acceptingInputMutex);
		if (List_count(senderList) < 1) {
			pthread_cond_wait(&messagesToSend, &acceptingInputMutex);
		}
		messageToSend = (char *) List_first(senderList);

		len = sendto(socketDescriptor, (char *) messageToSend, MSG_MAX_LEN, 0, result->ai_addr, result->ai_addrlen);

		List_remove(senderList); // Removing the first item on the list

		if (*messageToSend == '!') {
		    pthread_cancel(inputThread); // Sending a cancellation request to the input thread
            pthread_cancel(receiverThread); // Sending a cancellation request to the receiver thread
		    pthread_cancel(displayThread); // Sending a cancellation request to the display thread

            pthread_exit(NULL); // Exiting the thread
		}

		free(messageToSend); // Freeing the dynamically allocated char array

		pthread_cond_signal(&inputSpotAvailable);
        pthread_mutex_unlock(&acceptingInputMutex);

	}

	pthread_cleanup_pop(1);
}

void senderInit(void *argv) {
      pthread_create(&senderThread, NULL, sender, argv);
}

void senderDestructor() {
    pthread_join(senderThread, NULL);
}