#include "com.h"



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
        move_speed = 0.5;
        break;
    case 'C':
        move_speed = 4.;
        break;
    case 'M':
        move_speed = 32.;
        break;
    case 'S':
        move_speed = 120.;
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

