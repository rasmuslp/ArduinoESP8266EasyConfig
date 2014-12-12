void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
    while (!Serial1) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.println("Sketch started");
}

void loop() {
  if (Serial.available())
    Serial1.write(Serial.read());
  if (Serial1.available())
    Serial.write(Serial1.read());
}

