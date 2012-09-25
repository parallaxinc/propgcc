'#define DEBUG

CON

  _clkmode = XTAL1 + PLL16X
  _clkfreq = 80000000

  hub_memory_size = 32 * 1024
  cache_mbox_size = cacheint#_MBOX_SIZE * 4
  vm_mbox_size    = 4
  
  FLASH_BASE        = $30000000
  
OBJ

  cacheint : "cache_interface"
  vmstart : "vm_start"
#ifdef DEBUG
  ser : "FullDuplexSerial"
#endif

PUB start | cache

  ' initialize the cache driver
  cache := hub_memory_size - p_cache_size
  cache_mbox := cache - cache_mbox_size
  cache_line_mask := cacheint.start(@cache_code, cache_mbox, cache, p_cache_param1, p_cache_param2, p_cache_param3, p_cache_param4)
  
  ' load the vm
  vm_mbox := cache_mbox - vm_mbox_size
  long[vm_mbox] := 0
  cognew(@vm_code, vm_mbox)
  
#ifdef DEBUG
  ser.start(31, 30, 0, 115200)

  ser.str(STRING("p_cache_size: "))
  ser.hex(p_cache_size, 8)
  ser.crlf
  ser.str(STRING("p_cache_param1: "))
  ser.hex(p_cache_param1, 8)
  ser.crlf
  ser.str(STRING("p_cache_param2: "))
  ser.hex(p_cache_param2, 8)
  ser.crlf
  ser.str(STRING("p_cache_param3: "))
  ser.hex(p_cache_param3, 8)
  ser.crlf
  ser.str(STRING("p_cache_param4: "))
  ser.hex(p_cache_param4, 8)
  ser.crlf
  ser.str(STRING("cache: "))
  ser.hex(cache, 8)
  ser.crlf
  ser.str(STRING("cache_mboxcmd: "))
  ser.hex(cache_mboxcmd, 8)
  ser.crlf
  ser.str(STRING("vm_mbox: "))
  ser.hex(vm_mbox, 8)
  ser.crlf
  ser.str(STRING("cache_linemask: "))
  ser.hex(cache_linemask, 8)
  ser.crlf
  ser.str(STRING("entry: "))
  ser.hex(cacheint.readLong(entry), 8)
  ser.crlf
  ser.str(STRING("bss_start: "))
  ser.hex(cacheint.readLong(bss_start), 8)
  ser.crlf
  ser.str(STRING("bss_end: "))
  ser.hex(cacheint.readLong(bss_end), 8)
  ser.crlf
  ser.str(STRING("data_image: "))
  ser.hex(cacheint.readLong(data_image), 8)
  ser.crlf
  ser.str(STRING("data_start: "))
  ser.hex(cacheint.readLong(data_start), 8)
  ser.crlf
  ser.str(STRING("data_end: "))
  ser.hex(cacheint.readLong(data_end), 8)
  ser.crlf
  ser.str(STRING("CLKFREQ: "))
  ser.hex(cacheint.readLong(cacheint.readLong(data_image)), 8)
  ser.crlf
  
  repeat
  
#endif

   ' start the xmm kernel boot code
  coginit(cogid, vmstart.entry, @image_base)

DAT

' parameters filled in before downloading flash_loader.binary
p_cache_size        long    0
p_cache_param1      long    0
p_cache_param2      long    0
p_cache_param3      long    0
p_cache_param4      long    0
p_vm_code_off       long    @vm_code - @p_cache_size
p_cache_code_off    long    @cache_code - @p_cache_size

image_base          long    FLASH_BASE
cache_mbox          long    0
cache_line_mask     long    0
vm_mbox             long    0

vm_code             long    0[496]
cache_code          long    0[496]
