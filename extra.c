
// void *keyboardInput(void *senderList) {
// 	List *sl = senderList;

// 	while (1)
// 	{
// 		pthread_mutex_lock(&mVar); // Locking mutext
// 		if (sl->size > 0) {
// 			pthread_cond_wait(&Buffer_Not_Full,&mVar); // Do a wait on the condition that we have no more room for a message
// 		}
		
// 		printf("Enter a value: "); // Getting message from the user
// 		char *ptr = (char*) malloc(MSG_MAX_LEN * sizeof(char)); // Dynamically allocating an array of char for message
// 		scanf("%s", ptr); // Getting user input

// 		List_append(sl, ptr); // Putting user input on the list

// 		pthread_mutex_unlock(&mVar); // Unlocking mutext

// 		pthread_cond_signal(&Buffer_Not_Empty);  // Signaling incase the consumer did a wait earlier, it is now ready

// 	}

// 	return NULL;
// }

// void *sender(void *receiverList) {
// 	List *rl = receiverList;
// 	while (1) {

// 		pthread_mutex_lock(&mVar);
// 		if (rl->size < 1)
// 		{
// 			pthread_cond_wait(&Buffer_Not_Empty,&mVar); // Doing a wait since there are no messages
// 		}
		

// 		printf(">> %s\n", (char *) List_first(rl)); // Printing the message on the terminal

// 		Node *temp = List_curr(rl);
		
// 		List_remove(receiverList); // Removing the first item on the list

// 		free(temp); // Freeing the dynamically allocated char array

// 		pthread_mutex_unlock(&mVar); // Unlocking mutex variable
//         pthread_cond_signal(&Buffer_Not_Full); // Signalling incase the producer is waiting for space


// 	}
	
// 	return NULL;
// }
