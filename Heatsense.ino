/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include <PubSubClient.h>
#include <DHT.h>
#define TOKEN "A1E-hnmAj3vsfHjtg64HU8ewdvX" 
#define MQTT_CLIENT_NAME "ESP32_1"
#define VARIABLE_LABEL "Temperature" 
#define DEVICE_LABEL "esp32" 
#define SENSOR 12
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

char mqttBroker[]  = "things.ubidots.com";
char payload[100];
char topic[150];
// Space to store values to send
char str_sensor[10];

WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}



void setup()
{
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    //WiFi.mode(WIFI_STA);
    Serial.println("Setup done");
    // WiFi connection set up
    WiFi.mode(WIFI_AP_STA);
     WiFi.disconnect();
    
    WiFi.beginSmartConfig();
    
      
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("SmartConfig done.");

   
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    //Wifi connection done

    pinMode(SENSOR, INPUT);
    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);  
    
    dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label
  
  //float sensor = analogRead(SENSOR); 
  float sensor = dht.readTemperature();
  if(isnan(sensor))
  {
    Serial.println("default 40.35");
    sensor=40.35;
  }
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(sensor, 4, 2, str_sensor);
  
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud ");
  Serial.println(payload);
  client.publish(topic, payload);
  client.loop();
  delay(1000);
}
