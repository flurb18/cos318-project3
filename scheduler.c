/* scheduler.c
 * COS 318, Fall 2019: Project 3 Preemptive Scheduler
 * Process scheduler for the kernel
 */

#include "common.h"
#include "interrupt.h"
#include "queue.h"
#include "printf.h"
#include "scheduler.h"
#include "util.h"
#include "syslib.h"

pcb_t *current_running;
node_t ready_queue;
node_t sleep_wait_queue;
// More variables...
volatile uint64_t time_elapsed;

// Change this to TRUE to implement extra credit priority scheduling
enum {EXTRA_CREDIT = FALSE};

// Comparison function of priority-time; used in unblock() (see README)
int compare_ptime(node_t *a, node_t *b) {
  ASSERT(disable_count);
  pcb_t *apcb = (pcb_t *) a;
  pcb_t *bpcb = (pcb_t *) b;
  return \
  (apcb->priority * apcb->entry_count < bpcb->priority * bpcb->entry_count);  
}

// Comparison function of deadline in pcb; used in do_sleep()
int compare_deadline(node_t *a, node_t *b) {
  return (((pcb_t *) a)->deadline < ((pcb_t *) b)->deadline);
}

// Round-robin scheduling: Save current_running before preempting
void put_current_running() {
  ASSERT(disable_count);
  // Put it on the ready queue
  unblock(current_running);  
}

// Change current_running to the next task
void scheduler() {
  ASSERT(disable_count);
  while (is_empty(&ready_queue)) {     
    leave_critical();
    enter_critical();
  }
  current_running = (pcb_t *) dequeue(&ready_queue);
  ASSERT(NULL != current_running);
  ++current_running->entry_count;
}

/* Blocking sleep
   Calculate deadline (timestamp when task should be awakened), set it in
   the pcb, and insert it into the sleep wait queue at the right place, then
   set the status and yield control to the scheduler */
void do_sleep(int milliseconds) {
  ASSERT(!disable_count);
  enter_critical();
  current_running->status = BLOCKED;
  current_running->deadline = time_elapsed + milliseconds;
  enqueue_sort(&sleep_wait_queue, &current_running->node, &compare_deadline);
  scheduler_entry();
  leave_critical();
}

/* Check if we can wake up sleeping processes.
   Sleep wait queue is sorted by deadline in ascending order, so we just keep
   popping off elements from the front of the queue until the front of the
   queues deadline is greater than or equal to time_elapsed */
void check_sleeping() {
  ASSERT(disable_count);
  if (!is_empty(&sleep_wait_queue)) { 
    while (((pcb_t *) peek(&sleep_wait_queue))->deadline < time_elapsed) { 
     unblock((pcb_t *) dequeue(&sleep_wait_queue));
    }
  }
}

/* DO NOT MODIFY ANY OF THE FOLLOWING FUNCTIONS */

void do_yield() {
  enter_critical();
  put_current_running();
  scheduler_entry();
  leave_critical();
}

void do_exit() {
  enter_critical();
  current_running->status = EXITED;
  scheduler_entry();
  // No need for leave_critical() since scheduler_entry() never returns
}

void block(node_t * wait_queue) {
  ASSERT(disable_count);
  current_running->status = BLOCKED;
  enqueue(wait_queue, (node_t *) current_running);
  scheduler_entry();
}

// We modify this function slightly to implement extra credit
void unblock(pcb_t * task) {
  ASSERT(disable_count);
  task->status = READY;
  if (EXTRA_CREDIT) {
    enqueue_sort(&ready_queue, (node_t *) task, &compare_ptime);
  } else {
    enqueue(&ready_queue, (node_t *) task);
  }
}

pid_t do_getpid() {
  pid_t pid;  
  enter_critical();
  pid = current_running->pid;
  leave_critical();
  return pid;
}

uint64_t do_gettimeofday(void) {
  return time_elapsed;
}

// Reserved for Extra Credit
priority_t do_getpriority() {
  priority_t priority;  
  enter_critical();
  priority = current_running->priority;
  leave_critical();
  return priority;
}

// Reserved for Extra Credit
void do_setpriority(priority_t priority) {
  if( priority >= 1 ) {
    enter_critical();
    current_running->priority = priority;
    leave_critical();
  }
}
