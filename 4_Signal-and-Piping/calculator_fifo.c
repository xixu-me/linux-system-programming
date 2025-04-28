#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/collector_fifo"

int main() {
	int fd;
	int num;
	int sum = 0;

	printf("Calculator started. Waiting for collector...\n");

	// Open FIFO for reading
	fd = open(FIFO_PATH, O_RDONLY);
	if (fd == -1) {
		perror("Failed to open FIFO");
		exit(EXIT_FAILURE);
	}

	printf("Connected! Ready to receive integers.\n");

	// Read integers from the FIFO and calculate the sum
	while (read(fd, &num, sizeof(int)) > 0) {
		sum += num;
		printf("Received: %d, Current Sum: %d\n", num, sum);
	}

	close(fd);
	printf("Calculator exiting.\n");

	return 0;
}