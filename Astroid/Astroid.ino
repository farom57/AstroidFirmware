
#include "com.h"



#define SIDERAL_RATE  144.*3.*200./4./86164.


// Motor pin names
#define X_STEP_PIN         15
#define X_DIR_PIN          21
#define Y_STEP_PIN         22
#define Y_DIR_PIN          23
#define Z_STEP_PIN         3
#define Z_DIR_PIN          2
#define E0_STEP_PIN        1
#define E0_DIR_PIN         0
#define XYE0_ENABLE_PIN    14
#define Z_ENABLE_PIN       26
#define LED_PIN            27
#define FAN_PIN            4 
#define HEATER_END_PIN     13
#define HEATER_BED_PIN     12
#define TEMP_0_PIN          7   // Analogue pin
#define TEMP_BED_PIN        6   // Analogue pin
#define X_MIN_PIN          18
#define Y_MIN_PIN          19
#define Z_MIN_PIN          20

// timerinterupt speed
#define UPDATE_FREQ 2500L
#define UPDATE_TIME 1./UPDATE_FREQ
#define UPDATE_TIME_MS 1000./UPDATE_FREQ

#define TIMEOUT_STATE 0.1*UPDATE_FREQ

//Choose which motor correspond to DE and HA
#define HA_STEP_PIN    Z_STEP_PIN
#define HA_DIR_PIN     Z_DIR_PIN 
#define HA_ENABLE_PIN  Z_ENABLE_PIN 
#define DE_STEP_PIN    Y_STEP_PIN
#define DE_DIR_PIN     Y_DIR_PIN 
#define DE_ENABLE_PIN  XYE0_ENABLE_PIN 
#define FOCUS_STEP_PIN E0_STEP_PIN
#define FOCUS_DIR_PIN  E0_DIR_PIN 
#define AUX1_PIN       HEATER_BED_PIN
#define AUX2_PIN       HEATER_END_PIN
#define AUX3_PIN       FAN_PIN
#define BULB_PIN       X_MIN_PIN

const unsigned int random_seq[]=
{
  8,85,27,55,221,136,153,233,190,219,122,89,91,81,156,5,25,29,108,1,160,96,140,
  114,42,238,197,145,74,137,4,78,146,14,181,172,38,44,127,141,103,9,86,43,191,149,
  70,49,180,95,165,173,176,129,188,123,52,100,198,237,69,7,239,174,143,79,184,113,
  16,185,224,182,163,105,20,57,204,187,134,161,104,217,242,231,166,34,251,12,133,
  112,11,179,256,154,107,222,83,216,88,152,249,24,201,102,60,26,115,67,53,225,116,
  144,230,73,41,56,22,71,45,30,124,223,139,214,32,28,93,245,54,106,130,229,162,
  212,126,110,17,170,213,234,167,6,98,228,19,215,75,244,117,87,132,99,82,135,84,
  189,247,203,61,31,109,120,157,92,183,206,51,158,3,205,138,58,218,15,148,192,209,
  46,142,125,159,66,101,68,76,97,248,255,232,220,226,210,236,119,246,171,23,211,
  77,121,63,39,10,2,128,111,65,13,208,62,196,243,175,169,48,94,131,186,193,21,37,
  90,177,80,47,235,72,194,40,254,227,164,50,59,147,250,178,199,18,200,36,253,241,
  35,207,252,150,64,195,155,151,168,118,240,202,33};

long clock=0;

float ustep_speed_ha=SIDERAL_RATE*UPDATE_TIME*64.;
float ustep_speed_de=0;
float ustep_speed_focus=0;
long ustep_ha=0;
long ustep_de=0;
long ustep_focus=0;
float move_speed_ha=1.;
float move_speed_de=0;
float move_speed_focus=0;
float power_ha=1.;
float power_de=0.;
float power_focus=0.;
float ha_ustep_fraction=0;
float de_ustep_fraction=0;
float focus_ustep_fraction=0;
unsigned int power_aux1=0;
unsigned int power_aux2=0;
unsigned int power_aux3=0;
byte bulb_state=0;
int counter_aux1=0;
int counter_aux2=0;
int counter_aux3=0;


//Interrupt Service Routine (ISR) for Timer3 overflow
ISR(TIMER3_COMPA_vect ){ 
  clock += 1;

  ha_ustep_fraction += ustep_speed_ha;
  de_ustep_fraction += ustep_speed_de;
  focus_ustep_fraction += ustep_speed_focus;





  while(ha_ustep_fraction>=1.){
    ha_ustep_fraction-=1.;
    digitalWrite(HA_STEP_PIN, LOW);
    digitalWrite(HA_DIR_PIN, power_ha>0);
    delayMicroseconds(2);
    digitalWrite(HA_STEP_PIN, HIGH);
    ustep_ha++;
  }

  while(ha_ustep_fraction<=-1.){
    ha_ustep_fraction+=1.;
    digitalWrite(HA_STEP_PIN, LOW);
    digitalWrite(HA_DIR_PIN, power_ha<0);
    delayMicroseconds(2);
    digitalWrite(HA_STEP_PIN, HIGH);
    ustep_ha--;
  }

  while(de_ustep_fraction>=1.){
    de_ustep_fraction-=1.;
    digitalWrite(DE_STEP_PIN, LOW);
    digitalWrite(DE_DIR_PIN, power_de>0);
    delayMicroseconds(2);
    digitalWrite(DE_STEP_PIN, HIGH);
    ustep_de++;
  }

  while(de_ustep_fraction<=-1.){
    de_ustep_fraction+=1.;
    digitalWrite(DE_STEP_PIN, LOW);
    digitalWrite(DE_DIR_PIN, power_de<0);
    delayMicroseconds(2);
    digitalWrite(DE_STEP_PIN, HIGH);
    ustep_de--;
  }
  
    while(focus_ustep_fraction>=1.){
    focus_ustep_fraction-=1.;
    digitalWrite(FOCUS_STEP_PIN, LOW);
    digitalWrite(FOCUS_DIR_PIN, power_focus>0);
    delayMicroseconds(2);
    digitalWrite(FOCUS_STEP_PIN, HIGH);
    ustep_focus++;
  }

  while(focus_ustep_fraction<=-1.){
    focus_ustep_fraction+=1.;
    digitalWrite(FOCUS_STEP_PIN, LOW);
    digitalWrite(FOCUS_DIR_PIN, power_focus<0);
    delayMicroseconds(2);
    digitalWrite(FOCUS_STEP_PIN, HIGH);
    ustep_focus--;
  }
  delayMicroseconds(1);

  
}

void initMotors()
{
  // X
  pinMode(X_STEP_PIN, OUTPUT); 
  pinMode(X_DIR_PIN, OUTPUT); 
  pinMode(XYE0_ENABLE_PIN, OUTPUT); // X Y and E0 enable pin is common
  digitalWrite(DE_ENABLE_PIN, HIGH); // it's a !ENABLE pin, OFF by default

  // Y
  pinMode(Y_STEP_PIN, OUTPUT); 
  pinMode(Y_DIR_PIN, OUTPUT); 

  // E0
  pinMode(E0_STEP_PIN, OUTPUT); 
  pinMode(E0_DIR_PIN, OUTPUT); 

  // Z
  pinMode(Z_STEP_PIN, OUTPUT); 
  pinMode(Z_DIR_PIN, OUTPUT); 
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(HA_ENABLE_PIN, LOW); // it's a !ENABLE pin 

}

void initAux()
{
  pinMode(AUX1_PIN, OUTPUT);  
  pinMode(AUX2_PIN, OUTPUT);  
  pinMode(AUX3_PIN, OUTPUT);  
  pinMode(BULB_PIN, INPUT);

  digitalWrite(AUX1_PIN, LOW);
  digitalWrite(AUX2_PIN, LOW);
  digitalWrite(AUX3_PIN, LOW);
  digitalWrite(BULB_PIN, LOW);
  
  bitClear(TCCR0B,CS02); 
  bitClear(TCCR0B,CS01);
  bitSet(TCCR0B,CS00); 
 
  bitClear(TCCR1B,CS12); 
  bitClear(TCCR1B,CS11);
  bitSet(TCCR1B,CS10);  
}

void initInterrupt(void)
{
  /* First disable the timer overflow interrupt while we're configuring */
  bitClear(TIMSK3,TOIE3);  


  //configure TIMER1 in CTC mode
  bitClear(TCCR3A,WGM31);
  bitClear(TCCR3A,WGM30);  
  bitSet(TCCR3B,WGM32);
  bitClear(TCCR3B,WGM33);  

  ///* Select clock source: internal I/O clock */  
  //ASSR &= ~(1<<AS2);  

  /* Now configure the prescaler to CPU clock divided by 8 */
  bitClear(TCCR3B,CS32); 
  bitSet(TCCR3B,CS31);
  bitClear(TCCR3B,CS30);    

  /* We need to calculate a proper value to load the timer counter. 
   * The following loads the value xxx into the Timer 3 counter register 
   * The math behind this is: 
   * OCR5A = Fclk/(Fdesired * prescaler) -1
   *       = 16MHz/(1kHz * 8 ) - 1
   *       = 1999
   *
   */
  OCR3A = 16000000L/(UPDATE_FREQ*8) - 1;

  //Enable compare match A
  bitClear(TCCR3A,COM3A1);
  bitClear(TCCR3A,COM3A0);  

  // Finally enable the timer    
  bitSet(TIMSK3,OCIE3A);
  bitSet(TIMSK3,TOIE3); 
}


void setup(void){
  initInterrupt();
  initMotors();
  initAux();

  // Serial
  Serial.begin(9600);
  Serial1.begin(9600);


}



void loop(void){
  static long clock_state = 0;
  

  receiveCommand();

  if(clock >= clock_state + TIMEOUT_STATE){
    clock_state = clock;
    digitalWrite(LED_PIN, HIGH);
    sendStatus();
    digitalWrite(LED_PIN, LOW);
  }


  

  if(bulb_state)
  {
    digitalWrite(LED_PIN, LOW);
    pinMode(BULB_PIN, OUTPUT);
  }
  else{
    pinMode(BULB_PIN, INPUT);
  }


}



