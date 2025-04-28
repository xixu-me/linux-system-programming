#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "/tmp/collector_fifo"

int main() {
	int fd;
	int num;

	// Create the FIFO if it doesn't exist
	if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
		perror("mkfifo error");
		exit(EXIT_FAILURE);
	}

	printf("Collector started. Connecting to calculator...\n");

	// Open FIFO for writing
	fd = open(FIFO_PATH, O_WRONLY);
	if (fd == -1) {
		perror("Failed to open FIFO");
		exit(EXIT_FAILURE);
	}

	printf("Connected! Enter integers (Ctrl+D to exit):\n");

	while (scanf("%d", &num) == 1) {
		if (write(fd, &num, sizeof(int)) == -1) {
			perror("Write error");
			break;
		}
		printf("Sent: %d\n", num);
	}

	close(fd);
	printf("Collector exiting.\n");

	return 0;
}