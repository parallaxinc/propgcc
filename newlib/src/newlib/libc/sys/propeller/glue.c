/*
 * simple glue functions for I/O
 */

/* default to serial */
extern int _serial_tx(int);

int (*_putc)(int) = _serial_tx;
