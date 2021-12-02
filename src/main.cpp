#include <Arduino.h>
#include "ESPRotary.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#include <communication.h>

// PWM CHANNELS
#define LED_PIN_1 5  // LED PWM channel 1
#define LED_PIN_2 16 // LED PWM channel 2
#define FAN_PIN 4    // FAN PWM

// ENCODER SETUP
// #define ROTARY_PIN1 0
// #define ROTARY_PIN2 12

// DS18B20 SENSOR SETUP
#define TEMPERATURE_PIN 2
OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

// TEMPERATURE TARGET AND CONTROLS
const int TEMPERATURE_TARGET = 40;

float dT_min = 0;
float dT_max = 20;
float fan_pwm_min = 0;
float fan_pwm_max = 200;

float linear_fan_control_slope = (fan_pwm_max - fan_pwm_min) / (dT_max - dT_min);
float linear_fan_control_b = fan_pwm_min - linear_fan_control_slope * dT_min;

float temp_c;
float fan_pwm = 255;

void setup()
{
  Serial.begin(9600);

  analogWriteFreq(5000);

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  analogWrite(LED_PIN_1, 255);
  analogWrite(LED_PIN_2, 255);
  analogWrite(FAN_PIN, 255);

  // communication_setup();
}

unsigned long previous_millis_led_temperature = 0;
const long interval_led_temperature = 1000; // interval at which to update led temperature

unsigned long previous_millis_fan_speed = 0;
const long interval_fan_speed = interval_led_temperature; // interval at which to update fan speed

unsigned long previous_millis_notification = 0;
const long interval_notification = 5000; // interval at which to send data

unsigned long previous_millis_led = 0;
const long interval_led = 1000; // interval at which to send data

int pot_max_val = 850;
int pot_min_val = 15;
float led_pwm_max = 255;
float led_pwm_min = 10;

float linear_led_control_slope = (led_pwm_max - led_pwm_min) / (pot_max_val - pot_min_val);
float linear_led_control_b = led_pwm_min - linear_led_control_slope * pot_min_val;

int round_up(int numToRound, int multiple)
{
    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

int get_led_pwm(){
  int raw_val = analogRead(A0);
  
  int pwm_result = 0;
  if (raw_val <= pot_min_val) {
    return led_pwm_min;
  } else if (raw_val >= pot_max_val) {
    return led_pwm_max;
  } else {
    pwm_result = linear_led_control_slope * raw_val + linear_led_control_b;
  }
  pwm_result = round_up(pwm_result, 30);
  return pwm_result;
}


void loop()
{
  // communication_loop();

  // Update LED temperature and adjust fan speed
  unsigned long currentMillis = millis();
  if (currentMillis - previous_millis_led_temperature >= interval_led_temperature)
  {
    previous_millis_led_temperature = currentMillis;

    sensors.requestTemperatures();
    temp_c = sensors.getTempCByIndex(0);
  }

  if (currentMillis - previous_millis_fan_speed >= interval_fan_speed)
  {
    previous_millis_fan_speed = currentMillis;
    float dT = temp_c - TEMPERATURE_TARGET;

    if (dT > 0)
    {
      // Linear fan control
      if (dT < dT_min)
      {
        fan_pwm = fan_pwm_min;
      }
      else if (dT > dT_max)
      {
        fan_pwm = fan_pwm_max;
      }
      else
      {
        fan_pwm = linear_fan_control_slope * dT + linear_fan_control_b;
      }
    } else {
      fan_pwm = fan_pwm_min;
    }
    analogWrite(FAN_PIN, 255 - fan_pwm);
  }

  if (currentMillis - previous_millis_led >= interval_led)
  {
    previous_millis_led = currentMillis;

    float led_pwm = get_led_pwm();

    analogWrite(LED_PIN_1, 255 - led_pwm);
    analogWrite(LED_PIN_2, 255 - led_pwm);
  }

  // if (currentMillis - previous_millis_notification >= interval_notification)
  // {
  //   previous_millis_notification = currentMillis;

  //   send_current_status(temp_c, fan_pwm, led_pwm);
  // }

}