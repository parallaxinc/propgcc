CON

INIT_BASE         = 0
INIT_STATE        = 1
INIT_MBOX         = 2
INIT_CACHE_MBOX   = 3
INIT_CACHE_MASK   = 4
_INIT_SIZE        = 5

MBOX_CMD          = 0
MBOX_ARG_STS      = 1
MBOX_ARG2_FCN     = 2
_MBOX_SIZE        = 3

STATE_FP          = 0
STATE_SP          = 1
STATE_TOS         = 2
STATE_PC          = 3
STATE_STEPPING    = 4
STATE_STACK       = 5
STATE_STACK_SIZE  = 6
_STATE_SIZE       = 7

VM_Continue       = 1
VM_ReadLong       = 2
VM_WriteLong      = 3
VM_ReadByte       = 4
_VM_Last          = 4

STS_Fail          = 0
STS_Halt          = 1
STS_Step          = 2
STS_Trap          = 3
STS_Success       = 4
STS_StackOver     = 5
STS_DivideZero    = 6
STS_IllegalOpcode = 7

TRAP_GetChar      = 0
TRAP_PutChar      = 1

' image header - must match db_image.h FileHdr
IMAGE_TAG               = $00   ' "XLOD"
IMAGE_VERSION           = $04   ' $0100
IMAGE_UNUSED            = $06   ' (unused)
IMAGE_MAIN_CODE         = $08
IMAGE_STACK_SIZE        = $0c
IMAGE_SECTION_COUNT     = $10
IMAGE_SECTIONS          = $14   ' beginning of first section
_IMAGE_SIZE             = $14

' section header - must match db_image.h FileHdr
SECTION_BASE            = $00
SECTION_OFFSET          = $04
SECTION_SIZE            = $08
_SECTION_SIZE           = $0c

' must match memory base addresses in db_config.h
HUB_BASE		= $00000000	' must be zero
COG_BASE		= $10000000
RAM_BASE		= $20000000
FLASH_BASE		= $30000000

' external memory base
EXTERNAL_BASE		= RAM_BASE

VAR

  long cog

PUB start(code, params) | mbox
  mbox := long[params][INIT_MBOX]
  long[mbox][MBOX_CMD] := 1
  cog := cognew(code, params)
  return poll(mbox)

PUB run(mbox, state)
  long[state][STATE_STEPPING] := 0
  long[mbox][MBOX_CMD] := VM_Continue

PUB single_step(mbox, state)
  long[state][STATE_STEPPING] := 1
  long[mbox][MBOX_CMD] := VM_Continue

PUB continue(mbox)
  long[mbox][MBOX_CMD] := VM_Continue

PUB poll(mbox)
  repeat while long[mbox][MBOX_CMD] <> 0
  return long[mbox][MBOX_ARG_STS]

PUB read_long(mbox, p_address)
  long[mbox][MBOX_ARG_STS] := p_address
  long[mbox][MBOX_CMD] := VM_ReadLong
  repeat while long[mbox][MBOX_CMD] <> 0
  return long[mbox][MBOX_ARG2_FCN]

PUB write_long(mbox, p_address, value)
  long[mbox][MBOX_ARG_STS] := p_address
  long[mbox][MBOX_ARG2_FCN] := value
  long[mbox][MBOX_CMD] := VM_WriteLong
  repeat while long[mbox][MBOX_CMD] <> 0

PUB read_byte(mbox, p_address)
  long[mbox][MBOX_ARG_STS] := p_address
  long[mbox][MBOX_CMD] := VM_ReadByte
  repeat while long[mbox][MBOX_CMD] <> 0
  return long[mbox][MBOX_ARG2_FCN]

