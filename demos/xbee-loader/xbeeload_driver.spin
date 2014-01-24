''*************************************************
''* Xbee Load Driver                              *
''*  Copyright (c) 2014 David Betz                *
''* Portions adapted from code in sdspiFemto.spin *
''*  Copyright (c) 2009 Michael Green             *
''* Released under the MIT License                *
''*************************************************

{
  init structure:
        long mailbox        '0: frame driver mailbox
        long ldbuf          '1: buffer containing initial data to load
        long ldcount        '2: count of bytes in ldbuf
        long ldaddr         '3: hub load address
        long ldtotal        '4: hub load byte count
        long response       '5: response
        long rcount         '6: count of bytes in response
}

CON

  ' status codes
  #0
  STATUS_IDLE   ' must be zero
  STATUS_BUSY
  
  ' IPV4 RX frame
  IPV4RX = $b0
  IPV4RX_DATA_OFFSET = 11
  
  ' program preamble
  PREAMBLE0 = $0000
  PREAMBLE1 = $0004
  PREAMBLE2 = $0008
  PREAMBLE3 = $000c

  ' spin program preamble offsets
  clkfreqVal = $0000                                ' Current CLKFREQ value stored here
  clksetVal  = $0004                                ' Current CLKSET value stored here

DAT

'
'
' Entry
'
entry                   mov     t1, par             'get init structure address

                        rdlong  t2, t1              'get the mailbox address
                        mov     rx_frame_ptr, t2
                        add     t2, #4
                        mov     rx_length_ptr, t2
                        add     t2, #4
                        mov     rx_status_ptr, t2
                        add     t2, #4
                        mov     tx_frame_ptr, t2
                        add     t2, #4
                        mov     tx_length_ptr, t2
                        add     t2, #4
                        mov     tx_status_ptr, t2
                        add     t1, #4
                        rdlong  src, t1             'get the initial buffer pointer
                        add     t1, #4
                        rdlong  count, t1           'get the initial buffer count
                        add     t1, #4
                        rdlong  ld_ptr, t1          'get the load address
                        add     t1, #4
                        rdlong  ld_remaining, t1    'get the total number of bytes to load
                        add     t1, #4
                        rdlong  response, t1        'get the response pointer
                        add     t1, #4
                        rdlong  rcount, t1          'get number of bytes in the response
                        
                        ' from sdspiFemto.spin
                        rdlong  SaveClkFreq,#clkfreqVal ' Save clock frequency and mode
                        rdbyte  SaveClkMode,#clksetVal

                        ' check for no data to load
                        tjz     ld_remaining, #load_done

                        ' check for an empty initial buffer
                        tjz     count, #release_frame
                        
                        call    #copy_data
              if_z      jmp     #load_done

release_frame           mov     t1, #STATUS_IDLE
                        wrlong  t1, rx_status_ptr
                        
wait_for_frame          rdlong  t1, rx_status_ptr
                        cmp     t1, #STATUS_IDLE wz
              if_z      jmp     #wait_for_frame

                        rdlong  src, rx_frame_ptr
                        rdlong  count, rx_length_ptr

                        rdbyte  t1, src
                        cmp     t1, #IPV4RX wz
              if_nz     jmp     #release_frame      'ignore frames other than IPV4RX
              
                        add     src, #IPV4RX_DATA_OFFSET
                        sub     count, #IPV4RX_DATA_OFFSET wz
              if_z      jmp     #release_frame
                        call    #copy_data
              if_nz     jmp     #release_frame

load_done               ' send the response
                        wrlong  response, tx_frame_ptr
                        wrlong  rcount, tx_length_ptr
                        mov     t1, #STATUS_BUSY
                        wrlong  t1, tx_status_ptr
:wait                   rdlong  t1, tx_status_ptr
                        cmp     t1, #STATUS_IDLE wz
              if_nz     jmp     #:wait              

'' Adapted from code in sdspiFemto.spin
'' After reading is finished for a boot, the stack marker is added below dbase
'' and memory is cleared between that and vbase (the end of the loaded program).
'' Memory beyond the stack marker is not cleared.  Note that if ioNoStore is set,
'' we go through the motions, but don't actually change memory or the clock.

                        rdlong  t1,#PREAMBLE2
                        shr     t1,#16             ' Get dbase value
                        sub     t1,#4
                        wrlong  StackMark,t1       ' Place stack marker at dbase
                        sub     t1,#4
                        wrlong  StackMark,t1
                        rdlong  t2,#PREAMBLE2      ' Get vbase value
                        and     t2,WordMask
                        sub     t1,t2
                        shr     t1,#2         wz   ' Compute number of longs between
:zeroIt         if_nz   wrlong  zero,t2            '  vbase and below stack marker
                if_nz   add     t2,#4
                if_nz   djnz    t1,#:zeroIt        ' Zero that space (if any)
                        rdlong  t1,#PREAMBLE0
                        cmp     t1,SaveClkFreq wz  ' Is the clock frequency the same?
                        rdlong  t1,#PREAMBLE1
                        and     t1,#$FF            ' Is the clock mode the same also?
                if_ne   jmp     #:changeClock
                        cmp     t1,SaveClkMode wz  ' If both same, just go start COG
                if_e    jmp     #:justStartUp
:changeClock            and     t1,#$F8            ' Force use of RCFAST clock while
                        clkset  t1                 '  letting requested clock start
                        mov     t1,time_xtal
:startupDelay           djnz    t1,#:startupDelay  ' Allow 20ms@20MHz for xtal/pll to settle
                        rdlong  t1,#PREAMBLE1
                        and     t1,#$FF            ' Then switch to selected clock
                        clkset  t1
:justStartUp            cogid   t1
                        or      t1, interpreter
                        coginit t1

'copy count bytes from src to ld_ptr but stop if ld_remaining goes to zero
copy_data               rdbyte  t1, src
                        add     src, #1
                        wrbyte  t1, ld_ptr
                        add     ld_ptr, #1
                        sub     ld_remaining, #1 wz
              if_z      jmp     copy_data_ret
                        djnz    count, #copy_data
copy_data_ret           ret

debug                   or      outa, :debug_leds
                        or      dira, :debug_leds
:debug                  jmp     #:debug
:debug_leds             long    (1 << 26) | (1 << 27)

'
' Initialized data
'

zero                    long    0

' taken from sdspiFemto.spin
StackMark               long    $FFF9FFFF               ' Two of these mark the base of the stack
interpreter             long    ($0004 << 16) | ($F004 << 2) | %0000
WordMask                long    $0000FFFF
time_xtal               long    20 * 20000 / 4 / 1      ' 20ms (@20MHz, 1 inst/loop)
SaveClkFreq             long    0                       ' Initial clock frequency (clkfreqVal)
SaveClkMode             long    0                       ' Initial clock mode value (clksetVal)

'
' Uninitialized data
'
t1                      res     1
t2                      res     1

rx_frame_ptr            res     1
rx_length_ptr           res     1
rx_status_ptr           res     1

tx_frame_ptr            res     1
tx_length_ptr           res     1
tx_status_ptr           res     1

src                     res     1
count                   res     1

ld_ptr                  res     1
ld_remaining            res     1

response                res     1
rcount                  res     1

                        fit     496
