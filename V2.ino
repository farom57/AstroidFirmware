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



long ms_count;
float upas_ra=0;
float upas_de=0;
float move_speed=0.5;
float move_dir_ra=0;
float move_dir_de=0;

#define ST4_S 24
#define ST4_E 26
#define ST4_W 28
#define ST4_N 30







//Interrupt Service Routine (ISR) for Timer5 overflow at 1kHz
ISR(TIMER5_COMPA_vect ){ 
    ms_count++;

    float speed_ra=SIDERAL_RATE*(1. + (!digitalRead(ST4_W)?0.5:0.) + (!digitalRead(ST4_E)?-0.5:0.))+move_speed*move_dir_ra;
    float speed_de=SIDERAL_RATE*((!digitalRead(ST4_N)?0.5:0.) + (!digitalRead(ST4_S)?-0.5:0.))+move_speed*move_dir_de;

    upas_ra+=speed_ra*0.001*1024;
    upas_de+=speed_de*0.001*1024;

    if(upas_ra>=1024.)
        upas_ra-=1024.;
    if(upas_ra<0.)
        upas_ra+=1024.;
    if(upas_de>=1024.)
        upas_de-=1024.;
    if(upas_de<0.)
        upas_de+=1024.;

    unsigned int upas_ra_int = upas_ra;
    unsigned int upas_de_int = upas_de;

    setRAStep(upas_ra_int);
    setDEStep(upas_de_int);

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

    Serial.begin(115200);


}

void loop(void){
    receiveCommand();
    processCommand(); 
}




