#include "header.h"
static thread_pool_task* thread_pool_task_create(task t, void* rank) {
    thread_pool_task *task = (thread_pool_task*)malloc(sizeof(thread_pool_task));
    if (task) {
        task->func = t;
        task->arg  = rank;
        task->next = NULL;
    }
    return task;
}

static void thread_pool_task_destroy(thread_pool_task *work) {
    if (work == NULL) {
       return;
    }
    free(work);
}

static thread_pool_task *thread_pool_task_get(thread_pool* tp) {
    thread_pool_task *task;

    task = tp->work_first;
    if (task == NULL) {
        return NULL;
    }

    if (task->next == NULL) {
        tp->work_first = NULL;
        tp->work_last  = NULL;
    } else {
        tp->work_first = task->next;
    }

    return task;
}

static void *thread_pool_worker(void *arg) {
    thread_pool* tp = (thread_pool*)arg;
    thread_pool_task *work;

    while (1) {
        pthread_mutex_lock(&(tp->work_mutex));

        while (tp->work_first == NULL && !tp->stop) {
            pthread_cond_wait(&(tp->work_cond), &(tp->work_mutex));
        }

        if (tp->stop)
            break;

        work = thread_pool_task_get(tp);
        tp->working_cnt++;
        pthread_mutex_unlock(&(tp->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            thread_pool_task_destroy(work);
        }

        pthread_mutex_lock(&(tp->work_mutex));
        tp->working_cnt--;
        if (!tp->stop && tp->working_cnt == 0 && tp->work_first == NULL)
            pthread_cond_signal(&(tp->working_cond));
        pthread_mutex_unlock(&(tp->work_mutex));
    }

    tp->thread_cnt--;
    pthread_cond_signal(&(tp->working_cond));
    pthread_mutex_unlock(&(tp->work_mutex));
    return NULL;
}

// creates workers
thread_pool* thread_pool_create(int size) {
    thread_pool *tp;
    size_t i;

    if (size == 0) {
        size = 2;
    }

    tp = calloc(1, sizeof(*tp));
    if (tp) {
        tp->threads = (pthread_t*)malloc(size * sizeof(pthread_t));
        tp->thread_cnt = size;

        pthread_mutex_init(&(tp->work_mutex), NULL);
        pthread_cond_init(&(tp->work_cond), NULL);
        pthread_cond_init(&(tp->working_cond), NULL);

        tp->work_first = NULL;
        tp->work_last  = NULL;

        if (tp->threads) {
            for (i = 0; i < size; i++) {
                pthread_create(&tp->threads[i], NULL, thread_pool_worker, tp);
                pthread_detach(tp->threads[i]);
            }
        }
    }

    return tp;
}

void thread_pool_destroy(thread_pool* tp) {
    thread_pool_task *task;
    thread_pool_task *task2;

    if (tp == NULL)
        return;

    pthread_mutex_lock(&(tp->work_mutex));
    task = tp->work_first;
    while (task != NULL) {
        task2 = task->next;
        thread_pool_task_destroy(task);
        task = task2;
    }
    tp->stop = 1;
    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    thread_pool_wait(tp);

    pthread_mutex_destroy(&(tp->work_mutex));
    pthread_cond_destroy(&(tp->work_cond));
    pthread_cond_destroy(&(tp->working_cond));

    free(tp->threads);
    free(tp);
}

void thread_pool_request(thread_pool* tp, task t, void* rank) {
    if (tp == NULL)
        return 0;

    thread_pool_task *task = thread_pool_task_create(t, rank);
    if (task == NULL)
        return 0;

    pthread_mutex_lock(&(tp->work_mutex));
    if (tp->work_first == NULL) {
        tp->work_first = task;
        tp->work_last  = tp->work_first;
    } else {
        tp->work_last->next = task;
        tp->work_last       = task;
    }

    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    return 1;
}

void thread_pool_wait(thread_pool* tp) {
    if (tp == NULL)
        return;

    pthread_mutex_lock(&(tp->work_mutex));
    while (1) {
        if ((!tp->stop && tp->working_cnt != 0) || (tp->stop && tp->thread_cnt != 0)) {
            pthread_cond_wait(&(tp->working_cond), &(tp->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->work_mutex));
}