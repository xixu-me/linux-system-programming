#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile int number = 0;	// Current even number
volatile int direction = 1; // 1 for ascending, -1 for descending

void handle_sigint(int sig) {
	// Change direction when CTRL+C is pressed
	direction = -direction;
	printf("\nChanging direction to %s\n",
		(direction > 0) ? "ascending" : "descending");
	// Re-register the signal handler
	signal(SIGINT, handle_sigint);
}

int main() {
	// Register SIGINT handler
	signal(SIGINT, handle_sigint);

	printf("Starting program: outputting even numbers\n");
	printf("Press Ctrl+C to change direction\n");

	while (1) {
		printf("%d\n", number);
		number += 2 * direction; // Add or subtract 2 based on direction
		fflush(stdout);			 // Force output to be displayed immediately
		sleep(1);				 // Wait for 1 second
	}

	return 0;
}