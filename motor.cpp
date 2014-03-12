#include "motor.h" 



void latchTx() 
{
  byte i;
  digitalWrite(MOTORLATCH, LOW);
  digitalWrite(MOTORDATA, LOW);

  for (i=0; i<8; i++) // - For each bit of latch_state:
  {
    digitalWrite(MOTORCLK, LOW);

    if (latch_state & (1<<(7-i)))
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
  latch_state = 0;
  latchTx();  // "reset"
  digitalWrite(MOTORENABLE, LOW); // enable the chip outputs!

}

void powerCoil(byte coil_num, int power)
{
  byte a, b, p;
    
  switch (coil_num) {
  case 1:
    a = C1_A;
    b = C1_B;
    C1_POWER=abs(power);
    //analogWrite(11,abs(power));
    break;
  case 2:
    a = C2_A;
    b = C2_B;
    C2_POWER=abs(power);
    //analogWrite(3,abs(power));
    break;
  case 3:
    a = C3_A;
    b = C3_B;
    C3_POWER=abs(power);
    //analogWrite(6,abs(power));
    break;
  case 4:
    a = C4_A;
    b = C4_B;
    C4_POWER=abs(power);
    //analogWrite(5,abs(power));
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
  
  //latchTx();
        
}


void initInterrupt(void)
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
   *       = 16MHz/(1kHz * 8 ) - 1
   *       = 1999
   *
   */  
  OCR5A = 1999;
  
  //Enable compare match A
  bitClear(TCCR5A,COM5A1);
  bitClear(TCCR5A,COM5A0);  

  // Finally enable the timer    
  bitSet(TIMSK5,OCIE5A);
}




