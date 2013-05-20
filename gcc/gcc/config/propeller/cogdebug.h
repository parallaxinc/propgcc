#ifndef DBG_CMD_STATUS
/*
 * packets have the form:
 * byte 0: HOST_PACKET
 * byte 1: cmd | target, where:
 *         target is the COG number (0x0f for broadcast)
 *         cmd is one of the commands below
 * byte 2: len (remaining bytes in command)
 * bytes 3..2+n: command specific
 */
#define HOST_PACKET        0xfd

/* requests 4 bytes: cogid, cogflags, cogpc */
#define DBG_CMD_STATUS     0x00

/* orders the cog to resume */
#define DBG_CMD_RESUME     0x10

/* read data from the cog: data is 1 byte len (in bytes), 2 bytes address */
#define DBG_CMD_READCOG    0x20

/* write data to cog: data is 2 bytes address and then a sequence of bytes to write there */
#define DBG_CMD_WRITECOG   0x30

/* read bytes from hub ram: data is 1 byte len, 4 bytes address */
#define DBG_CMD_READHUB    0x40

/* write bytes to hub ram: data is 4 bytes address and then bytes to write there */
#define DBG_CMD_WRITEHUB   0x50

/* requests the breakpoint command (4 bytes) */
#define DBG_CMD_QUERYBP    0x60

/* request to single step the LMM interpreter */
#define DBG_CMD_LMMSTEP    0x70

/* set LMM hardware breakpoint: data is 1 byte bkpt number, 4 bytes address */
#define DBG_CMD_LMMBRK     0x80

/* response packets */
/* responses always have the form:
 *  byte 0: response type (0xf8-0xff)
 *  byte 1: cog responding
 *  bytes 2..n: data
 */
#define RESPOND_STATUS 0xf8  /* always 4 bytes coming back */
#define RESPOND_DATA   0xf9  /* same number of bytes as host requested */
#define RESPOND_ACK    0xfa  /* 1 byte data, intended to be checksum but always 0 for now */
#define RESPOND_ERR    0xfe  /* 1 byte data error code */

/* bits in the cogflags register */
#define COGFLAGS_C      0x01
#define COGFLAGS_NZ     0x02

#define COGFLAGS_VERSION 0x30 /* mask for propeller version */
#define COGFLAGS_P2     0x10  /* code is running on Propeller 2 */
#define COGFLAGS_CMM    0x40
#define COGFLAGS_STEP   0x80  /* break on next LMM loop */


#define ERR_NOCMD       0xe1
#define ERR_READLEN     0xe2
#define ERR_BRKLEN      0xe3
#define ERR_BRKNUM      0xe4

#endif
