/*
 * 
 */

#ifndef __THREADSALIVE_H__
#define __THREADSALIVE_H__
#include <ucontext.h>
#include <signal.h>
#include <stdbool.h>
/* ***************************
        type definitions
   *************************** */

typedef struct t_list t_list_t;
typedef struct talock talock_t;

struct t_list {
    ucontext_t context;
    int blocked;
    t_list_t *prev;
    t_list_t *next;
    talock_t *held_lock;
};

typedef struct {
    int value;
    t_list_t *head;
} tasem_t;

struct talock {
    tasem_t sem;
};

typedef struct {
    t_list_t *head;
    t_list_t *tail;
} tacond_t;

 
/* ***************************
       helper functions
   *************************** */

static void context_init(t_list_t *, unsigned char *);
static void t_list_insert(t_list_t *, t_list_t **);
static void t_list_main_insert(t_list_t *);
static t_list_t *t_list_extract(t_list_t *, t_list_t **);
static t_list_t* t_list_tail(t_list_t *);
static bool t_list_contains(t_list_t *, t_list_t *);

/* ***************************
       stage 1 functions
   *************************** */

void ta_libinit(void);
void ta_create(void (*)(void *), void *);
void ta_yield(void);
int ta_waitall(void);

/* ***************************
       stage 2 functions
   *************************** */

void ta_sem_init(tasem_t *, int);
void ta_sem_destroy(tasem_t *);
void ta_sem_post(tasem_t *);
void ta_sem_wait(tasem_t *);
void ta_lock_init(talock_t *);
void ta_lock_destroy(talock_t *);
void ta_lock(talock_t *);
void ta_unlock(talock_t *);

/* ***************************
       stage 3 functions
   *************************** */

void ta_cond_init(tacond_t *);
void ta_cond_destroy(tacond_t *);
void ta_wait(talock_t *, tacond_t *);
void ta_signal(tacond_t *);

#endif /* __THREADSALIVE_H__ */
