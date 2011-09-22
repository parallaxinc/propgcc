CON

  P_IMAGE_BASE      = 0
  P_CACHE_MBOX      = 4
  P_CACHE_LINE_MASK = 8
  P_VM_MBOX         = 12
  
  HDR_ENTRY         = 0
  HDR_BSS_START     = 4
  HDR_BSS_END       = 8
  HDR_DATA_IMAGE    = 12
  HDR_DATA_START    = 16
  HDR_DATA_END      = 20

OBJ

  cacheint : "cache_interface"

PUB entry
    return @boot
    
DAT
                    org     0

boot                mov     t1, PAR
                    rdlong  image_base, t1
                    add     t1, #4
                    rdlong  cache_mboxcmd, t1
                    add     t1, #4
                    mov     cache_mboxdat, cache_mboxcmd
                    add     cache_mboxdat, #4
                    rdlong  cache_linemask, t1
                    add     t1, #4
                    rdlong  vm_mbox, t1
                    
initialize_data     mov     t1, image_base
                    add     t1, #HDR_DATA_IMAGE
                    call    #read_long
                    mov     src, t1
                    mov     t1, image_base
                    add     t1, #HDR_DATA_START
                    call    #read_long
                    mov     dst, t1
                    mov     t1, image_base
                    add     t1, #HDR_DATA_END
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

clear_bss           mov     t1, image_base
                    add     t1, #HDR_BSS_START
                    call    #read_long
                    mov     dst, t1
                    mov     t1, image_base
                    add     t1, #HDR_BSS_END
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

initialize_stack    mov     dst, vm_mbox
                    mov     t1, image_base
                    add     t1, #HDR_ENTRY
                    call    #read_long
                    sub     dst, #4
                    wrlong  t1, dst
                    sub     dst, #4
                    wrlong  cache_linemask, dst
                    sub     dst, #4
                    wrlong  cache_mboxcmd, dst
                    
'Store the cache driver mailbox address at $00000006
                    wrword  cache_mboxcmd, #$00000006 '__xmm_mbox

'Start the xmm kernel cog and stop this cog.
launch              wrlong  dst, vm_mbox
                    cogid   t1
                    cogstop t1

image_base          long    0
cache_mboxcmd       long    0
cache_mboxdat       long    0
cache_linemask      long    0
vm_mbox             long    0

src                 long    0
dst                 long    0
count               long    0
temp                long    0

read_long           call    #cache_read
                    rdlong  t1, memp
read_long_ret       ret

write_long          call    #cache_write
                    wrlong  t2, memp
write_long_ret      ret

cache_write         mov     memp, t1                    'save address for index
                    mov     memp, t1                    'save address for index
                    andn    t1, #cacheint#CMD_MASK      'ensure a write is not a read
                    or      t1, #cacheint#WRITE_CMD
                    jmp     #cache_access

cache_read          mov     memp, t1                    'save address for index
                    mov     temp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                    andn    temp, cache_linemask
                    cmp     cacheaddr, temp wz          'if cacheaddr == addr, just pull form cache
        if_e        jmp     #cache_hit                  'memp gets overwriteen on a miss
        
cache_read_miss     or      t1, #cacheint#READ_CMD      'read must be 3 to avoid needing andn addr,#cache#CMD_MASK

cache_access        wrlong  t1, cache_mboxcmd
                    mov     cacheaddr, t1               'Save new cache address. it's free time here
                    andn    cacheaddr, cache_linemask   'Kill command bits in free time
:waitres            rdlong  temp, cache_mboxcmd wz
        if_nz       jmp     #:waitres
                    rdlong  cacheptr, cache_mboxdat     'Get new buffer
cache_hit           and     memp, cache_linemask
                    add     memp, cacheptr              'add ptr to memp to get data address
cache_read_ret
cache_write_ret     ret

t1                  long    0
t2                  long    0
memp                long    0
cacheaddr           long    0
cacheptr            long    0

                    fit     496
