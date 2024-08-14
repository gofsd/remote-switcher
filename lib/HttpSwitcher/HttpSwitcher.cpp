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
#include "CronAlarms.h"
#include <LittleFS.h>
#include <map>

// Global map to store cron strings associated with their alarm IDs
std::map<int, String> cronMap;
    const size_t capacity = 1024;  // Adjust size based on your JSON structure

DynamicJsonDocument jsonData(capacity);

CronId id;

struct Settings
{
  char ssid[64];
  char password[64];
  boolean isSTA = true;
  char secret[64];
  char cer[2048];
  char key[2048];
  char date[20];
} settings;

// Data structure to hold your tasks
struct Task {
  String id;
  String name;
  String cronExpression;
  bool enabled;
};

std::vector<Task> tasks;

String generateId() {
  return String(random(0xFFFFFF), HEX);
}

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
char result[1024]; // array to hold the result.
//DynamicJsonDocument doc;
DynamicJsonDocument updDoc(512);
DynamicJsonDocument updSettings(512);
bool shouldReboot = false;

const char *PARAM_MESSAGE = "message";
const char *PIN = "pin";
const char *VALUE = "value";
char html_buffer[1024];
int max_len = sizeof html_buffer;
const char doctype[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <title> %s </title>
</head>

<body>
%s
</body>

</html>)rawliteral";

const char setting[] PROGMEM =  R"rawliteral(
    <form action="/setting">
      <label for="fname">First name:</label><br>
      <input type="text" id="fname" name="fname" value="John" disabled><br>
      <label for="lname">Last name:</label><br>
      <input type="text" id="lname" name="lname" value="Doe">
    </form>
)rawliteral";
    char* s = "geeksforgeeks";



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
Task newTask;

// Function to get the cron string associated with an alarm ID
String getCronString(int alarmId) {
    if (cronMap.find(alarmId) != cronMap.end()) {
        return cronMap[alarmId];
    } else {
        return "Cron string not found";
    }
}
void executeTask() {
    int currentAlarmId = Cron.getTriggeredCronId();
    String cronStr;
    if (cronMap.find(currentAlarmId) != cronMap.end()) {
        cronStr= cronMap[currentAlarmId];
    } else {
        cronStr= "Cron string not found";
    }

    JsonObject obj = jsonData["cron"][cronStr].as<JsonObject>();
    for (JsonPair kv : obj) {
      Serial.print(kv.key().c_str());

    }

    Serial.println("Task executed: " + newTask.name);
}

void setPin(String k, JsonObject obj) {
    if (obj["mode"] == "analogInput") {
      pinMode(k.toInt(), INPUT);
      int analogValue = analogRead(k.toInt());
      jsonData["pins"][k]["mode"] = "analogInput";
      jsonData["pins"][k]["value"] = analogValue;
    } else if (obj["mode"] == "digitalInput") {
      pinMode(k.toInt(), INPUT);
      int digitalValue = digitalRead(k.toInt());
      jsonData["pins"][k]["mode"] = "digitalInput";
      jsonData["pins"][k]["value"] = digitalValue;
    } else if (obj["mode"] == "digitalOutput") {
      pinMode(k.toInt(), OUTPUT);
      digitalWrite(k.toInt(), obj["value"].as<int>());
      jsonData["pins"][k]["mode"] = "digitalOutput";
      jsonData["pins"][k]["value"] = obj["value"].as<int>();
    } else if (obj["mode"] == "pwmOutput") {
      pinMode(k.toInt(), OUTPUT);
      analogWrite(k.toInt(), obj["value"].as<int>());
      jsonData["pins"][k]["mode"] = "pwmOutput";
      jsonData["pins"][k]["value"] = obj["value"].as<int>();
    }
}

// Function to handle getting all tasks
void getTasks(AsyncWebServerRequest *request) {
    StaticJsonDocument<500> doc;
    JsonArray array = doc.to<JsonArray>();

    for (const Task& task : tasks) {
        JsonObject obj = array.createNestedObject();
        obj["id"] = task.id;
        obj["name"] = task.name;
        obj["cronExpression"] = task.cronExpression;
        obj["enabled"] = task.enabled;
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

// Function to handle updating a task
void updateTask(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    String id = doc["id"].as<String>();
    String newName = doc["name"].as<String>();
    String newCronExpression = doc["cronExpression"].as<String>();
    bool newEnabled = doc["enabled"].as<bool>();

    for (Task& task : tasks) {
        if (task.id == id) {
            task.name = newName;
            task.cronExpression = newCronExpression;
            task.enabled = newEnabled;

            // Update the cron job here if needed
            break;
        }
    }

    request->send(200, "application/json", "{\"message\": \"Task updated\"}");
}



// Function to handle deleting a task
void deleteTask(AsyncWebServerRequest *request) {
    String id = request->getParam("id")->value();

    auto it = std::remove_if(tasks.begin(), tasks.end(), [&id](Task &task) {
        return task.id == id;
    });

    if (it != tasks.end()) {
        tasks.erase(it, tasks.end());
        request->send(200, "application/json", "{\"message\": \"Task deleted\"}");
    } else {
        request->send(404, "application/json", "{\"message\": \"Task not found\"}");
    }
}

int createCronJob(const char* cronExpression) {
    int alarmId = Cron.create((char*)cronExpression, executeTask, false);
    cronMap[alarmId] = String(cronExpression); // Store the cron string with its ID
    return alarmId;
}


void start()
{

    // Begin LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  if (true)
  {
    Serial.printf("From memory: ");
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
server.on("/task", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    String cronExpression = doc["cron"];

    int pin = doc["pin"].as<int>();
    String mode = doc["mode"];
    int value = doc["value"].as<int>();

    createCronJob((char*)cronExpression.c_str());

    if (mode == "analogInput") {
      jsonData["cron"][cronExpression][String(pin)]["mode"] = "analogInput";
      jsonData["cron"][cronExpression][String(pin)]["value"] = value;
    } else if (mode == "digitalInput") {
      jsonData["cron"][cronExpression][String(pin)]["mode"] = "digitalInput";
      jsonData["cron"][cronExpression][String(pin)]["value"] = value;
    } else if (mode == "digitalOutput") {
      jsonData["cron"][cronExpression][String(pin)]["mode"] = "digitalOutput";
      jsonData["cron"][cronExpression][String(pin)]["value"] = value;
    } else if (mode == "pwmOutput") {
      jsonData["cron"][cronExpression][String(pin)]["mode"] = "pwmOutput";
      jsonData["cron"][cronExpression][String(pin)]["value"] = value;
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      return;
    }



  request->send(200, "application/json", "{\"message\": \"Task created\", \"id\": \"" + newTask.id + "\"}");
});

    server.on("/tasks", HTTP_GET, getTasks);
    server.on("/task", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL,  updateTask);
    server.on("/task", HTTP_DELETE, deleteTask);

  // server.onRequestBody(onBody);

  server.on("/cron", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                Serial.println("cron endpoint");

              //Cron.create("*/2 * * * * *", MorningAlarm, false);
              

    request->send(200, "text/plain", "test"); });

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  //           {
  //             snprintf(html_buffer, max_len, doctype, s, setting);
  //             Serial.println(html_buffer);
  //             request->send(200, "text/html", html_buffer); });

  server.on("/getall", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncResponseStream *response = request->beginResponseStream("application/json");

        //serializeJson(doc, *response);
        request->send(response); });

  server.on("/set-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("ssid") && request->hasParam("password")) {
            strcpy(settings.ssid, request->getParam("ssid")->value().c_str());
            strcpy(settings.password, request->getParam("password")->value().c_str());
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

    // Create (POST): Set pin state
  server.on("/pin", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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

    if (mode == "analogInput") {
      pinMode(pin, INPUT);
      int analogValue = analogRead(pin);
      jsonData["pins"][String(pin)]["mode"] = "analogInput";
      jsonData["pins"][String(pin)]["value"] = analogValue;
    } else if (mode == "digitalInput") {
      pinMode(pin, INPUT);
      int digitalValue = digitalRead(pin);
      jsonData["pins"][String(pin)]["mode"] = "digitalInput";
      jsonData["pins"][String(pin)]["value"] = digitalValue;
    } else if (mode == "digitalOutput") {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, value);
      jsonData["pins"][String(pin)]["mode"] = "digitalOutput";
      jsonData["pins"][String(pin)]["value"] = value;
    } else if (mode == "pwmOutput") {
      pinMode(pin, OUTPUT);
      analogWrite(pin, value);
      jsonData["pins"][String(pin)]["mode"] = "pwmOutput";
      jsonData["pins"][String(pin)]["value"] = value;
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      return;
    }

    request->send(200, "application/json", "{\"status\":\"pin set\"}");
  });

  // Read (GET): Get current pin states
  server.on("/pin", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonStr;
    serializeJson(jsonData, jsonStr);
    request->send(200, "application/json", jsonStr);
  });

  // Update (PUT): Update pin state
  server.on("/pin", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    int pin = doc["pin"].as<int>();
    String mode = doc["mode"];
    int value = doc["value"].as<int>();

    if (mode == "analogInput") {
      pinMode(pin, INPUT);
      int analogValue = analogRead(pin);
      jsonData["pins"][String(pin)]["mode"] = "analogInput";
      jsonData["pins"][String(pin)]["value"] = analogValue;
      Serial.print(analogValue);
    } else if (mode == "digitalInput") {
      pinMode(pin, INPUT);
      int digitalValue = digitalRead(pin);
      jsonData["pins"][String(pin)]["mode"] = "digitalInput";
      jsonData["pins"][String(pin)]["value"] = digitalValue;
    } else if (mode == "digitalOutput") {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, value);
      jsonData["pins"][String(pin)]["mode"] = "digitalOutput";
      jsonData["pins"][String(pin)]["value"] = value;
    } else if (mode == "pwmOutput") {
      pinMode(pin, OUTPUT);
      analogWrite(pin, value);
      jsonData["pins"][String(pin)]["mode"] = "pwmOutput";
      jsonData["pins"][String(pin)]["value"] = value;
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      return;
    }

    request->send(200, "application/json", "{\"status\":\"pin updated\"}");
  });

  // Delete (DELETE): Reset pin state
  server.on("/pin", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    if (request->hasParam("pin", true)) {
      int pin = request->getParam("pin", true)->value().toInt();
      jsonData["pins"].remove(String(pin));
      pinMode(pin, INPUT);  // Reset pin to default state
      request->send(200, "application/json", "{\"status\":\"pin reset\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"No pin specified\"}");
    }
  });

  // Get all pins state
  server.on("/getAllPins", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    request->send(200, "application/json", jsonStr);
  });
server.serveStatic("/", LittleFS, "/static/").setDefaultFile("index.html");

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
      //doc[pin] = true;
      setPin(i, true);
    }
    else
    {
      //doc[pin] = false;
      setPin(i, false);
    }
  }
  //serializeJson(doc, result);
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