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
        printf("ERROR: %s (@%d): Not Enough Arguments \"\"\n", __func__, __LINE__);
        exit(1);
	}

	printf("Welcome to s-talk! \n");
	printf("Enter \"!<ENTER>\" to exit \n");

	char *HOSTPORT = argv[1];
	char *REMOTENAME = argv[2];
	char *REMOTEPORT = argv[3];

	struct senderArgs sendArgs = {REMOTENAME, REMOTEPORT};

	receiverInit(HOSTPORT);
	inputInit();
	displayInit();

	senderInit(&sendArgs);

	senderDestructor();

    inputDestructor();
    displayDestructor();
    receiverDestructor();


printf("\n Goodbye! \n");

return 0;
}