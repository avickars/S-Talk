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
    if (pthread_cond_signal(&messagesToSend) != 0) {
        exit(1);
    }
    if (pthread_mutex_unlock(&acceptingInputMutex) != 0) {
        exit(1);
    }
}

void *input(void *unused) {
    int oldState;
    int oldType;
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldType) != 0) {
        exit(1);
    }
    int numBytes;

    pthread_cleanup_push(inputCleanUp, NULL);
    while (1) {
        // Locking mutex
		if (pthread_mutex_lock(&acceptingInputMutex) != 0) {
		    exit(1);
		}
		if (List_count(senderList) > 0) {
            // Do a wait on the condition that we have no more room for a message
			if(pthread_cond_wait(&inputSpotAvailable,&acceptingInputMutex) != 0) {
			    exit(1);
			}
		}
        // Disabling the cancellation state to ensure that space that is allocated makes it onto the senderList(So it can be freed later on)
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0) {
		    exit(1);
		}
		char *messageFromUser = (char *) malloc(MSG_MAX_LEN * sizeof(char)); // Dynamically allocating an array of char for message
        List_append(senderList, messageFromUser); // Putting user input on the list
        if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState) != 0) {
            exit(1);
        }
        // Enabling the cancellation state now that the allocated memory is on the list

		printf("\n Enter a message: ");
		// scanf("%s", messageFromUser);

//		fgets(messageFromUser, MSG_MAX_LEN, stdin); // Getting user input

		numBytes = read(0, messageFromUser, MSG_MAX_LEN);
//        do {
//            numBytes = read(0, messageFromUser, MSG_MAX_LEN);
//        } while (numBytes > 0);

		printf("numbytes: %d", numBytes);

        // Signaling incase the consumer did a wait earlier, it is now ready
		if (pthread_cond_signal(&messagesToSend) != 0) {
		    exit(1);
		}
        // Unlocking mutext
        if (pthread_mutex_unlock(&acceptingInputMutex) != 0) {
            exit(1);
        }
	}
    pthread_cleanup_pop(1);
}


void inputInit() {
    senderList = List_create(); // Initializing the Receiver List
    if (pthread_create(&inputThread, NULL, input, NULL) != 0) {
        exit(1);
    }
}

void inputDestructor() {
    if (pthread_cond_destroy(&messagesToSend) != 0) {
        exit(1);
    }

    if (pthread_cond_destroy(&inputSpotAvailable) != 0) {
        exit(1);
    }

    if (pthread_mutex_destroy(&acceptingInputMutex) != 0) {
        exit(1);
    }

    List_free(senderList, freeItem);

    if (pthread_join(inputThread,NULL) != 0) {
        exit(1);
    }
}