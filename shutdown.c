//
// Created by aidan on 2020-07-18.
//
#include <pthread.h>
#include "input.h"
#include "sender.h"
#include "receiver.h"
#include "display.h"

pthread_mutex_t shutdownMutex =PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t shutdownSignal=PTHREAD_COND_INITIALIZER; // Creating condition variables

void mainMutexLock() {
    pthread_cond_wait(&shutdownSignal, &shutdownMutex);
}

void mainMutexUnlock() {
    pthread_cond_signal(&shutdownSignal);
}

void mainMutexDestroy() {
    pthread_mutex_destroy(&shutdownMutex);
}

void shutDown() {
    cancelSender();
    cancelDisplay();
    cancelInput();
    cancelReceiver();
}
