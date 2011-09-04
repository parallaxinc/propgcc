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

PUB start | cache, cache_mbox, cache_line_mask, image_p, data_p, end, sp

  ' initialize the cache driver
  cache := hub_memory_size - p_cache_size
  cache_mbox := cache - cache_mbox_size
  cache_line_mask := cacheint.start(@cache_code, cache_mbox, cache, p_cache_param1, p_cache_param2)

  ' clear .bss
  data_p := cacheint.readLong(FLASH_BASE + HDR_BSS_START)
  end := cacheint.readLong(FLASH_BASE + HDR_BSS_END)
  repeat while data_p < end
    cacheint.writeLong(data_p, 0)
    data_p += 4

  ' initialize .text from the flash image
  image_p := cacheint.readLong(FLASH_BASE + HDR_DATA_IMAGE)
  data_p := cacheint.readLong(FLASH_BASE + HDR_DATA_START)
  end := cacheint.readLong(FLASH_BASE + HDR_DATA_END)
  repeat while data_p < end
    cacheint.writeLong(data_p, cacheint.readLong(image_p))
    image_p += 4
    data_p += 4
    
  ' setup the stack with the initialization parameters
  sp := cache_mbox
  sp -= 4
  long[sp] := cacheint.readLong(FLASH_BASE + HDR_ENTRY)
  sp -= 4
  long[sp] := cache_line_mask
  sp -= 4
  long[sp] := cache_mbox

  ' start the xmm kernel
  coginit(cogid, vm_code, sp)

DAT

' parameters filled in before downloading flash_loader.binary
params
p_cache_size        long    0
p_cache_param1      long    0
p_cache_param2      long    0
p_vm_code_off       long    @vm_code - @params
p_cache_code_off    long    @cache_code - @params

vm_code             long    0[496]
cache_code          long    0[496]
