#ifndef Motors_h
#define Motors_h


#include "Arduino.h"
#include "config.h"
//#include <avr/pgmspace.h>




/*
#define LATCH 4
#define LATCH_DDR DDRB
#define LATCH_PORT PORTB

#define CLK_PORT PORTD
#define CLK_DDR DDRD
#define CLK 4

#define ENABLE_PORT PORTD
#define ENABLE_DDR DDRD
#define ENABLE 7

#define SER 0
#define SER_DDR DDRB
#define SER_PORT PORTB
*/





class Motor
{
  public:
  Motor(byte numero, int speed_mult, int speed_div);
  int getSpeedDivisor();
  int getSpeedMultiplier();
  int getCurrentUstep();
  long getCurrentStep();
  
  void setSpeedDivisor(int temp);
  void setSpeedMultiplier(int temp);
  

  void incrementStep();

  
  

  private:
  void setCurrentStep(long temp);
  int speed_multiplier;
  int speed_divisor;
  int ustep_current;
  long step_current;
  byte no;

};
  static uint8_t latch_state;
  void powerCoil(byte coil_num, int power);
  void latchTx(void);
  void initMotors(void);
  void initInterrupt(unsigned int timer5_top);
  void changeTimebase(unsigned int timer5_top);





 


#endif

