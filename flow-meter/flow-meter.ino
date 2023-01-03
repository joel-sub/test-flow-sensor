#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <WiFiServer.h>
#include <ESP8266WiFi.h>
#define WIFI_SSID "casita"
#define WIFI_PASSWORD "1q2w3e4r5t"
#define PULSE_PIN D2  //gpio4  sensor de flujo


const int serverPort = 80;
WiFiServer server(serverPort);

// setings for flow sensor
volatile long pulseCount=0;
float calibrationFactor = 4.5;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
float totalLitres;
unsigned long oldTime;

void ICACHE_RAM_ATTR pulseCounter()
{
  pulseCount++;
}


void setup() 
{
  Serial.begin(9600);
  pinMode(D0, OUTPUT);

  Serial.print("/\n");
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0; 
  pinMode(PULSE_PIN, INPUT);
  attachInterrupt(PULSE_PIN, pulseCounter, FALLING);


WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi network with IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.print("Server started at http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(serverPort);



  if (WiFi.status() == WL_CONNECTED) {
  // The Arduino is connected to a network
  Serial.println("Connected to network YEYYYY :D");
} else {
  // The Arduino is not connected to a network
  Serial.println("Not connected to network buuuu :c");
}

}

int n = 0;

void loop() 
{
  // Check for new client connections
  WiFiClient client = server.available();
  if (!client) {
    // Serial.println("No client connected");
    return;
  }
  // Serial.println("Client connected");

  // Read the request and parse the path
  String request = client.readStringUntil('\r');
  int pathIndex = request.indexOf("GET /");
  if (pathIndex == -1) {
    Serial.println("Invalid request");
    return;
  }
  Serial.println("Valid request received");

   int endIndex = request.indexOf(" ", pathIndex + 5);
  String path = request.substring(pathIndex + 5, endIndex);

  // Check the request path and return the appropriate response
  if (path == "data") {
    handleData(client);
  } else {
    // Return a 404 Not Found response
    client.print("HTTP/1.1 404 Not Found\r\n\r\n");
  }

  // Close the client connection
  client.stop();

  if((millis() - oldTime) > 1000)    // Only process counters once per second
  {
    detachInterrupt(PULSE_PIN);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;     
    totalLitres = totalMilliLitres * 0.001;
    unsigned int frac;   
    Serial.print(" Caudal por minuto: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    Serial.print("  Caudal actual: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
    Serial.print("  Total consumido: ");             // Output separator
    Serial.print(totalLitres);
    Serial.println("Lt");

    pulseCount = 0;
    attachInterrupt(PULSE_PIN, pulseCounter, FALLING);
  }
}

// Callback function for handling data requests
void handleData(WiFiClient client) {
  // Send the HTTP headers
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Connection: close\r\n");
  client.print("Access-Control-Allow-Origin: *\r\n"); // Add the CORS header
  client.print("\r\n");

  // Send the serial data
  client.print(flowMilliLitres);
}

