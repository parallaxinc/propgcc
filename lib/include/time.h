#ifndef _TIME_H
#define _TIME_H

typedef unsigned int clock_t;
typedef unsigned int time_t;

extern clock_t _clkfreq;

#define CLOCKS_PER_SEC _clkfreq

clock_t clock(void);
time_t  time(time_t *);

#endif
