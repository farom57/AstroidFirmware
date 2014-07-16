#include "com.h"
#include "motor.h"
#include <Streaming.h>
#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

// USB
// accessory descriptor. It's how Arduino identifies itself to Android
char applicationName[] = "Mega_ADK"; // the app on your phone
char accessoryName[] = "Mega_ADK"; // your Arduino board
char companyName[] = "Farom";

// make up anything you want for these
char versionNumber[] = "1.0";
char serialNumber[] = "1";
char url[] = "http://labs.arduino.cc/adk/ADK_count"; // the URL of your app online
// initialize the accessory:
AndroidAccessory usb(companyName, applicationName,accessoryName,versionNumber,url,serialNumber);

#define SIDERAL_RATE  86400./86164./4.
#define TIMEOUT_STATE 200



long ms_count;
long step_ra=0;
long step_de=0;
float ustep_ra=0;
float ustep_de=0;
float move_speed_ra=0;
float move_speed_de=0;


#define ST4_S 24
#define ST4_E 26
#define ST4_W 28
#define ST4_N 30







//Interrupt Service Routine (ISR) for Timer5 overflow at 1kHz
ISR(TIMER5_COMPA_vect ){ 
    ms_count++;

    float speed_ra=-SIDERAL_RATE*(1. + (!digitalRead(ST4_W)?0.5:0.) + (!digitalRead(ST4_E)?-0.5:0.)+move_speed_ra);
    float speed_de=SIDERAL_RATE*((!digitalRead(ST4_N)?0.5:0.) + (!digitalRead(ST4_S)?-0.5:0.)+move_speed_de);

    ustep_ra+=speed_ra*0.001*1024;
    ustep_de+=speed_de*0.001*1024;

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
}



void setup(void){
    initInterrupt();
    initMotors();


    // ST4
    pinMode(ST4_S, INPUT_PULLUP);
    pinMode(ST4_N, INPUT_PULLUP);
    pinMode(ST4_W, INPUT_PULLUP);
    pinMode(ST4_E, INPUT_PULLUP);
    
    // start the connection to the device over the USB host:
    usb.powerOn();

    Serial.begin(9600);


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




