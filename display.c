#include <pthread.h> // For pthreads
#include <stdio.h> // For printf
#include <stdlib.h> // For free
#include "receiver.h" // Need variables from this


pthread_t displayThread; // Defining the Display Thread

void *display(void *unused) {
    while (1) {
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (receiverList->size < 1) {
			pthread_cond_wait(&messagesToDisplay,&receiverDisplayMutex); // Do a wait on the condition that we have no more room for a message
		}

		printf(">> %s\n", (char *) List_first(receiverList)); // Printing the message on the terminal

		Node *temp = List_curr(receiverList);
		
		List_remove(receiverList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array


		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext

		pthread_cond_signal(&receiverSpotAvailable);  // Signaling incase the consumer did a wait earlier, it is now ready

	}

	return NULL;

}

void displayInit() {
    pthread_create(&displayThread, NULL, display, NULL);
}

void displayDestructor() {
    pthread_join(displayThread,NULL);
}