//#define DEBUG
#include <propeller.h>
#include <sys/thread.h>
#include <stdlib.h>
#include <compiler.h>

#ifdef DEBUG
#include <stdio.h>
#endif

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
    int numthreads;
    int started;
    volatile int hold;
    volatile unsigned barrier_arrived;
    volatile unsigned section_num;
    volatile unsigned total_sections;
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

static int
startthreads(int max)
{
    int i;
    int cog;
    char *stack;
    size_t maxsize;

    team.threadmap[0] = __builtin_propeller_cogid();
    team.numthreads = 1;

    maxsize = STACKSIZE + sizeof(_thread_state_t);
    stack = malloc(maxsize);
    for (i = 1; i < max; i++) {
        if (!stack) break;
        cog = _start_cog_thread(stack + maxsize, workerthread, (void *)&team.work[i], (_thread_state_t *)stack);
        if (cog < 0) break;
        team.threadmap[i] = cog;
        team.stacks[i] = stack;
        stack = malloc(maxsize);
        team.numthreads++;
    }
    free(stack);
    team.started = 1;
#ifdef DEBUG
    printf("team has %d cogs\n", team.numthreads);
#endif
    return team.numthreads;
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
    int cog = __builtin_propeller_cogid();
    int i;
    for (i = 0; i < MAX_THREADS; i++) {
        if (team.threadmap[i] == cog)
            return i;
    }
    return -1;
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
GOMP_parallel_sections_start( void (*fn)(void *), void *data, unsigned num_threads, unsigned section_count)
{
    int i;
    int run = 0;

    if (team.started)
      return;

    if (num_threads == 0 || num_threads > MAX_THREADS)
      num_threads = MAX_THREADS;

#ifdef DEBUG
    printf("parallel_start: requested %d threads ", num_threads);
#endif
    num_threads = startthreads(num_threads);
#ifdef DEBUG
    printf("started %d threads\n", num_threads);
#endif
    team.total_sections = section_count;
    team.section_num = 0;
    for (i = 1; i < num_threads; i++) {
        team.work[i].done = 0;
        team.work[i].arg = data;
        team.work[i].fn = fn;
        run++;
    }
}

void
GOMP_parallel_start( void (*fn)(void *), void *data, unsigned num_threads)
{
  GOMP_parallel_sections_start(fn, data, num_threads, 0 );
}

void
GOMP_parallel_end()
{
    int i;

    /* make sure other cogs have finished */
    wait_others();

    /* shut down other cogs and free their memory */
    for (i = 1; i < team.numthreads; i++) {
      __builtin_propeller_cogstop(team.threadmap[i]);
      free(team.stacks[i]);
    }
    team.started = 0;
}

/* this routine is called when the thread completes processing of the
 * section currently assigned to it
 * returns the 1 based section number to perform, or 0 if all work is
 * already assigned
 */
unsigned
GOMP_sections_next (void)
{
  unsigned result = __sync_add_and_fetch(&team.section_num, 1);

  if (result > team.total_sections) {
    return 0;
  }
  return result;
}

unsigned
GOMP_sections_start (unsigned num_sections)
{
  __sync_val_compare_and_swap(&team.total_sections, 0, num_sections);
  return GOMP_sections_next ();
}

void
GOMP_sections_end_nowait (void)
{
  team.total_sections = 0;
  team.section_num = 0;
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

/* wait for all threads to reach here */
void
GOMP_barrier()
{
  int who;


  who = omp_get_thread_num();
  __addlock(&team.hold, 1);
  __addlock(&team.barrier_arrived, 1);

  if (who == 0) {
    while (team.barrier_arrived != team.numthreads)
      ;
    team.hold = team.barrier_arrived = 0;
  } else {
    while (team.hold)
      ;
  }
}
