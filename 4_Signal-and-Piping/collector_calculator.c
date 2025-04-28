#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
	int pipe_fd[2];
	pid_t pid;
	int num, sum = 0;

	// Create pipe
	if (pipe(pipe_fd) == -1) {
		perror("Pipe creation failed");
		exit(EXIT_FAILURE);
	}

	// Create child process
	pid = fork();

	if (pid < 0) {
		// Fork failed
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		// Child process - calculator
		close(pipe_fd[1]); // Close write end of pipe

		printf("[Calculator] Started. Waiting for integers...\n");

		while (read(pipe_fd[0], &num, sizeof(int)) > 0) {
			sum += num;
			printf("[Calculator] Received: %d, Current Sum: %d\n", num, sum);
		}

		close(pipe_fd[0]);
		printf("[Calculator] Exiting.\n");
		exit(EXIT_SUCCESS);
	}
	else {
		// Parent process - collector
		close(pipe_fd[0]); // Close read end of pipe

		printf("[Collector] Started. Enter integers (Ctrl+D to exit):\n");

		while (scanf("%d", &num) == 1) {
			if (write(pipe_fd[1], &num, sizeof(int)) == -1) {
				perror("Write error");
				break;
			}
			printf("[Collector] Sent: %d\n", num);
		}

		close(pipe_fd[1]); // Close write end to signal EOF to child
		printf("[Collector] Waiting for calculator to finish...\n");

		// Wait for child to finish
		wait(NULL);

		printf("[Collector] Exiting.\n");
	}

	return 0;
}