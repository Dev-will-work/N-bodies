#ifndef N_BODIES
#define N_BODIES

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h> 

#include <windows.h>

typedef void(*task)(void* rank);

typedef struct thread_pool_task {
    task func;
    void *arg;
    struct thread_pool_task *next;
} thread_pool_task;

typedef struct {
  thread_pool_task *work_first;
  thread_pool_task *work_last;
  pthread_mutex_t work_mutex;
  pthread_cond_t work_cond;
  pthread_cond_t working_cond;
  int working_cnt;
  int thread_cnt;
  int stop;
  pthread_t* threads;
} thread_pool;

thread_pool* thread_pool_create(int size);
void thread_pool_destroy(thread_pool* tp);
void thread_pool_request(thread_pool* tp, task t, void* rank);
void thread_pool_wait(thread_pool* tp);



typedef struct {
    double x, y;
} vector;

#define DT 0.05
#define LOG_SOURCE_COUNT 3

#endif
