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
#include "AsyncJson.h"
#include "CronAlarms.h"
#include <LittleFS.h>
#include <map>

std::map<int, String> cronMap;
const size_t capacity = 1024;

DynamicJsonDocument jsonData(capacity);

CronId id;

String generateId()
{
  return String(random(0xFFFFFF), HEX);
}

void setPinFromStrs(String, String);
void setPin(int, bool);
// void getAll();
void handleBody(AsyncWebServerRequest, uint8_t, size_t, size_t, size_t);
bool getRebootStatus();
void buildJsonFromStructure(JsonObject &jsonObj, const String &path);

String isTrueStr(bool);
// void buildJsonFromStructure(JsonObject, String);

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
char result[1024]; // array to hold the result.
bool shouldReboot = false;




boolean toggle = false;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void MorningAlarm()
{
  Serial.println("Alarm: - turn lights off");
}

String getCronString(int alarmId)
{
  if (cronMap.find(alarmId) != cronMap.end())
  {
    return cronMap[alarmId];
  }

  return "Cron string not found";
}

bool trySetPin(String pin, String mod, int value)
{

  if (mod == "analogInput")
  {
    pinMode(pin.toInt(), INPUT);
    int analogValue = analogRead(pin.toInt());
    jsonData["pins"][pin]["mode"] = "analogInput";
    jsonData["pins"][pin]["value"] = analogValue;
  }
  else if (pin == "digitalInput")
  {
    pinMode(pin.toInt(), INPUT);
    int digitalValue = digitalRead(pin.toInt());
    jsonData["pins"][pin]["mode"] = "digitalInput";
    jsonData["pins"][pin]["value"] = digitalValue;
  }
  else if (mod == "digitalOutput")
  {
    pinMode(pin.toInt(), OUTPUT);
    digitalWrite(pin.toInt(), value);
    jsonData["pins"][pin]["mode"] = "digitalOutput";
    jsonData["pins"][pin]["value"] = value;
  }
  else if (mod == "pwmOutput")
  {
    pinMode(pin.toInt(), OUTPUT);
    analogWrite(pin.toInt(), value);
    jsonData["pins"][pin]["mode"] = "pwmOutput";
    jsonData["pins"][pin]["value"] = value;
  }
  else
  {
    return false;
  }
  return true;
}

void executeTask()
{
  int currentAlarmId = Cron.getTriggeredCronId();
  String cronStr;
  if (cronMap.find(currentAlarmId) != cronMap.end())
  {
    cronStr = cronMap[currentAlarmId];
  }
  else
  {
    cronStr = "Cron string not found";
  }

  JsonObject children = jsonData["cron"][cronStr];

  for (JsonPair kv : children)
  {

    JsonObject sibling = kv.value().as<JsonObject>();

    trySetPin(kv.key().c_str(), sibling["mode"].as<String>(), sibling["value"].as<int>());
  }
}

// Function to handle deleting a task
void deleteTask(AsyncWebServerRequest *request)
{
  String cron = request->getParam("cron")->value();

  if (request->hasParam("cron", true))
  {
    int cron = request->getParam("cron", true)->value().toInt();
    jsonData["cron"].remove(String(cron));
    pinMode(cron, INPUT); // Reset pin to default state
    request->send(200, "application/json", "{\"status\":\"cron reset\"}");
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"No cron specified\"}");
  }
}

int createCronJob(const char *cronExpression)
{
  int alarmId = Cron.create((char *)cronExpression, executeTask, false);
  cronMap[alarmId] = String(cronExpression); // Store the cron string with its ID

  Serial.print((char *)cronExpression);
  Serial.print(alarmId);
  Serial.print("from create job");
  return alarmId;
}

void initState()
{
  JsonObject doc = jsonData.to<JsonObject>();
  buildJsonFromStructure(doc, "/state");
}

void configureNetwork()
{
  if (!jsonData["settings"]["isAP"].as<bool>())
  {
    Serial.printf("From memory: ");
    Serial.println("esp32");
    Serial.println("Password1");
    WiFi.mode(WIFI_STA);
    WiFi.begin("esp32", "Password1");
  }
  else
  {
    Serial.println("EEPROM size have been changed or there are no stored data in memory");
  }

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.softAP("esp32", "Password1");

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
}

void init()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  initState();
  configureNetwork();
}

void processCronRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, data);

  if (error)
  {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  String cronExpression = doc["cron"];

  int pin = doc["pin"].as<int>();
  String mode = doc["mode"];
  int value = doc["value"].as<int>();

  createCronJob((char *)cronExpression.c_str());
  // trysetpin
  if (mode == "analogInput")
  {
    jsonData["cron"][cronExpression][String(pin)]["mode"] = "analogInput";
    jsonData["cron"][cronExpression][String(pin)]["value"] = value;
  }
  else if (mode == "digitalInput")
  {
    jsonData["cron"][cronExpression][String(pin)]["mode"] = "digitalInput";
    jsonData["cron"][cronExpression][String(pin)]["value"] = value;
  }
  else if (mode == "digitalOutput")
  {
    jsonData["cron"][cronExpression][String(pin)]["mode"] = "digitalOutput";
    jsonData["cron"][cronExpression][String(pin)]["value"] = value;
  }
  else if (mode == "pwmOutput")
  {
    jsonData["cron"][cronExpression][String(pin)]["mode"] = "pwmOutput";
    jsonData["cron"][cronExpression][String(pin)]["value"] = value;
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
    return;
  }

  request->send(200, "application/json", "{\"message\": \"Task created\", \"id\": \"1\"}");
}

void handleCronRequest(AsyncWebServerRequest *request)
{
  // This is the initial handler, usually used for parameter validation
  // or for tasks that don’t require access to the body.
  // No body is processed here, so just exit this handler.
}

void start()
{

  init();
  server.on("/cron", HTTP_POST, handleCronRequest, NULL, processCronRequestBody);

  server.on("/cron", HTTP_DELETE, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    serializeJson(doc, Serial);
    if (doc["cron"] == "") {
            request->send(400, "application/json", "{\"error\":\"No cron specified\"}");
    }
    jsonData["cron"].remove(doc["cron"].as<String>());
      Serial.println(doc["cron"].as<String>());
      serializeJson(jsonData, Serial);

    request->send(200, "application/json", "{\"status\":\"cron reset\"}"); });

  server.on("/cron", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                Serial.println("cron endpoint");

              

    request->send(200, "text/plain", "test"); });

  server.on("/getall", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncResponseStream *response = request->beginResponseStream("application/json");

        //serializeJson(doc, *response);
        request->send(response); });

  server.on("/set-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("ssid") && request->hasParam("password")) {
            strcpy("esp32", request->getParam("ssid")->value().c_str());
            strcpy("Password1", request->getParam("password")->value().c_str());
            shouldReboot = true;
        }

        request->send(200, "text/plain", "saved"); });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String message;
        if (true) {
            message = "All EEPROM data wiped";
        } else {
            message = "EEPROM data could not be wiped from flash store";
        }        
        shouldReboot = true;
        request->send(200, "text/plain", message); });
  // Route for root index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  // Route for root index.css
  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.css", "text/css"); });

  // Route for root index.js
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.js", "text/javascript"); });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        shouldReboot = true;
        request->send(200, "text/plain", "reboot"); });

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
      //json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
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

  // Create (POST): Set pin state
  server.on("/pin", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    serializeJson(doc, Serial);
    int pin = doc["pin"].as<int>();
    String mode = doc["mode"];
    int value = doc["value"].as<int>();

    if (trySetPin(doc["pin"].as<String>(), mode, value)) {
      request->send(200, "application/json", "{\"status\":\"pin set\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      return;
    }

   });

  // Read (GET): Get current pin states
  server.on("/pin", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String jsonStr;
    serializeJson(jsonData, jsonStr);
    request->send(200, "application/json", jsonStr); });

  // Update (PUT): Update pin state
  server.on("/pin", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    int pin = doc["pin"].as<int>();
    String mode = doc["mode"];
    int value = doc["value"].as<int>();

    if (trySetPin(doc["pin"].as<String>(), mode, value)) {
      request->send(200, "application/json", "{\"status\":\"pin set\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      return;
    }

 });

  // Delete (DELETE): Reset pin state
  server.on("/pin", HTTP_DELETE, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    serializeJson(doc, Serial);
    if (doc["pin"] != "") {
      int pin = doc["pin"].as<int>();
      jsonData["pins"].remove(doc["pin"].as<String>());
      pinMode(pin, INPUT);  // Reset pin to default state
      request->send(200, "application/json", "{\"status\":\"pin reset\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"No pin specified\"}");
    } });

  // Get all pins state
  server.on("/getAllPins", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    for (JsonPair kv : jsonData["pins"].as<JsonObject>()) {
      const char* key = kv.key().c_str();
      int pin = atoi(key);
      const char* mode = kv.value()["mode"];

      if (strcmp(mode, "analogInput") == 0) {
        jsonData["pins"][key]["value"] = analogRead(pin);
      } else if (strcmp(mode, "digitalInput") == 0) {
        jsonData["pins"][key]["value"] = digitalRead(pin);
      }
    }

    String jsonStr;
    serializeJson(jsonData, jsonStr);
    request->send(200, "application/json", jsonStr); });
  server.serveStatic("/", LittleFS, "/static/").setDefaultFile("index.html");

  server.onNotFound(notFound);

  server.begin();
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

void createFileStructure(JsonObject jsonObj, const String &path)
{
  for (JsonPair kv : jsonObj)
  {
    String key = kv.key().c_str();
    JsonVariant value = kv.value();

    // If the value is an object, create a directory and recurse
    if (value.is<JsonObject>())
    {
      String dirPath = path + "/" + key;
      LittleFS.mkdir(dirPath);
      createFileStructure(value.as<JsonObject>(), dirPath);
    }
    // If the value is a primitive type, create a file and write the value
    else
    {
      String filePath = path + "/" + key;
      File file = LittleFS.open(filePath, "w");
      if (file)
      {
        file.print(value.as<String>());
        file.close();
      }
    }
  }
}
// Function to build JSON from a file structure
void buildJsonFromStructure(JsonObject &jsonObj, const String &path)
{
  File root = LittleFS.open(path);

  if (!root || !root.isDirectory())
  {
    Serial.println("Failed to open directory or it's not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    String fileName = file.name();
    fileName.remove(0, path.length() + 1); // Remove path prefix from file name

    if (file.isDirectory())
    {
      // Create a new JsonObject for the directory
      JsonObject subObj = jsonObj.createNestedObject(fileName);
      buildJsonFromStructure(subObj, String(file.name()));
    }
    else
    {
      // Read file content and add it to the JSON object
      String value = file.readString();
      jsonObj[fileName] = value;
    }

    file = root.openNextFile();
  }
}