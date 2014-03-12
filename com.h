#ifndef COM_H
#define COM_H

#include "Arduino.h"
#include <Streaming.h>


//uart & command variables
byte waiting_command = false;
String str_command = "";
byte receiving_command = false;

void receiveCommand();
void processCommand_M();
void processCommand_Q();
void processCommand_R();
void processCommand();





#endif
