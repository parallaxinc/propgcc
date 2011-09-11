ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

KERNEL="
  /* the LMM kernel that is loaded into the cog */
  .kernel ${RELOCATING-0} :
  {
    *(.lmmkernel)
    *(.kernel)
  } >cog AT>hub
"
TEXT_MEMORY=">hub AT>hub"
HUBTEXT_MEMORY=">hub AT>hub"
DATA_MEMORY=">hub"
