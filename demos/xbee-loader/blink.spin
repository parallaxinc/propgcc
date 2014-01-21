CON
  LED1 = 26
  LED2 = 27

PUB main : mask
  mask := (1 << LED1) | (1 << LED2)
  DIRA := mask
  OUTA := (1 << LED1)
  repeat
    OUTA ^= mask
    waitcnt(CNT + CLKFREQ / 2)

