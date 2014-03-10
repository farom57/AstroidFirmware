#include "motor.h"
#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>
#include <Streaming.h>

#define SIDERAL_RATE  86400./86164./4.

//uart & command variables
byte waiting_command = false;
String str_command = "";
byte receiving_command = false;

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


//Interrupt Service Routine (ISR) for Timer5 overflow at 1kHz
ISR(TIMER5_COMPA_vect ){ 
    ms_count++;

    float speed_ra=SIDERAL_RATE*(1. + (!digitalRead(ST4_W)?0.5:0.) + (!digitalRead(ST4_E)?-0.5:0.))+move_speed*move_dir_ra;
    float speed_de=SIDERAL_RATE*((!digitalRead(ST4_N)?0.5:0.) + (!digitalRead(ST4_S)?-0.5:0.))+move_speed*move_dir_de;
    
    upas_ra+=speed_ra*0.001*256;
    upas_de+=speed_de*0.001*256;
    
    if(upas_ra>=256.)
        upas_ra-=256.;
    if(upas_ra<0.)
        upas_ra+=256.;
    if(upas_de>=256.)
        upas_de-=256.;
    if(upas_de<0.)
        upas_de+=256.;
    
    unsigned int upas_ra_int = upas_ra;
    unsigned int upas_de_int = upas_de;
    
    powerCoil(1,UPAS_CURVE[upas_ra_int]);
    powerCoil(2,UPAS_CURVE[(upas_ra_int+64) & 0xFF]);
    powerCoil(3,UPAS_CURVE[upas_de_int]);
    powerCoil(4,UPAS_CURVE[(upas_de_int+64) & 0xFF]);

    latchTx();
}

void receiveCommand()
{

  if (usb.isConnected() && !waiting_command) { // isConnected makes sure the USB connection is ope
      char buff[5];
      int len = usb.read(buff, 16,1000);
      if(len!=-1){
        buff[len]=0;
        str_command=String(buff);
        Serial.println(buff);
        waiting_command = true;
        receiving_command = false;
      }

  }
  

}

void processCommand_M()
{
  //move commands
  //:Me# : move EST
  //:Mw# : move WEST
  //:Mn# : move NORTH
  //:Ms# : move SOUTH
  //:M#  : GOTO

  switch (str_command[2])
  {
  case 'e':
    move_dir_ra=-1;
    break;
  case 'w':
    move_dir_ra=+1;
    break;
  case 'n':
    move_dir_de=1;
    break;
  case 's':
    move_dir_de=-1;
    break;
  default:
    Serial << "Unknown command#";

  }

}

void processCommand_Q()
{  
  //stop move commands
  //:Qe# : stop RA move
  //:Qw# : stop RA move
  //:Qn# : stop DE move
  //:Qs# : stop DE move
  //:Q# : stop any move (follow mode)

  switch (str_command[2])
  {
  case 'e':
    move_dir_ra=0;
    break;
  case 'w':
    move_dir_ra=0;
    break;
  case 'n':
    move_dir_de=0;
    break;
  case 's':
    move_dir_de=0;
    break;
  case '#':
    move_dir_ra=0;
    move_dir_de=0;
    break;
  default:
    Serial << "Unknown command#";

  }
}

void processCommand_R()
{
  //speed commands
  //:RG# : speed 0 
  //:RC# : speed 1
  //:RM# : speed 2
  //:RS# : speed 3
  switch(str_command[2])
  {
  case 'G':
    move_speed = SIDERAL_RATE*0.5;
    break;
  case 'C':
    move_speed = SIDERAL_RATE*4.;
    break;
  case 'M':
    move_speed = SIDERAL_RATE*32.;
    break;
  case 'S':
    move_speed = SIDERAL_RATE*120.;
    break;
  default:
    Serial << "Unknown command#";
  }  

}

void processCommand()
{
  if(waiting_command)
  {
    switch (str_command[1])
    {
//    case 'C':
//      processCommand_C();
//      break;
//    case 'G':
//      processCommand_G();
//      break;      
    case 'M':
      processCommand_M();
      break;
    case 'Q':
      processCommand_Q();
      break;
    case 'R':
      processCommand_R();
      break;
//    case 'S':
//      processCommand_S();
//      break;
//    case 'U':
//      processCommand_U();
//      break;
//    case 'W':
//      processCommand_W();
//      break;
//    case 'p':
//      processCommand_p();
//      break;
//    case 6: //ASCII ACK
//      processCommand_ACK();
    default:
      Serial << "Unknown command#";

    }
    waiting_command = false;
  }
}

void setup(void){
    initInterrupt();
    initMotors();
    Serial.begin(9600);
    
    // ST4
    pinMode(ST4_S, INPUT_PULLUP);
    pinMode(ST4_N, INPUT_PULLUP);
    pinMode(ST4_W, INPUT_PULLUP);
    pinMode(ST4_E, INPUT_PULLUP);
    
      // start the connection to the device over the USB host:
  usb.powerOn();
}

void loop(void){
  receiveCommand();
  processCommand(); 
    
}


