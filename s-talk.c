#include <stdio.h>
#include <stdlib.h> // For malloc -- DELETE LATER
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

	printf("Welcome to s-talk! \n");
	printf("Enter \"!<ENTER>\" to exit \n");

	char *HOSTPORT = argv[1];
	char *REMOTENAME = argv[2];
	char *REMOTEPORT = argv[3];

	struct senderArgs sendArgs = {REMOTENAME, REMOTEPORT};
	// struct senderArgs sendArgs = {&argv[2], &argv[3]};

	receiverInit(HOSTPORT);
	displayInit();
	inputInit();
	senderInit(&sendArgs);

	inputDestructor();
	displayDestructor();
	receiverDestructor();
    senderDestructor();

	

	
	
	








printf("\n Goodbye! \n");

return 0;
}