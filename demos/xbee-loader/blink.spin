CON
'   _clkmode = RCFAST

   _clkmode = xtal1 + pll16x
   _xinfreq = 5_000_000

  LED1 = 26
  LED2 = 27

PUB main : mask
  mask := (1 << LED1) | (1 << LED2)
  DIRA := mask
  OUTA := (1 << LED1)
  repeat
    OUTA ^= mask
    waitcnt(CNT + CLKFREQ / 2)

