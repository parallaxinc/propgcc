//#define DEBUG
#include <propeller.h>
#include <sys/thread.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

//#define ONLYONE

#define MAX_THREADS 8
//#define MAX_THREADS 4
#define STACKSIZE 512

struct workstruct {
    void (*fn)(void *);
    void *arg;
    int done;
    int cogid;
};

static struct team {
    int threadmap[MAX_THREADS];
    volatile struct workstruct work[MAX_THREADS];
    char *stacks[MAX_THREADS];
    int numcogs;
    int numthreads;
    int started;
#ifdef ONLYONE
    int curcog;
#endif
} team;

static void
workerthread(void *arg)
{
    volatile struct workstruct *workptr = (void *)arg;
    void (*fn)(void *);

    workptr->cogid = __builtin_propeller_cogid();
    for(;;) {
        workptr->done = 1;
        while (0 == (fn = workptr->fn))
            ;
        fn(workptr->arg);
    }
}

static void
startthreads(int max)
{
    int i;
    int cog;
    char *stack;
    size_t maxsize;

    team.threadmap[0] = __builtin_propeller_cogid();
    team.numcogs = 1;

    maxsize = STACKSIZE + sizeof(_thread_state_t);
    stack = malloc(maxsize);
    for (i = 1; i < max; i++) {
        if (!stack) break;
        cog = _start_cog_thread(stack + maxsize, workerthread, (void *)&team.work[i], (_thread_state_t *)stack);
        if (cog < 0) break;
        team.threadmap[i] = cog;
        team.stacks[i] = stack;
        stack = malloc(maxsize);
        team.numcogs++;
    }
    free(stack);
    team.started = 1;
#ifdef DEBUG
    printf("team has %d cogs\n", team.numcogs);
#endif
}

int
omp_get_num_threads()
{
    if (!team.started)
        return 1;
    return team.numthreads;
}

int
omp_get_thread_num()
{
#ifdef ONLYONE
    return team.curcog;
#else
    int cog = __builtin_propeller_cogid();
    int i;
    for (i = 0; i < MAX_THREADS; i++) {
        if (team.threadmap[i] == cog)
            return i;
    }
    return -1;
#endif
}

static void
wait_others()
{
    int i;
    for (i = 1; i < team.numthreads; i++) {
#ifdef DEBUG
        printf("waiting for thread %d (cog %d)\n", i, team.threadmap[i]);
#endif
        while (!team.work[i].done)
            ;
    }
}

void
GOMP_parallel_start( void (*fn)(void *), void *data, unsigned num_threads)
{
    int i;
    int run = 0;

    if (team.started)
      return;

    if (num_threads == 0 || num_threads > MAX_THREADS)
      num_threads = MAX_THREADS;

    startthreads(num_threads);

#ifdef DEBUG
    printf("parallel_start: requested %d threads\n", num_threads);
#endif
    if (num_threads == 0)
        num_threads = team.numcogs;
    if (num_threads > team.numcogs)
    {
        num_threads = team.numcogs;
#ifdef DEBUG
    printf("parallel_start: using %d threads\n", num_threads);
#endif
    }
    team.numthreads = num_threads;
    for (i = 1; i < num_threads; i++) {
        team.work[i].done = 0;
        team.work[i].arg = data;
        team.work[i].fn = fn;
        run++;
    }
#ifdef ONLYONE
    team.curcog = num_threads - 1;
    for(;;) {
        fn(data);
        team.work[team.curcog].done = 1;
        if (team.curcog == 1) break;
        --team.curcog;
    }
    team.curcog = 0;
#endif

}

void
GOMP_parallel_end()
{
    wait_others();
    team.started = 0;
}

static _atomic_t atomic;
static _atomic_t critical;

void
GOMP_atomic_start (void)
{
    __lock(&atomic);
}

void
GOMP_atomic_end (void)
{
    __unlock(&atomic);
}

void
GOMP_critical_start (void)
{
    __lock(&critical);
}

void
GOMP_critical_end (void)
{
    __unlock(&critical);
}
