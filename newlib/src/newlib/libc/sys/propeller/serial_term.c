#include <errno.h>
#include <reent.h>
#include "propdev.h"
#include "FdSerial.h"

static FdSerial_t serialData;

static int serial_getc(void);
static int serial_putc(int ch);

int InitSerialTerm(int rxpin, int txpin, int mode, int baudrate, int flags)
{
    int ret;
    if ((ret = FdSerial_start(&serialData, rxpin, txpin, mode, baudrate)) != 0) {
        if (flags & TERM_IN) {
            _term_read_p = _term_read;
            _term_getc_p = serial_getc;
        }
        if (flags & TERM_OUT) {
            _term_write_p = _term_write;
            _term_putc_p = serial_putc;
            _error_write_p = _term_write;
            _error_putc_p = serial_putc;
        }
    }
    return ret;
}

static int serial_getc(void)
{
    return FdSerial_rx(&serialData);
}

static int serial_putc(int ch)  
{
    FdSerial_tx(&serialData, ch);
    return 0;
}
