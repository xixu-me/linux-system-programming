// data_structures.h
#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <pthread.h>   // For pthread_t, though not strictly needed in Program 2 if monitor is in main
#include <semaphore.h> // For sem_t
#include <time.h>	   // For time_t (optional)

#define NUM_WORKSHOPS 10
#define SHM_NAME "/workshop_monitoring_shm"
#define SEM_MUTEX_NAME "/workshop_monitoring_mutex_sem"

// Min/Max values for simulation
#define MIN_TEMP 10.0f
#define MAX_TEMP 40.0f
#define MIN_HUMIDITY 20.0f
#define MAX_HUMIDITY 80.0f

// Interval for workshop data generation (seconds)
#define WORKSHOP_UPDATE_INTERVAL 2
// Interval for monitoring display (seconds)
#define MONITOR_DISPLAY_INTERVAL 5

typedef struct {
	int workshop_id;
	float temperature;
	float humidity;
	// time_t last_updated; // Optional: timestamp of last update
} WorkshopData;

typedef struct {
	WorkshopData workshops[NUM_WORKSHOPS];
	// Could add a generation counter or other metadata if needed
	// int data_ready_count; // Could be used with condition variables if within one process
} SharedData;

#endif // DATA_STRUCTURES_H