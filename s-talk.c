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
pthread_mutex_t receiverDisplayMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receiverSpotAvailable=PTHREAD_COND_INITIALIZER; // Creating condition variables


pthread_cond_t messagesToSend=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_mutex_t acceptingInputMutext=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inputSpotAvailable=PTHREAD_COND_INITIALIZER;

pthread_mutex_t screenMutext=PTHREAD_MUTEX_INITIALIZER;


#define MSG_MAX_LEN 1024 // Defining the max size of a message
#define PORT 22110
#define MAX_LIST_SIZE 100

struct receiverData {
	List *senderList;
	int *sd;
};

struct senderData {
	List *receiverList;
	int *sd;
	struct sockaddr_in *serverAddress;
};

void *input(void *senderList) {
	printf("In input \n");
	List *sl = senderList;

	while (1)
	{
		// pthread_mutex_lock(&screenMutext);
		pthread_mutex_lock(&acceptingInputMutext); // Locking mutext
		if (sl->size > 0) {
			pthread_cond_wait(&inputSpotAvailable,&acceptingInputMutext); // Do a wait on the condition that we have no more room for a message
		}

		char *messageRx = (char *) malloc(MSG_MAX_LEN *sizeof(char)); // Dynamically allocating an array of char for message
		printf("Enter a message: ");
		scanf("%s", messageRx); // Getting user input

		List_append(sl, messageRx); // Putting user input on the list

		pthread_mutex_unlock(&acceptingInputMutext); // Unlocking mutext
		// pthread_mutex_unlock(&screenMutext);
		pthread_cond_signal(&messagesToSend);  // Signaling incase the consumer did a wait earlier, it is now ready

	}

	return NULL;
}

void *sender(void *senderDataArg) {
	struct senderData *senderDataPtr = senderDataArg;
	struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);
	while(1) {
		pthread_mutex_lock(&acceptingInputMutext);
		if (senderDataPtr->receiverList->size < 1) {
			pthread_cond_wait(&messagesToSend, &acceptingInputMutext);
		}
		Node *temp = List_first(senderDataPtr->receiverList);
		printf("Sent message %s\n", (char *) temp);
		// int len = sendto(*(senderDataPtr->sd), (const char*) temp->item, MSG_MAX_LEN, 0, (const struct sockaddr *) senderDataPtr->serverAddress, sinLen);

				
		List_remove(senderDataPtr->receiverList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array

		pthread_mutex_unlock(&acceptingInputMutext);
		pthread_cond_signal(&inputSpotAvailable);

	}
	return NULL;
}

void *display(void *receiverList) {
	List *rl = receiverList;

	while (1) {
		pthread_mutex_lock(&screenMutext);
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (rl->size < 1) {
			pthread_cond_wait(&messagesToDisplay,&receiverDisplayMutex); // Do a wait on the condition that we have no more room for a message
		}

		printf(">> %s\n", (char *) List_first(rl)); // Printing the message on the terminal

		Node *temp = List_curr(rl);
		
		List_remove(receiverList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array


		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext
		pthread_mutex_unlock(&screenMutext);

		pthread_cond_signal(&inputSpotAvailable);  // Signaling incase the consumer did a wait earlier, it is now ready

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
			// pthread_cond_signal(&messagesToDisplay);
			pthread_cond_wait(&receiverSpotAvailable,&receiverDisplayMutex); // Immediately waking up diplay
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
	// } 
	
	if (argc < 2) {
		printf("Incorrect Number of arguements \n");
		return 0;
	}

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
	// sin.sin_port = htons(PORT); // Host to network short
	sin.sin_port = htons((unsigned short)strtoul(argv[1], NULL, 0)); // Host to network short

	


	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Creating the socket for UDP

	bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	struct receiverData rd = {receiverList, &socketDescriptor};
	struct senderData sd = {senderList, &socketDescriptor, &sin};

	pthread_t receiverThread;
	pthread_t senderThread;
	pthread_t inputThread;
	pthread_create(&receiverThread, NULL, receiver, &rd);
	pthread_create(&senderThread, NULL, sender, &sd);
	pthread_create(&inputThread, NULL, input, senderList) ;
	display(receiverList);
	// input(senderList);
	// sender(&sd);

	
	
	





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