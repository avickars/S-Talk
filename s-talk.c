#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define MSG_MAX_LEN 1024
#define PORT 22110

int main() {
	printf("Aidan Net Listen Test on UDP Port %d:\n",PORT);
	printf("Connect using: \n");
	printf("netcate -u 127.0.0.1 %d\n",PORT);
	
	struct sockaddr_in sin;
	memset(&sin, 0 ,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(PORT);

	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	while (1) {
		struct sockaddr_in sinRemote;
		unsigned int sin_len = sizeof(sinRemote);
		char messageRx[MSG_MAX_LEN];
		int bytesRx = recvfrom(socketDescriptor,
		messageRx, MSG_MAX_LEN, 0,
		(struct sockaddr *) &sinRemote, &sin_len);

		int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx: MSG_MAX_LEN - 1;

		printf(">> %s ", messageRx);


	}
	
   

    return 0;
}



