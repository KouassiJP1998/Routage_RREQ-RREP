// if you know you have an ESP8266 based board, uncomment next line
//#define ESP8266

// if you know you have an Heltec WiFi LoRa based board, uncomment next 
//#define ARDUINO_Heltec_WIFI_LoRa_32

#if defined ESP8266 || defined ARDUINO_ESP8266_ESP01
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

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

// Test LoRa gateway channel, channel ID 66794
String apiKey = "SGSH52UGPVAUYG3S";

const char* server = "api.thingspeak.com";

WiFiClient espClient;

#define TEMP_PIN_READ  A0 //will be pin 36 on the Heltec

//3.3v boards
//#define TEMP_SCALE  3300.0
//5.0V boards
#define TEMP_SCALE  5000.0

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

  unsigned long start=millis();
  
  while (WiFi.status() != WL_CONNECTED && millis()-start<15000) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  if (WiFi.status() == WL_CONNECTED) {
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
  else {
      Serial.println("WiFi not connected");
#ifdef OLED
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.drawString(0, 2, "Not connected");      
#endif          
  }
} 

void setup() {
  delay(3000); 
  Serial.begin(38400);

  // for the temperature sensor
  pinMode(TEMP_PIN_READ, INPUT);
  
  // Print a start message
  Serial.println(F("Simple Temp WiFi+ThingSpeak demo"));  

#ifdef OLED
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Temp+ThingSpeak"); 
#endif   

  setup_wifi();
}

void loop() {

    if (WiFi.status() != WL_CONNECTED) {

#ifdef OLED
        u8x8.clearLine(2);
        u8x8.clearLine(3);
        u8x8.drawString(0, 2, "Connecting to ");    
        u8x8.drawString(0, 3, ssid);   
#endif 
        unsigned long start=millis();
    
        while (WiFi.status() != WL_CONNECTED && millis()-start<15000) {
          Serial.print("Attempting to connect to WPA SSID: ");
          Serial.println(ssid);
          // Connect to WPA/WPA2 network
          WiFi.begin(ssid, password);
          delay(500);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to AP");
#ifdef OLED
        u8x8.clearLine(2);
        u8x8.clearLine(3);
        u8x8.drawString(0, 2, "Connected");    
        u8x8.drawString(0, 3,  WiFi.localIP().toString().c_str());   
#endif    
    }
    else {
        Serial.println("WiFi not connected");
#ifdef OLED
        u8x8.clearLine(2);
        u8x8.clearLine(3);
        u8x8.drawString(0, 2, "Not connected");      
#endif        
    }

    int value = analogRead(TEMP_PIN_READ);
    Serial.print(F("Reading "));
    Serial.println(value);

    float temp;
  
    // change here how the temperature should be computed depending on your sensor type
  
    //LM35DZ
    //the LM35DZ needs at least 4v as supply voltage
    //can be used on 5v board
    temp = (value*TEMP_SCALE/1024.0)/10;
    
    //set a defined value for testing
    //temp = 24.5;
  
    if (isnan(temp)) {
      Serial.println("Failed to read from Temp sensor!");
    }
    else {
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.println(" degrees Celcius");

#ifdef OLED
        String tString=String(temp);
        u8x8.clearLine(4);
        sprintf(oled_msg, "t=%s", tString.c_str()); 
        u8x8.drawString(0, 4, oled_msg); 
#endif 
             
      // "184.106.153.149" or api.thingspeak.com
      if (WiFi.status() == WL_CONNECTED && espClient.connect(server,80)) { 
          String postStr = apiKey;
          postStr +="&field3=";
          postStr += String(temp);
          postStr += "\r\n\r\n";
          
          espClient.print("POST /update HTTP/1.1\n");
          espClient.print("Host: api.thingspeak.com\n");
          espClient.print("Connection: close\n");
          espClient.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
          espClient.print("Content-Type: application/x-www-form-urlencoded\n");
          espClient.print("Content-Length: ");
          espClient.print(postStr.length());
          espClient.print("\n\n");
          espClient.print(postStr);
          
          Serial.println("Upload to Thingspeak");
  
#ifdef OLED
          u8x8.clearLine(5);
          u8x8.drawString(0, 5, "Upload to TS");
#endif         
          espClient.stop();    
        }      
    }
    
    delay(10000);
    
#ifdef OLED
    u8x8.clearLine(5);
    u8x8.drawString(0, 5, "Waiting...");  
#endif    
  
    delay(10000);  
 }  
