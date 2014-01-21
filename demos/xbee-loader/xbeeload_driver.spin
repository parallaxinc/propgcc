''*********************************************
''* Xbee Load Driver                          *
''*  (C) 2014 David Betz                      *
''* Released under the MIT License            *
''*********************************************

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
                        
                        tjz     ld_remaining, #load_done

                        cmp     count, #0 wz
              if_nz     call    #copy_data
              if_z      jmp     #load_done
              
wait_for_frame          rdlong  t1, rx_status_ptr
                        cmp     t1, #STATUS_IDLE wz
              if_z      jmp     #wait_for_frame

                        rdlong  src, rx_frame_ptr
                        rdlong  count, rx_length_ptr
                        
                        rdbyte  t1, src
                        cmp     t1, #IPV4RX wz
              if_nz     jmp     #wait_for_frame     'ignore frames other than IPV4RX
              
                        add     src, #IPV4RX_DATA_OFFSET
                        sub     count, #IPV4RX_DATA_OFFSET wz
              if_z      jmp     #wait_for_frame
                        call    #copy_data
              if_nz     jmp     #wait_for_frame

load_done               ' send the response
                        wrlong  response, tx_frame_ptr
                        wrlong  rcount, tx_length_ptr
                        mov     t1, #STATUS_BUSY
                        wrlong  t1, tx_status_ptr
                        
                        or      outa, led
                        or      dira, led

':done                   jmp     #:done

                        ' start the program in this COG
                        cogid   t1
                        or      t1, interpreter
                        coginit t1

led                     long    1 << 26 ' led on the propeller activity board

'copy count bytes from src to ld_ptr but stop if ld_remaining goes to zero
copy_data               rdbyte  t1, src
                        add     src, #1
                        wrbyte  t1, ld_ptr
                        add     ld_ptr, #1
                        sub     ld_remaining, #1 wz
              if_z      jmp     copy_data_ret
                        djnz    count, #copy_data
copy_data_ret           ret

' taken from sdspiFemto.spin
interpreter             long    ($0004 << 16) | ($F004 << 2) | %0000

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
