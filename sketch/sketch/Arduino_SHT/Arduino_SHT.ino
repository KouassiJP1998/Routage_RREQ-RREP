// if you know you have an ESP8266 based board, uncomment next line
//#define ESP8266

// if you know you have an Heltec WiFi LoRa based board, uncomment next 
//#define ARDUINO_Heltec_WIFI_LoRa_32

#include "Sensirion.h"

#define SHTD_PIN 2
#define SHTC_PIN 3
#define SHT2x

//we can also power the SHT with a digital pin, here pin 6
#define PIN_POWER 6

#ifdef SHT2x
Sensirion sht = Sensirion(SHTD_PIN, SHTC_PIN, 0x40);
#else
Sensirion sht = Sensirion(SHTD_PIN, SHTC_PIN);
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

float h;
float t;
int ret;
int retry=0;
    
void setup() {

  delay(3000);
  Serial.begin(38400);

  // and to power the temperature sensor
  pinMode(PIN_POWER,OUTPUT);

#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#endif
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Arduino SHT"); 
#endif   
}

void loop () {

    digitalWrite(PIN_POWER,HIGH);
    delay(1000);
    
    while ( (ret != S_Meas_Rdy) ) {
      ret=sht.measure(&t, &h);
      retry++;
      Serial.print("[");
      Serial.print(ret);
      Serial.print(":");
      Serial.print(retry);
      Serial.print("] ");      
    }

    if (ret != S_Meas_Rdy) {
        Serial.println("Failed to read from SHT sensor!");
#ifdef OLED
        u8x8.clearLine(4);
        u8x8.clearLine(5);
        u8x8.drawString(0, 4, "error SHT...");  
#endif     
    }
    else {         
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.print(" degrees Celcius Humidity: ");
        Serial.print(h);
        Serial.println("%");    

#ifdef OLED
        String tString=String(t);
        String hString=String(h);
        u8x8.clearLine(4);
        sprintf(oled_msg, "t=%s h=%s", tString.c_str(), hString.c_str()); 
        u8x8.drawString(0, 4, oled_msg); 
#endif       
    }

    digitalWrite(PIN_POWER,LOW);
    ret=0;
    retry=0;
    delay(5000);
}
