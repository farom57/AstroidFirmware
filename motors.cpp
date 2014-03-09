#ifndef Motors_c
#define Motors_c

#include "motors.h"

//class Functions
Motor::Motor(byte numero, int speed_mult, int speed_div)
{
  no = numero;
  step_current = 0;
  ustep_current = 0;
  setSpeedMultiplier(speed_mult);
  setSpeedDivisor(speed_div);
}

int Motor::getSpeedDivisor()
{
  return speed_divisor;
}

int Motor::getSpeedMultiplier()
{
  return speed_multiplier;
}

int Motor::getCurrentUstep()
{
  return ustep_current;
}

long Motor::getCurrentStep()
{
  return step_current;
}
  
void Motor::setSpeedDivisor(int temp)
{
  if (temp == 0)
  {
    Serial.println("Motor : Can't set speed divisor to 0");
  }
  else
  {
    speed_divisor = temp;
  }
}

void Motor::setSpeedMultiplier(int temp)
{
  speed_multiplier = temp;
}

/*
void Motor::SetUstepCurrent(int temp)
{
  ustep_current = temp;
}
*/

void Motor::setCurrentStep(long temp)
{
  step_current = temp;
}

void Motor::incrementStep()
{
  if (speed_divisor == 1 )
  {
    ustep_current += speed_multiplier;
  }
  else
  {
    static int i;
    i ++;
    if( i >= speed_divisor )
    {
      ustep_current += speed_multiplier;
      i = 0;
    }
  }
      
  while(ustep_current > 255)
  {
    step_current ++;
    ustep_current -= 256;
  }
  while(ustep_current < 0)
  {
    step_current --;
    ustep_current += 256;
  }
  powerCoil(no*2-1,(speed_multiplier == 0 ? 0 : pgm_read_word_near(UPAS_CURVE + ustep_current)));
  powerCoil(no*2,(speed_multiplier == 0 ? 0 : pgm_read_word_near(UPAS_CURVE + byte(ustep_current+64))));
}
  

//Service Functions

void latchTx() 
{
  byte i;
  digitalWrite(MOTORLATCH, LOW);
  digitalWrite(MOTORDATA, LOW);

  for (i=0; i<8; i++) // - For each bit of latch_state:
  {
    digitalWrite(MOTORCLK, LOW);

    if (latch_state & _BV(7-i))
    {
      digitalWrite(MOTORDATA, HIGH);
    } 
    else                   
    {
      digitalWrite(MOTORDATA, LOW);
    }
    
    digitalWrite(MOTORCLK, HIGH);
  }
  digitalWrite(MOTORLATCH, HIGH);
}







void initMotors()
{
  
  //init PWM1 (pin11)
  TCCR1A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc1a
  TCCR1B = (freq12 & 0x7) | _BV(WGM12);
  C1_POWER = 0;
  pinMode(11, OUTPUT);
  
  //init PWM2 (pin3)
  TCCR3A |= _BV(COM1C1) | _BV(WGM10); // fast PWM, turn on oc3c
  TCCR3B = (freq12 & 0x7) | _BV(WGM12);
  C2_POWER = 0;
  pinMode(3, OUTPUT);
  
  //init PWM3 (pin6)
  TCCR4A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc4a
  TCCR4B = (freq34 & 0x7) | _BV(WGM12);
  C3_POWER = 0;
  pinMode(6, OUTPUT);
  
  //init PWM4 (pin5)
  TCCR3A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc3a
  TCCR3B = (freq34 & 0x7) | _BV(WGM12);
  C4_POWER = 0;
  pinMode(5, OUTPUT);
  
  //enable latch
  pinMode(MOTORLATCH, OUTPUT);
  pinMode(MOTORENABLE, OUTPUT);
  pinMode(MOTORDATA, OUTPUT);
  pinMode(MOTORCLK, OUTPUT);
  //latch_state = 0;
  //latchTx();  // "reset"
  digitalWrite(MOTORENABLE, LOW); // enable the chip outputs!

}

void powerCoil(byte coil_num, int power)
{
  byte a, b, p;
    
  switch (coil_num) {
  case 1:
    a = C1_A;
    b = C1_B;
    C1_POWER= abs(power);
    break;
  case 2:
    a = C2_A;
    b = C2_B;
    C2_POWER= abs(power);
    break;
  case 3:
    a = C3_A;
    b = C3_B;
    C3_POWER= abs(power);
    break;
  case 4:
    a = C4_A;
    b = C4_B;
    C4_POWER= abs(power);
    break;
  default:
    return;
  }
  
  if (power == 0)
  {
    latch_state &= ~_BV(a); // a = LOW, b = LOW
    latch_state &= ~_BV(b); 
  }
  else if(power > 0)
  {
    latch_state |= _BV(a); // a = HIGH, b = LOW
    latch_state &= ~_BV(b);
  }
  else
  {
    latch_state &= ~_BV(a); // a = LOW, b = HIGH
    latch_state |= _BV(b);
  }
  
  latchTx();
        
}








void initInterrupt(unsigned int timer5_top)
{
  /* First disable the timer overflow interrupt while we're configuring */  
  bitClear(TIMSK5,TOIE5);  
  
 
  //configure TIMER5 in CTC mode
  bitClear(TCCR5A,WGM51);
  bitClear(TCCR5A,WGM50);  
  bitSet(TCCR5B,WGM52);
  bitClear(TCCR5B,WGM53);  
  
  ///* Select clock source: internal I/O clock */  
  //ASSR &= ~(1<<AS2);  
   
  
  /* Now configure the prescaler to CPU clock divided by 8 */  
  bitClear(TCCR5B,CS52); 
  bitSet(TCCR5B,CS51);
  bitClear(TCCR5B,CS50);    
  
  /* We need to calculate a proper value to load the timer counter. 
   * The following loads the value xxx into the Timer 5 counter register 
   * The math behind this is: 
   * OCR5A = Fclk/(Fdesired * prescaler) -1
   *       = 16MHz/(64.175 * 8 ) - 1
   *       = 31162  
   * 
   * With Fdesired = ( uStep by day ) / (lengh of an sideral day)
   *               = 144*200*3*64/86164,09890369
   *               = 64.175
   */  
  //OCR5A = 31162;
  OCR5A = timer5_top;
  
  //Enable compare match A
  bitClear(TCCR5A,COM5A1);
  bitClear(TCCR5A,COM5A0);  

  // Finally enable the timer    
  bitSet(TIMSK5,OCIE5A);
}




void changeTimebase(unsigned int timer5_top)
{
  static unsigned int old_timer5_top = 31162;
  if(old_timer5_top != timer5_top)
  {
    bitClear(TIMSK5,OCIE5A);
    OCR5A = timer5_top;
    bitSet(TIMSK5,OCIE5A);
    old_timer5_top = timer5_top;
  }
}







#endif
