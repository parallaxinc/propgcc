ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

TEXT_MEMORY=">cog AT>hub"
DATA_MEMORY=">hub AT>hub"
DATA_BSS_MEMORY=">hub AT>hub"
HUBTEXT_MEMORY=">hub AT>hub"

HUB_HEAP=1
