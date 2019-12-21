#define OLED

// if you know you have an ESP8266 based board, uncomment next line
//#define ESP8266

// if you know you have an Heltec WiFi LoRa based board, uncomment next 
//#define ARDUINO_Heltec_WIFI_LoRa_32

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

//you can also power the OLED screen with a digital pin, here pin 2
#define OLED_PWR_PIN 2
#endif
#endif

int i=1;

// put your setup code here, to run once:
void setup() {

  delay(2000);
  // open serial to print data to serial monitor
  // in serial monitor, set the baud rate to the same defined in the sketch, here 38400
  Serial.begin(38400);

#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#endif

#ifdef OLED
  u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_pxplustandynewtv_r);
  u8x8.drawString(0, 1, "Hello Arthur");
  u8x8.drawString(0, 2, "How are you?");
#endif

  Serial.println("Hello Arthur");

  unsigned long now=millis();
  
  Serial.print("millis() counter is ");
  Serial.println(now);
}

void loop() {
  delay(500);

  if (i==10) {
    Serial.println("Please answer me my friend...");
    i=1;
  }
  else
    i++;
    
  // put your main code here, to run repeatedly:
  Serial.println("What can I do for you...?");
  
#ifdef OLED
  u8x8.drawString(0, 3, "What can I do   ");
  u8x8.drawString(0, 4, "you my friend?  ");
#endif 

  delay(1000);

#ifdef OLED
  u8x8.drawString(0, 3, "****************");
  u8x8.drawString(0, 4, "++++++++++++++++");
#endif   

  delay(1000);
}
