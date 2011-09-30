ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

KERNEL="
  /* the LMM kernel that is loaded into the cog */
  .lmmkernel ${RELOCATING-0} :
  {
    *(.lmmkernel)
    *(.kernel)
  } >cog AT>hub
"
KERNEL_NAME=.lmmkernel
TEXT_MEMORY=">hub AT>hub"
HUBTEXT_MEMORY=">hub AT>hub"
DATA_MEMORY=">hub AT>hub"
DATA_BSS_MEMORY=">hub AT>hub"
HUB_DATA="
    *(.data)
    *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
"
DATA_DATA="
"