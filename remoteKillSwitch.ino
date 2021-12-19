#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Wire.h>

const char* ssid = "Not Creepy Van Outside";      // WiFi name
const char* password = "Athlete99!";              // WiFi password
const char* host = "192.168.1.3";                 // Local ESP8266 IP
const char* TSserver = "api.thingspeak.com";      // Thingspeak API server
const String apiKey = "8C5RJLITHO074GJ5";         // Thingspeak write API key

const int relayPin = 5;                           // Relay signal wire connected to pin D1 on ESP (D1 = GPIO 5)

WiFiServer server(301);


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
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(""); 

  server.begin();
  Serial.println("Server started");
  Serial.println("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Go to 199.120.96.230:301 to access heater controls");
}




void loop()
{
 
  WiFiClient client = server.available();
  
  if ( !client )                                  // Check if a client has connected
  {
    return;
  }

  if ( client )
  {
    delay(50);

    if ( client.available() )
    {
      String req = client.readStringUntil('\r');      // Read first line of request
      Serial.print("Client request: "); Serial.println(req);
      client.flush();

      if ( req.indexOf("") != -10 )                   // Match the client's request
      {
        if ( req.indexOf("/OFF") != -1 )              // Check for OFF request
        {
          digitalWrite(relayPin, LOW);
          Serial.println("You clicked OFF");
        }
        if ( req.indexOf("/ON") != -1 )               // Check for ON request
        {
          digitalWrite(relayPin, HIGH);
          Serial.println("You clicked ON");
        }
      }

      else
      {
        Serial.println("Invalid request");
        client.stop();
        return;
      }

      String s = "HTTP/1.1 200 OK\r\n";                 // Prepare response to client
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html>\r\n";
      s += "<br><input type=\"button\" name=\"bl\" value=\"Turn heater ON \" onclick=\"location.href='/ON'\">";
      s += "<br><br><br>";
      s += "<br><input type=\"button\" name=\"bl\" value=\"Turn heater OFF\" onclick=\"location.href='/OFF'\">";
      s += "</html>\n";

      client.flush();

      client.print(s);                                  // Send the response to the client
      delay(1);
    }
    
  }

}
