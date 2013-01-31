#include <time.h>
#include <stdio.h>
#include <sys/rtc.h>
#include <unistd.h>
#include <propeller.h>

void
pause(int secs)
{
  while (secs > 0) {
    waitcnt(_CNT+_clkfreq);
    --secs;
  }
}

void
printtime(void)
{
  time_t now;

  now = time(NULL);
  printf("%s", asctime(localtime(&now)));
}

int
main()
{
  _rtc_start_timekeeping_cog();
  printtime();
  pause(1);
  printtime();
  pause(2);
  printtime();
  pause(60);
  printtime();
  return 0;
}
