#include <adk.h>
#include <usbhub.h>
#include "com.h"
#include "motor.h"
#include <Streaming.h>


// USB
USB Usb;
USBHub hub0(&Usb);
USBHub hub1(&Usb);
ADK adk(&Usb,"Farom",
            "Mega_ADK",
            "Mega_ADK",
            "1.0",
            "http://www.android.com",
            "0000000012345678");

#define SIDERAL_RATE  86400./86164./4.
#define TIMEOUT_STATE 200



long ms_count=0;

long step_ha=0;
long step_de=0;
float ustep_ha=0;
float ustep_de=0;
float move_speed_ha=1.;
float move_speed_de=0;
float power_ha=1.;
float power_de=1.;

unsigned int ticks_servo = 0;


#define ST4_S 24
#define ST4_E 26
#define ST4_W 28
#define ST4_N 30

#define PIN_SERVO_EXPOSE A0
#define PIN_ENABLE_SERVO A5
#define PIN_FOCUS A3




//Interrupt Service Routine (ISR) for Timer5 overflow at 1kHz
ISR(TIMER5_COMPA_vect ){ 
    ms_count += UPDATE_TIME_MS;
    
    if(ms_count % 20 < UPDATE_TIME_MS){
      if(ticks_servo>=10){
        digitalWrite(PIN_SERVO_EXPOSE,HIGH);
      }
    }

    float speed_ha=SIDERAL_RATE*((!digitalRead(ST4_W)?0.5:0.) + (!digitalRead(ST4_E)?-0.5:0.)+move_speed_ha);
    float speed_de=SIDERAL_RATE*((!digitalRead(ST4_N)?0.5:0.) + (!digitalRead(ST4_S)?-0.5:0.)+move_speed_de);

    ustep_ha+=speed_ha*UPDATE_TIME*1024;
    ustep_de+=speed_de*UPDATE_TIME*1024;

    if(ustep_ha>=1024.){
        ustep_ha-=1024.;
        step_ha++;
    }
    if(ustep_ha<0.){
        ustep_ha+=1024.;
        step_ha--;
    }
    if(ustep_de>=1024.){
        ustep_de-=1024.;
        step_de++;
    }
    if(ustep_de<0.){
        ustep_de+=1024.;
        step_de--;
    }

    unsigned int ustep_ha_int = ustep_ha;
    unsigned int ustep_de_int = ustep_de;



      setHAStep(ustep_ha_int, power_ha);
      setDEStep(ustep_de_int, power_de);

    
    latchTx();
    
    if(ms_count % 20 <= UPDATE_TIME_MS && ticks_servo>=10){
      unsigned int temp = TCNT5;
      while(ticks_servo > temp){
        temp = TCNT5;
      }
      digitalWrite(PIN_SERVO_EXPOSE,LOW);
    }
}



void setup(void){
  initInterrupt();
  initMotors();
  
  
  // ST4
  pinMode(ST4_S, INPUT_PULLUP);
  pinMode(ST4_N, INPUT_PULLUP);
  pinMode(ST4_W, INPUT_PULLUP);
  pinMode(ST4_E, INPUT_PULLUP);
  
  // Serial
  Serial.begin(9600);
  
  // USB
  if (Usb.Init() == -1) {
    Serial.println("OSCOKIRQ failed to assert");
  }
  
  pinMode(PIN_ENABLE_SERVO, INPUT);
  pinMode(PIN_FOCUS, INPUT);
  pinMode(PIN_SERVO_EXPOSE, INPUT);
}



void processServoExpose(){
  switch(ticks_servo){
    case 0: // everything disabled
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, INPUT);
      pinMode(PIN_SERVO_EXPOSE, INPUT);
      break;
      
    case 1: // canon focus
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, OUTPUT);
      pinMode(PIN_SERVO_EXPOSE, INPUT);
      digitalWrite(PIN_FOCUS,LOW);
      break;
    case 2: // canon expose
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, INPUT);
      pinMode(PIN_SERVO_EXPOSE, OUTPUT);
      digitalWrite(PIN_SERVO_EXPOSE,LOW);
      break;
    case 3: // canon focus and expose
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, OUTPUT);
      pinMode(PIN_SERVO_EXPOSE, OUTPUT);
      digitalWrite(PIN_FOCUS,LOW);
      digitalWrite(PIN_SERVO_EXPOSE,LOW);
      break;
      
    case 4: // reserved
    case 5: // reserved
    case 6: // reserved
    case 7: // reserved
    case 8: // reserved, the following code is executed for ticks servo for 4 to 8
      ticks_servo = 9;
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, INPUT);
      pinMode(PIN_SERVO_EXPOSE, INPUT);
      break;
    case 9: // ticks_servo is set to 9 whenan incorrect value is set (4-8 or above 3500)
      pinMode(PIN_ENABLE_SERVO, INPUT);
      pinMode(PIN_FOCUS, INPUT);
      pinMode(PIN_SERVO_EXPOSE, INPUT);
      break;
    default:      
      if(ticks_servo>3500){
        ticks_servo = 9;
        pinMode(PIN_ENABLE_SERVO, INPUT);
        pinMode(PIN_FOCUS, INPUT);
        pinMode(PIN_SERVO_EXPOSE, INPUT);
      }else{
        pinMode(PIN_ENABLE_SERVO, OUTPUT);
        pinMode(PIN_FOCUS, INPUT);
        pinMode(PIN_SERVO_EXPOSE, OUTPUT);
        digitalWrite(PIN_ENABLE_SERVO,LOW);
        digitalWrite(PIN_SERVO_EXPOSE,LOW);
      }
      break;
  }
}
  

void loop(void){
    static long t_last_state = 0;
    if(ms_count >= t_last_state + TIMEOUT_STATE){
        t_last_state = ms_count;
        sendStatus();
    }
    
    receiveCommand();
    //processCommand();

}




