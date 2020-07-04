#include <pthread.h> // For pthread
#include <stdio.h> // For printf
#include <stdlib.h> // For free
#include "input.h"
#include "list.h"

#include <netdb.h> // For socket

pthread_t senderThread; // Defining the Sender thread

void *sender(void *unused) {
    struct sockaddr_in sinRemote;
	unsigned int sinLen = sizeof(sinRemote);
	while(1) {
		pthread_mutex_lock(&acceptingInputMutext);
		if (List_count(senderList) < 1) {
			pthread_cond_wait(&messagesToSend, &acceptingInputMutext);
		}
		Node *temp = List_first(senderList);
		printf("Sent message %s\n", (char *) temp);
		// int len = sendto(*(senderDataPtr->sd), (const char*) temp->item, MSG_MAX_LEN, 0, (const struct sockaddr *) senderDataPtr->serverAddress, sinLen);

		List_remove(senderList); // Removing the first item on the list

		free(temp); // Freeing the dynamically allocated char array

		pthread_mutex_unlock(&acceptingInputMutext);
		pthread_cond_signal(&inputSpotAvailable);

	}
	return NULL;
    return NULL;
}

void senderInit() {
      pthread_create(&senderThread, NULL, sender, NULL);
}

void senderDestructor() {
    pthread_join(senderThread, NULL);
}