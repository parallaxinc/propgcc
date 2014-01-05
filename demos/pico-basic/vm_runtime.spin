CON

  ' make some vm_interface constants available
  _MBOX_SIZE = vm#_MBOX_SIZE
  _STATE_SIZE = vm#_STATE_SIZE
  FLASH_BASE = vm#FLASH_BASE
  RAM_BASE = vm#RAM_BASE
  HUB_BASE = vm#HUB_BASE

  ' character codes
  CR = $0d
  LF = $0a

OBJ
  ser : "FullDuplexSerial"
  vm : "vm_interface"

PUB init_serial(baudrate, rxpin, txpin)
  ser.start(rxpin, txpin, 0, baudrate)

PUB init(mbox, state, code, data, cache_mbox, cache_line_mask) | params[vm#_INIT_SIZE]
  params[vm#INIT_BASE] := data
  params[vm#INIT_STATE] := state
  params[vm#INIT_MBOX] := mbox
  params[vm#INIT_CACHE_MBOX] := cache_mbox
  params[vm#INIT_CACHE_MASK] := cache_line_mask
  vm.start(code, @params)

PUB load(mbox, state, image, data_end) | main, stack, stack_size, count, p, i, base, offset, size

  main := vm.read_long(mbox, image + vm#IMAGE_MAIN_CODE)
  stack_size := vm.read_long(mbox, image + vm#IMAGE_STACK_SIZE)
  stack := data_end - stack_size
  long[state][vm#STATE_PC] := main
  long[state][vm#STATE_STACK] := stack
  long[state][vm#STATE_SP] := stack + stack_size
  long[state][vm#STATE_FP] := stack + stack_size
  long[state][vm#STATE_STACK_SIZE] := stack_size

  count := vm.read_long(mbox, image + vm#IMAGE_SECTION_COUNT)
  p := image + vm#_IMAGE_SIZE

  repeat i from 0 to count - 1
    base := vm.read_long(mbox, p + vm#SECTION_BASE)
    offset := vm.read_long(mbox, p + vm#SECTION_OFFSET)
    size := vm.read_long(mbox, p + vm#SECTION_SIZE)
    if i > 0
      repeat while size > 0
        vm.write_long(mbox, base, vm.read_long(mbox, image + offset))
        base += 4
        offset += 4
        size -= 4
    p += vm#_SECTION_SIZE

PUB single_step(mbox, state)
  state_header(state)
  process_requests(mbox, state, vm#STS_Step)

PUB run(mbox, state)
  vm.run(mbox, state)
  process_requests(mbox, state, vm.poll(mbox))

PRI process_requests(mbox, state, sts)
  repeat
    case sts
      vm#STS_Step:
        do_step(mbox, state)
      vm#STS_Trap:
        do_trap(mbox, state)
      vm#STS_Halt:
        halt(mbox, state, string("HALT"))
      vm#STS_StackOver:
        halt(mbox, state, string("STACK OVERFLOW"))
      vm#STS_DivideZero:
        halt(mbox, state, string("DIVIDE BY ZERO"))
      vm#STS_IllegalOpcode:
        halt(mbox, state, string("ILLEGAL OPCODE"))
      other:
        ser.str(string("sts: "))
        ser.hex(sts, 8)
        halt2(mbox, state)
    sts := vm.poll(mbox)

PRI halt(mbox, state, reason)
  ser.str(reason)
  ser.str(string(": "))
  halt2(mbox, state)

PRI halt2(mbox, state)
  state_header(state)
  show_status(mbox, state)
  repeat

PRI state_header(state) | stack
 stack := long[state][vm#STATE_STACK]
  ser.str(string("STACK "))
  ser.hex(stack, 8)
  ser.str(string(", STACK_TOP "))
  ser.hex(stack + long[state][vm#STATE_STACK_SIZE], 8)
  ser.crlf
  ser.str(string("PC       OP FP       SP       TOS      SP[0]    SP[1]    SP[2]    SP[3]", $a))

PRI do_step(mbox, state)
  show_status(mbox, state)
  repeat while ser.rx <> " "
  vm.single_step(mbox, state)

PRI do_trap(mbox, state) | p, len, ch
  case long[mbox][vm#MBOX_ARG2_FCN]
    vm#TRAP_GetChar:
	  push_tos(state)
      long[state][vm#STATE_TOS] := ser.rx
    vm#TRAP_PutChar:
      ser.tx(long[state][vm#STATE_TOS])
      pop_tos(state)
  if long[state][vm#STATE_STEPPING]
    do_step(mbox, state)
  else
    vm.continue(mbox)

PRI push_tos(state) | sp
  sp := long[state][vm#STATE_SP] - 4
  long[sp] := long[state][vm#STATE_TOS]
  long[state][vm#STATE_SP] := sp

PRI pop_tos(state) | sp
  sp := long[state][vm#STATE_SP]
  long[state][vm#STATE_TOS] := long[sp]
  long[state][vm#STATE_SP] := sp + 4

PRI show_status(mbox, state) | pc, sp, i
  pc := long[state][vm#STATE_PC]
  ser.hex(pc, 8)
  ser.tx(" ")
  ser.hex(vm.read_byte(mbox, pc), 2)
  ser.tx(" ")
  ser.hex(long[state][vm#STATE_FP], 8)
  ser.tx(" ")
  sp := long[state][vm#STATE_SP]
  ser.hex(sp, 8)
  ser.tx(" ")
  ser.hex(long[state][vm#STATE_TOS], 8)
  repeat i from 0 to 3 
    ser.tx(" ")
    ser.hex(long[sp][i], 8)
  ser.crlf

