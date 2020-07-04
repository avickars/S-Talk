#include <stdio.h>
#include "receiver.h"
#include "display.h"
#include "input.h"
#include "sender.h"

typedef struct senderArgs_s senderArgs;
struct  senderArgs{
 	void *REMOTENAME;
	void *REMOTEPORT;
};


int main (int argc, char *argv[]) {
	// if (argc > 1) {
	// 	for (int i = 1; i < argc; i++)
	// 	{
	// 		printf("%d --- %s\t", i,argv[i]);
		
		
	// 	} 
	// } 

	char *HOSTPORT = argv[1];
	char *REMOTENAME = argv[2];
	char *REMOTEPORT = argv[3];

	struct senderArgs sendArgs = {REMOTENAME, REMOTEPORT};
	// struct senderArgs sendArgs = {&argv[2], &argv[3]};

	receiverInit(HOSTPORT);
	displayInit();
	inputInit();
	senderInit(&sendArgs);
	senderDestructor();
	inputDestructor();
	displayDestructor();
	receiverDestructor();
	

	
	
	










return 0;
}