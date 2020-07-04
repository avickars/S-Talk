#include <pthread.h> // For pthread
#include <stdio.h> // For printf
#include <stdlib.h> // For free
#include <string.h> // For memset
#include "input.h"
#include "list.h"

#include <netdb.h> // For socket
#include <sys/socket.h>
#include <sys/types.h>

#define MSG_MAX_LEN 1024
pthread_t senderThread; // Defining the Sender thread

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};

void *sender(void *args) {
	struct senderArgs *senderArgsPtr = args;
	printf("In Sender \n");
	printf("%s \n", (char *) senderArgsPtr->REMOTENAME);
	printf("%s \n", (char *) senderArgsPtr->REMOTEPORT);

	// Getting Remote Machine Info: 
	// CITATION: http://beej.us/guide/bgnet/html/#getaddrinfoman
	// CITATION: https://www.youtube.com/watch?v=MOrvead27B4
    struct addrinfo hints;
	struct addrinfo *results;   
	struct addrinfo *result;    
	struct sockaddr_in sinRemote;
	int rv;
	int socketDescriptor;
	int len;
    
	memset(&hints, 0, sizeof(hints));        
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	

	if ((rv = getaddrinfo((char *) senderArgsPtr->REMOTENAME, (char *) senderArgsPtr->REMOTEPORT, &hints, &results) != 0)) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (result = results; result != NULL; result = results->ai_next) {
		socketDescriptor = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (socketDescriptor == -1) {
			continue;
		}

		// if (bind(socketDescriptor, (struct sockaddr *) &sinRemote, sizeof(sinRemote) == -1) {
		// 	close(socketDescriptor);
		// 	continue;
		// }
		// break;
	}

	freeaddrinfo(results); // Freeing the linked list

	// bind(socketDescriptor, (struct sockaddr *) &sinRemote, sizeof(sinRemote));
	// int bytesRx = recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
	


	while(1) {
		pthread_mutex_lock(&acceptingInputMutext);
		if (List_count(senderList) < 1) {
			pthread_cond_wait(&messagesToSend, &acceptingInputMutext);
		}
		Node *temp = List_first(senderList);
		printf("Sent message %s\n", (char *) temp);
		// int len = sendto(*(senderDataPtr->sd), (const char*) temp->item, MSG_MAX_LEN, 0, (const struct sockaddr *) senderDataPtr->serverAddress, sinLen);
		len = sendto(socketDescriptor, List_first(senderList), MSG_MAX_LEN, 0, result->ai_addr, result->ai_addrlen);

		List_remove(senderList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array

		pthread_mutex_unlock(&acceptingInputMutext);
		pthread_cond_signal(&inputSpotAvailable);

	}
	return NULL;
    return NULL;
}

void senderInit(void *argv) {
      pthread_create(&senderThread, NULL, sender, argv);
}

void senderDestructor() {
    pthread_join(senderThread, NULL);
}