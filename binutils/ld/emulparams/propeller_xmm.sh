ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

TEXT_MEMORY=">rom"
DATA_MEMORY=">rom AT>rom"
HUBTEXT_MEMORY=">hub AT>rom"

KERNEL="
  /* the LMM kernel that is loaded into the cog */
  .xmmkernel ${RELOCATING-0} :
  {
    *(.xmmkernel) *(.kernel)
  } >cog AT>dummy
"
KERNEL_NAME=.xmmkernel
XMM_HEADER="
    .header : {
        LONG(entry)
        LONG(ADDR(.bss))
        LONG(ADDR(.bss) + SIZEOF(.bss))
        LONG(LOADADDR(.data))
        LONG(ADDR(.data))
        LONG(ADDR(.data) + SIZEOF(.data) + SIZEOF(.hubtext) + SIZEOF(.ctors) + SIZEOF(.dtors))
    } >rom
"