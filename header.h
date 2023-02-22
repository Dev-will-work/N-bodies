#ifndef N_BODIES
#define N_BODIES

#define _CRT_SECURE_NO_WARNINGS
#define HAVE_STRUCT_TIMESPEC
#define DT 0.05

#include <stdio.h> // printf
#include <math.h> // ceil/floor
#include <time.h>  // clock
#include <pthread.h> // pthread functions

typedef struct {
    double x, y;
} vector;

#endif
