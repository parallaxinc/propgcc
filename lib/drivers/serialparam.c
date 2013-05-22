/* globals that the loader may change; these represent the default
 * pins to use
 */
#if defined(__PROPELLER2__)
unsigned int _rxpin = 91;
unsigned int _txpin = 90;
#else
unsigned int _rxpin = 31;
unsigned int _txpin = 30;
#endif
unsigned int _baud = 115200;
