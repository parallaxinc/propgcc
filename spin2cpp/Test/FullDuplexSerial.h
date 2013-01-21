#ifndef FullDuplexSerial_Class_Defined__
#define FullDuplexSerial_Class_Defined__

#include <stdint.h>

class FullDuplexSerial {
public:
  static const int Eol = 10;
  static uint8_t dat[];
  int32_t	Start(int32_t Rxpin, int32_t Txpin, int32_t Mode, int32_t Baudrate);
  int32_t	Stop(void);
  int32_t	Rxflush(void);
  int32_t	Rxcheck(void);
  int32_t	Rxready(void);
  int32_t	Rxtime(int32_t Ms);
  int32_t	Rx(void);
  int32_t	Tx(int32_t Txbyte);
  int32_t	Str(int32_t Stringptr);
  int32_t	Dec(int32_t Value);
  int32_t	Hex(int32_t Value, int32_t Digits);
  int32_t	Bin(int32_t Value, int32_t Digits);
private:
  int32_t	Cog;
  int32_t	Rx_head;
  int32_t	Rx_tail;
  int32_t	Tx_head;
  int32_t	Tx_tail;
  int32_t	Rx_pin;
  int32_t	Tx_pin;
  int32_t	Rxtx_mode;
  int32_t	Bit_ticks;
  int32_t	Buffer_ptr;
  uint8_t	Rx_buffer[16];
  uint8_t	Tx_buffer[16];
  uint8_t	Txlock;
  uint8_t	Strlock;
};

#endif
