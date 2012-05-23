//
// To tell the library how your SD card is attached to the Propeller,
// fill in a _SD_Params structure and pass its address to LoadSDDriver.
//
// The SD Driver has 3 modes: Single SPI, C3/Serial deMUX, and Parallel deMUX.
//
// For all modes, MISO, CLK, MOSI, and CS are required.  For these parameters, the
// library user specifies an PIO and LoadSDDriver() converts the pin to a mask
// and sends to the driver.
//
// It should be obvious, but specify logical pin numbers (The P#) instead of
// physical pin numbers.  For example, if you attached the SD card's MISO
// to the Propeller pin 23, that's P20 so you, specify MISO=20.
//
// Use Single SPI mode if your SD card is the only thing on your SPI bus.
// To use this mode:
// - Initialize all structure elements to 0
// - Set MISO, CLK, MOSI, and CS to the appropriate pin numbers
// - Set the attachment type to SingleSPI
//
// Use Serial deMUX mode for the C3 or any other board where the SPI bus is
// controlled by a serial counter attached to an inverting deMUX.  Two pins
// and a count control the counter and deMUX:
//
// - CLR connects to the reset/clear pin of the serial counter (hence "CLR"
//   stands for "Clear"; like CS, this is an active low signal).
// - INC connects to the clock/count pin of the serial counter (hence "INC"
//   stands for "Increment")
// - ADDR is the address of the SD card; it is the count of clocks needed
//   on the INC pin so that the SD card becomes selected on the SPI bus.
//
// To use this mode:
// - Initialize all structure elements to 0
// - Set MISO, CLK, MOSI, CLR, and INC to the appropriate pin numbers
// - Set the ADDR to the appropriate number
// - Set the attachment type to SerialDeMUX
// 
// Use Parallel deMUX mode for Bill Henning's boards or any other board where the
// SPI bus is controlled by an inverting deMUX; one pin connects to the deMUX's
// active-low enable, and two or more Propeller pins connect to the deMUX's
// address lines. You specify the active-low enable as a pin number:
//
// - CS the pin number that connects to the deMUX's active-low enable.
//
// However, because more than one pin could be used to address the deMUX,
// so you specify the address connections as masks:
//
// - MSK is a mask with a bit set for each pin connected to the address
//   deMUX; the driver clears all these pins from OUTA when addressing the
//   SD card.
// - SEL is a mask of the deMUX address bits for the SD card; the driver ORs
//   this mask into OUTA when addressing the SD card.  Note that the code does
//   not enforce that the bits specified in SEL are a subset of the bits
//   specified in MSK (i.e. SEL & MSK == SEL).
//
// To use this mode:
// - Initialize all structure elements to 0
// - Set MISO, CLK, MOSI, and CS to the appropriate pin numbers
// - Set SEL, MSK to the appropriate masks
// - Set the attachment type to ParallelDeMUX
//

#ifndef _SYS_SD_H
#define _SYS_SD_H

#include <time.h>  /* for struct tm */

typedef enum { _SDA_SingleSPI, _SDA_SerialDeMUX, _SDA_ParallelDeMUX, _SDA_ConfigWords } _SDAttachType;

typedef struct 
{
    uint32_t  MISO;    // The pin attached to the SD card's MISO or DO output
    uint32_t  CLK;     // The pin attached to the SD card's CLK or SCLK input
    uint32_t  MOSI;    // The pin attached to the SD card's MOSI or DI input
    uint32_t  CS;      // The pin attached to the SD card's CS input
} _SD_SingleSPI;

typedef struct 
{
    uint32_t  MISO;    // The pin attached to the SD card's MISO or DO output
    uint32_t  CLK;     // The pin attached to the SD card's CLK or SCLK input
    uint32_t  MOSI;    // The pin attached to the SD card's MOSI or DI input
    uint32_t  CLR;     // The pin attached to the counter's reset/clear pin
    uint32_t  INC;     // The pin attached to the counter's clock/count pin
    uint32_t  ADDR;    // The SD card's demux address (the counter's count)
} _SD_SerialDeMUX;

typedef struct 
{
    uint32_t  MISO;    // The pin attached to the SD card's MISO or DO output
    uint32_t  CLK;     // The pin attached to the SD card's CLK or SCLK input
    uint32_t  MOSI;    // The pin attached to the SD card's MOSI or DI input
    uint32_t  CS;      // The pin attached to the inverting deMUX's active-low enable
    uint32_t  START;   // The starting pin of the mask for selecting the SD card's deMUX address
    uint32_t  WIDTH;   // The width of the mask for selecting the SD card's deMUX address
    uint32_t  ADDR;    // The SD card's demux address
} _SD_ParallelDeMUX;

typedef struct 
{
    uint32_t  CONFIG1; // The value of the loader patched variable _sdspi_config1
    uint32_t  CONFIG2; // The value of the loader patched variable _sdspi_config2
} _SD_ConfigWords;

typedef struct
{
    _SDAttachType AttachmentType;

    union
    {
        _SD_SingleSPI     SingleSPI;
        _SD_SerialDeMUX   SerialDeMUX;
        _SD_ParallelDeMUX ParallelDeMUX;
        _SD_ConfigWords   ConfigWords;
    } pins;
} _SD_Params;

uint32_t dfs_mount(_SD_Params* params);
uint32_t dfs_mount_defaults(void);
void dfs_use_lock(uint32_t lockId);
void dfs_setDefaultFileDateTime(struct tm* tm);

#endif
