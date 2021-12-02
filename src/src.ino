// Implementación Miguel Pérez para Tomassito IOT

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "Cstring.h"
#include "myMQTTBroker.h"

#define echoPin 14
#define trigPin 12

long duration;
int distance; 

int MB2 = 2;
int MB1 = 4;
int MA2 = 0;
int MA1 = 5;

int buzzer = 13;

int statusCode;
const char* ssid = "Default_SSID";
const char* passphrase = "Default_Password";
String st;
String content;

bool testWifi(void);
void launchWeb(void);
void setupAP(void);

ESP8266WebServer server(80);
myMQTTBroker broker;

unsigned long Time;
unsigned long freeRam;

void setup()
{
  Serial.begin(115200);

  pinMode(MB2, OUTPUT);     
  pinMode(MB1, OUTPUT);
  pinMode(MA2, OUTPUT);
  pinMode(MA1, OUTPUT);

  pinMode(buzzer, OUTPUT);
  
  WiFi.disconnect();
  EEPROM.begin(512); //Inicializamos para guardar en EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    launchWeb();
    broker.init();
    broker.subscribe("#");
    return;
  }
  else
  {
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while (true)
  {
    delay(100);
    server.handleClient();
  }
}

void distanceGenerator(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);  
    
    duration = pulseIn(echoPin, HIGH);
    
    distance = duration * 0.034 / 2;
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {
    server.handleClient();

    distanceGenerator();
    
    int d = broker.tmst;
    int b = broker.buzz;
    Serial.print("Moving: ");
    Serial.println(b);

    // Aquí va lo bueno:
    digitalWrite(MA1, d & 1);           // 00 00 00 01 Left
    digitalWrite(MB1, (d >> 2) & 1);    // 00 00 01 00 Right
    digitalWrite(MA2, (d >> 4) & 1);    // 00 01 00 00 LBackMode
    digitalWrite(MB2, (d >> 6) & 1);    // 01 00 00 00 RBackMode

    if(b > 0){
      //tone(buzzer, b); 
    }

    broker.publish("tmst/distance", distance + "cm");
    
    delay(100);
    
    if(millis()-Time>1000)
    {
      Time=millis();
      if(ESP.getFreeHeap()!=freeRam)
      {
        freeRam=ESP.getFreeHeap();
      Serial.print("RAM:");
      Serial.println(freeRam);
      }
    }    
  }
}

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  {
    createAltWebServer();
  }
  else{
    createWebServer();
  }
  // Start the server
  server.begin();
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("NodeMCU", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createAltWebServer(){
    server.on("/", []() {
      server.send(200, "text/html", cstring);
    });
}

void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}
