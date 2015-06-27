#include "com.h"

#define STATUS_MSG_SIZE 31
#define CMD_MSG_SIZE 11
#define RX_TIMEOUT 1000

void sendStatus() {
  byte buffer[STATUS_MSG_SIZE];
  /* buffer bytes (big-endian):
   * 0-3: ms_count (long)
   * 4-7; step_ra (long)
   * 8-11: step_de (long)
   * 12-15: ustep_ra (float)
   * 16-19: ustep_de (float)
   * 20-23: move_speed_ra (float)
   * 24-27: move_speed_de (float)
   * 28-29: ticks_servo (unsigned int)
   * 30: checksum
   */

  int i = 0;

  buffer[i++] = (byte)(ms_count >> 24);
  buffer[i++] = (byte)(ms_count >> 16);
  buffer[i++] = (byte)(ms_count >> 8);
  buffer[i++] = (byte)(ms_count);

  buffer[i++] = (byte)(step_ra >> 24);
  buffer[i++] = (byte)(step_ra >> 16);
  buffer[i++] = (byte)(step_ra >> 8);
  buffer[i++] = (byte)(step_ra);

  buffer[i++] = (byte)(step_de >> 24);
  buffer[i++] = (byte)(step_de >> 16);
  buffer[i++] = (byte)(step_de >> 8);
  buffer[i++] = (byte)(step_de);

  buffer[i++] = (byte)((*(long*)&ustep_ra) >> 24);
  buffer[i++] = (byte)((*(long*)&ustep_ra) >> 16);
  buffer[i++] = (byte)((*(long*)&ustep_ra) >> 8);
  buffer[i++] = (byte)((*(long*)&ustep_ra));

  buffer[i++] = (byte)((*(long*)&ustep_de) >> 24);
  buffer[i++] = (byte)((*(long*)&ustep_de) >> 16);
  buffer[i++] = (byte)((*(long*)&ustep_de) >> 8);
  buffer[i++] = (byte)((*(long*)&ustep_de));

  buffer[i++] = (byte)((*(long*)&move_speed_ra) >> 24);
  buffer[i++] = (byte)((*(long*)&move_speed_ra) >> 16);
  buffer[i++] = (byte)((*(long*)&move_speed_ra) >> 8);
  buffer[i++] = (byte)((*(long*)&move_speed_ra));

  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 24);
  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 16);
  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 8);
  buffer[i++] = (byte)((*(long*)&move_speed_de));
  
  buffer[i++] = (byte)(ticks_servo >> 8);
  buffer[i++] = (byte)(ticks_servo);

  //checksum
  buffer[STATUS_MSG_SIZE - 1] = 0;
  for (i = 0; i < STATUS_MSG_SIZE - 1; i++) {
    buffer[STATUS_MSG_SIZE - 1] += buffer[i];
  }

  Serial.write(buffer, STATUS_MSG_SIZE);

  if ( adk.isReady()) {
    byte rcode = adk.SndData(STATUS_MSG_SIZE, buffer );
  }

  Usb.Task();
}


float toFloat(byte* ptr) {
  byte temp[4];
  temp[0] = ptr[3];
  temp[1] = ptr[2];
  temp[2] = ptr[1];
  temp[3] = ptr[0];
  float result = *((float*)temp);
  return result;
}

void processCommand(byte* buffer){
  /* buffer bytes (big-endian):
   * 0-3: move_speed_ra (float)
   * 4-7: move_speed_de (float)
   * 8-9: ticks_servo (unsigned int)
   * 10: checksum
   */
  byte sum = 0;
  for (int i = 0; i < CMD_MSG_SIZE - 1; i++) {
    sum += buffer[i];
  }

  if (sum == buffer[CMD_MSG_SIZE - 1]) {
    move_speed_ra = toFloat(buffer);
    move_speed_de = toFloat(buffer + 4);
    ticks_servo = (buffer[8]<<8) + buffer[9];
    if(ticks_servo > 4000){
      digitalWrite(PIN_ENABLE_SERVO,HIGH);
    }else{
      digitalWrite(PIN_ENABLE_SERVO,LOW);
    }
  }
}

void receiveCommand()
{
  static byte buffer[CMD_MSG_SIZE];
  static int pos = 0;
  static long last_rx = 0;

// --- Serial ---
  while (Serial.available() > 0 && pos < CMD_MSG_SIZE) {
    buffer[pos] = Serial.read();
    last_rx = ms_count;
    pos++;
  }

  if (pos == CMD_MSG_SIZE) {
    processCommand(buffer);    
    pos = 0;
  }

  if (ms_count - last_rx > RX_TIMEOUT) {
    pos = 0;
  }

// --- AOA ---
  uint16_t len = CMD_MSG_SIZE;  
  
  if ( adk.isReady()) {
  uint8_t rcode = adk.RcvData(&len, buffer);
      if (len == CMD_MSG_SIZE) {
        processCommand(buffer);
      }    
  }

  Usb.Task();


}








