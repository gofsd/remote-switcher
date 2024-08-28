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
DynamicJsonDocument jsonBodyReqData(capacity);

CronId id;

String generateId()
{
  return String(random(0xFFFFFF), HEX);
}

void setPinFromStrs(String, String);
void setPin(int, bool);
void handleBody(AsyncWebServerRequest, uint8_t, size_t, size_t, size_t);
bool getRebootStatus();
void buildJsonFromStructure(JsonObject &jsonObj, const String &path);

AsyncWebServer server(80);

bool shouldReboot = false;

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
  if (jsonData["settings"]["is_sta"].isNull())
  {
    jsonData["settings"]["is_sta"] = false;
  }
  if (jsonData["settings"]["sta_ssid"].isNull())
  {
    jsonData["settings"]["sta_ssid"] = "ESP32";
  }
  if (jsonData["settings"]["sta_password"].isNull())
  {
    jsonData["settings"]["sta_password"] = "Password1";
  }
  if (jsonData["settings"]["ap_ssid"].isNull())
  {
    jsonData["settings"]["ap_ssid"] = "AP_FOR_ESP32";
  }
  if (jsonData["settings"]["ap_password"].isNull())
  {
    jsonData["settings"]["ap_password"] = "Password1";
  }
  if (jsonData["settings"]["ap_connect_retry"].isNull())
  {
    jsonData["settings"]["ap_connect_retry"] = 3;
  }
}

void configureNetwork()
{
  if (jsonData["settings"]["is_sta"].as<bool>())
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(jsonData["settings"]["sta_ssid"].as<String>(), jsonData["settings"]["sta_password"].as<String>());
  }
  else
  {
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      WiFi.softAP(jsonData["settings"]["ap_ssid"].as<String>(), jsonData["settings"]["ap_password"].as<String>());

      if (WiFi.waitForConnectResult() != WL_CONNECTED)
      {
        delay(1000);
      }
    }
  }
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

void onRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  // Static buffer for accumulating the body
  static String requestBody;

  // Append received data to the requestBody
  requestBody.concat((const char *)data, len);

  // When the entire body is received
  if (index + len == total) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, requestBody);

    if (error)
    {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    String methodName;
    switch (request->method())
    {
    case HTTP_GET:
      methodName = "GET";
      break;
    case HTTP_POST:
      methodName = "POST";
      break;
    case HTTP_PUT:
      methodName = "PUT";
      break;
    case HTTP_PATCH:
      methodName = "PATCH";
      break;
    case HTTP_DELETE:
      methodName = "DELETE";
      break;
    default:
      methodName = "UNKNOWN";
      break;
    }
    jsonBodyReqData[request->url()][methodName] = doc;
    serializeJson(doc, Serial);
  }
}

void onFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (index == 0)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    // Here, you could open a file to save the uploaded data
  }

  Serial.printf("Received %u bytes\n", len);

  // Here, you could write the received data to a file

  if (final)
  {
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    // Here, you could close the file
  }
}

void handleCronRequest(AsyncWebServerRequest *request)
{
  
  String cronExpression = jsonBodyReqData[request->url()][request->method()]["cron"];

  createCronJob((char *)cronExpression.c_str());

  request->send(200, "application/json", "{\"message\": \"Task created\", \"id\": \"1\"}");
}

void start()
{

  init();
  server.on("/cron", HTTP_POST, handleCronRequest);

  server.on("/cron", HTTP_DELETE, [](AsyncWebServerRequest *request)
            {

              DynamicJsonDocument doc(512);
              doc = jsonBodyReqData[request->url()][request->method()];
    
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
  server.on("/pin", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              DynamicJsonDocument doc(512);
              doc = jsonBodyReqData[request->url()][request->method()];
              serializeJson(jsonBodyReqData, Serial);
              int pin = doc["pin"].as<int>();
              String mode = doc["mode"];
              int value = doc["value"].as<int>();

              if (trySetPin(doc["pin"].as<String>(), mode, value))
              {
                request->send(200, "application/json", "{\"status\":\"pin set\"}");
              }
              else
              {
                request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
                return;
              } });

  // Read (GET): Get current pin states
  server.on("/pin", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String jsonStr;
    serializeJson(jsonData, jsonStr);
    request->send(200, "application/json", jsonStr); });

  // Update (PUT): Update pin state
  server.on("/pin", HTTP_PUT, [](AsyncWebServerRequest *request)
            {
              DynamicJsonDocument doc(512);
              doc = jsonBodyReqData[request->url()][request->method()];
                            int pin = doc["pin"].as<int>();
              String mode = doc["mode"];
              int value = doc["value"].as<int>();

              if (trySetPin(doc["pin"].as<String>(), mode, value))
              {
                request->send(200, "application/json", "{\"status\":\"pin set\"}");
              }
              else
              {
                request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
                return;
              } });

  server.on("/pin", HTTP_DELETE, [](AsyncWebServerRequest *request)
            {

              DynamicJsonDocument doc(512);
              doc = jsonBodyReqData[request->url()][request->method()];
    serializeJson(doc, Serial);
    if (doc["pin"] != "") {
      int pin = doc["pin"].as<int>();
      jsonData["pins"].remove(doc["pin"].as<String>());
      pinMode(pin, INPUT);
      request->send(200, "application/json", "{\"status\":\"pin reset\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"No pin specified\"}");
    } });

  server.serveStatic("/", LittleFS, "/static/").setDefaultFile("index.html");
  server.onFileUpload(onFileUpload);
  server.onRequestBody(onRequestBody);
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