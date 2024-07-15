#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266HTTPClient.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include "AsyncJson.h"
#include <ESP_EEPROM.h>
#include "CronAlarms.h"
CronId id;

struct Settings
{
  char ssid[100];
  char password[100];

} settings;

void setPinFromStrs(String, String);
void setPin(int, bool);
// void getAll();
void handleBody(AsyncWebServerRequest, uint8_t, size_t, size_t, size_t);
bool getRebootStatus();
String isTrueStr(bool);
AsyncWebServer server(80);
const char *host = "192.168.4.1";
const char *port = "8080";
const char *TRUE = "true";
const char *FALSE = "false";
// const char* ssid = "netis_581C6C";
const char *ssid = "esp8266";
// const char* password = "password2115";
const char *password = "Password1";
const int kNetworkDelay = 1000;
const int kNetworkTimeout = 30 * 1000;
char result[]; // array to hold the result.
JsonDocument doc(1024);
DynamicJsonDocument updDoc(512);
DynamicJsonDocument updSettings(512);
bool shouldReboot = false;

const char *PARAM_MESSAGE = "message";
const char *PIN = "pin";
const char *VALUE = "value";

boolean toggle = false;
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}
void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  // Handle upload
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if (!index)
  {
    Serial.printf("BodyStart: %u B\n", total);
  }
  for (size_t i = 0; i < len; i++)
  {
    Serial.write(data[i]);
  }
  if (request->url() == "/setall")
  {
    deserializeJson(updDoc, data);
  }
  if (index + len == total)
  {
    Serial.printf("BodyEnd: %u B\n", total);
  }
}
void MorningAlarm()
{
  Serial.println("Alarm: - turn lights off");
}

void start()
{

  EEPROM.begin(sizeof(settings));
  if (EEPROM.percentUsed() >= 0)
  {
    Serial.printf("From memory: ");
    EEPROM.get(0, settings);
    Serial.println(settings.ssid);
    Serial.println(settings.password);
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.ssid, settings.password);
  }
  else
  {
    Serial.println("EEPROM size have been changed or there are no stored data in memory");
  }

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.softAP(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      delay(1000);
      Serial.printf("TEST");
    }
  }
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP Address: ");
  Serial.println(IP);

  Serial.println(WiFi.localIP());

  // server.onRequestBody(onBody);

  server.on("/cron", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                Serial.println("cron endpoint");

              //Cron.create("*/2 * * * * *", MorningAlarm, false);
              

    request->send(200, "text/plain", "test"); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hello, world"); });

  server.on("/pin", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam(PIN) && request->hasParam(VALUE)) {
            setPinFromStrs(request->getParam(PIN)->value(), request->getParam(VALUE)->value());
        }
        request->send(200, "text/plain", result); });

  server.on("/getall", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncResponseStream *response = request->beginResponseStream("application/json");

        serializeJson(doc, *response);
        request->send(response); });

  server.on("/set-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("ssid") && request->hasParam("password")) {
            strcpy(settings.ssid, request->getParam("ssid")->value().c_str());
            strcpy(settings.password, request->getParam("password")->value().c_str());
            EEPROM.put(0, settings);
            EEPROM.commit();
            shouldReboot = true;
        }

        request->send(200, "text/plain", "saved"); });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        boolean result = EEPROM.wipe();
        String message;
        if (result) {
            message = "All EEPROM data wiped";
        } else {
            message = "EEPROM data could not be wiped from flash store";
        }        
        shouldReboot = true;
        request->send(200, "text/plain", message); });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        setPin(5, toggle);
        setPin(16, false);
        setPin(4, false);
        setPin(0, false);
        setPin(2, false);
        setPin(14, false);
        setPin(12, false);
        setPin(13, false);
        toggle = !toggle;
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message); });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        shouldReboot = true;
        request->send(200, "text/plain", "reboot"); });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/setall", HTTP_POST, [](AsyncWebServerRequest *request)
            {
        for(int i = 0; i < 20; i++) {
            if (updDoc.containsKey(String(i))) {
                setPinFromStrs(String(i), isTrueStr(updDoc[String(i)]));
            }
        }
  request->send(200, "text/plain", result); }, NULL, onBody);
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";
  request->send(200, "application/json", json);
  json = String(); });

  server.onNotFound(notFound);

  server.begin();
}

void setPin(int pin, bool high)
{
  if (pin == 16)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 5)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 4)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 0)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 2)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 14)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 12)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  }
  else if (pin == 13)
  {
    if (high == true)
    {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
    }
    else
    {
      pinMode(pin, INPUT);
      digitalWrite(pin, LOW);
    }
  } // else if (pin == 15) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 3) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 1) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 10) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 9) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 8) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 11) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 7) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   } else if (pin == 6) {
  //     if (high == true){
  //       pinMode(pin, OUTPUT);
  //       digitalWrite(pin, HIGH);
  //     } else {
  //       pinMode(pin, INPUT);
  //       digitalWrite(pin, LOW);
  //     }
  //   }
}

void setPinFromStrs(String pin, String value)
{
  if ((pin.length() == 1 && isDigit(pin[0])) || (pin.length() == 2 && isDigit(pin[0]) && isDigit(pin[1])) && (value == TRUE || value == FALSE))
  {
    int i = pin.toInt();
    if (value == TRUE)
    {
      doc[pin] = true;
      setPin(i, true);
    }
    else
    {
      doc[pin] = false;
      setPin(i, false);
    }
  }
  serializeJson(doc, result);
}

// void getAll() {
//   WiFiClient client;

//   HTTPClient http;

//   Serial.print("[HTTP] begin...\n");

//   // Establish the connection
//   if (http.begin(client, "http://192.168.1.3:8080")) {

//     Serial.print("[HTTP] POST...\n");
//     // start connection and send HTTP header, set the HTTP method and request
//     // body
//     int httpCode = http.GET();

//     // httpCode will be negative on error
//     if (httpCode > 0) {
//       // HTTP header has been send and Server response header has been handled
//       Serial.printf("[HTTP] GET... code: %d\n", httpCode);

//       // file found at server
//       if (httpCode == HTTP_CODE_OK) {
//         // read response body as a string
//         String payload = http.getString();
//         Serial.println(payload);
//       }
//     } else {
//       // print out the error message
//       Serial.printf("[HTTP] POST... failed, error: %s\n",
//                     http.errorToString(httpCode).c_str());
//     }

//     // finish the exchange
//     http.end();
//   } else {
//     Serial.printf("[HTTP] Unable to connect\n");
//   }
// }

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if (!index)
  {
    Serial.printf("BodyStart: %u B\n", total);
  }
  for (size_t i = 0; i < len; i++)
  {
    Serial.write(data[i]);
  }
  if (index + len == total)
  {
    Serial.printf("BodyEnd: %u B\n", total);
  }
}
String isTrueStr(bool v)
{
  if (v)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

void tick()
{
  Cron.delay();
  if (shouldReboot)
  {
    delay(100);
    ESP.restart();
    Serial.println("Rebooting...");
  }
}