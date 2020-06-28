#include <stdio.h>
#include <stdlib.h> // For malloc
#include <pthread.h>
#include <unistd.h> // For sleep
#include <assert.h> // For assert
#include "list.h"


#define MAX_NUM_CHARS 100
pthread_cond_t Buffer_Not_Full=PTHREAD_COND_INITIALIZER;
pthread_cond_t Buffer_Not_Empty=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mVar=PTHREAD_MUTEX_INITIALIZER;

void *keyboardInput(void *senderList) {
	List *sl = senderList;

	while (1)
	{
		pthread_mutex_lock(&mVar); // Locking mutext
		if (sl->size > 0) {
			pthread_cond_wait(&Buffer_Not_Full,&mVar); // Do a wait on the condition that we have no more room for a message
		}
		
		printf("Enter a value: "); // Getting message from the user
		char *ptr = (char*) malloc(MAX_NUM_CHARS * sizeof(char)); // Dynamically allocating an array of char for message
		scanf("%s", ptr); // Getting user input

		List_append(sl, ptr); // Putting user input on the list

		pthread_mutex_unlock(&mVar); // Unlocking mutext

		pthread_cond_signal(&Buffer_Not_Empty);  // Signaling incase the consumer did a wait earlier, it is now ready

	}

	return NULL;
}

void *sender(void *receiverList) {
	List *rl = receiverList;
	while (1) {

		pthread_mutex_lock(&mVar);
		if (rl->size < 1)
		{
			pthread_cond_wait(&Buffer_Not_Empty,&mVar); // Doing a wait since there are no messages
		}
		

		printf(">> %s\n", (char *) List_first(rl)); // Printing the message on the terminal

		Node *temp = List_curr(rl);
		
		List_remove(receiverList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array

		pthread_mutex_unlock(&mVar); // Unlocking mutex variable
        pthread_cond_signal(&Buffer_Not_Full); // Signalling incase the producer is waiting for space


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
	//	return 0;
	// }

	/*
	Intializing the program by first creating the lists
	*/
	List *senderList, *receiverList;
	senderList  = List_create(); // Initializing the list to contain the elements to send
	receiverList = List_create(); // Initializing the list to contain the elements received, and are waiting to display on the screen

	pthread_t producerThread, consumerThread;
	pthread_create(&producerThread, NULL, keyboardInput, senderList);
	sender(senderList);
	// pthread_create(&consumerThread, NULL, consumer, senderList);
	// pthread_t keyboardInputThread;
	// pthread_create(&keyboardInputThread, NULL, screen_output, senderList);
	// keyboard_input(senderList);





return 0;
}