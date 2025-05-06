// producer.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <signal.h> // For signal handling
#include <float.h>	// For FLT_MAX etc. (not strictly needed here but good for min/max init)
#include <stdint.h> // For intptr_t

#include "data_structures.h" // Common header

SharedData *shared_data_ptr = NULL;
int shm_fd = -1;
sem_t *mutex_sem = SEM_FAILED;
pthread_t workshop_threads[NUM_WORKSHOPS];
volatile sig_atomic_t keep_running = 1;

void cleanup_ipc() {
	printf("\nProducer cleaning up IPC...\n");
	if (shared_data_ptr != MAP_FAILED && shared_data_ptr != NULL) {
		if (munmap(shared_data_ptr, sizeof(SharedData)) == -1) {
			perror("munmap");
		}
	}
	if (shm_fd != -1) {
		close(shm_fd);
		if (shm_unlink(SHM_NAME) == -1) {
			// perror("shm_unlink"); // Might have been unlinked by another instance or if it never fully started
		}
	}
	if (mutex_sem != SEM_FAILED) {
		sem_close(mutex_sem);
		if (sem_unlink(SEM_MUTEX_NAME) == -1) {
			// perror("sem_unlink"); // Similar to shm_unlink
		}
	}
	printf("Producer IPC cleanup complete.\n");
}

void sigint_handler(int sig) {
	printf("\nProducer received SIGINT. Shutting down...\n");
	keep_running = 0;
	// Give threads a moment to notice keep_running flag
	// In a more robust system, you'd signal threads to exit using pthread_cancel or condition variables.
	// For this example, they will check keep_running.
}

// Function for each workshop thread
void *workshop_thread_func(void *arg) {
	int workshop_id = (int)(intptr_t)arg; // Safer than &i from main loop
	srand(time(NULL) ^ pthread_self());	  // Seed random number generator per thread

	printf("Workshop thread %d started.\n", workshop_id);

	while (keep_running) {
		// Simulate data generation
		float current_temp = MIN_TEMP + ((float)rand() / RAND_MAX) * (MAX_TEMP - MIN_TEMP);
		float current_humidity = MIN_HUMIDITY + ((float)rand() / RAND_MAX) * (MAX_HUMIDITY - MIN_HUMIDITY);

		// Acquire semaphore
		if (sem_wait(mutex_sem) == -1) {
			perror("sem_wait in workshop thread");
			pthread_exit(NULL);
		}

		// Write data to shared memory
		shared_data_ptr->workshops[workshop_id].temperature = current_temp;
		shared_data_ptr->workshops[workshop_id].humidity = current_humidity;
		// shared_data_ptr->workshops[workshop_id].last_updated = time(NULL); // Optional

		printf("Workshop %d: Temp = %.2f C, Humidity = %.2f %%\n",
			workshop_id, current_temp, current_humidity);

		// Release semaphore
		if (sem_post(mutex_sem) == -1) {
			perror("sem_post in workshop thread");
			// Continue, but this is problematic
		}

		// Sleep for a while
		for (int i = 0; i < WORKSHOP_UPDATE_INTERVAL && keep_running; ++i) {
			sleep(1);
		}
	}

	printf("Workshop thread %d exiting.\n", workshop_id);
	pthread_exit(NULL);
}

int main() {
	// Register signal handler for Ctrl+C
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; // or SA_RESTART to restart syscalls if interrupted by this signal
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction for SIGTERM");
		return EXIT_FAILURE;
	}

	// 1. Create or open POSIX shared memory object
	// O_EXCL can be used to ensure we are the first to create it,
	// but for robust restart, we might remove it and handle existing SHM.
	// For this assignment, let's assume clean startup.
	shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0666);
	if (shm_fd == -1) {
		perror("shm_open (Is another producer instance running or /dev/shm full?)");
		// Attempt to open if it already exists (for cleanup or if O_EXCL was removed)
		shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
		if (shm_fd == -1) {
			perror("shm_open (secondary attempt)");
			return EXIT_FAILURE;
		}
		// If opened existing, we might not want to ftruncate, but for a fresh start this is okay
		// Or better, unlink first if it exists from a previous bad run.
		printf("Warning: Shared memory %s already existed. Unlinking and recreating.\n", SHM_NAME);
		shm_unlink(SHM_NAME); // Clean up if it exists
		shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
		if (shm_fd == -1) {
			perror("shm_open (after unlink)");
			return EXIT_FAILURE;
		}
	}

	// 2. Set the size of the shared memory object
	if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
		perror("ftruncate");
		close(shm_fd);
		shm_unlink(SHM_NAME); // Clean up
		return EXIT_FAILURE;
	}

	// 3. Map the shared memory object into the process's address space
	shared_data_ptr = mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shared_data_ptr == MAP_FAILED) {
		perror("mmap");
		close(shm_fd);
		shm_unlink(SHM_NAME); // Clean up
		return EXIT_FAILURE;
	}
	printf("Shared memory created/opened and mapped successfully.\n");

	// Initialize workshop IDs and default data
	for (int i = 0; i < NUM_WORKSHOPS; ++i) {
		shared_data_ptr->workshops[i].workshop_id = i;
		shared_data_ptr->workshops[i].temperature = 0.0f; // Initial value
		shared_data_ptr->workshops[i].humidity = 0.0f;	  // Initial value
	}

	// 4. Create or open a POSIX named semaphore (acting as a mutex)
	// O_EXCL can be used here as well for similar reasons to shm_open.
	mutex_sem = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0666, 1); // Initial value 1 (unlocked)
	if (mutex_sem == SEM_FAILED) {
		perror("sem_open (Is another producer instance running with this semaphore?)");
		sem_unlink(SEM_MUTEX_NAME); // Attempt to clean up if it exists
		mutex_sem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
		if (mutex_sem == SEM_FAILED) {
			perror("sem_open (after unlink)");
			cleanup_ipc(); // Full cleanup
			return EXIT_FAILURE;
		}
		printf("Warning: Semaphore %s already existed. Unlinking and recreating.\n", SEM_MUTEX_NAME);
	}
	printf("Mutex semaphore created/opened successfully.\n");

	// Seed random number generator for main (primarily for workshop thread seeding)
	srand(time(NULL));

	// 5. Create workshop threads
	printf("Creating %d workshop threads...\n", NUM_WORKSHOPS);
	for (long i = 0; i < NUM_WORKSHOPS; ++i) {
		// Note: Casting 'i' to void* and back to int (via intptr_t) is a common shorthand
		// for passing small integer IDs. For complex arguments, a struct pointer is better.
		if (pthread_create(&workshop_threads[i], NULL, workshop_thread_func, (void *)i) != 0) {
			perror("pthread_create");
			keep_running = 0; // Signal other threads to stop
			// Join already created threads before exiting
			for (long j = 0; j < i; ++j) {
				pthread_join(workshop_threads[j], NULL);
			}
			cleanup_ipc();
			return EXIT_FAILURE;
		}
	}

	printf("All workshop threads created. Producer is running. Press Ctrl+C to exit.\n");

	// Keep the main thread alive while workshop threads run
	// The threads will check 'keep_running' which is modified by the signal handler
	while (keep_running) {
		sleep(1); // Check periodically
	}

	// 6. Wait for all workshop threads to complete
	printf("Waiting for workshop threads to exit...\n");
	for (int i = 0; i < NUM_WORKSHOPS; ++i) {
		if (pthread_join(workshop_threads[i], NULL) != 0) {
			perror("pthread_join");
		}
	}
	printf("All workshop threads have exited.\n");

	// 7. Cleanup: Unmap, close, and unlink shared memory and semaphore
	// This will be called by the signal handler or at the end of normal execution
	cleanup_ipc();

	return EXIT_SUCCESS;
}