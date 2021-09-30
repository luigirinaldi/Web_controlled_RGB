/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <ColorConverterLib.h>

//RGB stuff
#define R_pin 16
#define G_pin 17
#define B_pin 5

const int freq = 5000;
const int R_ledChannel = 0;
const int G_ledChannel = 1;
const int B_ledChannel = 2;
const int resolution = 8; //number of bits

float hue_val = 0;
float sat_val = 1;
float bri_val = 1;

// Replace with your network credentials
const char* ssid = "Wi-Fi Luigi";
const char* password = "LuigiWiFi";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  char msg[18];
  sprintf(msg,"H%.3fS%.3fL%.3f",hue_val,sat_val,bri_val);
  Serial.println(msg);
  ws.textAll(msg);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    bool notify = false;
    String value = String((char*)data);
    if (value.startsWith("getC")){
      notifyClients();
    } else {    
      Serial.print("new val:");
      Serial.println(value);
      //val is H*.***S*.***L*.***
      hue_val = value.substring(1,6).toFloat();
      sat_val = value.substring(7,12).toFloat();
      bri_val = value.substring(13,18).toFloat();
      char msg[50];
      sprintf(msg,"new col: H%.3fS%.3fL%.3f\n",hue_val,sat_val,bri_val);
      Serial.println(msg);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "VALUE"){
    return String(hue_val);
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  ledcSetup(R_ledChannel,freq,resolution);
  ledcSetup(G_ledChannel,freq,resolution);
  ledcSetup(B_ledChannel,freq,resolution);

  ledcAttachPin(R_pin,R_ledChannel);
  ledcAttachPin(B_pin,B_ledChannel);
  ledcAttachPin(G_pin,G_ledChannel);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/index.html");
  if(!file){
    Serial.println("Failed to open index.html");
    return;
  }

  /*
  Serial.println("index.html content:");
  while(file.available()){
    Serial.write(file.read());
  }
  Serial.write("\n");
  file.close();*/
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send(SPIFFS, "/index.html", String(), false, processor);
    request->send(SPIFFS, "/index.html");
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/custom_range.css", "text/css");
  });


  // Start server
  server.begin();
}


uint8_t red, green, blue = 0;

void loop() {
  ws.cleanupClients();
  
  ColorConverter::HsvToRgb((double)hue_val,(double)sat_val,(double)bri_val,red,green,blue);  
  ledcWrite(R_ledChannel,red);
  ledcWrite(G_ledChannel,green);
  ledcWrite(B_ledChannel,blue);
 
  delay(10);
}
