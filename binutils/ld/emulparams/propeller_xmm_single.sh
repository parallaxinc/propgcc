ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

TEXT_MEMORY=">ram AT>ram"
DATA_MEMORY=">ram AT>ram"
DATA_BSS_MEMORY=">ram AT>ram"
HUBTEXT_MEMORY=">hub AT>hub"
DRIVER_MEMORY=">coguser AT>ram"

HUB_HEAP=1
DATA_HEAP=1

KERNEL="
  /* the LMM kernel that is loaded into the cog */
  .xmmkernel ${RELOCATING-0} :
  {
    *(.xmmkernel)
    *(.kernel)
  } >kermem AT>dummy
"
KERNEL_NAME=.xmmkernel

HUB_DATA="
"
DATA_DATA="
    *(.data)
    *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
"