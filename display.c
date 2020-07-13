#include <pthread.h> // For pthreads
#include <stdio.h> // For printf
#include <stdlib.h> // For free

#include "receiver.h" // Need variables from this
#include "sender.h"
#include "input.h"


pthread_t displayThread; // Defining the Display Thread

void displayCleanUp(void *unused) {
    pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext
    pthread_cond_signal(&messagesToDisplay);

}

void *display(void *unused) {
    int oldState;
    int oldType;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType);
    pthread_cleanup_push(displayCleanUp, NULL);

    char *messageToDisplay;
    while (1) {
		pthread_mutex_lock(&receiverDisplayMutex); // Locking mutext
		if (receiverList->size < 1) {
			pthread_cond_wait(&messagesToDisplay,&receiverDisplayMutex); // Do a wait on the condition that we have no more room for a message
		}

		messageToDisplay = (char *) List_first(receiverList);
		printf("\n >> %s", messageToDisplay); // Printing the message on the terminal


        if (*messageToDisplay == '!') {
            pthread_cancel(receiverThread);
            pthread_cancel(inputThread);
            pthread_cancel(senderThread);

            pthread_exit(NULL);
        }

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldState);
		List_remove(receiverList); // Removing the first item on the list
		free(messageToDisplay); // Freeing the dynamically allocated char array
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);

		pthread_mutex_unlock(&receiverDisplayMutex); // Unlocking mutext
		pthread_cond_signal(&receiverSpotAvailable);  // Signaling incase the consumer did a wait earlier, it is now ready

	}
    pthread_cleanup_pop(1);

}

void displayInit() {
    pthread_create(&displayThread, NULL, display, NULL);
}

void displayDestructor() {
    pthread_join(displayThread,NULL);
}