
#include <PubSubClient.h>  
#include <WiFiClientSecure.h>
#include <WiFi.h>   

// shared variable
TaskHandle_t t0;
TaskHandle_t t1;

const char* ssid = "61009558";       
const char* wifi_password = "as562031";
const char* mqtt_server = "";  
const char* mqtt_username = ""; 
const char* mqtt_password = "";  
const char* clientID = "";  

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883,wifiClient); 

void setup() {
 pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
    //create a task to handle Mqtt (core 0)
  xTaskCreatePinnedToCore(
    tMqtt,       
    "Mqtt",       
    10000,        
    NULL,         
    1,            
    &t0,          
    1);         
  delay(500);
  // create a task to handle led LED_BUILTIN (core 1)
  xTaskCreatePinnedToCore(
    tLedFunc,     
    "Led",       
    10000,        
    NULL,         
    1,           
    &t1,          
    0);           
  delay(500);
}

void loop() {
  Serial.print("Loop running on core ");
  Serial.println(xPortGetCoreID());
  delay(1000);
}
//setup_wifi
void tWifi(){    
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);
  client.setServer(mqtt_server, 1883);
  
 while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);   
    Serial.println("Turn off led when unable to connect to Wifi.");
    delay(500);
    Serial.print(".");
 }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   //connect MQTT Broker
   if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Set on loop Core1 : Connected MQTT Broker!");
  }
  else {
    Serial.println("Set on loop Core1 : Connection MQTT Broker fail.");
  }
}
  
#define MSG_BUFFER_SIZE  (50)
long trylastReconnect = 0;
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;

boolean reconnect() {
  Serial.println("reconnect active");
  if (!client.connected() ) {
    Serial.println("connecting...");
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.println("Connected MQTT broker waiting publish");
      
      client.subscribe("#");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5sec");
    }
  }
  return client.connected();
}

void tMqtt(void *params){
  // local Variables
  bool lastState = false;
  Serial.print("tMqtt running on core ");
  Serial.println(xPortGetCoreID());
  // loop
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); 
    Serial.println("turn off led when cannot connect Wifi");
    delay(1000);
   }  
    if(WiFi.status() == WL_CONNECTED){
    Serial.println("loop core0 : Connect WIFI, turn on led 500ms ");  
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.println("loop core0 : Connect WIFI, turn off led 500ms ");  
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500); 
    }
    else if(client.connected()){
    digitalWrite(LED_BUILTIN, LOW);  
    Serial.println("loop core0 : Connect MQTT, Turn on led ");
    }
  } 
}
// tLedFunc: blinks every 1000ms
void tLedFunc(void *params) {
  // local variable
  
  // setup  
  Serial.print("tLedFunc running on core ");
  Serial.println(xPortGetCoreID());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // connect WiFi
  tWifi();
  // Connect MQTT 
  while (true) {
    if (!client.connected()) {
    long now = millis();
    if (now - trylastReconnect > 5000) {
      trylastReconnect = now;
      Serial.println("Loop core 1 : Counting.. ,not connect");
      Serial.println("Counter is reset : 0");
      Serial.print("Counter Change to : ");
      Serial.println(trylastReconnect);
      // try to reconnect
      if (reconnect()) {
        trylastReconnect = 0;
        Serial.println("loop core 1 : Reconnecting..., turn off LED: ");   
      }
    }
  }
    else if (client.connected()) {
    client.loop();
    Serial.println("loop core1 : Connected...");
    Serial.print("IP address:  ");
    Serial.println(WiFi.localIP()); 
      
     client.disconnect();
     delay(50);
  }
  delay(1000);
  }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("loop core1 : led swap");
    delay(1000);
}
