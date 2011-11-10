#ifndef __VM_START_H__
#define __VM_START_H__

#define P_IMAGE_BASE           0
#define P_CACHE_MBOX           4
#define P_CACHE_LINE_MASK      8
#define P_VM_MBOX              12
  
#define HDR_ENTRY              0
#define HDR_INIT_COUNT         4
#define HDR_INIT_TABLE_OFFSET  8

#define INIT_VADDR             0
#define INIT_PADDR             4
#define INIT_SIZE              8
#define INIT_ENTRY_SIZE        12
    
#define WRITE_CMD              2
#define READ_CMD               3
#define CMD_MASK               3

#endif
