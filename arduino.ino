#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFiNINA.h>
#include <PubSubClient.h>

// Wi-Fi and MQTT settings
const char *ssid = "Sanchit";
const char *password = "12345678";
const char *mqttServer = "broker.hivemq.com"; // HiveMQ broker address
const int mqttPort = 1883;                    // HiveMQ broker port
const char *topic = "health_monitor_system";  // HiveMQ topic

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;
bool startSensing = true;

void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("arduino-client"))
    {
      Serial.println("Connected to MQTT");
      client.subscribe("health_monitor_control");
    }
    else
    {
      Serial.println("MQTT connection failed, retrying in 5 seconds...");
      delay(5000);
    }
  }

  pinMode(10, INPUT);
  pinMode(11, INPUT);

  Serial.print("Initializing pulse oximeter..");

  if (!pox.begin())
  {
    Serial.println("FAILED");
    for (;;)
      ;
  }
  else
  {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  pox.update();

  if (pox.getIR() && pox.getRed())
  {
    char payload[100];
    int ecgValue = readECG(); 

    int heartRate = pox.getHeartRate();
    int spo2 = pox.getSpO2();

    if (true)
    {
      if (client.connected())
      {
        if (startSensing)
        {
          snprintf(payload, sizeof(payload), "{\"ecg_value\": %d, \"heart_rate\": %d, \"spo2\": %d}", ecgValue, heartRate, spo2);
          tsLastReport = millis();

          client.publish(topic, payload);
        }
      }
      else
      {
        reconnect();
      }
      client.loop();
    }
    delay(250);
  }
}

int readECG()
{
 
  int ecgValue = analogRead(A0); 
  return ecgValue;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String message = "";
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  if (message == "start_sensing")
  {
    startSensing = true;
    Serial.println("Sensing started.");
  }
  else if (message == "stop_sensing")
  {
    startSensing = false;
    Serial.println("Sensing stopped.");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Reconnecting to MQTT...");
    if (client.connect("arduino-client"))
    {
      Serial.println("Connected to MQTT");
      client.subscribe("health_monitor_control");
    }
    else
    {
      Serial.println("MQTT connection failed, retrying in 5 seconds...");
      delay(5000);
    }
  }
}
