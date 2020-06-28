#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For malloc
#include <pthread.h> // For pthread

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "list.h"





// pthread_cond_t Buffer_Not_Full=PTHREAD_COND_INITIALIZER; // Creating condition variables
// pthread_cond_t Buffer_Not_Empty=PTHREAD_COND_INITIALIZER;
// pthread_mutex_t mVar=PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t messagesToDisplay=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_cond_t roomOnReceiverList=PTHREAD_COND_INITIALIZER;
pthread_mutex_t receiverDisplayMutex=PTHREAD_MUTEX_INITIALIZER;

#define MSG_MAX_LEN 1024 // Defining the max size of a message
#define PORT 22110
#define MAX_LIST_SIZE 100

struct receiverData {
	List *senderList;
	int *sd;
};


void *display(void *receiverList) {
	List *rl = receiverList;

	while (1) {
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (rl->size < 1) {
			pthread_cond_wait(&messagesToDisplay,&receiverDisplayMutex); // Do a wait on the condition that we have no more room for a message
		}

		printf(">> %s\n", (char *) List_first(rl)); // Printing the message on the terminal

		Node *temp = List_curr(rl);
		
		List_remove(receiverList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array


		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext

		// pthread_cond_signal(&roomOnReceiverList);  // Signaling incase the consumer did a wait earlier, it is now ready

	}

	return NULL;
}

void *receiver(void *receiverDataArg) {
	struct receiverData *receiverDataPtr = receiverDataArg;
	List *rl = receiverDataPtr->senderList;
	struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);

	while (1) {
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (rl->size > 0) {
			pthread_cond_signal(&messagesToDisplay);
			// pthread_cond_wait(&roomOnReceiverList,&receiverDisplayMutex); // Immediately waking up diplay
		}

		char *messageRx = (char *) malloc(MSG_MAX_LEN *sizeof(char));

		int bytesRx = recvfrom(*(receiverDataPtr->sd), messageRx, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
		messageRx[terminateIdx] = 0;

		List_append(rl, messageRx); // Putting user input on the list

		printf(">> %s", (char *) List_first(rl)); // Printing the message on the terminal

		Node *temp = List_curr(rl);
		
		List_remove(rl); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array

		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext

		pthread_cond_signal(&messagesToDisplay);  // Signaling incase the consumer did a wait earlier, it is now ready

	}

	return NULL;
}

int main (int argc, char *argv[]) {
	// Testing for command line arguements
	// if (argc > 1) {
	// 	for (int i = 1; i < argc; i++)
	// 	{
	// 		printf("%s\t", argv[i]);
		
		
	// 	} 
	// } else {
	// 	printf("Incorrect Number of arguements \n");
	// 	return 0;
	// }

	// connect using: netcat -u 127.0.0.1 22110

	// PROGRAM INITIALIZATION
	List *senderList, *receiverList; // Defining the sendlist (messages to send) and the receiverList (messages received)
	senderList  = List_create(); // Initializing the list to contain the elements to send
	receiverList = List_create(); // Initializing the list to contain the elements received, and are waiting to display on the screen

	// NOTE: Socket Set Up Taken from Dr. Brian Fraser CMPT 300 Workshop on June 26 2020
	struct sockaddr_in sin;
	memset(&sin, 0 ,sizeof(sin));
	sin.sin_family = AF_INET; // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network long
	sin.sin_port = htons(PORT); // Host to network short

	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Creating the socket for UDP

	bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	struct receiverData rd = {receiverList, &socketDescriptor};
	pthread_t receiverThread;
	// pthread_create(&receiverThread, NULL, receiver, &rd);
	// display(receiverList);

	receiver(&rd);





	// while (1)
	// {
	// 	struct sockaddr_in sinRemote;
	// 	unsigned int sinLen = sizeof(sinRemote);
	// 	char messageRx[MSG_MAX_LEN];
	// 	int bytesRx = recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sinLen);
	// 	int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;
	// 	messageRx[terminateIdx] = 0;
	// 	printf(">> %s", messageRx);

	// }
	








return 0;
}