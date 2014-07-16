#include "com.h"

#define STATUS_MSG_SIZE 29
#define CMD_MSG_SIZE 9
#define RX_TIMEOUT 1000

void sendStatus(){
    byte buffer[STATUS_MSG_SIZE];
    /* buffer bytes (big-endian):
     * 0-3: ms_count (long)
     * 4-7; step_ra (long)
     * 8-11: step_de (long)
     * 12-15: ustep_ra (float)
     * 16-19: ustep_de (float) 
     */

    int i = 0;

    buffer[i++]=(byte)(ms_count >> 24);
    buffer[i++]=(byte)(ms_count >> 16);
    buffer[i++]=(byte)(ms_count >> 8);
    buffer[i++]=(byte)(ms_count);

    buffer[i++]=(byte)(step_ra >> 24);
    buffer[i++]=(byte)(step_ra >> 16);
    buffer[i++]=(byte)(step_ra >> 8);
    buffer[i++]=(byte)(step_ra);

    buffer[i++]=(byte)(step_de >> 24);
    buffer[i++]=(byte)(step_de >> 16);
    buffer[i++]=(byte)(step_de >> 8);
    buffer[i++]=(byte)(step_de);

    buffer[i++]=(byte)((*(long*)&ustep_ra) >> 24);
    buffer[i++]=(byte)((*(long*)&ustep_ra) >> 16);
    buffer[i++]=(byte)((*(long*)&ustep_ra) >> 8);
    buffer[i++]=(byte)((*(long*)&ustep_ra));

    buffer[i++]=(byte)((*(long*)&ustep_de) >> 24);
    buffer[i++]=(byte)((*(long*)&ustep_de) >> 16);
    buffer[i++]=(byte)((*(long*)&ustep_de) >> 8);
    buffer[i++]=(byte)((*(long*)&ustep_de));

    buffer[i++]=(byte)((*(long*)&move_speed_ra) >> 24);
    buffer[i++]=(byte)((*(long*)&move_speed_ra) >> 16);
    buffer[i++]=(byte)((*(long*)&move_speed_ra) >> 8);
    buffer[i++]=(byte)((*(long*)&move_speed_ra));

    buffer[i++]=(byte)((*(long*)&move_speed_de) >> 24);
    buffer[i++]=(byte)((*(long*)&move_speed_de) >> 16);
    buffer[i++]=(byte)((*(long*)&move_speed_de) >> 8);
    buffer[i++]=(byte)((*(long*)&move_speed_de));

    //checksum
    buffer[STATUS_MSG_SIZE-1]=0;
    for(i=0; i<STATUS_MSG_SIZE-1; i++){
        buffer[STATUS_MSG_SIZE-1]+=buffer[i];
    }

    Serial.write(buffer, STATUS_MSG_SIZE);

    //    if (usb.isConnected()) {
    //        usb.beginTransmission();
    //        usb.write(buffer, STATUS_MSG_SIZE);
    //        usb.endTransmission();
    //    }
}


float toFloat(byte* ptr){
    byte temp[4];
    temp[0]=ptr[3];
    temp[1]=ptr[2]; 
    temp[2]=ptr[1]; 
    temp[3]=ptr[0];
    float result = *((float*)temp);
    return result;
}

void receiveCommand()
{
    static byte buffer[CMD_MSG_SIZE];
    static int pos = 0;
    static long last_rx = 0;

    if(Serial.available()>0){
        buffer[pos]=Serial.read();
        last_rx=ms_count;
        pos++;
    }
    
    if(pos == CMD_MSG_SIZE){
        pos=0;
        
        //checksum
        byte sum=0;
        for(int i=0; i<CMD_MSG_SIZE-1; i++){
            sum+=buffer[i];
        }
        
        if(sum==buffer[CMD_MSG_SIZE-1]){  
            move_speed_ra = toFloat(buffer);
            move_speed_de = toFloat(buffer+4);
        }
        //sendStatus();
    }
    
    if(ms_count-last_rx > RX_TIMEOUT){
        pos=0;
    }


    //    if (usb.isConnected() && !waiting_command) { // isConnected makes sure the USB connection is ope
    //        char buff[5];
    //        int len = usb.read(buff, 16,5);
    //        if(len!=-1){
    //            buff[len]=0;
    //            str_command=String(buff);
    //            //Serial.println(buff);
    //            waiting_command = true;
    //            receiving_command = false;
    //        }
    //
    //    }


}

//void processCommand_M()
//{
//    //move commands
//    //:Me# : move EST
//    //:Mw# : move WEST
//    //:Mn# : move NORTH
//    //:Ms# : move SOUTH
//    //:M#  : GOTO
//
//    switch (str_command[2])
//    {
//    case 'e':
//        move_dir_ra=-1;
//        break;
//    case 'w':
//        move_dir_ra=+1;
//        break;
//    case 'n':
//        move_dir_de=1;
//        break;
//    case 's':
//        move_dir_de=-1;
//        break;
//        //default:
//        //Serial << "Unknown command#";
//
//    }
//
//}
//
//void processCommand_Q()
//{  
//    //stop move commands
//    //:Qe# : stop RA move
//    //:Qw# : stop RA move
//    //:Qn# : stop DE move
//    //:Qs# : stop DE move
//    //:Q# : stop any move (follow mode)
//
//    switch (str_command[2])
//    {
//    case 'e':
//        move_dir_ra=0;
//        break;
//    case 'w':
//        move_dir_ra=0;
//        break;
//    case 'n':
//        move_dir_de=0;
//        break;
//    case 's':
//        move_dir_de=0;
//        break;
//    case '#':
//        move_dir_ra=0;
//        move_dir_de=0;
//        break;
//        //default:
//        //Serial << "Unknown command#";
//
//    }
//}
//
//void processCommand_R()
//{
//    //speed commands
//    //:RG# : speed 0 
//    //:RC# : speed 1
//    //:RM# : speed 2
//    //:RS# : speed 3
//    switch(str_command[2])
//    {
//    case 'G':
//        move_speed = 0.5;
//        break;
//    case 'C':
//        move_speed = 4.;
//        break;
//    case 'M':
//        move_speed = 32.;
//        break;
//    case 'S':
//        move_speed = 120.;
//        break;
//        //default:
//        //Serial << "Unknown command#";
//    }  
//
//}
//
//void processCommand()
//{
//    if(waiting_command)
//    {
//        switch (str_command[1])
//        {
//            //    case 'C':
//            //      processCommand_C();
//            //      break;
//            //    case 'G':
//            //      processCommand_G();
//            //      break;      
//        case 'M':
//            processCommand_M();
//            break;
//        case 'Q':
//            processCommand_Q();
//            break;
//        case 'R':
//            processCommand_R();
//            break;
//            //    case 'S':
//            //      processCommand_S();
//            //      break;
//            //    case 'U':
//            //      processCommand_U();
//            //      break;
//            //    case 'W':
//            //      processCommand_W();
//            //      break;
//            //    case 'p':
//            //      processCommand_p();
//            //      break;
//            //    case 6: //ASCII ACK
//            //      processCommand_ACK();
//            //default:
//            //Serial << "Unknown command#";
//
//        }
//        waiting_command = false;
//    }
//}






