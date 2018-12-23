#include "com.h"

#define STATUS_MSG_SIZE 56
#define CMD_MSG_SIZE 32
#define RX_TIMEOUT 100

void sendStatus() {
  byte buffer[STATUS_MSG_SIZE];
  /* Preamble: 0x55
   * buffer bytes (big-endian):
   * 0-3: ms_count (long)
   * 4-7; step_ha (long)
   * 8-11: step_de (long)
   * 12-15: ustep_ha (float)
   * 16-19: ustep_de (float)
   * 20-23: move_speed_ha (float)
   * 24-27: move_speed_de (float)
   * 28-31: power_ha (float)
   * 32-35: power_de (float)
   * 36-37: power_aux1 (uint)
   * 38-39: power_aux2 (uint)
   * 40-41: power_aux3 (uint)
   * 42: bulb_state (byte)
   * 43-46: step_focus (long)
   * 47-50: ustep_focus (float)
   * 51-54: move_speed_focus (float)
   * 55: checksum
   */

  int i = 0;
  long ms_count = UPDATE_TIME_MS * clock;
  buffer[i++] = (byte)(ms_count >> 24);
  buffer[i++] = (byte)(ms_count >> 16);
  buffer[i++] = (byte)(ms_count >> 8);
  buffer[i++] = (byte)(ms_count);
  //Serial.print("ms_count:");
  //Serial.println(ms_count);

  // step = ustep/64
  buffer[i++] = (byte)(ustep_ha >> 30);
  buffer[i++] = (byte)(ustep_ha >> 22);
  buffer[i++] = (byte)(ustep_ha >> 14);
  buffer[i++] = (byte)(ustep_ha >> 6);
  //Serial.print("ustep_ha:");
  //Serial.println(ustep_ha);

  // step = ustep/64
  buffer[i++] = (byte)(ustep_de >> 30);
  buffer[i++] = (byte)(ustep_de >> 22);
  buffer[i++] = (byte)(ustep_de >> 14);
  buffer[i++] = (byte)(ustep_de >> 6);
  //Serial.print("ustep_de:");
  //Serial.println(ustep_de);

  // step fraction = ustep%64 * 16
  float step_fraction_ha = (float) ((ustep_ha & 0x3F) << 4);
  buffer[i++] = (byte)((*(long*)&step_fraction_ha) >> 24);
  buffer[i++] = (byte)((*(long*)&step_fraction_ha) >> 16);
  buffer[i++] = (byte)((*(long*)&step_fraction_ha) >> 8);
  buffer[i++] = (byte)((*(long*)&step_fraction_ha));
  //Serial.print("step_fraction_ha:");
  //Serial.println(step_fraction_ha);

  float step_fraction_de = (float) ((ustep_de & 0x3F) << 4);
  buffer[i++] = (byte)((*(long*)&step_fraction_de) >> 24);
  buffer[i++] = (byte)((*(long*)&step_fraction_de) >> 16);
  buffer[i++] = (byte)((*(long*)&step_fraction_de) >> 8);
  buffer[i++] = (byte)((*(long*)&step_fraction_de));
  //Serial.print("step_fraction_de:");
  //Serial.println(step_fraction_de);

  buffer[i++] = (byte)((*(long*)&move_speed_ha) >> 24);
  buffer[i++] = (byte)((*(long*)&move_speed_ha) >> 16);
  buffer[i++] = (byte)((*(long*)&move_speed_ha) >> 8);
  buffer[i++] = (byte)((*(long*)&move_speed_ha));
  //Serial.print("move_speed_ha:");
  //Serial.println(move_speed_ha);

  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 24);
  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 16);
  buffer[i++] = (byte)((*(long*)&move_speed_de) >> 8);
  buffer[i++] = (byte)((*(long*)&move_speed_de));
  //Serial.print("move_speed_de:");
  //Serial.println(move_speed_de);

  buffer[i++] = (byte)((*(long*)&power_ha) >> 24);
  buffer[i++] = (byte)((*(long*)&power_ha) >> 16);
  buffer[i++] = (byte)((*(long*)&power_ha) >> 8);
  buffer[i++] = (byte)((*(long*)&power_ha));
  //Serial.print("power_ha:");
  //Serial.println(power_ha);

  buffer[i++] = (byte)((*(long*)&power_de) >> 24);
  buffer[i++] = (byte)((*(long*)&power_de) >> 16);
  buffer[i++] = (byte)((*(long*)&power_de) >> 8);
  buffer[i++] = (byte)((*(long*)&power_de));
  //Serial.print("power_de:");
  //Serial.println(power_de);

  buffer[i++] = (byte)(power_aux1 >> 8);
  buffer[i++] = (byte)(power_aux1);

  buffer[i++] = (byte)(power_aux2 >> 8);
  buffer[i++] = (byte)(power_aux2);

  buffer[i++] = (byte)(power_aux3 >> 8);
  buffer[i++] = (byte)(power_aux3);

  buffer[i++] = bulb_state;

  buffer[i++] = (byte)(ustep_focus >> 30);
  buffer[i++] = (byte)(ustep_focus >> 22);
  buffer[i++] = (byte)(ustep_focus >> 14);
  buffer[i++] = (byte)(ustep_focus >> 6);

  float step_fraction_focus = (float) ((ustep_focus & 0x3F) << 4);
  buffer[i++] = (byte)((*(long*)&step_fraction_focus) >> 24);
  buffer[i++] = (byte)((*(long*)&step_fraction_focus) >> 16);
  buffer[i++] = (byte)((*(long*)&step_fraction_focus) >> 8);
  buffer[i++] = (byte)((*(long*)&step_fraction_focus));

  buffer[i++] = (byte)((*(long*)&move_speed_focus) >> 24);
  buffer[i++] = (byte)((*(long*)&move_speed_focus) >> 16);
  buffer[i++] = (byte)((*(long*)&move_speed_focus) >> 8);
  buffer[i++] = (byte)((*(long*)&move_speed_focus));
  //Serial.println("");

  //checksum
  buffer[STATUS_MSG_SIZE - 1] = 0;
  for (i = 0; i < STATUS_MSG_SIZE - 1; i++) {
    buffer[STATUS_MSG_SIZE - 1] += buffer[i];
  }
  Serial.write(0x55);
  Serial.write(buffer, STATUS_MSG_SIZE);
  Serial1.write(0x55);
  Serial1.write(buffer, STATUS_MSG_SIZE);





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
   * 0-3: move_speed_ha (float)
   * 4-7: move_speed_de (float)
   * 8-11: power_ha
   * 12-15: power_de
   * 16-17: power_aux1 (uint)
   * 18-19: power_aux2 (uint)
   * 20-21: power_aux3 (uint)
   * 22: bulb_state (byte)
   * 23-26: move_speed_focus (float)
   * 27-30: power_focus
   * 31: checksum
   */
  byte sum = 0;
  for (int i = 0; i < CMD_MSG_SIZE - 1; i++) {
    sum += buffer[i];
  }

  if (sum == buffer[CMD_MSG_SIZE - 1]) {
    move_speed_ha = toFloat(buffer);
    move_speed_de = toFloat(buffer + 4);
    move_speed_focus = toFloat(buffer + 23);

    power_ha = toFloat(buffer + 8);
    power_de = toFloat(buffer + 12);
    power_focus = toFloat(buffer + 27);

    power_aux1 = (buffer[16]<<8) + buffer[17];
    power_aux2 = (buffer[18]<<8) + buffer[19];
    power_aux3 = (buffer[20]<<8) + buffer[21];
    bulb_state = buffer[22];

    updateOutputCtrl();
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
    last_rx = clock;
    pos++;
  }

  while (Serial1.available() > 0 && pos < CMD_MSG_SIZE) {
    buffer[pos] = Serial1.read();
    last_rx = clock;
    pos++;
  }

  if (pos == CMD_MSG_SIZE) {
    processCommand(buffer);    
    pos = 0;
  }

  if (clock - last_rx > RX_TIMEOUT) {
    pos = 0;
  }




}













