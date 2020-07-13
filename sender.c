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

struct addrinfo *result;

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};

void senderCleanup(void *socketDescriptor) {
    // Shutting down the socket
    if (shutdown(*(int *)socketDescriptor, 2) !=0) {
//        exit(1);
    }
    freeaddrinfo(result); // Freeing the structure that getaddrinfo() dynamically allocates

    if (pthread_cond_signal(&inputSpotAvailable) != 0) {
        exit(1);
    }
//
    if (pthread_mutex_unlock(&acceptingInputMutex) != 0) {
//        exit(1);
    }
}

void *sender(void *args) {
    int oldState;
	struct senderArgs *senderArgsPtr = args;

	// Getting Remote Machine Info: 
	// CITATION: http://beej.us/guide/bgnet/html/#getaddrinfoman
	// CITATION: https://www.youtube.com/watch?v=MOrvead27B4
    struct addrinfo hints;
	struct sockaddr_in sinRemote;
	int rv;
	int len;
	int socketDescriptor;

	pthread_cleanup_push(senderCleanup, &socketDescriptor);
    
	memset(&hints, 0, sizeof(hints));        
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	if (getaddrinfo((char *) senderArgsPtr->REMOTENAME, (char *) senderArgsPtr->REMOTEPORT, &hints, &result) != 0) {
		exit(1);
	}

	socketDescriptor = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (socketDescriptor == -1) {
	    exit(1);
	}

	while(1) {
        if (pthread_mutex_lock(&acceptingInputMutex) != 0) {
            exit(1);
        }
		if (List_count(senderList) < 1) {
			pthread_cond_wait(&messagesToSend, &acceptingInputMutex);
		}
		messageToSend = (char *) List_first(senderList);

		len = sendto(socketDescriptor, (char *) messageToSend, MSG_MAX_LEN, 0, result->ai_addr, result->ai_addrlen);
        if (len == -1) {
            exit(1);
        }

        // Time to shutdown
		if (*messageToSend == '!') {
            // Sending a cancellation request to the input thread
		    if(pthread_cancel(inputThread) != 0) {
		        exit(1);
		    }

            // Sending a cancellation request to the receiver thread
            if (pthread_cancel(receiverThread) != 0) {
                exit(1);
            }

            // Sending a cancellation request to the display thread
		    if (pthread_cancel(displayThread) != 0){
		        exit(1);
		    }

            pthread_exit(NULL); // Exiting the thread
		}

		// Not allowing cancellation here as acting on a cancellation request here could result in a memory leakage
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0) {
		    exit(1);
		}
        List_remove(senderList); // Removing the first item on the list
		free(messageToSend); // Freeing the dynamically allocated char array
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState) != 0) {
		    exit(1);
		}

		if (pthread_cond_signal(&inputSpotAvailable) != 0) {
		    exit(1);
		}
        if (pthread_mutex_unlock(&acceptingInputMutex) != 0) {
            exit(1);
        }

	}

	pthread_cleanup_pop(1);
}

void senderInit(void *argv) {
      if (pthread_create(&senderThread, NULL, sender, argv) != 0) {
          exit(1);
      }
}

void senderDestructor() {
    if (pthread_join(senderThread, NULL) != 0) {
        exit(1);
    }
}