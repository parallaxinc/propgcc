/* serial_io_cygwin.c - serial i/o routines
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "osint.h"

static HANDLE hSerial;
static COMMTIMEOUTS original_timeouts;
static COMMTIMEOUTS timeouts;

static void ShowLastError(void);

int serial_init(const char *port, unsigned long baud)
{
    char fullPort[20];
    DCB state;

    sprintf(fullPort, "\\\\.\\%s", port);

    hSerial = CreateFile(
        fullPort,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
        return FALSE;

    GetCommState(hSerial, &state);
    switch (baud) {
    case 9600:
        state.BaudRate = CBR_9600;
        break;
    case 19200:
        state.BaudRate = CBR_19200;
        break;
    case 38400:
        state.BaudRate = CBR_38400;
        break;
    case 57600:
        state.BaudRate = CBR_57600;
        break;
    case 115200:
        state.BaudRate = CBR_115200;
        break;
    default:
        return FALSE;
    }
    state.ByteSize = 8;
    state.Parity = NOPARITY;
    state.StopBits = ONESTOPBIT;
    state.fOutxDsrFlow = FALSE;
    state.fDtrControl = DTR_CONTROL_ENABLE;
    state.fOutxCtsFlow = FALSE;
    state.fRtsControl = RTS_CONTROL_DISABLE;
    state.fInX = FALSE;
    state.fOutX = FALSE;
    state.fBinary = TRUE;
    state.fParity = FALSE;
    state.fDsrSensitivity = FALSE;
    state.fTXContinueOnXoff = TRUE;
    state.fNull = FALSE;
    state.fAbortOnError = FALSE;
    SetCommState(hSerial, &state);

    GetCommTimeouts(hSerial, &original_timeouts);
    timeouts = original_timeouts;
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;

	/* setup device buffers */
	SetupComm(hSerial, 10000, 10000);

	/* purge any information in the buffer */
	PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    return TRUE;
}

void serial_done(void)
{
	FlushFileBuffers(hSerial);
    CloseHandle(hSerial);
}

/**
 * transmit a buffer
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to send
 * @returns zero on failure
 */
int tx(uint8_t* buff, int n)
{
    DWORD dwBytes = 0;
    if(!WriteFile(hSerial, buff, n, &dwBytes, NULL)){
        printf("Error writing port\n");
        ShowLastError();
        return 0;
    }
    return dwBytes;
}

/**
 * receive a buffer
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to read
 * @returns number of bytes read
 */
int rx(uint8_t* buff, int n)
{
    DWORD dwBytes = 0;
    SetCommTimeouts(hSerial, &original_timeouts);
    if(!ReadFile(hSerial, buff, n, &dwBytes, NULL)){
        printf("Error reading port\n");
        ShowLastError();
        return 0;
    }
    return dwBytes;
}

/**
 * receive a buffer with a timeout
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to read
 * @param timeout - timeout in milliseconds
 * @returns number of bytes read or SERIAL_TIMEOUT
 */
int rx_timeout(uint8_t* buff, int n, int timeout)
{
    DWORD dwBytes = 0;
    timeouts.ReadTotalTimeoutConstant = timeout;
    SetCommTimeouts(hSerial, &timeouts);
    if(!ReadFile(hSerial, buff, n, &dwBytes, NULL)){
        printf("Error reading port\n");
        ShowLastError();
        return 0;
    }
    return dwBytes > 0 ? dwBytes : SERIAL_TIMEOUT;
}

/**
 * hwreset ... resets Propeller hardware using DTR
 * @returns void
 */
void hwreset(void)
{
    EscapeCommFunction(hSerial, SETDTR);
    Sleep(100);
    EscapeCommFunction(hSerial, CLRDTR);
    Sleep(50);
    // Purge here after reset to get rid of buffered data. Prevents "Lost HW Contact 0 f9"
    PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

static unsigned long getms()
{
    LARGE_INTEGER ticksPerSecond;
    LARGE_INTEGER tick;   // A point in time
    LARGE_INTEGER time;   // For converting tick into real time
    // get the high resolution counter's accuracy
    QueryPerformanceFrequency(&ticksPerSecond);
    if(ticksPerSecond.QuadPart < 1000) {
        printf("Your system does not meet timer requirement. Try another computer. Exiting program.\n");
        exit(1);
    }
    // what time is it?
    QueryPerformanceCounter(&tick);
    time.QuadPart = (tick.QuadPart*1000/ticksPerSecond.QuadPart);
    return (unsigned long)(time.QuadPart);
}

/**
 * sleep for ms milliseconds
 * @param ms - time to wait in milliseconds
 */
void msleep(int ms)
{
    unsigned long t = getms();
    while((t+ms+10) > getms())
        ;
}

static void ShowLastError(void)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);
    printf("    %s\n", (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

/* console i/o functions for Unix/Linux courtesy of 'jazzed' */

int console_kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

int console_getch(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  return ch;
}

void console_putch(int ch)
{
    putchar(ch);
}
