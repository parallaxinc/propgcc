' select debugging output to a TV
'#define TV_DEBUG

CON
  ' these will get overwritten with the correct values before loading
  _clkmode    = xtal1 + pll16x
  _xinfreq    = 5_000_000

  TYPE_VM_INIT = 1
  TYPE_CACHE_INIT = 2
  TYPE_FILE_WRITE = 3
  TYPE_FLASH_WRITE = 4
  TYPE_RAM_WRITE = 5
  TYPE_HUB_WRITE = 6
  TYPE_DATA = 7
  TYPE_EOF = 8
  TYPE_RUN = 9

  ' character codes
  CR = $0d
  LF = $0a

  WRITE_NONE = 0
  WRITE_FILE = 1
  WRITE_FLASH = 2
  WRITE_RAM = 3
  WRITE_HUB = 4

' hub memory layout
'   cache
'   cache_mbox
'   vm_mbox

  hub_memory_size = 32 * 1024
  cache_mbox_size = cache#_MBOX_SIZE * 4
  vm_mbox_size = 4
  
OBJ
  pkt : "packet_driver"
#ifdef TV_DEBUG
  tv   : "TV_Text"
#endif
  sd   : "fsrw"
  cache : "cache_interface"
  vmstart : "vm_start"
VAR

  long ioControl[2]      ' SD parameters

  long sd_mounted
  long load_address
  long write_mode

PUB start | type, packet, len, ok

  ' start the packet driver
  pkt.start(p_rxpin, p_txpin, 0, p_baudrate)

#ifdef TV_DEBUG
  tv.start(p_tvpin)
  tv.str(string("Serial Helper v0.1", CR))
#endif

  ' initialize
  sd_mounted := 0
  write_mode := WRITE_NONE

  ' handle packets
  repeat

    ' get the next packet from the PC
    ok := pkt.rcv_packet(@type, @packet, @len)
#ifdef TV_DEBUG
    if not ok
      tv.str(string("Receive packet error", CR))
#endif

    if ok
      case type
        TYPE_VM_INIT:           VM_INIT_handler
        TYPE_CACHE_INIT:        CACHE_INIT_handler(packet)
        TYPE_FILE_WRITE:        FILE_WRITE_handler(packet)
        TYPE_FLASH_WRITE:       FLASH_WRITE_handler
        TYPE_RAM_WRITE:         RAM_WRITE_handler
        TYPE_HUB_WRITE:         HUB_WRITE_handler
        TYPE_DATA:              DATA_handler(packet, len)
        TYPE_EOF:               EOF_handler
        TYPE_RUN:               RUN_handler
        other:
#ifdef TV_DEBUG
          tv.str(string("Bad packet type: "))
          tv.hex(type, 2)
          crlf
#endif
      pkt.release_packet

PRI VM_INIT_handler
#ifdef TV_DEBUG
  tv.str(string("VM_INIT", CR))
#endif
  long[p_vm_mbox] := 0
  cognew(@mm_data, p_vm_mbox)

PRI CACHE_INIT_handler(packet) | cache_size, param1, param2, param3, param4, extra, cache_lines
  cache_size := long[packet]
  param1 := long[packet+4]
  param2 := long[packet+8]
  param3 := long[packet+12]
  param4 := long[packet+16]
  extra := long[packet+20]
#ifdef TV_DEBUG
  tv.str(string("CACHE_INIT: "))
  tv.dec(cache_size)
#endif
  cache_lines := hub_memory_size - cache_size
  p_cache_mbox := cache_lines - cache_mbox_size - extra * 4
  p_cache_line_mask := cache.start(@mm_data, p_cache_mbox, cache_lines, param1, param2, param3, param4)
  p_vm_mbox := p_cache_mbox - vm_mbox_size
#ifdef TV_DEBUG
  tv.str(string(" "))
  tv.hex(p_cache_line_mask, 4)
  crlf
#endif

PRI FILE_WRITE_handler(name) | err
  mountSD
#ifdef TV_DEBUG
  tv.str(string("FILE_WRITE: "))
  tv.str(name)
  tv.str(string("...",))
#endif
  err := \sd.popen(name, "w")
#ifdef TV_DEBUG
  tv.dec(err)
  crlf
#endif
  write_mode := WRITE_FILE
  load_address := $00000000

PRI FLASH_WRITE_handler
#ifdef TV_DEBUG
  tv.str(string("FLASH_WRITE", CR))
#endif
  write_mode := WRITE_FLASH
  load_address := $00000000 ' offset into flash
  p_image_base := $30000000
  cache.eraseFlashBlock(load_address)

PRI RAM_WRITE_handler
#ifdef TV_DEBUG
  tv.str(string("RAM_WRITE", CR))
#endif
  write_mode := WRITE_RAM
  load_address := $20000000
  p_image_base := load_address

PRI HUB_WRITE_handler
#ifdef TV_DEBUG
  tv.str(string("HUB_WRITE", CR))
#endif
  write_mode := WRITE_HUB
  load_address := $00000000

PRI DATA_handler(packet, len) | i, val, err
  err := 0 ' assume no error
#ifdef TV_DEBUG
  tv.str(string("DATA: "))
  tv.hex(load_address, 8)
  tv.out(" ")
  tv.dec(len)
  tv.str(string("..."))
#endif
  case write_mode
    WRITE_FILE:
      err := \sd.pwrite(packet, len)
      load_address += len
    WRITE_FLASH:
      cache.WriteFlash(load_address, packet, len)
      load_address += len
      if (load_address & $00000fff) == 0
        cache.eraseFlashBlock(load_address)
    WRITE_RAM:
      repeat i from 0 to len - 1 step 4
        cache.writeLong(load_address, long[packet+i])
        load_address += 4
    WRITE_HUB:
      repeat i from 0 to len - 1 step 4
        long[@mm_data + load_address] := long[packet+i]
        load_address += 4
    other:
#ifdef TV_DEBUG
      tv.str(string("bad write_mode: "))
#endif
#ifdef TV_DEBUG
  tv.dec(err)
  crlf
#endif

PRI EOF_handler | err
  err := 0 ' assume no errors
#ifdef TV_DEBUG
  tv.str(string("EOF..."))
#endif
  case write_mode
    WRITE_FILE:
      err := \sd.pclose
      write_mode := WRITE_NONE
    WRITE_FLASH:
    WRITE_RAM:
    WRITE_HUB:
    WRITE_NONE:
    other:
#ifdef TV_DEBUG
      tv.str(string("bad write_mode: "))
#endif
#ifdef TV_DEBUG
  tv.dec(err)
  crlf
#endif

PRI RUN_handler
#ifdef TV_DEBUG
  tv.str(string("RUN", CR))
#endif

  ' stop all COGs except the one running the vm
  pkt.stop

   ' start the xmm kernel boot code
  coginit(cogid, vmstart.entry, @p_image_base)

#ifdef TV_DEBUG
  tv.stop
#endif

PRI mountSD | err
  if sd_mounted == 0
    repeat
#ifdef TV_DEBUG
      tv.str(string("Mounting SD card...", CR))
#endif
      err := \sd.mount_explicit(p_dopin, p_clkpin, p_dipin, p_cspin, p_sel_inc, p_sel_msk, p_sel_addr)
    until err == 0
    sd_mounted := 1
  return 1

#ifdef TV_DEBUG
PRI crlf
  tv.out(CR)

PRI taghex8(tag, val)
  tv.str(tag)
  tv.hex(val, 8)
  crlf
#endif

DAT

' parameters filled in before downloading serial_helper.binary
p_baudrate          long    0
p_rxpin             byte    0
p_txpin             byte    0
p_tvpin             byte    0
p_dopin             byte    0
p_clkpin            byte    0
p_dipin             byte    0
p_cspin             byte    0
p_sel_addr          byte    0
p_sel_inc           long    0
p_sel_msk           long    0

p_image_base        long    0
p_cache_mbox        long    0
p_cache_line_mask   long    0
p_vm_mbox           long    0

' additional data
mm_data       long      0[512]

