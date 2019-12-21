int i=0;

// put your setup code here, to run once:
void setup() {

  delay(2000);
  // open serial to print data to serial monitor
  // in serial monitor, set the baud rate to the same defined in the sketch, here 38400
  Serial.begin(38400);

  Serial.println("Hello world");

  unsigned long now=millis();
  
  Serial.print("millis() counter is ");
  Serial.println(now);
}

void loop() {
  delay(500);
  
  if (i==10) {
    Serial.println("The world is big?");
    i=1;
  }
  else
    i++;
    
  // put your main code here, to run repeatedly:
  Serial.println("How is the world?");

  delay(1000);
}
