{{
    Command Line Interface Memory Test Program
    for cache_interface.spin mdevices.
}}

{{
    Copyright (c) 2010-2011 by John Steven Denson (jazzed)
    TERMS OF USE: MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
}}

{{
=====================================================================
Clock and serial port baudrate
}}
con

  _CLKMODE      = XTAL1 + PLL16x
  _CLKFREQ      = 80_000_000

  BAUDRATE      = 115200

obj
{{
=====================================================================
objects used for testing
}}
  sx  : "FullDuplexSingleton"
  mem : "cache_interface"
  mdev : "eeprom_cache"

con
''
'' Cache parameters
''
'   CACHESIZE   = 4096
    CACHESIZE   = 8192
    config1     = 0     ' default config
    config2     = 0     ' default config

var
''
'' Cache interface variables must be in this order
''
    long command                ' mdevice driver command pointer
    long databuf                ' mdevice driver data or buffer pointer

    long cache[CACHESIZE>>2]    ' cache area
    long tagcount
    long linelen
    long lineshift

var
    long testsize               ' user's test size
    byte cmdbuf[128]            ' test interface command buffer

dat

{{
=====================================================================
main test
}}
pub main | n, a
    sx.start(31,30,0,BAUDRATE)  ' start serial interface
    waitcnt(clkfreq/2+cnt)      ' wait for user terminal to start
    sx.str(string($d,$a,$d,$a,"Cache Test.",$d,$a,$d,$a))

    linelen  := mem.start(mdev.image, @command, @cache, config1, config2)
    ifnot linelen 
        sx.str(string($d,$a,"Error driver returned tag count=0. Fix it.",$d,$a))
        repeat
    linelen++
    tagcount := CACHESIZE/linelen
    n := linelen
    repeat while n > 0
        n >>= 1
        lineshift <<= 1

{
    a := $00000
    repeat
        dumpMemBytes(a,$200)
        a += $200
        sx.rx
    memsize

    testsize := $8000           ' 32K brief test
    sx.str(string($d,$a,$d,$a,"Starting Cache Sanity Test in 2 seconds.",$d,$a,$d,$a))

    if sx.rxtime(2000) < 1
        testall
        sx.str(string($d,$a,$d,$a,"Brief RAM Test All Done. Now for full test, enter: t a",$d,$a,$d,$a))
'}

    testsize := $8000           ' 32KB
    help                        ' print help message at startup

    repeat                      ' print prompt and parse commands
        prompt(string("CACHE TEST"))
        result := \menu
        if result < 0
            sx.str(string($d,$a,"Bad command syntax. Enter ? for help."))

{{
=====================================================================
user interface code
}}
pub help
    lfcr
    sx.str(string($d,$a,"Cache TEST.",$d,$a,$d,$a," Cache size: "))
    sx.dec(CACHESIZE)
    sx.str(string(" Tag count: "))
    sx.dec(TAGCOUNT)
    sx.str(string(" Line size: "))
    sx.dec(LINELEN)
    lfcr
    sx.str(string(" ",34,"Test All",34," memory size: $"))
    sx.hex(testSize,8)
    sx.str(string(" or "))
'   sx.dec(testSize/(1024*1024))
'   sx.str(string("MB. "))
    sx.dec(testSize/1024)
    sx.str(string("KB. Change with ",34,"s n",34," command."))
    lfcr
    sx.str(string($d,$a,"User commands (numbers are expected to be hexadecimal):",$d,$a))
    lfcr
    sx.str(string("t n   - run test number n. t ? for help. t a runs all tests.",$d,$a))
    sx.str(string("T n   - loop run test number n. T a loop runs all tests.",$d,$a))
    lfcr
    sx.str(string("0     - run walking 0 address test.",$d,$a))
    sx.str(string("1     - run walking 1 address test.",$d,$a))
    sx.str(string("a n   - psrand address test from 0 to n.",$d,$a))
    sx.str(string("A n   - loop psrand address test from 0 to n.",$d,$a))
    sx.str(string("c     - dump cache",$d,$a))
    sx.str(string("d a n - dump n bytes of memory from address a",$d,$a))
    sx.str(string("f a n - Program Flash or EEPROM with cache contents. n = page size.",$d,$a))
    sx.str(string("i n   - incremental test from 0 to n.",$d,$a))
    sx.str(string("I n   - loop incremental test from 0 to n.",$d,$a))
    sx.str(string("p n   - psrand data test from 0 to n.",$d,$a))
    sx.str(string("P n   - loop psrand data test from 0 to n.",$d,$a))
    sx.str(string("r a   - read long from address a.",$d,$a))
    sx.str(string("R a   - loop read long from address a.",$d,$a))
    sx.str(string("s n   - change default test size to n.",$d,$a))
    sx.str(string("w a d - write long data d to address a.",$d,$a))
    sx.str(string("W a d - loop write long data d to address a.",$d,$a))
    sx.str(string("z     - zero cache.",$d,$a))
    sx.str(string("8     - fill cache with $88.",$d,$a))
    sx.str(string("5     - fill cache with $55.",$d,$a))
    sx.str(string("e     - fill cache with $ff.",$d,$a))
    sx.str(string("?     - show command help.",$d,$a,$d,$a))
    sx.str(string("Most commands are designed for RAM testing.",$d,$a))
    sx.str(string("Flash/EEPROM testing is limited to flash fill/dump.",$d,$a))

{{
=====================================================================
main menu code
}}
pub menu | s, ad, val, upper, dels
    gets(@cmdbuf)
    dels := string(" ",$9,$A,$d,$a)
    s := strtok(@cmdbuf,dels)

    upper := 0
    ifnot (byte[s][0] & $20)
        upper++

    case(byte[s][0])

        "?" : help

        "0" : walk0(testsize)

        "1" : walk1(testsize)

        "a","A" :
            s := strtok(0,dels)
            ad := atox(s)
            repeat
                psrandaddr(ad)
            while upper and not sx.rxready

        "c" :
            s := strtok(0,dels)
            if ishex(s) < 0
                dumpCache

        "d","D" :
            s := strtok(0,dels)
            ad := atox(s)
            s := strtok(0,dels)
            val := atox(s)
            repeat
                dumpMemBytes(ad,val)
            while upper and not sx.rxready

        "f","F" :
            sx.str(string($d,$a,"f "))
            s := strtok(0,dels)
            ad:= atox(s)
            sx.hex(ad,8)
            sx.out(" ")
            s := strtok(0,dels)
            val := atox(s)
            sx.hex(val,8)
            repeat  ' save repeat for sdcard test later
               flashCache(ad,val)
            while upper and not sx.rxready

        "i","I" :
            s := strtok(0,dels)
            ad := atox(s)
            repeat
                incrtest(ad)
            while upper and not sx.rxready

        "p","P" :
            s := strtok(0,dels)
            ad := atox(s)
            repeat
                psrand(ad)
            while upper and not sx.rxready

        "r","R" :
            s := strtok(0,dels)
            ad := atox(s)
            repeat
                val := mem.readLong(ad)
            while upper and not sx.rxready
            sx.str(string($d,$a,"R "))
            sx.hex(ad,8)
            sx.out(" ")
            sx.hex(val,8)

        "w","W" :
            sx.str(string($d,$a,"W "))
            s := strtok(0,dels)
            ad:= atox(s)
            sx.hex(ad,8)
            sx.out(" ")
            s := strtok(0,dels)
            val := atox(s)
            sx.hex(val,8)
            repeat
                mem.writeLong(ad,val)
            while upper and not sx.rxready

        "s" :
            s := strtok(0,dels)
            testsize := atox(s)

        "t","T" :
            s := strtok(0,dels)
            if(byte[s][0] == "?")
                testhelp
            else
                repeat
                    if (byte[s][0] == "a")
                        testall
                    else
                        testrun(atox(s))
                while upper and not sx.rxready

        "x","X" :

            sx.str(string($d,$a,"X "))
            s := strtok(0,dels)
            ad:= atox(s)
            sx.hex(ad,8)
            sx.out(" ")
            s := strtok(0,dels)
            val := atox(s)
            sx.hex(val,8)

            repeat
                mem.readLong(ad)
                mem.readLong(val)
            while upper and not sx.rxready

'{
            mem.writeLong(0,0)
            repeat
                mem.writeLong(0,$55555555)
                mem.writeLong(testsize,$AAAAAAAA)
            while upper and not sx.rxready
'}
'{
            repeat
                mem.readLong(0)
                mem.readLong(testsize)
            while upper and not sx.rxready
'}

        "z" :
            bytefill(@cache,0,CACHESIZE)
        "5" :
            bytefill(@cache,$55,CACHESIZE)
        "8" :
            bytefill(@cache,$88,CACHESIZE)
        "e" :
            bytefill(@cache,$FF,CACHESIZE)

pub prompt(s)
    sx.out($d)
    sx.out($a)
    sx.str(s)
    sx.str(string("> "))

pub lfcr
    sx.out($d)
    sx.out($a)

{{
=====================================================================
dumpcache - dumps contents of the cache
}}
pub dumpCache | n, m, mod

    sx.str(string($d,$a,$d,$a,"Cache Dump",$d,$a))
    sx.str(string("TAG TAGVAL  : Cache Line"))
{
    mod := 8
    repeat n from 0 to TAGCOUNT>>2-1
        lfcr
        sx.hex(n,3)
        sx.out(":")
        repeat m from 0 to LINELEN-1
            ifnot m // mod
                lfcr
            sx.out(" ")
            sx.hex(cache[n*LINELEN+m],8)
}            
'{
    mod := 8
    repeat n from 0 to constant(CACHESIZE>>2-1)
        ifnot n // mod
            lfcr
            sx.hex(n/(linelen>>2),3)
            sx.out(" ")
            sx.hex(n,8)
            sx.out(":")
        sx.out(" ")
        sx.hex(cache[n],8)
'}
    lfcr
{{
=====================================================================
flashCache - programs contents of the cache into flash memory
}}
pub flashCache(ad, plen) | n, j
    j := 0
    repeat n from 0 to constant(CACHESIZE>>0-1) step plen
        ifnot j++ // 8
            lfcr
        sx.hex(ad+n,8)
        sx.out(" ")
        mem.writeFlash(ad+n, @cache+n, plen)
'        waitcnt(_clkfreq/4+cnt)

    sx.str(string($d,$a,"Flash Cache Done",$d,$a))

{{
=====================================================================
flashErase - erase a flash sector
}}
pub flashErase(ad) | n
    mem.eraseFlashBlock(ad)

pub memsize | n,wval,rval,maxbit
{{
This test is used to detect the size of memory in use

Algorithm:
  write 0 to 0 address
  write 1<<address bit to 1<<address
  check non-zero at 0 address
  if non-zero at 0 address, report size found and exit
}}

{
    sx.dec(CACHESIZE)
    sx.out(" ")
    sx.dec(TAGCOUNT)
    sx.out(" ")
    sx.dec(LINELEN)
}
    mem.writelong(0,0)
    rval := mem.readlong(0)
    if rval <> 0
        sx.str(string($d,$a,$d,$a,"Memory Autosize ERROR! Expected 0 @ 0. Memory size indeterminant!"))
        sx.hex(rval,8)
        lfcr
        return 1

    sx.str(string($d,$a,$d,$a,"Detected MEMORY SIZE: "))

    repeat n from 2 to 22
        mem.writelong(1<<n,1<<n)
        rval := mem.readlong(0)

        if rval <> 0
'           sx.dec((1<<n)/(1024*1024))
'           sx.str(string("MB = "))
            sx.dec((1<<n)/1024)
            sx.str(string("KB = $"))
            sx.hex(1<<n,8)
            sx.str(string(" Bytes "))
            lfcr
            return 0

    return 0
{{
=====================================================================
test code
}}
pub testhelp
    sx.str(string($d,$a,"t ? for this help, t a runs all tests.",$d,$a,$d,$a))
    sx.str(string("Test List:",$d,$a))
    sx.str(string("t 0   - Walking 0's test.",$d,$a))
    sx.str(string("t 1   - Walking 1's test.",$d,$a))
    sx.str(string("t 2   - Incremental test.",$d,$a))
    sx.str(string("t 3   - Pseudo-Random Address test.",$d,$a))
    sx.str(string("t 4   - Pseudo-Random test.",$d,$a))

pub testall | n
    repeat n from 0 to 4
        if testrun(n)
            return

pub testrun(num)

    sx.str(string($d,$a,"Test "))
    sx.dec(num)
    lfcr

    result := 0
    case num
        0: result := walk0(testsize)
        1: result := walk1(testsize)
        2: result := incrtest(testSize)
        3: result := psrandaddr(testSize)
        4: result := psrand(testSize)

    return result

{{
=====================================================================
test code
}}
pub tmptest
    repeat
        mem.writeLong(0,0)
        mem.writeLong($100,0)

pri getMaxbit(len) | n
    n~
    len >>= 2
    repeat while len > 0
        n++
        len >>= 1
    sx.dec(n+1)
    sx.str(string(" address bits."))
    return n

pub walk1(len) | n,wval,rval,maxbit
{{
This test is used to detect stuck at 0 address bits.

Algorithm:
  write 0 to 0 address
  write 1<<address bit to 1<<address
  check non-zero at 0 address
}}
    sx.str(string($d,$a,"Address Walking 1's "))
{
    sx.dec(CACHESIZE)
    sx.out(" ")
    sx.dec(TAGCOUNT)
    sx.out(" ")
    sx.dec(LINELEN)
}
    maxbit := getMaxbit(len)

    mem.writelong(0,0)
    rval := mem.readlong(0)
    if rval <> 0
        sx.str(string($d,$a,$d,$a,"ERROR! Expected 0 @ 0 before march test. Got "))
        sx.hex(rval,8)
        lfcr
        return 1

    repeat n from 2 to maxbit
        'mem.writelong(0,0)
        mem.writelong(1<<n,1<<n)
        rval := mem.readlong(0)
        'rval := mem.readlong(1<<n)
        lfcr
        sx.hex(1<<n,8)
        sx.out(" ")
        sx.hex(rval,8)

        if rval <> 0
            sx.str(string($d,$a,$d,$a,"ERROR! Expected 0 @ 0 after write to address "))
            sx.hex(1<<n,8)
            sx.out(" ")
            sx.hex(rval,8)
            lfcr
            return 1

    return 0

pub walk0(len) | n,wval,rval,maxbit,maxaddr,naddr
{{
This test is used to detect unconnected stuck at 1 address bits.

Algorithm:
  write 0 to max address
  write 1<<address bit to !(1<<address)
  check non-zero at max address
}}
    sx.str(string($d,$a,"Address Walking 0's "))
{
    sx.dec(CACHESIZE)
    sx.out(" ")
    sx.dec(TAGCOUNT)
    sx.out(" ")
    sx.dec(LINELEN)
}
    maxbit := getMaxbit(len)
    maxaddr := ((1<<(maxbit+1))-1) & !3

    mem.writelong(maxaddr,0)
    rval := mem.readlong(maxaddr)
    if rval <> 0
        sx.str(string($d,$a,$d,$a,"ERROR! Expected 0 @ 0 before march test. Got "))
        sx.hex(rval,8)
        lfcr
        return 1

    repeat n from 2 to maxbit
        naddr := !(1<<n) & maxaddr
        mem.writelong(naddr,1<<n)
        rval := mem.readlong(maxaddr)
        lfcr
        sx.hex(naddr,8)
        sx.out(" ")
        sx.hex(rval,8)

        if rval <> 0
            sx.str(string($d,$a,$d,$a,"ERROR! Expected 0 @ "))
            sx.hex(maxaddr,8)
            sx.str(string(" after write to address "))
            sx.hex(!(1<<n) & maxaddr,8)
            sx.out(" ")
            sx.hex(rval,8)
            lfcr
            return 1

    return 0

var ' used for pseudo random tests
    long newseed

dat ' used for all data intensive tests
    mybuffer long $88442211 [1<<7]

pub incrtest(size) | n, a, base, d, j, end, seed, spit
{{
This test is used to detect shorted address bits.
It is also a memory persistence test.

Algorithm:
  write incremental pattern from 0 to size
  read and check incremental pattern from 0 to size
}}
    sx.str(string($d,$a,"Incremental Pattern Test "))

    end  := size-LINELEN
    spit := (end >> 6) - 1
    base := 0

    sx.dec(size/1000)
    sx.str(string(" KB",$d,$a))

    sx.str(string("----------------------------------------------------------------"))
    lfcr
    seed := 0
    repeat a from 0 to end step LINELEN
        repeat n from 0 to LINELEN-1 step 4
            'long[@mybuffer][n] := ++seed
            mem.writeLong(a+base+n, ++seed)
{
            ifnot n & 31
                lfcr
            sx.out(" ")
            sx.hex(long[@mybuffer][n],8)
'}
        'mem.writeBuffer(a+base,@mybuffer)
        ifnot a & spit
            sx.out("w")
    lfcr

{
    seed := 0
    repeat a from 0 to end step LINELEN
        repeat n from 0 to LINELEN-1
            mem.readBuffer(a+base,@mybuffer)

            ifnot n & 31
                lfcr
            sx.out(" ")
            sx.hex(long[@mybuffer][n],8)
    lfcr
'}

    seed := 0
    repeat a from 0 to end step LINELEN
        'mem.readBuffer(a+base,@mybuffer)
        ifnot a & spit
            sx.out("r")
        repeat n from 0 to LINELEN-1 step 4
            'd := long[@mybuffer][n]
            d := mem.readLong(a+base+n)
            if ++seed <> d
                sx.str(string($d,$a,$d,$a,"ERROR at $"))
                sx.hex(a,8)
                sx.str(string(" Expected $"))
                sx.hex(seed,8)
                sx.str(string(" Received $"))
                sx.hex(d,8)
                sx.str(string($d,$a,"Address  $"))
                sx.hex(a,8)
                'sx.str(string(" Buffer at $"))
                'sx.hex(mem.dataptr,8)
                sx.out(" ")
                sx.dec(a/1000)
                sx.str(string("K Page"))
                dumpCache
                return 1

    sx.str(string($d,$a,"Test Complete!",$d,$a))
    return 0

pub psrand(size) | n, a, base, d, j, end, seed, spit
{{
This test is used as a pseudo random data test of memory.
It is also a memory persistence test.

Algorithm:
  write pseudo random data from 0 to size
  read and check data from 0 to size
}}
    sx.str(string($d,$a,"Pseudo-Random Pattern Test "))

    end  := size-LINELEN
    spit := (end >> 6) - 1
    base := 0

    sx.dec(end/1000)
    sx.str(string(" KB",$d,$a))

    sx.str(string("----------------------------------------------------------------"))
    lfcr
    seed := newseed
    repeat a from 0 to end step LINELEN
        repeat n from 0 to LINELEN-1 step 4
            'long[@mybuffer][n] := ?seed
            mem.writeLong(a+base+n, ?seed)
{
            ifnot n & 31
                lfcr
            sx.out(" ")
            sx.hex(long[@mybuffer][n],8)
'}
        'mem.writeBuffer(a+base,@mybuffer)
        ifnot a & spit
            sx.out("w")
    lfcr

    seed := newseed
    repeat a from 0 to end step LINELEN
        'mem.readBuffer(a+base,@mybuffer)
        ifnot a & spit
            sx.out("r")
        repeat n from 0 to LINELEN-1 step 4
            'd := long[@mybuffer][n]
            d := mem.readLong(a+base+n)
            if ?seed <> d
                sx.str(string($d,$a,$d,$a,"ERROR at $"))
                sx.hex(a,8)
                sx.str(string(" Expected $"))
                sx.hex(seed,8)
                sx.str(string(" Received $"))
                sx.hex(d,8)
                sx.str(string($d,$a,"Address  $"))
                sx.hex(a,8)
                'sx.str(string(" Buffer at $"))
                'sx.hex(mem.dataptr,8)
                sx.out(" ")
                sx.dec(a/1000)
                sx.str(string("K Page"))
                'dumpCache
                return 1

    newseed := seed ' use different seed every test pass

    sx.str(string($d,$a,"Test Complete!",$d,$a))
    return 0


pub psrandaddr(size) | n, a, mask, d, j, end, seed, spit, madr, key
{{
This test is used as a brief pseudo random data test of memory.
It is also a memory persistence test.

Algorithm:
  write pseudo random data to pseudo random addresses in range 0 to size
  read and check data from from addresses in range 0 to size
}}
    sx.str(string($d,$a,"Random Addr Pattern Test "))

    end  := size-LINELEN
    spit := (end >> 6) - 1
    mask := (size-1) >> 1

    sx.dec(end/1000)
    sx.str(string(" KB",$d,$a))

    sx.str(string("----------------------------------------------------------------"))
    lfcr

    repeat a from 0 to end step LINELEN
        mem.writeLong(a,0)
        ifnot a & spit
            sx.out("c")
    lfcr

    newseed := 0
    seed := newseed | 1
    madr := newseed
    repeat a from 0 to end step LINELEN
        madr?
        madr &= mask & !(LINELEN-1)

        ifnot a & spit
            sx.out("w")

        if mem.readLong(a+madr)    ' already have a key?
            next

        'long[@mybuffer][0] := seed++
        repeat n from 1 to LINELEN-1 step 4
            'long[@mybuffer][n] := seed++
            'mem.writeBuffer(a+madr,@mybuffer)
            mem.writeLong(a+madr+n,seed++)
{
            ifnot n & 31
                lfcr
                sx.hex(madr,8)
                sx.out(":")
            sx.out(" ")
            sx.hex(long[@mybuffer][n],8)
'}
    lfcr

    seed := newseed | 1
    madr := newseed
    repeat a from 0 to end step LINELEN
        madr?
        madr &= mask & !(LINELEN-1)
        'mem.readBuffer(a+madr,@mybuffer)
        ifnot a & spit
            sx.out("r")
        if seed <> mem.readLong(a+madr)
            next
        repeat n from 0 to LINELEN-1 step 4
            'd := long[@mybuffer][n]
            d := mem.readLong(a+madr+n)
            if seed++ <> d
                sx.str(string($d,$a,$d,$a,"ERROR at $"))
                sx.hex(a+madr,8)
                sx.str(string(" Expected $"))
                sx.hex(seed,8)
                sx.str(string(" Received $"))
                sx.hex(d,8)
                sx.str(string($d,$a,"Address  $"))
                sx.hex(a+madr,8)
                sx.str(string($d,$a,"Tag "))
                sx.hex(((a+madr)>>LINESHIFT) & (TAGCOUNT-1),3)
                sx.str(string(" TagValue $"))
                sx.hex((a+madr)>>LINESHIFT,8)
                'sx.str(string(" Buffer at $"))
                'sx.hex(mem.dataptr,8)
                sx.out(" ")
                sx.dec((a+madr)/1000)
                sx.str(string("K Page"))
                return 1

    sx.str(string($d,$a,"Test Complete!",$d,$a))
    newseed := madr
    return 0


{{
=====================================================================
memory dump utility
addr - start address to dump
size - number of longs to dump
}}
pub dumpMem (addr, size) | n, a, base, d, j, end, seed, spit

    end  := addr+size-1
    spit := 32
    base := 0
{
    repeat a from addr to end step 4
        mem.readlong(a)
        lfcr
}

'{
    lfcr
    repeat a from addr to end step 4
        ifnot a & (spit-1)
            lfcr
            sx.hex(a,8)
            sx.out(":")
        sx.out(" ")
        sx.hex(mem.readlong(a),8)
    lfcr
'}

{{
=====================================================================
memory dump bytes utility
addr - start address to dump
size - number of longs to dump
}}
pub dumpMemBytes (addr, size) | n, a, base, d, j, end, seed, spit

    end  := addr+size-1
    spit := 16
    base := 0

    lfcr
    repeat a from addr to end
        ifnot a & (spit-1)
            lfcr
            sx.hex(a,8)
            sx.out(":")
        sx.out(" ")
        sx.hex(mem.readbyte(a),2)
    lfcr
{{
=====================================================================
string utility code
}}
pub atoi(s) | len,ch,n
'' if input s represents a valid positive decimal number
''   returns string to number
'' else
''  returns -1
''
    len := strsize(s)-1
    repeat n from 0 to len
        ch := byte[s][n]
        result *= 16
        case ch
            "0".."9":   result += ch-"0"
            other:      abort(-1)

pub atox(s) | len,ch,n
'' if input s represents a valid positive hexadecimal number
''   returns string to number
'' else
''
    len := strsize(s)-1
    repeat n from 0 to len
        ch := byte[s][n]
        result *= 16
        case ch
            "0".."9":   result += ch-"0"
            "a".."f":   result += ch-"a"+10
            "A".."F":   result += ch-"A"+10
            other:      abort(-1)

pub ishex(s) | n,len,ch
    len := strsize(s)-1
    repeat n from 0 to len
        ch := byte[s][n]
        result *= 16
        case ch
            "0".."9":   result += ch-"0"
            "a".."f":   result += ch-"a"+10
            "A".."F":   result += ch-"A"+10
            other:      return -1

pub gets(bp) | ch
    repeat
        ch := sx.rx
        byte[bp++] := ch
        if(ch <> $d)
            sx.out(ch)
    until ch == $A or ch == $D
    byte[bp++] := 0

pub strtok(str,ds) | j,k,count
'' strtok finds one of delmiter set ds in str and returns the next token as a string pointer
'' first call str should contain string pointer. subsequent calls str is 0
'' this method is not reentrant
'' returns 0 if not found
  return strrtok(str,ds,strtokp)

var long strtokp    ' used for non-reentrant strtok

pub strrtok(str,ds,bufp) | j,k,pos,plen,dlen
'' strrtok finds one of delmiter set ds in str and returns the next token as a string pointer
'' first call str should contain string pointer. subsequent calls str is 0
'' this method is reentrant
'' returns 0 if not found
  if str                        ' first call sets pointer
    long[bufp] := str
  ifnot long[bufp]              ' when end of string, parser nulls strtokp
    return 0

  plen := strsize(long[bufp])+0

  dlen := strsize(ds)
  repeat k from 0 to plen
    pos := dlen
    repeat j from 0 to dlen     ' find not delimiter beginning
      if byte[long[bufp]+k] <> byte[ds+j]
        if pos
          pos--
      else
        quit
    ifnot pos                   ' if no delimeters found, set begin
      long[bufp] += k
      quit
  dlen := strsize(ds)
  pos  := 0
  repeat k from 0 to plen
    repeat j from 0 to dlen     ' find delimiter end
      if byte[long[bufp]+k] == byte[ds+j]
        pos := k                ' got at least one delimeter, set end
        quit
    if pos
      quit

  if k > strsize(long[bufp])    ' if k > size, we're done
    return 0

  str := long[bufp]
  long[bufp] += k               ' increment pointer past delimiter
  ifnot byte[long[bufp]]        ' if null, we're done
    long[bufp]~
  else
    byte[long[bufp]] := 0       ' else, null the delimiter and
    long[bufp]++                ' go to the next part of the string

  return str

