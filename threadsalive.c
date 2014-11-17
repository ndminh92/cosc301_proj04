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

/* init a context */
static void context_init(t_list_t *newuc) {
    // Initialize context
    getcontext(&newuc->context);
    newuc -> context.uc_stack.ss_sp = newstack;
    newuc -> context.uc_stack.ss_size = STACKSIZE;
    newuc -> context.uc_link = &main_thread;    // Go back to main thread. Always
}

/* insert a context to a queue */
static void t_list_insert(t_list_t *context, t_list_t **q_head) {
    if (*q_head == NULL) { // Queue is empty
        *q_head = context;
    } else {
        t_list_tail(*q_head) -> next = context;
    }        
}

/* add a context to the main queue */
static void t_list_add(t_list_t *context) {
    context_insert(context, &head);
}

/* extract a context and shove it into another list */
static void t_list_extract(t_list_t *context, t_list_t **q_head) {
    if (!t_list_contains(context, *q_head)) { // Queue is empty
        return NULL;
    } else if (*q_head == context) {
        *q_head = *q_head -> next;
    } else {
        t_list_t *temp = *q_head;
        while (temp -> next != context) {
            temp = temp -> next;
        }
        temp -> next = temp -> next -> next;
    }
    return context;
}

/* returns the last node on the queue */
static t_list_t* t_list_tail(t_list_t *q_head) {
    while (q_head -> next != NULL) {
        q_head = q_head -> next;
    }
    return q_head;
}

/* insert a context to a queue */
static bool t_list_contains(t_list_t *context, t_list_t *q_head) {
    if (q_head == NULL) { // Queue is empty
        return false;
    } else {
        while (q_head -> next != NULL && q_head -> next != context) {
            q_head = q_head -> next;
        }
        return context == q_head -> next;
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

    context_init(newuc);
    makecontext(&newuc -> context, (void (*)(void))func, 1, arg);
    
    context_add(newuc);
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
        
        // Free allocated memory for the node
        free(temp -> context.uc_stack.ss_sp); // Free stack
        free(temp); // Free node itself
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
    sem -> head = NULL;
}

void ta_sem_destroy(tasem_t *sem) {
    /* Hello. My name is Inigo Montoya. You killed my father. Prepare to die. */
}

void ta_sem_post(tasem_t *sem) {
    (sem -> value)++;
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

