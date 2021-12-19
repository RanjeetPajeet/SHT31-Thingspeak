#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <Arduino_JSON.h>


const String apiKey = "8C5RJLITHO074GJ5";                     // Thingspeak write API key
const String OWapiKey = "480be2db7d1b6c9e2b2c714aa5cf4bc5";   // OpenWeather API key
const char *ssid = "Not Creepy Van Outside";                  // WiFi name
const char *pass = "Athlete99!";                              // WiFi password
const char* server = "api.thingspeak.com";                    // Thingspeak API server
const int TMP = A0;                                           // Analog input pin for TMP36
const int led = 4;                                            // LED for successful read feedback

float tempmemory[3];                                          // 15 minute memory for previous temperatures

WiFiClient client;

String jsonBuffer1;
String jsonBuffer2;
const String serverPath = "http://api.weatherapi.com/v1/current.json?key=0e8c22cc6cb047ea8dc221200210112&q=52340";
const String OWserverPath = "http://api.openweathermap.org/data/2.5/weather?lat=41.706&lon=-91.661&appid=480be2db7d1b6c9e2b2c714aa5cf4bc5&units=imperial";

int firstiterate = 0;
int iterate = 1;



void setup()
{
  Serial.begin(115200);
  delay(10);

  Serial.println("Connecting to "); Serial.println(ssid);
  
  WiFi.begin(ssid, pass);

  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(); Serial.println("WiFi connected!"); Serial.println();
}



void loop()
{
  
  jsonBuffer1 = httpGETRequest(serverPath.c_str());
  if ( jsonBuffer1 == "{}" )                        // Check for empty payload
  {
    while ( jsonBuffer1 == "{}" )
    {
      String jsonBuffer1 = httpGETRequest(serverPath.c_str());
      delay(10000);                                 // Wait 10 seconds before trying to get new temperature from API
    }
  }
  JSONVar APIresult1 = JSON.parse(jsonBuffer1);  
  double x = APIresult1["current"]["temp_f"];       // Extract temperature from API payload
  float APItemp1 = (float)x;                        // Convert temperature from type double to type float
  
  jsonBuffer2 = httpGETRequest(OWserverPath.c_str());
  if ( jsonBuffer2 == "{}" )                        // Check for empty payload
  {
    while ( jsonBuffer2 == "{}" )
    {
      String jsonBuffer2 = httpGETRequest(OWserverPath.c_str());
      delay(10000);                                 // Wait 10 seconds before trying to get new temperature from API
    }
  }
  JSONVar APIresult2 = JSON.parse(jsonBuffer2);  
  double y = APIresult2["main"]["temp"];            // Extract temperature from API payload
  float APItemp2 = (float)y;                        // Convert temperature from type double to type float
  
  int TMPanalog = 0;
  TMPanalog = analogRead(A0);                       // Read TMP sensor
  double z = (0.58 * TMPanalog) - 58;               // Convert analog reading to temperature (F)
  float TMPtemp = (float)z;                         // Convert temperature from type double to type float
  
  float tf = (APItemp1 + APItemp2 + TMPtemp) / 3;   // Average all outside temperature readings
  
  if ( firstiterate < 3 )
  {
    if ( client.connect(server, 80) )               // If connected to api.thingspeak.com, send the data
    {
      delay(100);
      tempmemory[firstiterate] = tf;
      String sendData = apiKey+"&field2="+String(tf)+"\r\n\r\n";
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
      client.stop();
    }
  }
  
  if ( (iterate % 4) == 0 )
  {
    if ( client.connect(server, 80) )               // If connected to api.thingspeak.com, send the data
    {
      delay(100);
      float sum = tempmemory[0] + tempmemory[1] + tempmemory[2] + tf;
      float tavg = sum / 4;
      String sendData = apiKey+"&field2="+String(tavg)+"\r\n\r\n";
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
      client.stop();
      sum = 0;
      tavg = 0;
    }
    iterate = 0;
    tempmemory[iterate] = tf;
    iterate = 1;
  }
  else if ( (iterate % 4) != 0  &&  firstiterate > 3)
  {
    if ( client.connect(server, 80) )               // If connected to api.thingspeak.com, send the data
    {
      delay(100);
      float sum = tempmemory[0] + tempmemory[1] + tempmemory[2] + tf;
      float tavg = sum / 4;
      String sendData = apiKey+"&field2="+String(tavg)+"\r\n\r\n";
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
      client.stop();
      sum = 0;
      tavg = 0;
    }
    tempmemory[iterate-1] = tf;                     // Store temperature in memory
  }

  firstiterate += 1;
  iterate += 1;
  
  delay(300000);                                    // Wait 5 minutes
  
}






String httpGETRequest(const char* serverName) 
{
  WiFiClient client;
  HTTPClient http; 
  http.begin(client, serverName);
  
  delay(100);
  
  int httpResponseCode = http.GET();  
  String payload = "{}"; 
  
  if ( httpResponseCode > 0 ) payload = http.getString();
  
  else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  client.stop();
  return payload; 
}
