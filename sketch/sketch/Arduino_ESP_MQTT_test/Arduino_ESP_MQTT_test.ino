// if you know you have an ESP8266 based board, uncomment next line
//#define ESP8266

// if you know you have an Heltec WiFi LoRa based board, uncomment next line
//#define ARDUINO_Heltec_WIFI_LoRa_32

#if defined ESP8266 || defined ARDUINO_ESP8266_ESP01
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <PubSubClient.h>

// uncomment if you connect an OLED screen
//#define OLED 
 
#ifdef OLED
#include <U8x8lib.h>

// connection may depend on the board. Use A5/A4 for most Arduino boards. On ESP8266-based board we use GPI05 and GPI04. Heltec ESP32 has embedded OLED.
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#elif defined ESP8266 || defined ARDUINO_ESP8266_ESP01
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 14, /* reset=*/ U8X8_PIN_NONE);

#else
//reset is not used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);

//you can also power the OLED screen with a digital pin, here pin 8
#define OLED_PWR_PIN 8
#endif
char oled_msg[20];
#endif

// http://esp8266.github.io/Arduino/versions/2.3.0/doc/libraries.html
// https://openhomeautomation.net/esp8266-battery/

// Update these with values suitable for your network.

char* ssid = "iPhoneD";
char* password = "345hello";

char *topicin = "UPPA/test"; 
char *topicout = "UPPA/test"; 
char *msgTemp  = "22.5"; 
char* mqtt_server = "test.mosquitto.org";

//char* mqtt_server = "broker.mqttdashboard.com";
// go to http://www.hivemq.com/demos/websocket-client/ and subscribe to UPPA/test
// or mosquitto_sub -h broker.mqttdashboard.com -i 'client12345' -t 'UPPA/test'

WiFiClient espClient;
PubSubClient client(espClient);

int WiFi_status = WL_IDLE_STATUS;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

#ifdef OLED
  u8x8.clearLine(2);
  u8x8.clearLine(3);
  u8x8.drawString(0, 2, "Connecting to ");    
  u8x8.drawString(0, 3, ssid);   
#endif 

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
#ifdef OLED
  u8x8.clearLine(2);
  u8x8.clearLine(3);
  u8x8.drawString(0, 2, "Connected");    
  u8x8.drawString(0, 3, WiFi.localIP().toString().c_str());   
#endif   
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    // do something
  } else {
    // do something else
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
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
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

void setup() {

  delay(3000);
  Serial.begin(38400);

  // Print a start message
  Serial.println(F("Simple MQTT demo"));
  
#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#endif
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Simple MQTTDemo"); 
#endif    
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  
  //client.loop();

  WiFi_status = WiFi.status();
  
  if ( WiFi_status != WL_CONNECTED) {

#ifdef OLED
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0, 2, "Connecting to ");    
    u8x8.drawString(0, 3, ssid);   
#endif 

    while ( WiFi_status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      WiFi_status = WiFi.begin(ssid, password);
      delay(500);
    }
    Serial.println("Connected to AP");
#ifdef OLED
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0, 2, "Connected");    
    u8x8.drawString(0, 3,  WiFi.localIP().toString().c_str());   
#endif    
  }
   
  Serial.print("Publishing: status=");

  int e=client.publish(topicout, msgTemp);
  Serial.println(e); 

#ifdef OLED
  u8x8.clearLine(4);
  u8x8.clearLine(5);
  u8x8.drawString(0, 4, "MQTT publish");
  if (e==1)    
    u8x8.drawString(0, 5, "ok");
  else  
    u8x8.drawString(0, 5, "error");   
#endif  

  delay(2500);

#ifdef OLED
  u8x8.clearLine(4);
  u8x8.clearLine(5);
  u8x8.drawString(0, 4, "Waiting...");  
#endif 
  
  delay(7500);   
 }  
