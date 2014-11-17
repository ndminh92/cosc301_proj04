/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>

#include "threadsalive.h"

#define __DEBUG__


/* ********************** *
 * Global static variable *
 * ********************** */
static int blocked_thread;
static ucontext_t main_thread;
static ucontext_t *current_thread;
static ucontext_t *last_thread;

static t_list_t context_list;
static t_list_t context_curr;
#define STACKSIZE 16384
/* ********************** */

/*
static void context_add(ucontext_t ctxt) {
    t_list_t *node = malloc(sizeof(t_list_t));
    node -> context = ctxt;
    node -> blocked = 0;

    if (context_list == NULL) {
        context_list = node;
        node -> prev = node;
        node -> next = node;
    }
    else {
        node -> prev = context_list -> prev;
        context_list -> prev -> next = node;
        node -> next = context_list;
        context_list -> prev = node;
    }
}
*/
// Compare two context. If their stack pointer are equal, they are equal
static int eq_context(ucontext_t *uc1, ucontext_t *uc2) {
#ifdef __DEBUG__
    //printf("context %p has stack %p\n", uc1, (uc1->uc_stack.ss_sp));
    //printf("context %p has stack %p\n", uc2, (uc2->uc_stack.ss_sp));
    printf("context %p has link %p\n", uc1, (uc1->uc_link));
    printf("context %p has link %p\n", uc2, (uc2->uc_link));
    fflush(stdout);
#endif
    if ((uc1 -> uc_link) == (uc2 -> uc_link)) {
        return 1;
    } else {
        return 0;   
    }
}

static void dbg_print_links() {
    ucontext_t *temp = current_thread;
    printf("main thread address is %p\n", &main_thread);
    while (temp != &main_thread) {
        printf("context address %p leads to %p\n", temp, temp->uc_link);
        temp = temp->uc_link;
    }
}

/* ***************************** 
     stage 1 library functions
   ***************************** */

void ta_libinit(void) {
    blocked_thread = 0;
    current_thread = NULL;
    last_thread = NULL;
    return;
}

// Create new thread, based on context of current thread. 
// Push new thread to end of queue
void ta_create(void (*func)(void *), void *arg) {
    ucontext_t *newuc = malloc(sizeof(ucontext_t));
    unsigned char *newstack = (unsigned char *)malloc(STACKSIZE);
    

    // Initialize context
    getcontext(newuc);
    newuc -> uc_stack.ss_sp = newstack;
    newuc -> uc_stack.ss_size = STACKSIZE;
    newuc -> uc_link = &main_thread;    // Go back to main thread
    makecontext(newuc, (void (*)(void))func, 1, arg);
    if (current_thread == NULL) { // Queue is empty
        current_thread = newuc;
    } else {
        last_thread -> uc_link = newuc;
    }        
    last_thread = newuc;
    fprintf(stdout, "Finished adding a thread context\n");
    return;
}

/* 
 * frees finished context before giving up control
 * then push the current context to the end
 * */
void ta_yield(void) {
    ucontext_t curr;
    getcontext(&curr);
#ifdef __DEBUG__
    if (curr.uc_stack.ss_sp == NULL) {
        printf("Houston, we have a null pointer problem.\n");
    }
#endif
    ucontext_t *temp;
    temp = current_thread;
    while (!eq_context(&curr, temp)) {
        current_thread = current_thread -> uc_link;
        free(temp -> uc_stack.ss_sp);
        free(temp);
        temp = current_thread;
    }

    current_thread = current_thread -> uc_link;

    last_thread -> uc_link = temp;
    last_thread = temp;
    last_thread -> uc_link = &main_thread;
#ifdef __DEBUG__
    dbg_print_links();
#endif
    if (swapcontext(last_thread, current_thread) != 0) {
        printf("error 0x05318008: yield failure\n");
    }
}

int ta_waitall(void) {
    //if (current_thread == NULL) {
        // empty queue, nothing to run
    //} 
    //else {
#ifdef __DEBUG__
    dbg_print_links();
#endif
    if (swapcontext(&main_thread, current_thread) != 0) {
        printf("error 0xDEADBEEF: waitall failure\n");
    }
    //}
    
    while (current_thread != &main_thread) {
        ucontext_t *temp = current_thread;
        current_thread = current_thread -> uc_link;
        free(temp -> uc_stack.ss_sp);
        free(temp);
    }

    if (blocked_thread == 0) { // No blocked thread
        return 0;
    } else {		// Some blocked threads
        return -1;                    
    }
}

// Extra function

/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
    sem -> value = value;
}

void ta_sem_destroy(tasem_t *sem) {
}

void ta_sem_post(tasem_t *sem) {
}

void ta_sem_wait(tasem_t *sem) {
}

void ta_lock_init(talock_t *mutex) {
    ta_sem_init(&mutex -> sem, 1);
}

void ta_lock_destroy(talock_t *mutex) {
    ta_sem_destroy(&mutex -> sem);
}

void ta_lock(talock_t *mutex) {
    ta_sem_wait(&mutex -> sem);
}

void ta_unlock(talock_t *mutex) {
    ta_sem_post(&mutex -> sem);
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

