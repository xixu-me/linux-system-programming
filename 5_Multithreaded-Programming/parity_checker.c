#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

// Shared variables
int total_count = 0;
int even_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool running = true;

// Thread to report total count every 3 seconds
void* report_total(void* arg) {
    while(running) {
        sleep(3);
        pthread_mutex_lock(&mutex);
        printf("Total numbers entered so far: %d\n", total_count);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Thread to report even count every 5 seconds
void* report_even(void* arg) {
    while(running) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        printf("Total even numbers entered so far: %d\n", even_count);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t total_thread, even_thread;
    int num;
    
    // Create the reporting threads
    pthread_create(&total_thread, NULL, report_total, NULL);
    pthread_create(&even_thread, NULL, report_even, NULL);
    
    printf("Enter integers (enter a negative number to exit):\n");
    
    while(1) {
        scanf("%d", &num);
        
        if(num < 0) {
            printf("Negative number entered. Exiting program.\n");
            break;
        }
        
        pthread_mutex_lock(&mutex);
        total_count++;
        
        if(num % 2 == 0) {
            even_count++;
            printf("%d is an even number.\n", num);
        } else {
            printf("%d is an odd number.\n", num);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    // Signal threads to terminate and wait for them
    running = false;
    pthread_join(total_thread, NULL);
    pthread_join(even_thread, NULL);
    
    pthread_mutex_destroy(&mutex);
    
    return 0;
}