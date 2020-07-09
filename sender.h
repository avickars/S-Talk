#ifndef _SENDER_H_
#define _SENDER_H_

extern pthread_t senderThread;

void senderInit(void *argv);

void senderDestructor();

#endif