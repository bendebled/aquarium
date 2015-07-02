#include <Wire.h>
#include "U8glib.h"
#include <OneButton.h>
#include <Time.h>
#include <avr/pgmspace.h>
#include <DS1307RTC.h>

#define OLED_RESET 4
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); //SLOW
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST); //FASTER
bool displayDimmed = false;
bool displayOn = true;

#define SCREEN_DIM 5000
#define SCREEN_OFF 10000
#define TOTALLED 8
#define REGULAR_FONT u8g_font_7x13B
#define BIG_FONT u8g_font_fur20
#define RELAY_PIN 2

#define MODE_MANUAL 0
#define MODE_100 1
#define MODE_CLOUD 2
#define MODE_ECO 3
#define MODE_DEMO 4
#define MODE_OFF 10

byte leds[] = {3,9,10,11,103,109,110,111}; //if # > 100, pin is on second arduino

OneButton b1(A0, true);
OneButton b2(A1, true);
OneButton b3(A2, true);
OneButton b4(A3, true);

byte b1State = 0;
byte b2State = 0;
byte b3State = 0;
byte b4State = 0;

byte displayState = 0;
byte selectNo = 0;

char c[] = "    "; 

byte mode = 0;

byte manualBrightness = 100;
byte manualNoOfLed = 0;

byte demoStep = 0;
long demoStepStartTime = 0;

bool autoOnDone = false;
bool autoOffDone = false;

void setup()   {                
  Serial.begin(9600);

  //Init led pins :
  for(int i = 0; i < 4; i++){
    pinMode(leds[i],OUTPUT);
    analogWrite(leds[i], 0);
  }

  //Init relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Initialize DS1307
  setSyncProvider(RTC.get);
  setSyncInterval(3600);
  tmElements_t tm;
  while(!RTC.read(tm)){;}
  setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);

  u8g.setRot180();
  u8g.setColorIndex(1);
  u8g.setFont(REGULAR_FONT);
  displayAll();
}

void loop() {
  b1State = b1.tick();
  b2State = b2.tick();
  b3State = b3.tick();
  b4State = b4.tick();
  b1.setClickTicks(20);
  b2.setClickTicks(20);
  b3.setClickTicks(20);
  b4.setClickTicks(20);

  if(displayOn && !displayDimmed){
    handleButtons();
  }

  modeManagement();
  
  screenPowerManagement();

  autoControl();

  relay();
}

void handleButtons(){
  bool change = false;
  if(displayState == 0){
    if (b2State == b2.CLICK_STATE){
      if(selectNo > 0){
        selectNo--;
      }
      else{
        selectNo = 3;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%4;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      if(selectNo == 3){
        mode = MODE_OFF;
      }
      else{
        displayState = (selectNo+1);
        if(displayState == 1){
          mode = MODE_MANUAL;
        }
        if(displayState == 2 || displayState == 3){
          selectNo = 0;
        }
      }
      change = true;
    }
  }

  
  else if(displayState == 1){
    if (b2State == b2.CLICK_STATE){
      if(manualBrightness >= 5){
        manualBrightness = manualBrightness - 5;
      }
      change = true;
    }
    if (b2State == b2.DURING_LONG_PRESS_STATE){
      if(manualBrightness >= 5){
        manualBrightness = manualBrightness - 5;
      }
      delay(25);
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      if(manualBrightness <= 95){
        manualBrightness = manualBrightness + 5;
      }
      change = true;
    }
    if (b3State == b3.DURING_LONG_PRESS_STATE){
      if(manualBrightness <= 95){
        manualBrightness = manualBrightness + 5;
      }
      delay(25);
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = 10;
      change = true;
    }
  }

  
  else if(displayState == 2){
    if (b1State == b1.CLICK_STATE){
      displayState = 0;
      selectNo = 0;
      change = true;
    }
    if (b2State == b2.CLICK_STATE){
      if(selectNo > 0){
        selectNo--;
      }
      else{
        selectNo = 2;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%3;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      mode = selectNo+1;
      displayState = 0;
      selectNo = 0;
      change = true;
    }
  }

  
  else if(displayState == 3){
    if (b1State == b1.CLICK_STATE){
      displayState = 0;
      selectNo = 0;
      change = true;
    }
    if (b2State == b2.CLICK_STATE){
      if(selectNo > 0){
        selectNo--;
      }
      else{
        selectNo = 2;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%3;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      if(selectNo == 2){
        mode = MODE_DEMO;
        displayState = 0;
        selectNo = 0;
        demoStep = 0;
        demoStepStartTime = millis();
      }
      else{
        //TO-DO
        displayState = 0;
        selectNo = 0;
      }
      change = true;
    }
  }

  
  else if(displayState == 10){
    if (b2State == b2.CLICK_STATE){
      if(manualNoOfLed > 0){
        manualNoOfLed--;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      if(manualNoOfLed < TOTALLED){
        manualNoOfLed++;
      }
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = 0;
      change = true;
    }
  }
  
  if(change){
    displayAll();
  }
}

//void displayButtonMenu(String str){
//  display.setTextSize(1);
//  display.setCursor(1,56);
//  display.setTextColor(BLACK);
//  
//  display.fillRect(0, 55, 128, 9, WHITE);
//  
//  display.println(str);
//}

void displayHeader(){
  u8g.drawBox( 0, 0, 128, 12);
  u8g.setColorIndex(0);
  u8g.setPrintPos(1, 10); 
  u8g.print(getModeStr());
  u8g.setPrintPos(93, 10); 
  u8g.print(getTimeStr());
  u8g.setColorIndex(1);
  //u8g.print((char)0xb0);  // Will display the °Character
}

void displayContent(){
  
  if(displayState == 0){
    setChevrons(4);
    u8g.setPrintPos(1, 25); 
    u8g.print(c[0]);
    u8g.print(F(" Manual"));
    u8g.setPrintPos(1, 40); 
    u8g.print(c[1]);
    u8g.print(F(" Auto"));
    u8g.setPrintPos(1, 55); 
    u8g.print(c[2]);
    u8g.print(F(" Settings"));
    u8g.setPrintPos(80, 25); 
    u8g.print(c[3]);
    u8g.print(F(" OFF"));
  }
  
  else if(displayState == 1){
    u8g.setFont(BIG_FONT);
    
    if(manualBrightness == 100){
      u8g.drawBox( 30, 14, 75, 24);
      u8g.setPrintPos(30, 35);
    }
    else{
      u8g.drawBox( 35, 14, 60, 24);
      u8g.setPrintPos(37, 35);
    }
    u8g.setColorIndex(0);
    u8g.print(manualBrightness);
    u8g.print(F("%"));
    u8g.setColorIndex(1);
    u8g.setPrintPos(55, 62); 
    u8g.print(manualNoOfLed);
    u8g.setFont(REGULAR_FONT);
    
  }
  
  if(displayState == 2){
    setChevrons(3);
    u8g.setPrintPos(1, 25); 
    u8g.print(c[0]);
    u8g.print(F(" 100%"));
    u8g.setPrintPos(1, 40); 
    u8g.print(c[1]);
    u8g.print(F(" Cloud"));
    u8g.setPrintPos(1, 55); 
    u8g.print(c[2]);
    u8g.print(F(" Eco"));
  }
  
  else if(displayState == 3){
    setChevrons(3);
    u8g.setPrintPos(1, 25); 
    u8g.print(c[0]);
    u8g.print(F(" Set Temp"));
    u8g.setPrintPos(1, 40); 
    u8g.print(c[1]);
    u8g.print(F(" On/Off Time"));
    u8g.setPrintPos(1, 55); 
    u8g.print(c[2]);
    u8g.print(F(" Demo"));
  }
  
  else if(displayState == 10){
    u8g.setFont(BIG_FONT);
    if(manualBrightness == 100){
      u8g.setPrintPos(30, 35);
    }
    else{
      u8g.setPrintPos(37, 35);
    }
    u8g.print(manualBrightness);
    u8g.print("%");
    
    u8g.drawBox( 53, 40, 20, 24);
    u8g.setColorIndex(0);
    u8g.setPrintPos(55, 62); 
    u8g.print(manualNoOfLed);
    u8g.setColorIndex(1);
    u8g.setFont(REGULAR_FONT);
  }
}

void displayAll(){
  u8g.firstPage();  
  do {
    displayHeader();
    displayContent();
  } while( u8g.nextPage() );
}

void setChevrons(byte n){
  c[selectNo] = '>';
  for(byte i = 0; i < n; i++){
    if (i != selectNo){
      c[i] = ' ';
    }
  }
}

void screenPowerManagement(){
// U8GLIB does not support dimming
//  if(!displayDimmed && b1.getLastActivityTime() + SCREEN_DIM < millis() && b2.getLastActivityTime() + SCREEN_DIM < millis() && b3.getLastActivityTime() + SCREEN_DIM < millis() && b4.getLastActivityTime() + SCREEN_DIM < millis()){
//    display.dim(true);
//    display.display();
//    displayDimmed = true;
//  }
//  if(displayDimmed && (b1State == 1 || b2State == 1 || b3State == 1 || b4State == 1)){
//    display.dim(false);
//    display.display();
//    displayDimmed = false;
//  }

  if(displayOn && b1.getLastActivityTime() + SCREEN_OFF < millis() && b2.getLastActivityTime() + SCREEN_OFF < millis() && b3.getLastActivityTime() + SCREEN_OFF < millis() && b4.getLastActivityTime() + SCREEN_OFF < millis()){
    u8g.sleepOn();
    displayOn = false;
  }
  if(!displayOn && (b1State == 1 || b2State == 1 || b3State == 1 || b4State == 1)){
    u8g.sleepOff();
    displayOn = true;
  }
}

void modeManagement(){
  if(mode == MODE_OFF){
    for(byte i = 0; i < TOTALLED; i++){
     setLedBrightness(i, 0);
    }
  }
  if(mode == MODE_MANUAL){
    byte i;
    for(i = 0; i < manualNoOfLed; i++){
     setLedBrightness(i, manualBrightness*2.55);
    }
    for(i;i<TOTALLED;i++){
      setLedBrightness(i, 0);
    }
  }
  else if (mode == MODE_100){
    for(byte i = 0; i < TOTALLED; i++){
     setLedBrightness(i, 255);
    }
  }
  else if (mode == MODE_CLOUD){
    
  }
  else if (mode == MODE_ECO){
    for(byte i = 0; i < TOTALLED; i++){
     setLedBrightness(i, 128);
    }
  }
  else{ //Mode demo
    if(demoStep == 0){
      for(byte i = 0; i < TOTALLED; i++){
        setLedBrightness(i, 255);
      }
      if(demoStepStartTime + 1000 < millis()){
        demoStep++;
        demoStepStartTime = millis();
      }
    }
    else if(demoStep == 1){
      int brightnessTemp = -demoStepStartTime - 2500 + millis();
      int brightness = ((float)abs(brightnessTemp)/2500)*255;
      for(byte i = 0; i < TOTALLED; i++){
       setLedBrightness(i, brightness);
      }
      if(demoStepStartTime + 5000 < millis()){
        demoStep++;
        demoStepStartTime = millis();
        //Turn off every leds
        for(byte i = 0; i < TOTALLED; i++){
          setLedBrightness(i, 0);
        }
      }
    }
    else if(demoStep == 2){
      int correctOrder[] = {0,2,4,6,7,5,3,1};
      for(byte i = 0; i < TOTALLED; i++){
        if(millis() - demoStepStartTime > i*(5000/TOTALLED) && millis() - demoStepStartTime < (i+1)*(5000/TOTALLED)){
          Serial.println(i);
          setLedBrightness(correctOrder[i], 255);
        }
        else{
          setLedBrightness(correctOrder[i], 0);
        }
      }
      if(demoStepStartTime + 5000 < millis()){
        demoStep++;
        demoStepStartTime = millis();
      }
    }
    else if(demoStep == 3){
      int brightnessTemp = -demoStepStartTime + millis();
      int brightness = ((float)abs(brightnessTemp)/5000)*255;
      int correctOrder[] = {0,2,4,6,7,5,3,1};
      for(byte i = 0; i < TOTALLED; i++){
        setLedBrightness(correctOrder[i], (brightness + (i*255)/7)%255);
      }
      if(demoStepStartTime + 5000 < millis()){
        demoStep++;
        demoStepStartTime = millis();
      }
    }
    else{
      mode = MODE_MANUAL;
    }
  }
}

String getModeStr(){
  if(mode == MODE_OFF){
    return "OFF";
  }
  if(mode == MODE_MANUAL){
    return "M"+(String)manualNoOfLed+"-"+(String)manualBrightness;
  }
  else if(mode == MODE_100){
    return "100%";
  }
  else if(mode == MODE_CLOUD){
    return "CLOUD";
  }
  else if(mode == MODE_ECO){
    return "ECO";
  }
  else if(mode == MODE_DEMO){
    return "DEMO";
  }
  else{
    return "???";
  }
}

String getTimeStr(){
  String spaceH = "";
  String spaceM = "";
  if(hour() < 10){
    spaceH = " ";
  }
  if(minute() < 10){
    spaceM = "0";
  }
  return spaceH + hour() + ":" + spaceM + minute();
}

void setLedBrightness(byte ledNo, byte brightness){
  if(leds[ledNo] < 100){
    analogWrite(leds[ledNo], brightness);
  }
  else {
    Serial.print(leds[ledNo]-100);
    Serial.print(',');
    Serial.println(brightness);
  }
}

void autoControl(){
  if(hour() == 20 && !autoOnDone){
    mode = MODE_MANUAL;
    manualNoOfLed = 8;
    manualBrightness = 80;
    autoOnDone = true;
  }

  if(hour() == 23 && !autoOffDone){
    mode = MODE_OFF;
    autoOffDone = true;
  }
  
  //Turn off everything at 23h59 (safetey reason)
  if(hour() == 0 && minute() == 0){
    autoOnDone = false;
    autoOffDone = false;
    mode = MODE_OFF;
  }
}

void relay(){
  if(mode == MODE_OFF || (mode == MODE_MANUAL && (manualNoOfLed == 0 || manualBrightness == 0))){
    digitalWrite(RELAY_PIN, HIGH); //turn off relay
  }
  else{
    digitalWrite(RELAY_PIN, LOW); //turn on relay
  }
}

