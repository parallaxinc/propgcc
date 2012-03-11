#include <stdio.h>
#include "boe.h"
#include "Pin.h"
#include "Term.h"

/* pin definitions on my PropBOE */
#define PIR_PIN     15
#define PING_PIN    14
#define LSERVO_PIN  16
#define RSERVO_PIN  17

/* conversion factors for ping responses */
#define US_PER_CM_1000      29033   // 29.033 * 1000
#define US_PER_INCH_1000    ((US_PER_CM_1000 * 254) / 100)

static void test_pir(int pin);
static void test_ping(int pin);
static int ping(int pin);
static void calibrate_servo(int pin);

int main(void)
{
    SerialTerm term(stdout);

    term.str("Hello, world!\n");

    //test_pir(PIR_PIN);
    //test_ping(PING_PIN);
    calibrate_servo(LSERVO_PIN);
    //calibrate_servo(RSERVO_PIN);
    
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
