ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

TEXT_MEMORY=">rom"
DATA_MEMORY=">hub AT>rom"
DATA_BSS_MEMORY=">hub AT>hub"
HUBTEXT_MEMORY=">hub AT>rom"
DRIVER_MEMORY=">coguser AT>rom"

KERNEL="
  /* the LMM kernel that is loaded into the cog */
  .xmmkernel ${RELOCATING-0} :
  {
    *(.xmmkernel)
    *(.kernel)
  } >cog AT>dummy
"
KERNEL_NAME=.xmmkernel
XMM_HEADER="
    .header : {
        LONG(entry)
        LONG(0)
        LONG(0)
    } >rom
"
HUB_DATA="
    *(.data)
    *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
"
DATA_DATA="
"