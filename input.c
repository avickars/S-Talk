#include <stddef.h> // For NULL
#include <pthread.h> // For pthreads
#include <stdlib.h> // For malloc
#include <stdio.h> // For printf
#include <unistd.h> //For read()
#include "list.h"

#define MSG_MAX_LEN 1024

List *senderList; // Defining the Sender List

pthread_t inputThread; // Defining the input thread

pthread_cond_t messagesToSend=PTHREAD_COND_INITIALIZER; // Creating condition variables
pthread_mutex_t acceptingInputMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inputSpotAvailable=PTHREAD_COND_INITIALIZER;

static void freeItem(void *pItem) {
    free(pItem);
}

void inputCleanUp(void *unused) {
    pthread_mutex_unlock(&acceptingInputMutex);

    pthread_cond_destroy(&messagesToSend);
    pthread_cond_destroy(&inputSpotAvailable);
    pthread_mutex_destroy(&acceptingInputMutex);

    List_free(senderList, freeItem);

}

void *input(void *unused) {
    int oldState;
    int oldType;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType);
    int numBytes;

    pthread_cleanup_push(inputCleanUp, NULL);
    while (1)
	{
		pthread_mutex_lock(&acceptingInputMutex); // Locking mutext
		if (List_count(senderList) > 0) {
			pthread_cond_wait(&inputSpotAvailable,&acceptingInputMutex); // Do a wait on the condition that we have no more room for a message
		}
        // Disabling the cancellation state to ensure that space that is allocated makes it onto the senderList(So it can be freed later on)
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState);
		char *messageFromUser = (char *) malloc(MSG_MAX_LEN * sizeof(char)); // Dynamically allocating an array of char for message
        List_append(senderList, messageFromUser); // Putting user input on the list
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
        // Enabling the cancellation state now that the allocated memory is on the list

		printf("\n Enter a message: ");
		// scanf("%s", messageFromUser);

//		fgets(messageFromUser, MSG_MAX_LEN, stdin); // Getting user input

		numBytes = read(0, messageFromUser, MSG_MAX_LEN);
//        do {
//            numBytes = read(0, messageFromUser, MSG_MAX_LEN);
//        } while (numBytes > 0);



		printf("numbytes: %d", numBytes);

		pthread_cond_signal(&messagesToSend);  // Signaling incase the consumer did a wait earlier, it is now ready
        pthread_mutex_unlock(&acceptingInputMutex); // Unlocking mutext
	}
    pthread_cleanup_pop(1);
}


void inputInit() {
    senderList = List_create(); // Initializing the Receiver List
    pthread_create(&inputThread, NULL, input, NULL);
}

void inputDestructor() {
    pthread_join(inputThread,NULL);
}