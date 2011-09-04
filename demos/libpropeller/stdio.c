extern int _serial_tx(int value);

/* provide a default value for the stdio _putc function */

int (*_putc)(int) = _serial_tx;
