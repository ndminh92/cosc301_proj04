/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ucontext.h>

#include "threadsalive.h"

/* ***************************** 
     stage 1 library functions
   ***************************** */

void ta_libinit(void) {
    blocked_thread = 0;
    getcontext(main_thread);
    first_thread = NULL;
    last_thread = NULL;
    return;
}

// Create new thread, based on context of current thread. 
// Push new thread to end of queue
void ta_create(void (*func)(void *), void *arg) {
    ucontext_t *newuc = malloc(sizeof(ucontext_t *));
    unsigned char *newstack = (unsigned char *)malloc(STACKSIZE);
    getcontext(newuc);

    // Initialize context
    newuc -> uc_stack.ss_sp = newstack;
    newuc -> uc_stack.ss_size = STACKSIZE;
    newuc -> uc_link = NULL;
    if (first_thread == NULL) { // Queue is empty
        first_thread = newuc;
        last_thread = newuc;
    } else {
        last_thread -> uc_link = newuc;
    }        
    makecontext(newuc, func, 1, arg);
    return;
}

void ta_yield(void) {
    ucontext_t *temp;
    getcontext(temp);
    
    // If no thread in queue, then does nothing    
    if (temp -> uc_link == NULL) {
       return;
    } else {
       last_thread -> uc_link = temp;
       ucontext_t *next_run  = temp -> uc_link;
       temp -> uc_link = NULL;
       swapcontext(temp, next_run);
    }
    return;
}

int ta_waitall(void) {
    if (first_thread == NULL) {
    } else {
        ucontext_t *temp;
        getcontext(temp);
        swapcontext(temp, first_thread);    
    }

    if (blocked_thread == 0) {
        return 1;
    } else {
        return -1;                    
    }
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
}

void ta_sem_destroy(tasem_t *sem) {
}

void ta_sem_post(tasem_t *sem) {
}

void ta_sem_wait(tasem_t *sem) {
}

void ta_lock_init(talock_t *mutex) {
}

void ta_lock_destroy(talock_t *mutex) {
}

void ta_lock(talock_t *mutex) {
}

void ta_unlock(talock_t *mutex) {
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
}

void ta_cond_destroy(tacond_t *cond) {
}

void ta_wait(talock_t *mutex, tacond_t *cond) {
}

void ta_signal(tacond_t *cond) {
}

