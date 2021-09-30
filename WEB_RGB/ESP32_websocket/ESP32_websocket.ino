/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
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

const char* ssid = "Wi-Fi Luigi";
const char* password = "LuigiWiFi";

double hue_val = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <p class="state">hue: <span id="hue">%HUE%</span></p>
      <p><input type="range" id="slider" min="0" max="1" step="0.01" value="0.5" ></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    document.getElementById('hue').innerHTML = event.data;
  }
  function onLoad(event) {
    initWebSocket();
    initSlider();
  }
  function initSlider() {
    document.getElementById('slider').addEventListener('input', newVal);
    document.getElementById('slider').addEventListener('change', finalNewVal);
  }
  function newVal(){
    websocket.send(document.getElementById('slider').value.toString());
    document.getElementById('hue').innerHTML = document.getElementById('slider').value;
  }
  function finalNewVal(){
    websocket.send("end"+document.getElementById('slider').value.toString());
  }
</script>
</body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(hue_val));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String value = String((char*)data);
    if (value.startsWith("end")){
      hue_val = value.substring(3).toFloat();
      notifyClients;
    } else{
      hue_val = value.toFloat();
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
  if(var == "HUE"){
    return String(hue_val);
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  //RGB setup
  ledcSetup(R_ledChannel,freq,resolution);
  ledcSetup(G_ledChannel,freq,resolution);
  ledcSetup(B_ledChannel,freq,resolution);

  ledcAttachPin(R_pin,R_ledChannel);
  ledcAttachPin(B_pin,B_ledChannel);
  ledcAttachPin(G_pin,G_ledChannel);

  
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
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

uint8_t red, green, blue = 0;
double saturation, lighting;

void loop() {
  ws.cleanupClients();
  saturation = 1;
  lighting = 1;
  ColorConverter::HsvToRgb((double)hue_val,saturation,lighting,red,green,blue);  
  ledcWrite(R_ledChannel,red);
  ledcWrite(G_ledChannel,green);
  ledcWrite(B_ledChannel,blue);  
}
