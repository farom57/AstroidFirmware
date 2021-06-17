
#include "com.h"



#define SIDERAL_RATE  144.*3.*200./4./86164.


// Motor pin names
#define X_STEP_PIN         15
#define X_DIR_PIN          21
#define Y_STEP_PIN         22
#define Y_DIR_PIN          23
#define Z_STEP_PIN         3
#define Z_DIR_PIN          2
#define E0_STEP_PIN        1
#define E0_DIR_PIN         0
#define XYE0_ENABLE_PIN    14
#define Z_ENABLE_PIN       26
#define LED_PIN            27
#define FAN_PIN            4
#define HEATER_END_PIN     13
#define HEATER_BED_PIN     12
#define TEMP_0_PIN          A7   // Analogue pin
#define TEMP_BED_PIN        A6   // Analogue pin
#define X_MIN_PIN          18
#define Y_MIN_PIN          19
#define Z_MIN_PIN          20

// timerinterupt speed
#define UPDATE_FREQ 2047L
#define UPDATE_TIME 1./UPDATE_FREQ
#define UPDATE_TIME_MS 1000./UPDATE_FREQ

#define TIMEOUT_STATE 0.1*UPDATE_FREQ


//Choose which motor correspond to DE and HA
#define HA_STEP_PIN    Y_STEP_PIN
#define HA_DIR_PIN     Y_DIR_PIN
#define HA_ENABLE_PIN  XYE0_ENABLE_PIN
#define DE_STEP_PIN    Z_STEP_PIN
#define DE_DIR_PIN     Z_DIR_PIN
#define DE_ENABLE_PIN  Z_ENABLE_PIN
#define FOCUS_STEP_PIN E0_STEP_PIN
#define FOCUS_DIR_PIN  E0_DIR_PIN
#define AUX1_PIN       HEATER_BED_PIN
#define AUX2_PIN       HEATER_END_PIN
#define AUX3_PIN       FAN_PIN
#define BULB_PIN      TEMP_BED_PIN
#define REMOTE_PIN      TEMP_0_PIN
#define BUZZER_PIN     Y_MIN_PIN

/* Go to sheet/remote astroid
  #define EOS_VCC        3.3
  #define MELZI_VCC      5.0
  #define LINE_R         10000.
  #define PULL_UP_R      47000.
  #define PULL_DOWN_R    47000.
  #define EOS_R          47000.
  #define ENTER_R        45000.
  #define LEFT_R         27000.
  #define UP_R           15000.
  #define RIGHT_R        6800.
  #define DOWN_R         0.
*/
#define EOS_V          2.74
#define UNCONNECTED_V  2.5
#define ENTER_V        1.77
#define LEFT_V         1.53
#define UP_V           1.29
#define RIGHT_V        1.04
#define DOWN_V         0.75
#define BTN_DELAY      0.15

#define EOS 0
#define UNCONNECTED 1
#define ENTER 2
#define LEFT 3
#define UP 4
#define RIGHT 5
#define DOWN 6

#define SPEED_1 0.5
#define SPEED_2 4.0
#define SPEED_3 32.0
#define SPEED_4 256.0

#define DEC_SLEEP_TIMEOUT 60L*UPDATE_FREQ

long clock = 0;

float ustep_speed_ha = SIDERAL_RATE * UPDATE_TIME * 64.;
float ustep_speed_de = 0;
float ustep_speed_focus = 0;
long ustep_ha = 0;
long ustep_de = 0;
long ustep_focus = 0;
float move_speed_ha = 1.;
float move_speed_de = 0.;
float joystick_speed_ha = 0.;
float joystick_speed_de = 0.;
float joystick_speed = SPEED_3;
float move_speed_focus = 0;
float power_ha = 1.;
float power_de = 1;
float power_focus = 0.;
float ha_ustep_fraction = 0;
float de_ustep_fraction = 0;
float focus_ustep_fraction = 0;
unsigned int power_aux1 = 0;
unsigned int power_aux2 = 0;
unsigned int power_aux3 = 0;
byte bulb_state = 0;
int counter_aux1 = 0;
int counter_aux2 = 0;
int counter_aux3 = 0;
long last_dec_active = 0;

void updateOutputCtrl();
void delayms(int ms);

//Interrupt Service Routine (ISR) for Timer3 overflow
ISR(TIMER3_COMPA_vect ) {
  clock += 1;

  ha_ustep_fraction += ustep_speed_ha;
  de_ustep_fraction += ustep_speed_de;
  focus_ustep_fraction += ustep_speed_focus;





  while (ha_ustep_fraction >= 1.) {
    ha_ustep_fraction -= 1.;
    digitalWrite(HA_STEP_PIN, LOW);
    digitalWrite(HA_DIR_PIN, power_ha > 0);
    delayMicroseconds(2);
    digitalWrite(HA_STEP_PIN, HIGH);
    ustep_ha++;
  }

  while (ha_ustep_fraction <= -1.) {
    ha_ustep_fraction += 1.;
    digitalWrite(HA_STEP_PIN, LOW);
    digitalWrite(HA_DIR_PIN, power_ha < 0);
    delayMicroseconds(2);
    digitalWrite(HA_STEP_PIN, HIGH);
    ustep_ha--;
  }

  while (de_ustep_fraction >= 1.) {
    de_ustep_fraction -= 1.;
    digitalWrite(DE_STEP_PIN, LOW);
    digitalWrite(DE_DIR_PIN, power_de > 0);
    delayMicroseconds(2);
    digitalWrite(DE_STEP_PIN, HIGH);
    ustep_de++;
  }

  while (de_ustep_fraction <= -1.) {
    de_ustep_fraction += 1.;
    digitalWrite(DE_STEP_PIN, LOW);
    digitalWrite(DE_DIR_PIN, power_de < 0);
    delayMicroseconds(2);
    digitalWrite(DE_STEP_PIN, HIGH);
    ustep_de--;
  }

  while (focus_ustep_fraction >= 1.) {
    focus_ustep_fraction -= 1.;
    digitalWrite(FOCUS_STEP_PIN, LOW);
    digitalWrite(FOCUS_DIR_PIN, power_focus > 0);
    delayMicroseconds(2);
    digitalWrite(FOCUS_STEP_PIN, HIGH);
    ustep_focus++;
  }

  while (focus_ustep_fraction <= -1.) {
    focus_ustep_fraction += 1.;
    digitalWrite(FOCUS_STEP_PIN, LOW);
    digitalWrite(FOCUS_DIR_PIN, power_focus < 0);
    delayMicroseconds(2);
    digitalWrite(FOCUS_STEP_PIN, HIGH);
    ustep_focus--;
  }
  delayMicroseconds(1);


}

void initMotors()
{
  // X
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(XYE0_ENABLE_PIN, OUTPUT); // X Y and E0 enable pin is common
  digitalWrite(DE_ENABLE_PIN, HIGH); // it's a !ENABLE pin, OFF by default

  // Y
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);

  // E0
  pinMode(E0_STEP_PIN, OUTPUT);
  pinMode(E0_DIR_PIN, OUTPUT);

  // Z
  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(HA_ENABLE_PIN, LOW); // it's a !ENABLE pin

}

void initAux()
{
  pinMode(AUX1_PIN, OUTPUT);
  pinMode(AUX2_PIN, OUTPUT);
  pinMode(AUX3_PIN, OUTPUT);
  pinMode(BULB_PIN, INPUT);
  pinMode(REMOTE_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(AUX1_PIN, LOW);
  digitalWrite(AUX2_PIN, LOW);
  digitalWrite(AUX3_PIN, LOW);
  digitalWrite(BULB_PIN, LOW);
  digitalWrite(REMOTE_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  bitClear(TCCR0B, CS02);
  bitClear(TCCR0B, CS01);
  bitSet(TCCR0B, CS00);

  bitClear(TCCR1B, CS12);
  bitClear(TCCR1B, CS11);
  bitSet(TCCR1B, CS10);
}

void initInterrupt(void)
{
  /* First disable the timer overflow interrupt while we're configuring */
  bitClear(TIMSK3, TOIE3);


  //configure TIMER3 in CTC mode
  bitClear(TCCR3A, WGM31);
  bitClear(TCCR3A, WGM30);
  bitSet(TCCR3B, WGM32);
  bitClear(TCCR3B, WGM33);

  /* Now configure the prescaler to CPU clock divided by 1 */
  bitClear(TCCR3B, CS32);
  bitSet(TCCR3B, CS31);
  bitClear(TCCR3B, CS30);

  /* We need to calculate a proper value to load the timer counter.
     The following loads the value xxx into the Timer 3 counter register
     The math behind this is:
     OCR5A = Fclk/(Fdesired * prescaler) -1
           = 16MHz/(1kHz * 8 ) - 1
           = 1999

  */
  //OCR3A = 16000000L/(UPDATE_FREQ*8) - 1;
  OCR3A = 2000L;

  //Enable compare match A
  bitClear(TCCR3A, COM3A1);
  bitClear(TCCR3A, COM3A0);

  // Finally enable the timer
  bitSet(TIMSK3, OCIE3A);
  bitSet(TIMSK3, TOIE3);
}


void setup(void) {
  initInterrupt();
  initMotors();
  initAux();

  // Serial
  Serial.begin(9600);
  Serial1.begin(9600);


}


void changeSpeed() {
  if (joystick_speed == SPEED_4) {
    joystick_speed = SPEED_1;
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);

  }
  else if (joystick_speed == SPEED_1) {
    joystick_speed = SPEED_2;
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);

  }
  else if (joystick_speed == SPEED_2) {
    joystick_speed = SPEED_3;
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);

  }
  else if (joystick_speed == SPEED_3) {
    joystick_speed = SPEED_4;
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
    delayms(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delayms(50);
    digitalWrite(BUZZER_PIN, LOW);
  }

}

void loop(void) {
  static long clock_state = 0;
  static byte button_value = UNCONNECTED;
  static long button_chg_time = 0;
  static byte speed_change_done;
  int meas;


  receiveCommand();

  if (clock >= clock_state + TIMEOUT_STATE) {
    clock_state = clock;
    digitalWrite(LED_PIN, HIGH);
    sendStatus();
    digitalWrite(LED_PIN, LOW);
  }




  if (bulb_state)
  {
    pinMode(BULB_PIN, OUTPUT);
    digitalWrite(BULB_PIN, LOW);
  } else {
    pinMode(BULB_PIN, INPUT);
  }



  

  //digitalWrite(BUZZER_PIN, LOW);
  joystick_speed_ha = 0;
  joystick_speed_de = 0;

  // --- Process 1st joystick port ---
  meas = analogRead(REMOTE_PIN);


  // down button pressed
  if (meas < (DOWN_V + RIGHT_V) * 102) { //*102 = /2 *1024 /5.0V
    if (button_value != DOWN)
      button_chg_time = clock;
    button_value = DOWN;
    if (clock - button_chg_time > BTN_DELAY * UPDATE_FREQ) {
      joystick_speed_de = -joystick_speed;
    }

    // right button pressed
  }
  else if (meas < (RIGHT_V + UP_V) * 102) {
    if (button_value != RIGHT)
      button_chg_time = clock;
    button_value = RIGHT;
    if (clock - button_chg_time > BTN_DELAY * UPDATE_FREQ) {
      joystick_speed_ha = joystick_speed;
    }

    // up button pressed
  }
  else if (meas < (UP_V + LEFT_V) * 102) {
    if (button_value != UP)
      button_chg_time = clock;
    button_value = UP;
    if (clock - button_chg_time > BTN_DELAY * UPDATE_FREQ) {
      joystick_speed_de = joystick_speed;
    }

    // left button pressed
  }
  else if (meas < (LEFT_V + ENTER_V) * 102) {
    if (button_value != LEFT)
      button_chg_time = clock;
    button_value = LEFT;
    if (clock - button_chg_time > BTN_DELAY * UPDATE_FREQ) {
      joystick_speed_ha = -joystick_speed;
    }

    // enter button pressed
  }
  else if (meas < (ENTER_V + UNCONNECTED_V) * 102) {
    if (button_value != ENTER) {
      speed_change_done = 0;
      button_chg_time = clock;
    }

    button_value = ENTER;
    if (clock - button_chg_time > BTN_DELAY * UPDATE_FREQ) {
      if (speed_change_done == 0) {
        changeSpeed();
        speed_change_done = 1;
      }
    }

    // joystick not connected or no button pressed or camera connected
  }
  else {
    button_value = UNCONNECTED;
  }

  updateOutputCtrl();


}

void updateOutputCtrl() {
  ustep_speed_ha = (move_speed_ha + joystick_speed_ha) * SIDERAL_RATE * UPDATE_TIME * 64.;
  ustep_speed_de = (move_speed_de + joystick_speed_de) * SIDERAL_RATE * UPDATE_TIME * 64.;
  ustep_speed_focus = move_speed_focus * UPDATE_TIME * 64.;

  if (power_ha == 0. || (move_speed_ha + joystick_speed_ha == 0. && abs(power_ha) < 0.5)) {
    digitalWrite(HA_ENABLE_PIN, HIGH); // it's a !ENABLE pin
  }
  else {
    digitalWrite(HA_ENABLE_PIN, LOW); // it's a !ENABLE pin
  }

  if (move_speed_de != 0. ||  joystick_speed_de != 0. || move_speed_focus == 0. || abs(power_de) > 1) {
    digitalWrite(DE_ENABLE_PIN, LOW); // it's a !ENABLE pin
    last_dec_active = clock;
  } else if (clock - last_dec_active < DEC_SLEEP_TIMEOUT) {
    digitalWrite(DE_ENABLE_PIN, LOW); // it's a !ENABLE pin
  } else {
    digitalWrite(DE_ENABLE_PIN, HIGH); // it's a !ENABLE pin
  }

  analogWrite(AUX1_PIN, power_aux1);
  analogWrite(AUX2_PIN, power_aux2);
  analogWrite(AUX3_PIN, power_aux3);
}

void delayms(int ms) {

  for (int i = 0; i < ms; i++) {
    delayMicroseconds(1000);
  }
}
