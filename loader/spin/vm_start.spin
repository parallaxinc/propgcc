CON

  P_IMAGE_BASE          = 0
  P_CACHE_MBOX          = 4
  P_CACHE_LINE_MASK     = 8
  P_VM_MBOX             = 12
  
  HDR_ENTRY             = 0
  HDR_INIT_COUNT        = 4
  HDR_INIT_TABLE_OFFSET = 8

  INIT_VADDR            = 0
  INIT_PADDR            = 4
  INIT_SIZE             = 8
  INIT_ENTRY_SIZE       = 12

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
                    rdlong  boot_clkfreq, #0
                    rdbyte  boot_clkmode, #4
                    
initialize_data     mov     t1, image_base
                    add     t1, #HDR_INIT_COUNT
                    call    #read_long
                    mov     entry_count, t1 wz
        if_z        jmp     #:done_init
                    mov     t1, image_base
                    add     t1, #HDR_INIT_TABLE_OFFSET
                    call    #read_long
                    mov     init_entry, t1
                    add     init_entry, image_base

:loop_entry         mov     t1, init_entry
                    add     t1, #INIT_VADDR
                    call    #read_long
                    mov     dst, t1
                    mov     t1, init_entry
                    add     t1, #INIT_PADDR
                    call    #read_long
                    mov     src, t1
                    mov     t1, init_entry
                    add     t1, #INIT_SIZE
                    call    #read_long
                    mov     count, t1
                    add     count, #3
                    shr     count, #2 wz
        if_z        jmp     #:done_data

:clear_or_copy      cmp     src, dst wz
        if_z        jmp     #:clear

:loop_copy          mov     t1, src
                    call    #read_long
                    mov     t2, t1
                    mov     t1, dst
                    call    #write_long
                    add     src, #4
                    add     dst, #4
                    djnz    count, #:loop_copy
                    jmp     #:done_data

:clear              mov     t2, #0
:loop_clear         mov     t1, dst
                    call    #write_long
                    add     dst, #4
                    djnz    count, #:loop_clear

:done_data          add     init_entry, #INIT_ENTRY_SIZE
                    djnz    entry_count, #:loop_entry
:done_init

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
                    
restore_clkfreq     wrlong  boot_clkfreq, #0
                    wrbyte  boot_clkmode, #4

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

external_start      long    $20000000
boot_clkfreq        long    0
boot_clkmode        long    0 ' for alignment - only using a byte here
src                 long    0
dst                 long    0
count               long    0
entry_count         long    0
init_entry          long    0
temp                long    0

read_long           call    #cache_read
                    rdlong  t1, memp
read_long_ret       ret

write_long          cmp     t1, external_start wc       'check for normal memory access
             if_b   mov     memp, t1
             if_ae  call    #cache_write
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
