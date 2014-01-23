DAT

spiSendByte
        shl     data, #24
        mov     bits, #8
spiSend rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #spiSend
        andn    outa, mosi_mask ' leave MOSI low
spiSendByte_ret
spiSend_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
spiRecv or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #spiRecv
spiRecvByte_ret
spiRecv_ret
        ret

data        long    0
bits        long    0

mosi_mask   long    0
miso_mask   long    0
sck_mask    long    0
