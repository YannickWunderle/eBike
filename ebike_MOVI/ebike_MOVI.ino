#include <EEPROM.h>

#define RELAIS_PIN 4
#define VOLTAGE_PIN A0
//#define TEMP_PIN A5
#define LED_PIN 5
#define BUTTON_PIN A2
#define PAS_PIN 2
#define THROTTLE_PIN 9


#define ADC_RES 1023
#define MAXIMUM_VOLTAGE 63
//#define TEMP_LIMIT 280 //290 = 100 C , lower value is higher temp
#define PAS_TIME 50
#define MIN_MOTOR_VOLTAGE 43
#define MIN_BAT_VOLTAGE 38
#define MIN_LIGHT_VOLTAGE 10.8
#define MAX_LIGHT_VOLTAGE 13

#define MAX_THROTTLE 190
#define MIN_THROTTLE 65

float lastVoltage = 50;  //for filtering
int ButtonState = 0;
unsigned long ButtonLastTime = 0;

bool Motor_enable = true;
unsigned long last_pas_millis = 0;
unsigned long pas_millis = 0;

unsigned int pedal_time = 1000;
unsigned int throttle = 0;


void setup() {
  Serial.begin(9600);
  pinMode(RELAIS_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(THROTTLE_PIN, OUTPUT);
  pinMode(PAS_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PAS_PIN), pas, CHANGE);

  digitalWrite(RELAIS_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  pas_millis = millis();
}



void loop() {
  checkVoltage();
  int ButtonState = checkButton();
  if (ButtonState == 2) {
    //Serial.println("Shutdown");
    shutdown();
  }
  if (ButtonState == 1) {
    Motor_enable = !Motor_enable;
    //Serial.print("Motor enable: ");
    //Serial.println(Motor_enable);
  }
  motor();
  delay(50);
}

void motor() {
  if (Motor_enable && (millis() - pas_millis < 220) && (pas_millis - last_pas_millis < 220)) {
    //Serial.println("Motor is spinning");
    if (throttle >= MAX_THROTTLE) {
      throttle = MAX_THROTTLE;
    } else {
      throttle++;
    }
    analogWrite(THROTTLE_PIN, throttle);
  } else {
    analogWrite(THROTTLE_PIN, 0);
    throttle = MIN_THROTTLE;
  }
}

void pas() {
  last_pas_millis = pas_millis;
  pas_millis = millis();
}

int checkButton() {  //0= no press, 1 = press, 2= long press
  if ((ButtonState != 1) and (digitalRead(BUTTON_PIN))) {
    ButtonState = 1;
    ButtonLastTime = millis();
  }
  if ((ButtonState == 1) and (!digitalRead(BUTTON_PIN))) {
    ButtonState = 0;
    if ((millis() - ButtonLastTime) > 1000) {
      ButtonLastTime = millis();
      return 2;
    } else {
      return 1;
    }
  }

  return 0;
}




void checkVoltage() {
  float Voltage = analogRead(VOLTAGE_PIN);
  Voltage = 0.9 * lastVoltage + 0.1 * (Voltage * MAXIMUM_VOLTAGE / ADC_RES);
  lastVoltage = Voltage;
  Serial.println(Voltage);
  if (Voltage < MIN_MOTOR_VOLTAGE) {
    Motor_enable = false;
  }
  if ((Voltage < MIN_LIGHT_VOLTAGE) or ((Voltage > MAX_LIGHT_VOLTAGE) && (Voltage < MIN_BAT_VOLTAGE))) {
    shutdown();
  }
}

void shutdown() {
  Serial.println("shutdown");
  if (millis() > 5000) {
    digitalWrite(RELAIS_PIN, false);
    while (1)
      ;
  }
}
