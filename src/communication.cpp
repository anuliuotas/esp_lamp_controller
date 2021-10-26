#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Network setup
const char *ssid = "MajoCrypt";
const char *password = "Po100pypkiu";

// MQQT setup
const char *mqtt_server = "192.168.0.106";
const char *clientID = "ESP8266";
const char *outTopic = "test";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

void communication_setup()
{
    WiFi.begin(ssid, password);
    client.setServer(mqtt_server, 1883);
}

unsigned long previousMqqtReconnectMillis = 0; 
const long mqqtReconnectInterval = 10000;      // interval at which to reconnect to mqqt server

void reconnect()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        String clientId = "ESP8266_LAMP-";
        clientId += String(random(0xffff), HEX);
        client.connect(clientId.c_str());
    } else {
        WiFi.begin(ssid, password);
    }
}

void communication_loop()
{
    if (!client.connected())
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMqqtReconnectMillis >= mqqtReconnectInterval)
        {
            previousMqqtReconnectMillis = currentMillis;
            reconnect();
        }
    } else {
        client.loop();
    }

}

void send_current_status(float led_temperature, float fan_pwm, float led_pwm) {
    snprintf (msg, 50, "%f %f %f", led_temperature, fan_pwm, led_pwm);
    client.publish(outTopic, msg);
}