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
	if (argc < 3) {
        exit(1);
	}

	char *HOSTPORT = argv[1];
	char *REMOTENAME = argv[2];
	char *REMOTEPORT = argv[3];

	struct senderArgs sendArgs = {REMOTENAME, REMOTEPORT};

	// Starting threads
	receiverInit(HOSTPORT);
	inputInit();
	displayInit();
	senderInit(&sendArgs);

	// Joining all threads
	senderDestructor();
    inputDestructor();
    displayDestructor();
    receiverDestructor();

return 0;
}