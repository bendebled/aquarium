String pin = "";
String brightness = "";
bool pinSet = false;

void setup() {
  Serial.begin(9600);
}

void loop() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if(!pinSet){
      pin += inChar;
    }
    else{
      brightness += inChar;
    }
    
    if (inChar == ',') {
      pinSet = true;
    }
    else if (inChar == '\n') {
      setLedBrightness((byte)pin.toInt(), (byte)brightness.toInt());
      pin = "";
      brightness = "";
      pinSet = false;
    }
  }
}

void setLedBrightness(byte ledNo, byte brightness){
  analogWrite(ledNo, brightness);
}
