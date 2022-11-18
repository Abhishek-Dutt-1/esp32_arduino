#include <Arduino.h>
#include "WiFi.h"

#define WIFI_NETWORK "JioFiber-403"
#define WIFI_PASSWORD "7259118416"
#define WIFI_TIMEOUT_MS 20000

WiFiServer server(80);
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";
String outputBuiltInState = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;
const int outputBuiltIn = LED_BUILTIN;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void connectToWiFi() {
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_NETWORK);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed!");
  } else {
    Serial.println("");
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
  }

  digitalWrite(LED_BUILTIN, LOW);
  server.begin();

}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  connectToWiFi();
}

void loop() {

  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client");
    String currentLine = "";
    while(client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            } else if (header.indexOf("GET /BuiltIn/on") >= 0) {
              Serial.println("GPIO BuiltIn on");
              outputBuiltInState = "on";
              digitalWrite(outputBuiltIn, HIGH);
            } else if (header.indexOf("GET /BuiltIn/off") >= 0) {
              Serial.println("GPIO BuiltIn off");
              outputBuiltInState = "off";
              digitalWrite(outputBuiltIn, LOW);
            }

            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");

            // Display current state, and ON/OFF buttons for LED_BUILTIN
            client.println("<p>LED_BUILTIN - State " + outputBuiltInState + "</p>");
            // If the output26State is off, it displays the ON button       
            if (outputBuiltInState=="off") {
              client.println("<p><a href=\"/BuiltIn/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/BuiltIn/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}