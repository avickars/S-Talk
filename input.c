#include <stddef.h> // For NULL
#include <pthread.h> // For pthreads
#include <stdlib.h> // For malloc
#include <stdio.h> // For printf
#include "list.h"

#define MSG_MAX_LEN 1024

List *senderList; // Defiing the Sender List

pthread_t inputThread; // Defining the input thread

pthread_cond_t messagesToSend=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_mutex_t acceptingInputMutext=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inputSpotAvailable=PTHREAD_COND_INITIALIZER;

void *input(void *unused) {
    while (1)
	{
		// pthread_mutex_lock(&screenMutext);
		pthread_mutex_lock(&acceptingInputMutext); // Locking mutext
		if (List_count(senderList) > 0) {
			pthread_cond_wait(&inputSpotAvailable,&acceptingInputMutext); // Do a wait on the condition that we have no more room for a message
		}

		char *messageRx = (char *) malloc(MSG_MAX_LEN *sizeof(char)); // Dynamically allocating an array of char for message
		printf("\n Enter a message: ");
		// scanf("%s", messageRx);
		fgets(messageRx, 1024, stdin); // Getting user input

		List_append(senderList, messageRx); // Putting user input on the list

		pthread_mutex_unlock(&acceptingInputMutext); // Unlocking mutext
		pthread_cond_signal(&messagesToSend);  // Signaling incase the consumer did a wait earlier, it is now ready

	}
    return NULL;
}


void inputInit() {
    senderList = List_create(); // Initializing the Receiver List
    pthread_create(&inputThread, NULL, input, NULL);
}

void inputDestructor() {
    pthread_join(inputThread,NULL);
}