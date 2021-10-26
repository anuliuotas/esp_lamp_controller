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
#define ROTARY_PIN1 0
#define ROTARY_PIN2 12

void encoderAction(ESPRotary &r);

#define CLICKS_PER_STEP 4
#define MIN_POS 0
#define MAX_POS 255
#define START_POS 100 // Initial duty cycle
#define INCREMENT 4

ESPRotary r;

// DS18B20 SENSOR SETUP
#define TEMPERATURE_PIN 2
OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

// TEMPERATURE TARGET AND CONTROLS
const int TEMPERATURE_TARGET = 40;

float dT_min = 0;
float dT_max = 20;
float fan_pwm_min = 50;
float fan_pwm_max = 200;

float linear_fan_control_slope = (fan_pwm_max - fan_pwm_min) / (dT_max - dT_min);
float linear_fan_control_b = fan_pwm_min - linear_fan_control_slope * dT_min;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN_1, INPUT);
  pinMode(LED_PIN_2, INPUT);

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
  r.setChangedHandler(encoderAction);

  analogWrite(LED_PIN_1, r.getPosition());
  analogWrite(LED_PIN_2, r.getPosition());

  communication_setup();
}

float temp_c;
float fan_pwm = 0;
float led_pwm = START_POS;

unsigned long previous_millis_led_temperature = 0;
const long interval_led_temperature = 1000; // interval at which to update led temperature

unsigned long previous_millis_fan_speed = 0;
const long interval_fan_speed = interval_led_temperature; // interval at which to update fan speed

unsigned long previous_millis_notification = 0;
const long interval_notification = 5000; // interval at which to send data

void loop()
{
  r.loop();
  communication_loop();

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
    }
    analogWrite(FAN_PIN, fan_pwm);
  }

  if (currentMillis - previous_millis_notification >= interval_notification)
  {
    previous_millis_notification = currentMillis;

    send_current_status(temp_c, fan_pwm, led_pwm);
  }
}

void encoderAction(ESPRotary &r)
{
  led_pwm = r.getPosition();
  analogWrite(LED_PIN_1, led_pwm);
  analogWrite(LED_PIN_2, led_pwm);
}