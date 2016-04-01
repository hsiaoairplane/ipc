
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MESSAGE "domain socket message test"

#define PROWE_MSG   0x8000  /* Control Message */
#define PROWE_MSG_ASSOC   0x0001 /* Association/Reassociation */

#define WHAL_EVT_REG_PATH "/tmp/whal_evt_reg.sock"

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_un client;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("opening stream sock_fdet");
        exit(1);
    }

	memset(&client, 0, sizeof(client));
    client.sun_family = AF_UNIX;
    strcpy(client.sun_path, WHAL_EVT_REG_PATH);

    if (connect(sock_fd, (struct sockaddr *) &client, sizeof(client)) < 0) {
        close(sock_fd);
        perror("connecting stream sock_fdet");
        exit(1);
    }

	char buf[1024];
	int evt = PROWE_MSG|PROWE_MSG_ASSOC;

	bzero(buf, sizeof(buf));
	sprintf(buf, "%d", evt);

//	if (send(sock_fd, MESSAGE, sizeof(MESSAGE), 0) < 0) {
	if (send(sock_fd, buf, sizeof(buf), 0) < 0) {
        perror("writing on stream sock_fdet");
	}

	if (sock_fd) close(sock_fd);

	return 0;
}

