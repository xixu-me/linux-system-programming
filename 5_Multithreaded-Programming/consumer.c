// consumer.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <float.h>	// For FLT_MAX, FLT_MIN (or DBL_MAX etc.)
#include <signal.h> // For signal handling

#include "data_structures.h" // Common header

SharedData *shared_data_ptr = NULL;
int shm_fd = -1;
sem_t *mutex_sem = SEM_FAILED;
volatile sig_atomic_t keep_running = 1;

void cleanup_ipc_consumer() {
	printf("\nConsumer cleaning up...\n");
	if (shared_data_ptr != MAP_FAILED && shared_data_ptr != NULL) {
		if (munmap(shared_data_ptr, sizeof(SharedData)) == -1) {
			perror("munmap");
		}
	}
	if (shm_fd != -1) {
		close(shm_fd);
		// Consumer does NOT unlink shared memory, producer owns it
	}
	if (mutex_sem != SEM_FAILED) {
		sem_close(mutex_sem);
		// Consumer does NOT unlink semaphore, producer owns it
	}
	printf("Consumer cleanup complete.\n");
}

void sigint_handler_consumer(int sig) {
	printf("\nConsumer received SIGINT. Shutting down...\n");
	keep_running = 0;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = sigint_handler_consumer;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction for SIGTERM");
		return EXIT_FAILURE;
	}

	// 1. Open existing POSIX shared memory object (DO NOT CREATE)
	shm_fd = shm_open(SHM_NAME, O_RDWR, 0666); // O_RDONLY if only reading
	if (shm_fd == -1) {
		perror("shm_open (Is the producer.c program running?)");
		return EXIT_FAILURE;
	}

	// 2. Map the shared memory object into the process's address space
	// Note: Size is known from SharedData struct. ftruncate is not needed here.
	shared_data_ptr = mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // PROT_READ if only reading
	if (shared_data_ptr == MAP_FAILED) {
		perror("mmap");
		close(shm_fd);
		return EXIT_FAILURE;
	}
	printf("Shared memory opened and mapped successfully.\n");

	// 3. Open existing POSIX named semaphore (DO NOT CREATE)
	mutex_sem = sem_open(SEM_MUTEX_NAME, 0); // Flags argument is 0 when opening existing
	if (mutex_sem == SEM_FAILED) {
		perror("sem_open (Is the producer.c program running and semaphore created?)");
		munmap(shared_data_ptr, sizeof(SharedData));
		close(shm_fd);
		return EXIT_FAILURE;
	}
	printf("Mutex semaphore opened successfully.\n");

	printf("Monitoring room display started. Press Ctrl+C to exit.\n");
	printf("Will display data every %d seconds.\n\n", MONITOR_DISPLAY_INTERVAL);

	// 4. Monitoring loop
	while (keep_running) {
		// Wait for semaphore
		if (sem_wait(mutex_sem) == -1) {
			if (keep_running)
				perror("sem_wait in consumer"); // Don't print error if shutting down
			break;								// Exit loop on error or interruption
		}

		float max_temp = -FLT_MAX;
		float min_temp = FLT_MAX;
		int hottest_workshop_idx = -1;
		int coldest_workshop_idx = -1;
		int valid_data_found = 0;

		for (int i = 0; i < NUM_WORKSHOPS; ++i) {
			// Check if the workshop data looks initialized (not default 0.0 or some other sentinel)
			// For this example, any non-zero temp can be considered, or rely on producer to fill.
			// A more robust way would be a 'valid' flag per workshop or timestamp.
			if (shared_data_ptr->workshops[i].temperature != 0.0f || shared_data_ptr->workshops[i].humidity != 0.0f) {
				valid_data_found = 1; // At least one workshop has some data
			}

			float current_temp = shared_data_ptr->workshops[i].temperature;

			if (current_temp > max_temp) {
				max_temp = current_temp;
				hottest_workshop_idx = i;
			}
			if (current_temp < min_temp) {
				min_temp = current_temp;
				coldest_workshop_idx = i;
			}
		}

		// Release semaphore
		if (sem_post(mutex_sem) == -1) {
			perror("sem_post in consumer");
			break; // Exit loop on error
		}

		// Display data
		if (hottest_workshop_idx != -1 && coldest_workshop_idx != -1 && valid_data_found) {
			printf("--- Monitoring Update (%s", ctime(&(time_t){ time(NULL) })); // ctime adds newline
			printf("    Highest Temp: Workshop %d (%.2f C, %.2f %% Humidity)\n",
				shared_data_ptr->workshops[hottest_workshop_idx].workshop_id,
				max_temp,
				shared_data_ptr->workshops[hottest_workshop_idx].humidity);
			printf("    Lowest Temp:  Workshop %d (%.2f C, %.2f %% Humidity)\n",
				shared_data_ptr->workshops[coldest_workshop_idx].workshop_id,
				min_temp,
				shared_data_ptr->workshops[coldest_workshop_idx].humidity);
			printf("---\n\n");
		}
		else if (!valid_data_found) {
			printf("[%s] Waiting for initial data from workshops...\n\n", ctime(&(time_t){ time(NULL) }));
		}
		else {
			printf("[%s] No valid temperature extremes found yet (all workshops might have same temp or no data).\n\n", ctime(&(time_t){ time(NULL) }));
		}
		fflush(stdout);

		// Sleep for the display interval
		for (int i = 0; i < MONITOR_DISPLAY_INTERVAL && keep_running; ++i) {
			sleep(1);
		}
	}

	// 5. Cleanup: Unmap shared memory, close file descriptor, close semaphore
	cleanup_ipc_consumer();

	return EXIT_SUCCESS;
}