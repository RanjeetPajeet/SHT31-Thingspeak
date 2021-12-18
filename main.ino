#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <Arduino_JSON.h>

String apiKey = "8C5RJLITHO074GJ5";               // Thingspeak write API key
const char *ssid = "Not Creepy Van Outside";      // WiFi name
const char *pass = "Athlete99!";                  // WiFi password
const char* server = "api.thingspeak.com";        // Thingspeak API server

WiFiClient client;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

String jsonBuffer;
const String serverPath = "http://api.weatherapi.com/v1/current.json?key=0e8c22cc6cb047ea8dc221200210112&q=52340";



void setup()
{

  Serial.begin(115200);
  delay(10);

  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.println();

  Serial.println("SHT31 test");

  if ( ! sht31.begin(0x44) )                      // Set to 0x45 for alternate I2C address
  {
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

}





void loop()
{

  String jsonBuffer = httpGETRequest(serverPath.c_str());

  if ( jsonBuffer == "{}" )
  {
    while ( jsonBuffer == "{}" )
    {
      String jsonBuffer = httpGETRequest(serverPath.c_str());
      delay(10000);   // Wait 10 seconds before trying to get new temperature from API
    }
  }

  JSONVar APIresult = JSON.parse(jsonBuffer);

  double x = APIresult["current"]["temp_f"];
  float tout = (float)x;

  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  float tf = ( t * 1.8 ) + 32;
  float dt = tf - tout;

  if ( client.connect(server, 80) )               // "184.106.153.149" or api.thingspeak.com
  {
    String sendData = apiKey+"&field1="+String(tf)+"&field2="+String(tout)+"&field3="+String(dt)+"&field4="+String(h)+"\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(sendData.length());
    client.print("\n\n");
    client.print(sendData);   
  }

  if ( ! isnan(t) )                               // Check if 'is not a number'
  {
    Serial.print("Temp *F = ");
    Serial.println(tf);
    Serial.print("Outside Temp *F = ");
    Serial.println(tout);
    Serial.print("Delta Temp *F = ");
    Serial.println(dt);
  }
  else
  {
    Serial.println("Failed to read temperature!");
  }
  
  if ( ! isnan(h) )                               // Check if 'is not a number'
  {
    Serial.print("Humidity % = ");
    Serial.println(h);
  }
  else
  {
    Serial.println("Failed to read humidity!");
  }

  Serial.println();
  delay(60000);                                   // Wait 1 minute
  
}





String httpGETRequest(const char* serverName) 
{
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if ( httpResponseCode > 0 ) 
  {
    payload = http.getString();
  }
  
  else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload; 
}
