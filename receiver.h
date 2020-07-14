#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <pthread.h> // For the confitions and mutexes
#include "list.h" // For the receiver List

extern List *receiverList; // Making the receiverList Available

extern pthread_cond_t messagesToDisplay; // Making this condition Available
extern pthread_mutex_t receiverDisplayMutex; // Making this mutex Available
extern pthread_cond_t receiverSpotAvailable; // Making this condition available

extern pthread_t receiverThread;

extern int socketDescriptor;

void receiverInit(void *argv);

void receiverDestructor();



#endif