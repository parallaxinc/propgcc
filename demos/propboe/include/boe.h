#ifndef __BOE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* pin directions */
#define IN      0
#define OUT     1

/* pin states */
#define HIGH    1
#define LOW     0

void input(int pin);
void output(int pin);
int getPin(int pin);
void setPin(int pin, int value);
void high(int pin);
void low(int pin);
void toggle(int pin);
void pause(int milliseconds);
int pulseIn(int pin, int state);
void pulseOut(int pin, int duration);

#if defined(__cplusplus)
}
#endif

#endif
