#ifndef COMMUNICATION_MODULE
#define COMMUNICATION_MODULE

void communication_setup();
void communication_loop();

void send_current_status(float led_temperature, float fan_pwm, float led_pwm);

#endif