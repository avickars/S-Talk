#ifndef _INPUT_H_
#define _INPUT_H_

#include <pthread.h> // For pthread
#include "list.h" // For senderList

// Making these accessible
extern pthread_cond_t messagesToSend; 
extern pthread_mutex_t acceptingInputMutext;
extern pthread_cond_t inputSpotAvailable;
extern List *senderList;

void inputInit();

void inputDestructor();

#endif