CON
'   _clkmode = RCFAST

   _clkmode = xtal1 + pll16x
   _xinfreq = 6_000_000

  LED = 26

PUB main : mask
  mask := 1 << LED
  OUTA := 0
  DIRA := mask
  repeat
    OUTA ^= mask
    waitcnt(CNT + CLKFREQ / 2)

