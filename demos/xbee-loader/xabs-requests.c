#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <propeller.h>

#ifdef SIMPLE_LIB
#include "simpletools.h"
#include "adcDCpropAB.h"
#include "servo.h"
#endif

#include "xbee-server.h"

#define OPTIONS_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Methods: GET, POST, OPTIONS, XPING, XABS\r\n\
Access-Control-Allow-Headers: XPING, XABS\r\n\
Access-Control-Max-Age: 1000000\r\n\
Keep-Alive: timeout=1, max=100\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n"

#define CANNED_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 1\r\n\
\r\n\
0"

#define XPING_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 6\r\n\
\r\n\
Hello!"

uint8_t *pkt_ptr;
int pkt_len;

void skip_spaces(void)
{
    while (pkt_len > 0 && *pkt_ptr == ' ') {
        --pkt_len;
        ++pkt_ptr;
    }
}

void  skip_white(void)
{
    while (pkt_len > 0 && isspace(*pkt_ptr)) {
        --pkt_len;
        ++pkt_ptr;
    }
}

int match(char *str)
{
    uint8_t *ptr;
    int len;
    skip_white();
    ptr = pkt_ptr;
    len = pkt_len;
    while (*str) {
        if (--len < 0 || *ptr++ != *str++)
            return FALSE;
    }
    pkt_ptr = ptr;
    pkt_len = len;
    return TRUE;
}

int token(char *str)
{
    int n = 0;
    skip_white();
    while(pkt_len > 0 && !isspace(*pkt_ptr)) {
        str[n] = *(pkt_ptr++);
        --pkt_len;
        n++;
    }
    str[n] = 0;
    return n;
}

static void handle_options_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT) {
        send_response(sock, (uint8_t *)OPTIONS_RESPONSE, sizeof(OPTIONS_RESPONSE) - 1);
    }
}

/*
 * ADC code
 */
static void high(int pin)
{
    int mask = (1 << pin);
    DIRA |= mask;
    OUTA |= mask;
}

static void low(int pin)
{
    int mask = (1 << pin);
    DIRA |= mask;
    OUTA &= ~mask;
}

static void input(int pin)
{
    DIRA &= ~(1 << pin);
}

static void set_output(int pin, int val)
{
    int mask = (1 << pin);
    DIRA |= mask;
    if(val) {
        OUTA |= mask;
    } else {
        OUTA &= ~mask;
    }
}

static unsigned int get_output(int pin)       // getOutput function definition
{
  return (OUTA >> pin) & 1;                   // Return OUTA shifted rt & masked
}

static int get_state(int pin)
{
    return (INA >> pin) & 1;
}

static char* itoa(int i, char b[], int base){
  char const digit[] = "0123456789ABCDEF";
  char* p = b;
  if(i<0){
    *p++ = '-';
    i = -1;
  }
  int shifter = i;
  do{ //Move to where representation ends
    ++p;
    shifter = shifter/base;
  }while(shifter);
  *p = '\0';
  do{ //Move back, inserting digits as u go
    *--p = digit[i%base];
    i = i/base;
  }while(i);
  return b;
}

static int dout, din, scl, cs;

int adc124S021dc(int channel);

void adc_init(int csPin, int sclPin, int doPin, int diPin)
{
  dout = doPin;
  din = diPin;
  scl = sclPin;
  cs = csPin;  
}

int adc_in(int channel)
{
  int val = 0;
  adc124S021dc(channel);
  val = adc124S021dc(channel);
  return val;
}

char *adc_str(int channel, char *str)
{
    int val = adc_in(channel);
    itoa(val, str, 10);
    //return ((float) adc_in(channel)) * 5.0 / 4096.0;
    return str;
}

int adc124S021dc(int channel)
{
  int val = 0;
  int i = 0;
  channel = (channel & 3) << 12;
  high(cs);
  high(scl);
  low(din);
  input(dout);
  low(cs);
  for(i = 15; i >= 0; i--)
  {
    val = val << 1;
    low(scl);
    high(scl);
    set_output(din, (channel >> i) & 1);
    val = val + (get_state(dout) & 1);
  }
  high(cs);
  return val;
}  

/*
 * Servo support functions
 */

static long iodt = 0;

static void set_io_dt(long clockticks)                 // setIoDt function definition
{
  iodt = clockticks;
}

static long t_timeout = 0;

static void set_io_timeout(long clockTicks)          // setTimeout function definition
{
  t_timeout = clockTicks;
}

static long t_mark = 0;

static void mark(void)                               // pause function definition
{
  // If dt not initialized, set it up to 1 us.
  if(iodt == 0)                               // If dt not initialized
  {
    set_io_dt(CLKFREQ/1000000);               // Set up timed I/O time increment
    set_io_timeout(CLKFREQ/4);                // Set up timeout
  }
  t_mark = CNT;
}

void wait(int time)                          // pause function definition
{
  // If dt not initialized, set it up to 1 us.
  if(!iodt)                                  // If dt not initialized
  {
    set_io_dt(CLKFREQ/1000000);              // Set up timed I/O time increment
    set_io_timeout(CLKFREQ/4);               // Set up timeout
  }
  time *= iodt;                              // Calculate system clock ticks
  waitcnt(t_mark += time);                   // Wait for system clock target
}

static void pulse_out(int pin, int time)              // pulseOut function definition
{
  if(iodt == 0)
  {
    set_io_dt(CLKFREQ/1000000);
    set_io_timeout(CLKFREQ/4);
  }
  signed long phsVal = -time * iodt;
  //int ctr = 0;
  int frq = 1;
  int phs = 0;
  int state = get_output(pin);
  if(state == 1)
  {
    phsVal = -phsVal;
    phs = -1;
    frq = -1;
  }
  if (CTRA == 0)
  {
    PHSA = phs;
    FRQA = frq;
    CTRA = pin;
    CTRA += (4 << 26);
    low(pin);
    PHSA = phsVal;
    while(get_state(pin) != state);
    set_output(pin, state);
    CTRA = 0;
  }
  else if (CTRB == 0)
  {
    PHSA = phs;
    FRQA = frq;
    CTRA = pin;
    CTRA += (4 << 26);
    low(pin);
    PHSA = phsVal;
    while(get_state(pin) != state);
    set_output(pin, state);
    CTRA = 0;
  }
}


/*
 * Servo Code
 */

static unsigned int stack[(160 + (64 * 4)) / 4];     // Stack          

static volatile int p[7] = {-1, -1, -1, -1,          // I/O pins
                     -1, -1, -1};
static volatile int t[7] = {-1, -1, -1, -1,          // Current iteration pulse widths
                     -1, -1, -1};
static volatile int tp[7] = {-1, -1, -1, -1,         // Previous iteration pulse widhts
                      -1, -1, -1};
static volatile int r[7] = {2000, 2000, 2000, 2000,  // Step sizes initialized to 2000
                     2000, 2000, 2000};

static int servoCog = 0;

static void servo(void *par);

static void servo_stop(void)                          // Stop servo process, free a cog
{
  if(servoCog)                                // If the cog is running
  {
    cogstop(servoCog-1);                      // Stop it
    servoCog = 0;                             // Remember that it's stopped
  }
}

static int servo_start(void)                          // Take cog & start servo process
{
  servo_stop();                                // Stop in case cog is running
  servoCog = cogstart(&servo, NULL, stack,    // Launch servo into new cog
             sizeof(stack)) + 1;
  return servoCog;                            // Return cog that was taken
}

static int servo_set(int pin, int time)               // Set pulse width to servo on pin 
{
  if(servoCog == 0)                           // If cog not started
  {
    int result = servo_start();                // Start the cog
    if(result == 0) return 0;                 // No cogs open
    if(result == -1) return -1;               // No locks open
  }
  int s = sizeof(p)/sizeof(int);              // Array size to s
  int i;                                      // Index variable
  //while(lockset(lockID));                     // Set the lock
  for(i = 0; i < s; i++)                      // Check if existing servo
  {
    if(p[i] == pin)                           // Yes?
    {
      t[i] = time;                            // Set pulse duration
      //lockclr(lockID);                        // Clear lock
      return 1;                               // Return success 
    }
  }
  if(i == s)                                  // Not existing servo?
  {
    for(i= 0; i < s; i++)                     // Look for empty slot
    {
      if(p[i]==-1)                            // Found one?
      {
        break;                                // Exit for loop, keep index
      }
    }
    if(i <= s)                                // Found empty slot?
    {
      p[i] = pin;                             // Set up pin and pulse durations
      t[i] = time;
      tp[i] = time;
      //lockclr(lockID);                        // Clear the lock 
      return 1;                               // Return success 
    }
    else                                      // Servo not found, no empty slots?
    {
      //lockclr(lockID);                        // Clear lock
      return 0;                               // Return, set not completed
    }
  }
  return 0;
}

static int servo_speed(int pin, int speed)            // Set continuous rotation speed
{
  return servo_set(pin, speed + 1500);         // Add center pulse width to speed
}

static int servo_setramp(int pin, int stepSize)      // Set ramp step for a servo
{
  int s = sizeof(p)/sizeof(int);              // Get array size
  int i;                                      // Local index variable
  //while(lockset(lockID));                     // Set lock
  for(i = 0; i < s; i++)                      // Find index for servo pin
  {
    if(p[i] == pin)                           // Found pin in array?
    {
      r[i] = stepSize;                        // Set ramp step
      //lockclr(lockID);                        // Clear lock
      return 1;                               // Return success
    }
  }
  //lockclr(lockID);                            // Clear lock
  return 0;                                  // Return -1, pin not found
}

static void servo(void *par)                         // Servo process in other cog
{
  int s = sizeof(p)/sizeof(int);              // Get size of servo array
  int i;                                      // Local index variable
  mark();                                     // Mark the current time
  while(1)                                    // Servo control loop
  {
    //while(lockset(lockID));                   // Set the lock 
    for(i = 0; i < s; i++)                    // Go through all possible servos
    {
      if(t[i] == 0)                           // Detach servo? 
      {
        input(p[i]);                          // Set I/O pin to input
        p[i] = -1;                            // Remove from list
        t[i] = -1;
      }
      if(p[i] != -1)                          // If servo entry in pin array
      {
        int tPulse =  t[i];                   // Copy requested position to var
        int diff = tPulse - tp[i];            // Check size of change
        int d = abs(diff);                    // Take absolute value
        if(r[i] < d)                          // If change larger than ramp step
        {
          int step = r[i];                    // Copy step entry to variable
          if(diff < 0)                        // If negative
          {
            step = -step;                     // Make step negative
          }
          tPulse = tp[i] + step;              // Increment pulse by step 
        }
        pulse_out(p[i], tPulse);               // Send pulse to servo
        tp[i] = tPulse;                       // Remember pulse for next time
      }
    }
    //lockclr(lockID);                          // Clear the lock
    wait(20000);
  }
}

/*
 * Request Handler
 */

#define RESP_NUM_START 65

static void handle_xabs_request(Socket_t *sock, int phase)
{
    int  num    = 0;
    int  left   = 0;
    int  right  = 0;

    /*
     * resp[RESP_NUM_START...] will be overwritten for some return values like XABS ADC.
     * NNN starts at 69. CCCCCCCCCC starts at 80
     */
    char *resp = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: NNN\r\n\r\nCCCCCCCCCC";
    char tmp[15]; // short itoa space
    int  len    = 0;

    char str[80]; // string space
    
    if (phase == HP_CONTENT)
    {
        // we have content!

        pkt_ptr = sock->content;
        pkt_len = sock->length;

        if (match("ADC"))
        {
            puts("ADC");
            if(token(str)) {
                num = atoi(str);
            }
            if(num > -1 && num < 4) {
                adc_init(21, 20, 19, 18);
                adc_str(num, str);
                len = strlen(str);
                if(len > 0) {
                    itoa(len, tmp, 10);
                    resp[RESP_NUM_START] = '\0';
                    strcat(resp, tmp);
                    strcat(resp, "\r\n\r\n");
                    strcat(resp, str);
                    strcat(resp, "");
                    //puts(resp);
                    send_response(sock, (uint8_t *)resp, strlen(resp) - 1);
                    return;
                }
            }
            // good exit point above sends data back
            //
            send_response(sock, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE) - 1);
        }

        /*
         * COMMAND: XABS SERVO [RUN|START|STOP] #LEFT #RIGHT
         */
        else

        if (match("SERVO"))
        { 
            // got servo

            puts("SERVO");
            if (match("START")) {
              puts("START");
              servo_start();
            }
            else if (match("STOP")) {
              puts("STOP");
              servo_stop();
            }
            else if (match("RUN")) {
                puts("RUN");
                if(token(str)) {
                    puts(str);
                    left = atoi(str);
                }
                if(token(str)) {
                    puts(str);
                    right = atoi(str);
                }
                servo_setramp(16, 1); // Change by up to 7/10 degree/20 ms 
                servo_setramp(17, 1); // Change by up to 7/10 degree/20 ms
                servo_speed(16, left); 
                servo_speed(17, right);
            }
            send_response(sock, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE) - 1);
        }
        /*
         * COMMAND: XABS PIN # [HIGH|LOW|TOGGLE|INPUT]
         */
        else
        if (match("PIN"))
        {
            // got pin
            puts("PIN");

            if(token(str)) {
                num = atoi(str);
            }
            skip_spaces();
            if (match("HIGH")) {
                DIRA |= 1 << num;
                OUTA |= 1 << num;
            }
            else if (match("LOW")) {
                DIRA |= 1 << num;
                OUTA &= ~(1 << num);
            }
            else if (match("TOGGLE")) {
                DIRA |= 1 << num;
                OUTA ^= 1 << num;
            }
            else if (match("INPUT")) {
                DIRA &= ~(1 << num);
            }
            send_response(sock, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE - 1));
        }
    }
}

static void handle_xping_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT) {
        DIRA &= ~(3 << 26);
        servo_stop();
        send_response(sock, (uint8_t *)XPING_RESPONSE, sizeof(XPING_RESPONSE) - 1);
    }
}

MethodBinding_t methodBindings[] = {
{   "OPTIONS",  handle_options_request  },
{   "XABS",     handle_xabs_request     },
{   "XPING",    handle_xping_request    },
{   NULL,       NULL                    }
};
