#include <pthread.h> // For pthreads
#include <stdio.h> // For printf
#include <stdlib.h> // For free

#include "receiver.h" // Need variables from this
#include "sender.h"
#include "input.h"


pthread_t displayThread; // Defining the Display Thread

void displayCleanUp(void *unused) {

    // Unlocking mutex
    if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

    if (pthread_cond_signal(&messagesToDisplay) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }

}

void *display(void *unused) {
    int oldState;
    int oldType;
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
    pthread_cleanup_push(displayCleanUp, NULL);

    char *messageToDisplay;
    while (1) {
        // Locking mutext
		if (pthread_mutex_lock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
		    exit(1);
		}

		if (receiverList->size < 1) {
            // Do a wait on the condition that we have no more room for a message
			if (pthread_cond_wait(&messagesToDisplay,&receiverDisplayMutex) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
			    exit(1);
			}
		}

		messageToDisplay = (char *) List_first(receiverList);
		printf("\n >> %s", messageToDisplay); // Printing the message on the terminal

		// Time to shutdown
        if (*messageToDisplay == '!') {
            // Sending a cancellation request to the sender thread
            if (pthread_cancel(senderThread) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }

            // Sending a cancellation request to the input thread
            if(pthread_cancel(inputThread) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }

            // Sending a cancellation request to the receiver thread
            if (pthread_cancel(receiverThread) != 0) {
                printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
                exit(1);
            }

            pthread_exit(NULL);
        }

        // Not allowing cancellation here as acting on a cancellation request here could result in a memory leakage
        if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }
		List_remove(receiverList); // Removing the first item on the list
		free(messageToDisplay); // Freeing the dynamically allocated char array
        if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }

        // Unlocking mutex
        if (pthread_cond_signal(&receiverSpotAvailable) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }

        // Signaling in case the consumer did a wait earlier, it is now ready
        if (pthread_mutex_unlock(&receiverDisplayMutex) != 0) {
            printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
            exit(1);
        }

	}
    pthread_cleanup_pop(1);

}

void displayInit() {
    if (pthread_create(&displayThread, NULL, display, NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}

void displayDestructor() {
    if (pthread_join(displayThread,NULL) != 0) {
        printf("ERROR: %s (@%d): failed condition \"\"\n", __func__, __LINE__);
        exit(1);
    }
}