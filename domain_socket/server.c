
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define WHAL_EVT_REG_PATH "/tmp/whal_evt_reg.sock"
#define BUF_MAX_LEN 1024

int main(int argc, char *argv[])
{
    int sock_fd, connect_fd;
    struct sockaddr_un server;
	struct sockaddr_un client;
	fd_set read_fds;
	socklen_t client_len = sizeof(client);
	char buf[BUF_MAX_LEN];

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("Error creating domain socket: %s(%d)\n", strerror(errno), errno);
        exit(1);
    }

	unlink(WHAL_EVT_REG_PATH);

	memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, WHAL_EVT_REG_PATH, sizeof(server.sun_path));

    if (bind(sock_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
		printf("Error binding domain: %s(%d)\n", strerror(errno), errno);
        exit(1);
    }

    if (listen(sock_fd, 5) < 0) {
		printf("Error listening domain socket %s(%d)\n", strerror(errno), errno);
		exit(1);
	}

	FD_ZERO(&read_fds);
	FD_SET(sock_fd, &read_fds);

    for (;;) {
		select(sock_fd + 1, &read_fds, NULL, NULL, NULL);
		if (FD_ISSET(sock_fd, &read_fds)) {
			bzero(&client, sizeof(client));
			
			if ((connect_fd = accept(sock_fd, (struct sockaddr *) &client, &client_len)) < 0) {
				printf("accept failed: %s(%d)\n", strerror(errno), errno);
				exit(1);
			}

			if (recv(connect_fd, buf, BUF_MAX_LEN, 0) < 0) {
				printf("recv failed: %s(%d)\n", strerror(errno), errno);
				exit(1);
			}

			printf("received message = 0x%x\n", atoi(buf));
			close(connect_fd);
		}
	}

	if (sock_fd) close(sock_fd);
	unlink(WHAL_EVT_REG_PATH);
	
	return 0;
}

