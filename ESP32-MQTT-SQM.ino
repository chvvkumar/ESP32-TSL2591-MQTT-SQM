#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include "wifi.h"
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

//============ Setup Sensor============
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

//============ Setup WIFI and MQTT Details============
WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument doc(256);

const char* ssid        = WIFI_SSID;
const char* password    = WIFI_PASS;
const char* host        = NETWORK_HOSTNAME;
const char* mqtt_broker = MQTT_BROKER;
const int mqtt_port     = MQTT_PORT;
String clientId         = MQTT_CLIENT;
const long interval     = MQTT_UPDATE_INTERVAL;
const char* sqm_json     = MQTT_SQM_JSON_TOPIC;
char json_string[256];
unsigned long previousMillis = 0;

//============ Configure Sensor============
void configureSensor(void)
{
  //Set gain to auto
  //tsl.setGain(TSL2591_GAIN_MAX);   // 9876x gain
  tsl.enableAutoRange(true);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
}
// Measure actual dark readings for IR and Full and enter them here.
// The datasheet suggest typically between 0 and 20, but I got readings between 0 and 1
// Simple run the program with zero entered and note the IR and Full readings with the sensor completely blanked off.
int DARK_IR = 0;
int DARK_FULL = 0;

//  And variables for IR, FULL, Lux and SQM so we can manipulate them
unsigned long IR, FULL;
float LUX, SQM, VISIBLE, INFRARED;

// This factor is used to calculate visual magnitude from Lux. Default is 1.00 which assumes
// the sensor reads accurately in Lux, change it until magnitude readings scale correctly.
// You can calculate the factor from an measures SQ value and the corect one with this formula:
//  CORRECTION_FACTOR  = 10 ^ (measured SQ * -0.4) / (10 ^ (actual SQ * -0.4)
float CORRECTION_FACTOR = 1; //2.454L;

// This factor is used to oversample the signal to get better resolution at lower light levels. It relies on the
// inherent 'dither' of readings due to inherent random noise in both the chip and low level signals.
int OVERSAMPLE = 4;

//============ WiFi Setup function ============
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}


//============ MQTT Reconnect Function ============
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      //if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//============ Setup web server for OTA ============
WebServer server(80);
/* Login page */
const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>ESP32 Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

/* Server Index Page */
const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

template <class T>
String type_name(const T&)
{   
    String s = __PRETTY_FUNCTION__;

    int start = s.indexOf("[with T = ") + 10;
    int stop = s.lastIndexOf(']');

    return s.substring(start, stop);
}

void jsonCreation() {
//  JsonObject SQM_Meter = doc.createNestedObject("SQM_Meter");
  doc["SQM"] = SQM;
  doc["VISIBLE"] = VISIBLE;
  doc["INFRARED"] = INFRARED;
  doc["LUX"] = LUX;
  char buffer[256];
  serializeJson(doc, buffer);
  int i = client.publish(sqm_json, buffer);
  Serial.println("----- JSON Data -----");
  Serial.println(buffer); 
  Serial.println("---------------------");
  Serial.print("Successeses: ");    Serial.println(i);
  Serial.print("sqm_json type: ");  Serial.println(type_name(sqm_json));
  Serial.print("buffer type: ");    Serial.println(type_name(buffer));
  Serial.println("---------------------");
}

//============ Primary Setup ============
void setup(void)
{
  Serial.begin(9600);
  delay(10000);
  setup_wifi();
  Serial.println("Attempting MQTT connection...");
  client.setServer(mqtt_broker, mqtt_port);
  client.setBufferSize(512);
  if (client.connect(clientId.c_str())) {
    Serial.print("MQTT connected, client state: ");
    Serial.println(client.state());
  }

  Serial.println("Testing TSL2591 connection...");
  if (tsl.begin())
  {
    Serial.println("Found a TSL2591 sensor");
  }
  else
  {
    Serial.println("No sensor found ... check your wiring?");
    while (1);
  }

  /* Configure the sensor */
  configureSensor();

  //============ Setup server ============
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://arduesp32sqm.lan
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

//============ Function to read IR and Full Spectrum at once and convert to lux ============
void advancedRead(void)
{
  // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;

  //add to global values (allow for oversampling)
  IR = IR + ir - DARK_IR;
  FULL = FULL + full - DARK_FULL;

  VISIBLE   = full - ir;
  INFRARED  = ir; 
}

//============ Calculate Results ============
void Results(void)
{
  IR = 0;
  FULL = 0;
  for (int reads = OVERSAMPLE; reads > 0; reads--) {
    advancedRead();
  }
  LUX = tsl.calculateLux(FULL, IR) * CORRECTION_FACTOR / OVERSAMPLE;
  SQM = (log10(LUX / (108000)) / -0.4);
}

//============ Primary loop ============
void loop(void)
{
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Results();
    jsonCreation();
  }
}
