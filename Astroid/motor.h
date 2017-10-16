#ifndef MOTOR_h
#define MOTOR_h


#include "Arduino.h"

// Motor pin names
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8
#define C1_A 2
#define C1_B 3
#define C2_A 1
#define C2_B 4
#define C4_A 0
#define C4_B 6
#define C3_A 5
#define C3_B 7

//---NE PAS MODIFIER CETTE PARTIE---
#define MOTOR12_64KHZ _BV(CS20)  // no prescale
#define MOTOR12_8KHZ _BV(CS21)   // divide by 8
#define MOTOR12_2KHZ _BV(CS21) | _BV(CS20) // divide by 32
#define MOTOR12_1KHZ _BV(CS22)  // divide by 64

#define MOTOR34_64KHZ _BV(CS00)  // no prescale
#define MOTOR34_8KHZ _BV(CS01)   // divide by 8
#define MOTOR34_1KHZ _BV(CS01) | _BV(CS00)  // divide by 64
#define C1_POWER OCR1A
#define C2_POWER OCR3C
#define C3_POWER OCR4A
#define C4_POWER OCR3A

// Parametres moteurs :
#define freq12 MOTOR12_64KHZ  //frequence PWM
#define freq34 MOTOR34_64KHZ

// Courbe des micro-pas
const int CURVE_IN[]={      
    0, 2, 25, 120, 126, 128}; 
const int CURVE_OUT[]={   
    0, 110, 128, 220, 235, 242};

#define UPDATE_FREQ 500
#define UPDATE_TIME 0.002
#define UPDATE_TIME_MS 2


static uint8_t latch_state;
void powerCoil(byte coil_num, int power);
void latchTx(void);
void initMotors(void);
void initInterrupt(void);
void stopDE();
void stopHA();
void setHAStep(int i, float power_modul);
void setDEStep(int i, float power_modul);
int powerCurve(int i);









#endif


