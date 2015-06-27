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

long step_ra=0;
long step_de=0;
float ustep_ra=0;
float ustep_de=0;
float move_speed_ra=0;
float move_speed_de=0;

unsigned int ticks_servo = 4500;


#define ST4_S 24
#define ST4_E 26
#define ST4_W 28
#define ST4_N 30

#define PIN_SERVO A3
#define PIN_ENABLE_SERVO A5



//Interrupt Service Routine (ISR) for Timer5 overflow at 1kHz
ISR(TIMER5_COMPA_vect ){ 
    ms_count += UPDATE_TIME_MS;
    
    if(ms_count % 20 < UPDATE_TIME_MS){
      digitalWrite(PIN_SERVO,HIGH);
    }

    float speed_ra=-SIDERAL_RATE*(1. + (!digitalRead(ST4_W)?0.5:0.) + (!digitalRead(ST4_E)?-0.5:0.)+move_speed_ra);
    float speed_de=SIDERAL_RATE*((!digitalRead(ST4_N)?0.5:0.) + (!digitalRead(ST4_S)?-0.5:0.)+move_speed_de);

    ustep_ra+=speed_ra*UPDATE_TIME*1024;
    ustep_de+=speed_de*UPDATE_TIME*1024;

    if(ustep_ra>=1024.){
        ustep_ra-=1024.;
        step_ra++;
    }
    if(ustep_ra<0.){
        ustep_ra+=1024.;
        step_ra--;
    }
    if(ustep_de>=1024.){
        ustep_de-=1024.;
        step_de++;
    }
    if(ustep_de<0.){
        ustep_de+=1024.;
        step_de--;
    }

    unsigned int ustep_ra_int = ustep_ra;
    unsigned int ustep_de_int = ustep_de;

    setRAStep(ustep_ra_int);
    setDEStep(ustep_de_int);

    latchTx();
    
    if(ms_count % 20 <= UPDATE_TIME_MS){
      unsigned int temp = TCNT5;
      while(ticks_servo > temp){
        temp = TCNT5;
      }
      digitalWrite(PIN_SERVO,LOW);
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

    pinMode(PIN_SERVO, OUTPUT);
    pinMode(PIN_ENABLE_SERVO, OUTPUT);
    digitalWrite(PIN_ENABLE_SERVO,HIGH);


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




