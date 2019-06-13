/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "pitches.h"

// Update these with values suitable for your network.

#define buttonPin D3
#define buzzer D5 //Buzzer control port, default D5
#define OCTAVE_OFFSET 0

//const char* mqtt_server = "broker.hivemq.com";

const int notes[] = { 
  0,
  NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4, //scale of notes and their frequencies
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
  NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
  NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String mqttname;
char mqttnametemp[50];
char host[50];
char mqtt_server[100];

void setup_wifi() {
WiFiManager wifiManager;
wifiManager.autoConnect("DoorBell");
/*  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
  }
*/
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void happyNoise(){
  int note = (int)random(13,39);
  beep(buzzer,notes[note], 200);
  delay(200);
  //tone1.stop();
  beep(buzzer,notes[note+1],100);
  delay(100);
  //tone1.stop();
  beep(buzzer,notes[note+2],100);
  delay(100);
  //tone1.stop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    char lwtopic[100];
    String lwt = "cmnd/"+ mqttname + "/MESSAGE";
    lwt.toCharArray(lwtopic, lwt.length()+1);
    if (client.connect(clientId.c_str(),lwtopic,0,0,"Door Bell Off Line")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      char pubTopic[100];
    String pub = "cmnd/"+mqttname+"/MESSAGE";
    pub.toCharArray(pubTopic, pub.length()+1);
      client.publish(pubTopic, "Door Bell On Line");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void otaUpdate() {
  ArduinoOTA.setHostname("DoorBell");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
    
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(buttonPin, INPUT);
   pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  Serial.begin(9600);

  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
yield();
  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }
  setup_wifi();
  otaUpdate();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();

if (digitalRead(buttonPin) == LOW) {
  bell();
  happyNoise();
  yield();
}

yield();
ArduinoOTA.handle();
}

void bell() {

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    char pubTopic[100];
    String pub = "cmnd/"+mqttname+"/IMAGE";
    pub.toCharArray(pubTopic, pub.length()+1);
    client.publish(pubTopic, "bell, FRONT DOOR!");
  }
}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration, unsigned long pause) {
pinMode (_pin, OUTPUT );
analogWriteFreq(frequency);
analogWrite(_pin,500);
delay(duration);
analogWrite(_pin,0);
delay(pause);
}

void beep (int speakerPin, float noteFrequency, long noteDuration){
  int x;
  // Convert the frequency to microseconds
  float microsecondsPerWave = 1000000/noteFrequency;
  // Calculate how many milliseconds there are per HIGH/LOW cycles.
  float millisecondsPerCycle = 1000/(microsecondsPerWave * 2);
  // Multiply noteDuration * number or cycles per millisecond
  float loopTime = noteDuration * millisecondsPerCycle;
  // Play the note for the calculated loopTime.
  for (x=0;x<loopTime;x++)
  {
  digitalWrite(speakerPin,HIGH);
  delayMicroseconds(microsecondsPerWave);
  digitalWrite(speakerPin,LOW);
  delayMicroseconds(microsecondsPerWave);
  }
}  

bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  strcpy(mqtt_server, (const char*)json["mqtt_server"]);
  strcpy(mqttnametemp,(const char*)json["mqttname"]);
  mqttname = String(mqttnametemp);
  strcpy(host,(const char*)json["host"]);
//statustop = "tele/" + mqttname + "/STATUS";

  return true;
}
