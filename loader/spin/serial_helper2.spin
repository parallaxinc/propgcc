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
'   cache tags
'   xmem mailbox

  hub_memory_size = 32 * 1024
  
OBJ
  pkt : "packet_driver"
#ifdef TV_DEBUG
  tv   : "TV_Text"
#endif
  sd   : "fsrw"
  xmem : "xmem_interface"

VAR

  long ioControl[2]      ' SD parameters

  long sd_mounted
  long load_address
  long initial_pc
  long write_mode
  
PUB start | type, packet, len, ok

  ' start the packet driver
  pkt.start(p_rxpin, p_txpin, 0, p_baudrate)

#ifdef TV_DEBUG
  tv.start(p_tvpin)
  tv.str(string("Serial Helper v0.2", CR))
#endif

  ' initialize
  sd_mounted := 0
  write_mode := WRITE_NONE
  
  ' pointer to a buffer that is 16-byte aligned (thanks Ariba!)
  mm_data_ptr := @mm_data_padded & !15

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

PRI CACHE_INIT_handler(packet) | index_width, offset_width, tags_size, cache_size, param1, param2, param3, param4, cogn
  p_cache_geometry := long[packet]
  index_width := p_cache_geometry >> 8
  offset_width := p_cache_geometry & $ff
  tags_size := (1 << index_width) * 4
  cache_size := 1 << (index_width + offset_width)
#ifdef TV_DEBUG
  tv.str(string("CACHE_INIT: "))
  tv.dec(tags_size + cache_size)
#endif
  p_cache_lines := hub_memory_size - cache_size
  p_cache_tags := p_cache_lines - tags_size
  p_xmem_mboxes := p_cache_tags - xmem#_MBOX2_SIZE * 4 * 8 - 4 ' one mailbox per COG
  cogn := xmem.start2(mm_data_ptr, p_xmem_mboxes, 8)
#ifdef TV_DEBUG
  tv.str(string(" -> "))
  tv.dec(cogn)
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
  load_address := $30000000
  initial_pc := load_address

PRI RAM_WRITE_handler
#ifdef TV_DEBUG
  tv.str(string("RAM_WRITE", CR))
#endif
  write_mode := WRITE_RAM
  load_address := $20000000
  initial_pc := load_address

PRI HUB_WRITE_handler
#ifdef TV_DEBUG
  tv.str(string("HUB_WRITE", CR))
#endif
  write_mode := WRITE_HUB
  load_address := $00000000

PRI DATA_handler(packet, len) | i, val, buf, count, err
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
      bytemove(mm_data_ptr, packet, pkt#PKTMAXLEN) ' buffer has to be 16 byte aligned
      buf := mm_data_ptr
      count := (len + 1023) >> 10 ' number of 1024 byte blocks
      repeat count
        xmem.writeBlock(load_address, buf, xmem#XMEM_SIZE_1024)
        load_address += 1024
        buf += 1024
    WRITE_RAM:
      bytemove(mm_data_ptr, packet, pkt#PKTMAXLEN) ' buffer has to be 16 byte aligned
      buf := mm_data_ptr
      count := (len + 1023) >> 10 ' number of 1024 byte blocks
      repeat count
        xmem.writeBlock(load_address, buf, xmem#XMEM_SIZE_1024)
        load_address += 1024
        buf += 1024
    WRITE_HUB:
      repeat i from 0 to len - 1 step 4
        long[mm_data_ptr + load_address] := long[packet+i]
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

PRI RUN_handler | sp
#ifdef TV_DEBUG
  tv.str(string("RUN", CR))
#endif

  ' stop all COGs except the one running the vm
  pkt.stop
#ifdef TV_DEBUG
  tv.stop
#endif

  ' setup the stack
  ' at start stack contains xmem_mbox, cache_tags, cache_lines, cache_geometry, pc
  sp := p_xmem_mboxes
  sp -= 4
  long[sp] := initial_pc
  sp -= 4
  long[sp] := p_cache_geometry
  sp -= 4
  long[sp] := p_cache_lines
  sp -= 4
  long[sp] := p_cache_tags
  sp -= 4
  long[sp] := p_xmem_mboxes

   ' start the xmm kernel boot code
  coginit(cogid, mm_data_ptr, sp)

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
  
PRI space
  tv.out(" ")

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
' end of parameters that are filled in before downloading

p_xmem_mboxes       long    0
p_cache_geometry    long    0
p_cache_tags        long    0
p_cache_lines       long    0
p_vm_mbox           long    0

' additional data
                    long    0[3]
mm_data_padded      long    0[512]
mm_data_ptr         long    0

