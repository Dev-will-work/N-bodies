#include "header.h"
#include "thread_pool.h"

threadpool tp;
int thread_count;
int bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;
pthread_rwlock_t rwlock;


vector addVectors(vector a, vector b) {
    vector c = {a.x + b.x, a.y + b.y};

    return c;
}

vector scaleVector(double b, vector a) {
    vector c = {b * a.x, b * a.y};

    return c;
}

vector subtractVectors(vector a, vector b) {
    vector c = {a.x - b.x, a.y - b.y};

    return c;
}

double mod(vector a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

void initiateSystem(char *fileName) {
    int i;
    FILE *fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

    for (i = 0; i < bodies; i++) {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
    }

    fclose(fp);
}

void resolveCollisions(void* rank) {
    long my_rank = (long)rank;
    double my_n = (double)(bodies-1) / thread_count;
    //both ceil if greedy, else both floor
    long m_first_i = ceil(my_n * my_rank);
    long m_last_i = ceil(my_n * (my_rank+1));
    int i, j;

    for (i = m_first_i; i < m_last_i; i++) {
        for (j = i + 1; j < bodies; j++) {
            if (positions[i].x == positions[j].x && positions[i].y == positions[j].y) {
                // on 3 points works good without wrlock, but maybe because of rare collisions
                //pthread_rwlock_wrlock(rwlock);
                vector temp = velocities[i];
                velocities[i] = velocities[j];
                velocities[j] = temp;
                //pthread_rwlock_unlock(rwlock);
            }
        }
    }
}

void computeAccelerations(void* rank) {
    long my_rank = (long)rank;
    double my_n = (double)bodies / thread_count;
    //both ceil if greedy, else both floor
    long m_first_i = ceil(my_n * my_rank);
    long m_last_i = ceil(my_n * (my_rank+1));
    int i, j;

    for (i = m_first_i; i < m_last_i; i++) {
        accelerations[i].x = 0;
        accelerations[i].y = 0;
        for (j = 0; j < bodies; j++) {
            if (i != j) {
                accelerations[i] = addVectors(accelerations[i], scaleVector(GravConstant * masses[j] / pow(mod(subtractVectors(positions[i], positions[j])), 3), subtractVectors(positions[j], positions[i])));
            }
        }
    }
}

void computeVelocities(void* rank) {
    long my_rank = (long)rank;
    double my_n = (double)bodies / thread_count;
    //both ceil if greedy, else both floor
    long m_first_i = ceil(my_n * my_rank);
    long m_last_i = ceil(my_n * (my_rank+1));
    int i;

    for (i = m_first_i; i < m_last_i; i++) {
        velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
    }
}

void computePositions(void* rank) {
    long my_rank = (long)rank;
    double my_n = (double)bodies / thread_count;
    //both ceil if greedy, else both floor
    long m_first_i = ceil(my_n * my_rank);
    long m_last_i = ceil(my_n * (my_rank+1));
    int i;

    for (i = m_first_i; i < m_last_i; i++) {
        positions[i] = addVectors(positions[i], scaleVector(DT,velocities[i]));
    }
}

void simulate() {
    // needs position, changes acceleration
    for (int i = 0; i < thread_count; i++) {
      thpool_add_work(tp, computeAccelerations, (void*)i);
    }
    //any one of 2 wait, before or after collisions
    thpool_wait(tp);
    // needs velocity + position, changes velocity
    for (int i = 0; i < thread_count; i++) {
      thpool_add_work(tp, resolveCollisions, (void*)i);
    }
    //thpool_wait(tp);
    // needs velocity, changes position
    for (int i = 0; i < thread_count; i++) {
      thpool_add_work(tp, computePositions, (void*)i);
    }
    thpool_wait(tp);
    // needs acceleration, changes velocity
    for (int i = 0; i < thread_count; i++) {
      thpool_add_work(tp, computeVelocities, (void*)i);
    }
}

int main(int argC, char *argV[]) {
    int i, j;

    thread_count = atoi(argV[2]);

    pthread_rwlock_init(&rwlock, NULL);

    tp = thpool_init(thread_count);
    if (argC < 2) {
        printf("Usage : %s <file name containing system configuration data>", argV[0]);
    } else {
        initiateSystem(argV[1]);
        printf("Body   :     x              y           vx              vy   ");
        for (i = 0; i < timeSteps; i++) {
            printf("\nCycle %d\n", i + 1);
            simulate();
            for (j = 0; j < bodies; j++) {
                printf("Body %d : %lf\t%lf\t%lf\t%lf\n", j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
            }
        }
    }

    thpool_destroy(tp);

    pthread_rwlock_destroy(&rwlock);
    return 0;
}