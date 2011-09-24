ARCH=propeller
SCRIPT_NAME=propeller
OUTPUT_FORMAT="elf32-propeller"
EMBEDDED=yes
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=propeller

TEXT_MEMORY=">cog"
DATA_MEMORY=">hub AT>hub"
HUBTEXT_MEMORY=">hub AT>hub"
