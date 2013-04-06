{{
  Counter-based SPI code based on code from:
  
  SPI interface routines for SD & SDHC & MMC cards

  Jonathan "lonesock" Dummer
  version 0.3.0  2009 July 19
}}

spiInit
        or writeMode,mosi_pin
        or clockLineMode,sck_pin
        mov ctra,writeMode      ' Counter A drives data out
        mov ctrb,clockLineMode  ' Counter B will always drive my clock line
spiInit_ret
        ret
        
clockLineMode   long    %00100 << 26
writeMode       long    %00100 << 26
        
spiSendByte
        mov phsa,data
        shl phsa,#24
        andn outa,mosi_mask 
        mov phsb,#0
        movi frqb,#%01_0000000        
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        mov frqb,#0
        ' don't shift out the final bit...already sent, but be aware 
        ' of this when sending consecutive bytes (send_cmd, for e.g.) 
spiSendByte_ret
        ret

spiRecvByte
        neg phsa,#1' DI high
        mov data,#0
        ' set up my clock, and start it
        movi phsb,#%011_000000
        movi frqb,#%001_000000
        ' keep reading in my value
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        mov frqb,#0 ' stop the clock
        rcl data,#1
        mov phsa,#0 'DI low
spiRecvByte_ret
        ret
