#include <stdio.h>
#include "receiver.h"
#include "display.h"
#include "input.h"
#include "sender.h"


int main (int argc, char *argv[]) {
	// if (argc > 1) {
	// 	for (int i = 1; i < argc; i++)
	// 	{
	// 		printf("%s\t", argv[i]);
		
		
	// 	} 
	// } 

	char *PORT = argv[1];
	char *REMOTENAME = argv[2];
	char *REMOTEPORT = argv[3];

	receiverInit(PORT);
	displayInit();
	inputInit();
	senderInit();
	senderDestructor();
	inputDestructor();
	displayDestructor();
	receiverDestructor();
	

	
	
	










return 0;
}