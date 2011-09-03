CON

  _clkmode = XTAL1 + PLL16X
  _clkfreq = 80000000

  hub_memory_size = 32 * 1024
  cache_mbox_size = cacheint#_MBOX_SIZE * 4
  
  HDR_ENTRY         = 0
  HDR_BSS_START     = 4
  HDR_BSS_END       = 8
  HDR_DATA_IMAGE    = 12
  HDR_DATA_START    = 16
  HDR_DATA_END      = 20

OBJ

  cacheint : "cache_interface"

PUB start | cache, cache_mbox, vm_mbox, vm_state, data, data_end, cache_line_mask
  cache := hub_memory_size - p_cache_size
  cache_mbox := cache - cache_mbox_size
  cache_line_mask := cacheint.start(@cache_code, cache_mbox, cache, 0, 0)

DAT

' parameters filled in before downloading flash_loader.binary
params
p_cache_size        long    0

' pointers to code images
vm_code_off         long    @vm_code - @params
cache_code_off      long    @cache_code - @params

vm_code             long    0[496]
cache_code          long    0[496]
