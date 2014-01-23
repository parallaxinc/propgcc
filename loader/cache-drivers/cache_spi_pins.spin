CON

  ' protocol bits
  CS_CLR_PIN_MASK       = $01
  INC_PIN_MASK          = $02   ' for C3-style CS
  MUX_START_BIT_MASK    = $04   ' low order bit of mux field
  MUX_WIDTH_MASK        = $08   ' width of mux field
  ADDR_MASK             = $10   ' device number for C3-style CS or value to write to the mux
  QUAD_SPI_HACK_MASK    = $20   ' set /WE and /HOLD for testing on Quad SPI chips
 
DAT

' param2: 0xooiiccee - oo=mosi ii=miso cc=sck pp=protocol
' param3: 0xaabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field
'   if QUAD_SPI_HACK_MASK ($20) is set, assume that pins miso+1 and miso+2 are /WP and /HOLD and assert them
' example:
'   for a simple single pin CS you should set the protocol byte to $01 and place the CS pin number in byte aa.

' get_spi_pins
'
' params:
'       t1          - pointer to the next configuration parameter
'
' uses:
'
'       t2, t3, t4
'
' outputs:
'       pindir      - pin direction bits for dira
'       pinout      - idle value for output pins
'       mosi_mask   - mosi pin mask
'       miso_mask   - miso pin mask
'       sck_mask    - clock pin mask
get_spi_pins
        ' get the pin definitions (cache-param2)
        rdlong  t2, t1
        add     t1, #4

        ' build the mosi mask
        mov     t3, t2
        shr     t3, #24
        mov     mosi_mask, #1
        shl     mosi_mask, t3
        or      pindir, mosi_mask
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WP and /HOLD
        test    t2, #QUAD_SPI_HACK_MASK wz
  if_nz mov     t4, #$0c
  if_nz shl     t4, t3      ' mosi pin from above
  if_nz or      pindir, t4
  if_nz or      pinout, t4
        
        ' build the miso mask
        mov     t3, t2
        shr     t3, #16
        and     t3, #$ff
        mov     miso_mask, #1
        shl     miso_mask, t3
        
        ' build the sck mask
        mov     t3, t2
        shr     t3, #8
        and     t3, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t3
        or      pindir, sck_mask
        
        ' get more pin definitions (cache-param3)
        rdlong  t3, t1
        add		t1, #4
        
        ' handle the CS or C3-style CLR pins
        test    t2, #CS_CLR_PIN_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t4
  if_nz or      pindir, cs_clr
  if_nz or      pinout, cs_clr
  
        ' handle the mux width
        test    t2, #MUX_WIDTH_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #8
  if_nz and     t4, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz sub     mask_inc, #1
  if_nz or      pindir, mask_inc
  
        ' handle the C3-style address or mux value
        test    t2, #ADDR_MASK wz
  if_nz mov     select_addr, t3
  if_nz and     select_addr, #$ff

        ' handle the C3-style INC pin
        mov     t4, t3
        shr     t4, #16
        and     t4, #$ff
        test    t2, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz mov     select, c3_select_jmp       ' We're in C3 mode, so replace select/release
  if_nz mov     release, c3_release_jmp     ' with the C3-aware routines
  if_nz or      pindir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    t2, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t4
  if_nz shl     mask_inc, t4
  if_nz or      pindir, mask_inc

get_spi_pins_ret
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
        or      outa, cs_clr
        andn    outa, mask_inc
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

