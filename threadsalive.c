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

static t_list_t *head;
static t_list_t *tail;
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
    head = NULL;
    tail = NULL;
    return;
}

// Create new thread, based on context of current thread. 
// Push new thread to end of queue
void ta_create(void (*func)(void *), void *arg) {
    t_list_t *newuc = malloc(sizeof(t_list_t));
    unsigned char *newstack = (unsigned char *)malloc(STACKSIZE);
    

    // Initialize context
    getcontext(&newuc->context);
    newuc -> context.uc_stack.ss_sp = newstack;
    newuc -> context.uc_stack.ss_size = STACKSIZE;
    newuc -> context.uc_link = &main_thread;    // Go back to main thread. Always
    makecontext(&newuc -> context, (void (*)(void))func, 1, arg);
    if (head == NULL) { // Queue is empty
        head = newuc;
    } else {
        tail -> next = newuc;
    }        
    tail = newuc;
    printf("Finished adding a thread context\n");
    return;
}

/* 
 * frees finished context before giving up control
 * then push the current context to the end
 * */
void ta_yield(void) {
    if (head == tail) { // Only one thread in queue, nothing to yield to
        return;
    } else { // Move thread to end of queue
        ucontext_t *thisuc = &head -> context;
        t_list_t *temp = head -> next;
        
        // Move head to end
        tail -> next = head;
        head -> next = NULL;
        tail = head;

        head = temp;
        ucontext_t *nextuc = &temp -> context;
        swapcontext(thisuc, nextuc);
    }
}

int ta_waitall(void) {
    while (head != NULL) {
        swapcontext(&main_thread, &head -> context);
        // Return here only if context finishes
        t_list_t *temp = head;
        if (head == tail) { // Last thread in queue
            head = NULL;
            tail = NULL;
        } else {
           head = head -> next;
        }
        
        // Free stuff
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

