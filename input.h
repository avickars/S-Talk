#ifndef _INPUT_H_
#define _INPUT_H_

#include <pthread.h> // For pthread
#include "list.h" // For senderList

// Making these accessible
extern pthread_cond_t messagesToSend; 
extern pthread_mutex_t inputSenderMutex;
extern pthread_cond_t inputSpotAvailable;

extern List *senderList;

extern pthread_t inputThread;

void inputInit();

void inputDestructor();

#endif