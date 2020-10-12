/* sync.h
 * COS 318, Fall 2019: Project 3 Preemptive Scheduler
 * Definitions and types of implementation of locks, condition variables,
 * semaphores, and barriers
 */ 

#ifndef THREAD_H
#define THREAD_H

#include "queue.h"
#include "scheduler.h"

typedef struct {
    enum {
        UNLOCKED,
        LOCKED
    } status;
    node_t wait_queue;
    pcb_t* held_task;
} lock_t;

// Condition variable just needs a queue
typedef struct {
  node_t wait_queue;
} condition_t;

// Fields are the semaphore value (number of resources available), wait queue
typedef struct {
  int val;
  node_t wait_queue;
} semaphore_t;

/* In a barrier we need to keep track of the number of threads that have
   waited (num), the target for this number which when reached the barrier will
   be released, and the queue of threads waiting on the barrier */ 
typedef struct {
  int num;
  int target;
  node_t wait_queue;
} barrier_t;

// Lock functions
void lock_init(lock_t *);
int lock_acquire(lock_t *);
void lock_release(lock_t *);

// Condition variable functions
void condition_init(condition_t * c);
void condition_wait(lock_t * m, condition_t * c);
void condition_signal(condition_t * c);
void condition_broadcast(condition_t * c);

// Semaphore functions
void semaphore_init(semaphore_t * s, int value);
void semaphore_up(semaphore_t * s);
void semaphore_down(semaphore_t * s);

// Barrier functions
void barrier_init(barrier_t * b, int n);
void barrier_wait(barrier_t * b);

#endif
