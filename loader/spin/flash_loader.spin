CON

  _clkmode = XTAL1 + PLL16X
  _clkfreq = 80000000

  hub_memory_size = 32 * 1024
  cache_mbox_size = cacheint#_MBOX_SIZE * 4
  
  FLASH_BASE        = $30000000
  
  HDR_ENTRY         = 0
  HDR_BSS_START     = 4
  HDR_BSS_END       = 8
  HDR_DATA_IMAGE    = 12
  HDR_DATA_START    = 16
  HDR_DATA_END      = 20

OBJ

  cacheint : "cache_interface"

PUB start | cache

  ' initialize the cache driver
  cache := hub_memory_size - p_cache_size
  cache_mbox := cache - cache_mbox_size
  cache_line_mask := cacheint.start(@cache_code, cache_mbox, cache, p_cache_param1, p_cache_param2)

   ' start the xmm kernel boot code
  coginit(cogid, @boot, 0)

DAT

                    org     0

' parameters filled in before downloading flash_loader.binary
boot                jmp     #boot_next
p_cache_size        long    0
p_cache_param1      long    0
p_cache_param2      long    0
p_vm_code_off       long    @vm_code - @boot
p_cache_code_off    long    @cache_code - @boot

cache_mbox          long    0
cache_mbox_dat      long    0
cache_line_mask     long    0

src                 long    0
dst                 long    0
count               long    0
coginit_dest        long    0
temp                long    0

entry               long     FLASH_BASE + HDR_ENTRY
bss_start           long     FLASH_BASE + HDR_BSS_START
bss_end             long     FLASH_BASE + HDR_BSS_END
data_start          long     FLASH_BASE + HDR_DATA_START
data_end            long     FLASH_BASE + HDR_DATA_END

'Copy PAR block into COG
boot_next           
                    'clear .bss
                    mov     t1, bss_start
                    call    #read_long
                    mov     dst, t1
                    mov     t1, bss_end
                    call    #read_long
                    mov     count, t1
                    sub     count, dst
                    shr     count, #2 wz
        if_z        jmp     #:done_bss
                    mov     t2, #0
:loop_bss           wrlong  t2, dst
                    add     dst, #4
                    djnz    count, #:loop_bss
:done_bss

                    'initialize .data
                    mov     t1, data_start
                    call    #read_long
                    mov     dst, t1
                    mov     t1, data_end
                    call    #read_long
                    mov     count, t1
                    sub     count, dst
                    shr     count, #2 wz
        if_z        jmp     #:done_data
:loop_data          mov     t1, src
                    call    #read_long
                    wrlong  t1, dst
                    add     src, #4
                    add     dst, #4
                    djnz    count, #:loop_data
:done_data

                    mov     dst, cache_mbox
                    mov     t1, entry
                    call    #read_long
                    sub     dst, #4
                    wrlong  t1, dst
                    sub     dst, #4
                    wrlong  cache_line_mask, dst
                    sub     dst, #4
                    wrlong  cache_mbox, dst

'Launch Zog in this COG.
go_zog              cogid   coginit_dest            'Get id of this cog for restarting
                    mov     temp, #vm_code          'Set pasm code address for coginit
                    shl     temp, #2                'Shift into bits 17:4 (only high 14 bits required)
                    or      coginit_dest, temp      'Place in dest for coginit

                    mov     temp, dst               'Set PAR to sp for coginit
                    shl     temp, #16               'Move to bits 31:18 (only high 14 bits required)
                    or      coginit_dest, temp      'Combine PASM addr and PAR addr

                    coginit coginit_dest

read_long           call    #cache_read
                    rdlong  t1, memp
read_long_ret       ret

write_long          call    #cache_write
                    wrlong  t2, memp
write_long_ret      ret

cache_write         mov     memp, t1                    'save address for index
                    andn    t1, #cacheint#CMD_MASK      'ensure a write is not a read
                    or      t1, #cacheint#WRITE_CMD
                    jmp     #cache_access

cache_read          mov     temp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                    andn    temp, cache_linemask
                    cmp     cacheaddr, temp wz          'if cacheaddr == addr, just pull form cache
        if_ne       jmp     #:next                      'memp gets overwriteen on a miss
                    mov     memp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                    and     memp, cache_linemask
                    add     memp, cacheptr              'add ptr to memp to get data address
                    jmp     #cache_read_ret
:next               mov     memp, t1                    'save address for index
                    or      t1, #cacheint#READ_CMD      'read must be 3 to avoid needing andn addr,#cache#CMD_MASK

cache_access        wrlong  t1, cache_mbox
                    mov     cacheaddr,t1                'Save new cache address. it's free time here
                    andn    cacheaddr,cache_linemask    'Kill command bits in free time
:waitres            rdlong  temp, cache_mbox wz
        if_nz       jmp     #:waitres
                    and     memp, cache_linemask        'memp is index into buffer
                    rdlong  cacheptr, cache_mbox_dat    'Get new buffer
                    add     memp, cacheptr              'memp is now HUB buf address of data to read
cache_read_ret
cache_write_ret     ret

t1                  long    0
t2                  long    0
cache_linemask      long    0
cache_mboxcmd       long    0
cache_mboxdat       long    0
memp                long    0
cacheaddr           long    0
cacheptr            long    0

                    fit     496

vm_code             long    0[496]
cache_code          long    0[496]

