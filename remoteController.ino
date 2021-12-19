#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Wire.h>

const char* ssid = "Not Creepy Van Outside";      // WiFi name
const char* password = "Athlete99!";              // WiFi password
const char* host = "192.168.1.4";                 // Local ESP8266 IP
const char* TSserver = "api.thingspeak.com";      // Thingspeak API server
const String apiKey = "8C5RJLITHO074GJ5";         // Thingspeak write API key

const int relayPin = 5;                           // Relay signal wire connected to pin D1 on ESP (D1 = GPIO 5)
const int port = 420;                             // Port through which requests are routed through

WiFiServer server(420);

bool stateChange = false;
int state;


void setup()
{
  Serial.begin(115200);
  delay(10);
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.println(); Serial.println();
  Serial.print("Connecting to "); Serial.println(ssid);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(); Serial.println("WiFi connected"); Serial.println();

  server.begin();
  
  Serial.println("Server started"); Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Go to 199.120.96.230:"); Serial.print(port);
  Serial.println(" to access heater controls");
}



void loop()
{

  if ( stateChange == false )
  {   
    WiFiClient client = server.available();
    
    if ( !client ) {return;}                            // Check if a client has connected

    if ( client )
    {
      delay(100);
  
      if ( client.available() )
      {
        delay(100);
        String req = client.readStringUntil('\r');      // Read first line of request
        client.flush();
  
        if ( req.indexOf("") != -10 )                   // Match the client's request
        {
          if ( req.indexOf("/OFF") != -1 )              // Check for OFF request
          {
            digitalWrite(relayPin, LOW);                // Turn relay off
            Serial.println("OFF");
            state = 0;
            stateChange = true;
          }
          if ( req.indexOf("/ON") != -1 )               // Check for ON request
          {
            digitalWrite(relayPin, HIGH);               // Turn relay on
            Serial.println("ON");
            state = 1;
            stateChange = true;
          }
        }
  
        else
        {
          Serial.println("Invalid request");
          client.stop();
          return;
        }       
  
        String s = "HTTP/1.1 200 OK\r\n";               // Build response to client
        s += "Content-Type: text/html\r\n\r\n";
        s += "<!DOCTYPE HTML>\r\n<html>\r\n";
        s += "<br><input type=\"button\" name=\"bl\" value=\"Turn heater ON \" onclick=\"location.href='/ON'\">";
        s += "<br><br><br>";
        s += "<br><input type=\"button\" name=\"bl\" value=\"Turn heater OFF\" onclick=\"location.href='/OFF'\">";
        s += "</html>\n";
  
        client.flush();
        client.print(s);                                // Send the response to the client
        delay(100);
        client.stop();
      }    
    }
  }

  
  else if ( stateChange = true )                        // Send updated heater state to Thingspeak
  {
    WiFiClient client;
    delay(100);
    for ( int c = 0; c < 3; c++ )                       // Spam 3 times
    {
      if ( client.connect(TSserver, 80) )
      {
        delay(100);
        String sendData = apiKey+"&field4="+String(state)+"&field5="+String(state)+"\r\n\r\n";
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(sendData.length());
        client.print("\n\n");
        client.print(sendData);
        delay(100); 
      }
    stateChange = false;
    delay(1000);
    }
  client.stop();
  }
}
