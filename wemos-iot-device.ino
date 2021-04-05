/**
Author: Thomas Lewis
Date: 5th April 2021
Description: 
Uses and light dependent resistor to assess how much light there currently is on your desk.
Gives feedback to the user by either lighting up the green LED (sufficient) or the red LED (insufficient).
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

const int LDR = A0;
const int greenLED = D3;
const int redLED = D2;
int ldr_val = 0;
char api_url[255] = "http://192.168.178.186/api/determine/";

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASSWORD");

}

void loop() {
  // read the value given by the LDR on A0
  ldr_val = analogRead(LDR);

  // convert the int value to a string
  char ldr_str[5];
  int num_base = 10;
  itoa(ldr_val, ldr_str, num_base);

  // print value to serial monitor
  Serial.print("LDR Value is: ");
  Serial.println(ldr_val);

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;
    // By using the underlying Stream, it bypasses the code that handles chunked transfer encoding, so we must switch to HTTP version 1.0.
    http.useHTTP10(true);

    // set up the api_url and add the ldr value to it
    strcat(api_url, ldr_str);

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, api_url)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // create a DynamicJsonDocument so we can stream the response to it
          DynamicJsonDocument doc(2048);
          deserializeJson(doc, http.getStream());
          Serial.println(doc["sufficient"].as<long>());
          // get the json value for sufficient
          long light_sufficient = doc["sufficient"].as<long>();

          // sufficient is either 1 (true) or 0 (false)
          if (light_sufficient > 0) {
            digitalWrite(greenLED, HIGH);
            digitalWrite(redLED, LOW);
          } else {
            digitalWrite(greenLED, LOW);
            digitalWrite(redLED, HIGH);
          }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  // do the request every 5 seconds
  delay(5000);
}
