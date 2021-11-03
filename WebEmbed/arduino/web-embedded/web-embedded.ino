#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>

// include EEPROM
#include <EEPROM.h>

// import "json help kit"
#include <ArduinoJson.h>  

// define EEPROM SIZE
#define SPI_FLASH_SEC_SIZE 1024

// define default ssid & pass


String apssid ;
String appass ;
String myapssid = "Esp32_Panupong"; // -[id]
String myappass = "12345678";

WebServer server(80);


void handleRoot() {
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "text/html", "hello from esp32!");
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApSetup(){
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <title> Connection</title> <style>body{}.input{padding: 5px;}.button{padding: 5px;}input{ width: 200px; padding: 5px; border: solid 1px black; border-radius: 10px;}button{font-family: inherit; padding: 6px; border: solid 1px gray; border-radius: 10px;}</style></head><body><div align='center'><div class='input'><label>SSID:</label><input id='ssid'></div><div class='input'><label>PASS: </label><input id='password'></div><div class='button'><button id='SubmitButton'>Submit</button></div></div></body><script>document.getElementById('SubmitButton').onclick = function() {console.log('Submit Button is click...');var xmlHttp = new XMLHttpRequest();xmlHttp.onreadystatechange = function() {if (xmlHttp.readyState == XMLHttpRequest.DONE) {if (xmlHttp.status == 200) {alert('Sending Success');} else if (xmlHttp.status == 500) {alert('Sending fail');}}};var data = JSON.stringify({ssid: document.getElementById('ssid').value,pass: document.getElementById('password').value});xmlHttp.open('POST', '/ap');xmlHttp.send(data);};document.getElementById('ReloadButton').click();</script> </html");
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApGet(){
  digitalWrite(LED_BUILTIN, LOW);
  
  // {"ssid" : "<apssid>", "pass", "<appass>"}
  String str = "";
  str += "{";
  str += "\"ssid\":\"" + apssid + "\", ";
  str += "\"pass\": \"" + appass + "\"";
  str += "}";
  
  server.send(200, "text/json", str); 
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApPost(){
  // json object
  // {ssid: <ssid>, pass: <pass>} -> args == 1
  
  digitalWrite(LED_BUILTIN, LOW);

  if (server.args() != 1){
    // web Client error
    server.send(400, "text/plain", "Argument Error!!"); 
  }
  else {
    String str = server.arg(0);
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, str);
    // server(board) error
    if (err) {
      server.send(500, "text/plain", "Server Error!!");
    }
    else {
      String _apssid = doc["ssid"].as<String>();
      String _appass = doc["pass"].as<String>();
      server.send(200, "text/plain", "Success");

      // protect write EEPROM when apssid = _apssid
      if(_apssid != apssid || _appass != appass){
        apssid = _apssid;
        appass = _appass;
        eepromWrite();
      }
    }
  }
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void eepromWrite(){
  // "@$" [n]<ssid>[m]<pass>
  char c;
  int addr = 0;
  unsigned char s, i;
  
  EEPROM.begin(SPI_FLASH_SEC_SIZE);

  // write "@$"
  c = '@'; EEPROM.put(addr, c); addr ++;
  c = '$'; EEPROM.put(addr, c); addr ++;

  // ssid
  s = (unsigned char)apssid.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++){
    c = apssid[i]; EEPROM.put(addr, c); addr ++;
  }
  // pass
  s = (unsigned char)appass.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++){
    c = appass[i]; EEPROM.put(addr, c); addr ++;
  }

  EEPROM.end();
}

void eepromRead(){
  // "@$" [n]<ssid>[m]<pass>
  char c;
  int addr = 0;
  unsigned char s, i;
  
  EEPROM.begin(SPI_FLASH_SEC_SIZE);

  // Read and Cheack "@$"
  char header[3] = {' ', ' ' ,'\0'};
  EEPROM.get(addr, header[0]); addr ++;
  EEPROM.get(addr, header[1]); addr ++;

  if (strcmp(header, "@$") != 0){
   eepromWrite();
   return;
  } else {
    //ssid
    EEPROM.get(addr, s); addr ++;
    apssid = "";
    for(i = 0;  i < s; i ++){
      EEPROM.get(addr, c); apssid = apssid + c; addr ++;
    }
    // pass
    EEPROM.get(addr, s); addr ++;
    appass = "";
    for(i = 0;  i < s; i ++){
      EEPROM.get(addr, c); appass = appass + c; addr ++;
    }
  }
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);

  eepromRead();
  
  // STA (Station) mode
  WiFi.begin(apssid.c_str(), appass.c_str());
  Serial.println("Connect to " + apssid + "");

  int counter = 0;
  do {
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print(".");
    counter += 1;
  } while(WiFi.status() != WL_CONNECTED && counter < 10);
  Serial.println("");
  if (WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    Serial.println("Fail To Connect..");
  }
  else{
    Serial.println("Connection Success..");
    Serial.println("IP address(STA mode): " + (WiFi.localIP()).toString());
    if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
    }
  }
  
  // get ChipId
  char temp[10];
  uint64_t chipid = ESP.getEfuseMac();
  sprintf(temp, "%04X", (uint16_t)(chipid >> 32));
  myapssid = myapssid + "-[" + String(temp);
  sprintf(temp, "%08X", (uint32_t)chipid);
  myapssid = myapssid + String(temp) + "]";
  Serial.println("AccessPoint ssid: " + myapssid);
  
  // AP mode (software)
  WiFi.softAP(myapssid.c_str(), myappass.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address(AccessPoint Mode): ");
  Serial.println(myIP);
  

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });


  //display ap config page [Frontend]
  server.on("/apsetup", handleApSetup);
  //webservices [Backend]
  server.on("/ap", HTTP_GET, handleApGet);
  server.on("/ap", HTTP_POST, handleApPost);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  // turn oof light
  digitalWrite(LED_BUILTIN, LOW);
}

void loop(void) {
  
  server.handleClient();
  
  delay(2); // allow the cpu to switch to other tasks
}
