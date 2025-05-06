#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>

#define NUM_THREADS 5

// Semaphores to control output order
sem_t semaphores[NUM_THREADS];

// Thread function
void* thread_function(void* arg) {
    int thread_num = *(int*)arg;
    int thread_creation_number = thread_num + 1;
    
    // Random delay between 1-5 seconds
    int delay = rand() % 5 + 1;
    printf("Thread %d created, will delay for %d seconds\n", thread_creation_number, delay);
    sleep(delay);
    
    // Wait for signal to ensure proper output order
    sem_wait(&semaphores[thread_num]);
    
    // Output thread information
    printf("Thread ID: %lu, Creation Number: %d\n", pthread_self(), thread_creation_number);
    
    // Signal the next thread in reverse order
    if (thread_num > 0) {
        sem_post(&semaphores[thread_num - 1]);
    }
    
    free(arg);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int i;
    
    // Seed the random number generator
    srand(time(NULL));
    
    // Initialize all semaphores
    for (i = 0; i < NUM_THREADS; i++) {
        sem_init(&semaphores[i], 0, 0);
    }
    
    // Create all threads
    printf("Creating threads...\n");
    for (i = 0; i < NUM_THREADS; i++) {
        int* thread_num = malloc(sizeof(int));
        *thread_num = i;
        pthread_create(&threads[i], NULL, thread_function, thread_num);
        usleep(100000); // Small delay between thread creation for clarity
    }
    
    // Signal the last created thread to start the reverse order output
    sem_post(&semaphores[NUM_THREADS - 1]);
    
    // Wait for all threads to complete
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Clean up semaphores
    for (i = 0; i < NUM_THREADS; i++) {
        sem_destroy(&semaphores[i]);
    }
    
    printf("All threads completed execution.\n");
    
    return 0;
}