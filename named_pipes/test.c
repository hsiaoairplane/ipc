#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
	int fd_1[2], fd_2[2];
	pid_t pid_1, pid_2;
	char inbuf[64];

	pipe(fd_1); /* new fifo pipe -> read: fds[0]; write: fds[1] */

	if ((pid_1 = fork()) == 0) {
		/* child process */
		dup2(fd_1[1], STDOUT_FILENO); /* copy new fifo write pipe to STDOUT */

		/* close parent read and write fd */
		close(fd_1[1]);
		close(fd_1[0]);

		execl("/bin/sh", "sh", "-c", "/home/jenting/workspace/whal_ioctl", (char *)0);
	} else {
		/* parent process */
		int status;

		/* wait child process */
		waitpid(pid_1, &status, 0);

		/* read from stdout */
		int l = read(fd_1[0], inbuf, 64);
		inbuf[l-1] = '\0';

		/* close read and write fd */
		close(fd_1[0]);
		close(fd_1[1]);

		printf("strlen(inbuf) = %d\n", strlen(inbuf));
		printf("parent: %d(%s)\n", strlen(inbuf), inbuf);
	}

#if 1
	pipe(fd_2); /* new fifo pipe -> read: fds[0]; write: fds[1] */

	if ((pid_2 = fork()) == 0) {
		/* child process */
		dup2(fd_2[1], STDOUT_FILENO); /* copy new fifo write pipe to STDOUT */

		/* close parent read and write fd */
		close(fd_2[1]);
		close(fd_2[0]);

		execl("/bin/sh", "sh", "-c", "/home/jenting/workspace/whal_ioctl", (char *)0);
	} else {
		/* parent process */
		int status;

		/* wait child process */
		waitpid(pid_2, &status, 0);

		/* read from stdout */
		int l = read(fd_2[0], inbuf, 64);
		inbuf[l-1] = '\0';

		/* close read and write fd */
		close(fd_2[0]);
		close(fd_2[1]);

		printf("strlen(inbuf) = %d\n", strlen(inbuf));
		printf("parent: %d(%s)\n", strlen(inbuf), inbuf);
	}
#endif

	return 0;
}

