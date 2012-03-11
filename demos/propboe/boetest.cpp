#include <stdio.h>
#include "boe.h"
#include "Pin.h"
#include "Term.h"

#define C3
//#define PROPBOE

/* pin definitions for C3 */
#ifdef C3
#define TV_PIN      12
#define VGA_PIN     16
#define VGA_ENABLE  15
#endif

/* pin definitions for the PropBOE */
#ifdef PROPBOE
#define VGA_PIN     8
#define PIR_PIN     15
#define PING_PIN    14
#define LSERVO_PIN  16
#define RSERVO_PIN  17
#endif

/* conversion factors for ping responses */
#define US_PER_CM_1000      29033   // 29.033 * 1000
#define US_PER_INCH_1000    ((US_PER_CM_1000 * 254) / 100)

static void test_pir(int pin);
static void test_ping(int pin);
static int ping(int pin);
static void calibrate_servo(int pin);

int main(void)
{
#ifdef TV_PIN
    TvTerm tv(TV_PIN);
	tv.str("Hello, world! (tv)\n");
#endif

#ifdef VGA_PIN
    VgaTerm vga(VGA_PIN);
	vga.str("Hello, world! (vga)\n");
#endif

#ifdef VGA_ENABLE
	Pin vgaEnable(VGA_ENABLE);
	vgaEnable.low();
#endif

    SerialTerm serial(stdout);
    serial.str("Hello, world! (serial)\n");

#ifdef PIR_PIN
    //test_pir(PIR_PIN);
#endif

#ifdef PING_PIN
    //test_ping(PING_PIN);
#endif

#ifdef LSERVO_PIN
    //calibrate_servo(LSERVO_PIN);
#endif

#ifdef LSERVO_PIN
    //calibrate_servo(RSERVO_PIN);
#endif

    for (;;)
        ;

    return 0;
}

static void test_pir(int pin)
{
    Pin pir(pin);
    
    pir.input();
    
    for (;;) {
    
        printf("Waiting for PIR\n");
        while (pir.get() == LOW)
            ;
            
        printf("PIR detected motion\n");
        while (pir.get() == HIGH)
            ;
    }
}

static void test_ping(int pin)
{
    for (;;) {
        printf("%d\n", (ping(pin) * 1000) / US_PER_INCH_1000);
        pause(1000);
    }
}

static int ping(int pin)
{
    Pin ping(pin);
    ping.low(); // make sure the pin starts low
    ping.pulseOut(5);
    return ping.pulseIn(HIGH);
}

static void calibrate_servo(int pin)
{
    Pin servo(pin);
    servo.low(); // make sure the pin starts low
    for (;;) {
        servo.pulseOut(1500);
        pause(20);
    }
}
