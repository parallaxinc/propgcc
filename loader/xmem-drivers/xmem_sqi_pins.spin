CON

  ' protocol bits
  CS_CLR_PIN_MASK       = $01   ' CS or C3-style CS CLR
  INC_PIN_MASK          = $02   ' for C3-style CS INC
  MUX_START_BIT_MASK    = $04   ' low order bit of mux field
  MUX_WIDTH_MASK        = $08   ' width of mux field
  ADDR_MASK             = $10   ' device number for C3-style CS or value to write to the mux
 
DAT

' param1: 0xssxxccee - ss=sio0 xx=unused cc=sck pp=protocol
' param2: 0xaabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field
' example:
'   for a simple single pin CS you should set the protocol byte to $01 and place the CS pin number in byte aa.

' get_sqi_pins
'
' outputs:
'       pindir      - pin direction bits for dira
'       pinout      - idle value for output pins
'       mosi_mask   - mosi pin mask
'       miso_mask   - miso pin mask
'       sck_mask    - clock pin mask
get_sqi_pins

        ' get the sio_shift and build the mosi, miso, and sio masks
        mov     sio_shift, xmem_param1
        shr     sio_shift, #24
        mov     mosi_mask, #1
        shl     mosi_mask, sio_shift
        mov     miso_mask, mosi_mask
        shl     miso_mask, #1
        mov     sio_mask, #$f
        shl     sio_mask, sio_shift
        or      pindir, mosi_mask
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WE and /HOLD
        mov     t1, #$0c
        shl     t1, sio_shift
        or      pindir, t1
        or      pinout, t1
                
        ' build the sck mask
        mov     t1, xmem_param1
        shr     t1, #8
        and     t1, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t1
        or      pindir, sck_mask
        
        ' handle the CS or C3-style CLR pins
        test    xmem_param1, #CS_CLR_PIN_MASK wz
  if_nz mov     t1, xmem_param2
  if_nz shr     t1, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t1
  if_nz or      pindir, cs_clr
  if_nz or      pinout, cs_clr
  
        ' handle the mux width
        test    xmem_param1, #MUX_WIDTH_MASK wz
  if_nz mov     t1, xmem_param2
  if_nz shr     t1, #8
  if_nz and     t1, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t1
  if_nz sub     mask_inc, #1
  if_nz or      pindir, mask_inc
  
        ' handle the C3-style address or mux value
        test    xmem_param1, #ADDR_MASK wz
  if_nz mov     select_addr, xmem_param2
  if_nz and     select_addr, #$ff

        ' handle the C3-style INC pin
        mov     t1, xmem_param2
        shr     t1, #16
        and     t1, #$ff
        test    xmem_param1, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t1
  if_nz mov     select, c3_select_jmp       ' We're in C3 mode, so replace select/release
  if_nz mov     release, c3_release_jmp     ' with the C3-aware routines
  if_nz or      pindir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    xmem_param1, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t1
  if_nz shl     mask_inc, t1
  if_nz or      pindir, mask_inc
  
get_sqi_pins_ret
        ret

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

select                              ' Single-SPI and Parallel-DeMUX
        andn    outa, mask_inc
        or      outa, select_addr
        andn    outa, cs_clr
select_ret
        ret

release                             ' Single-SPI and Parallel-DeMUX
        mov     outa, pinout
        mov     dira, pindir
release_ret
        ret

c3_select_jmp                       ' Serial-DeMUX Jumps
        jmp     #c3_select          ' Initialization copies these jumps
c3_release_jmp                      '   over the select and release
        jmp     #c3_release         '   when in C3 mode.

c3_select                           ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        mov     c3tmp, select_addr
:loop   or      outa, mask_inc
        andn    outa, mask_inc
        djnz    c3tmp, #:loop
        jmp     select_ret

c3_release                          ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        jmp     release_ret

cs_clr          long    0
mask_inc        long    0
select_addr     long    0
c3tmp           long    0

