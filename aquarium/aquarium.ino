#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
bool displayDimmed = false;
bool displayOn = true;

#define SCREEN_DIM 5000
#define SCREEN_OFF 10000

OneButton b1(A0, true);
OneButton b2(A1, true);
OneButton b3(A2, true);
OneButton b4(A3, true);

int b1State = 0;
int b2State = 0;
int b3State = 0;
int b4State = 0;

int displayState = 0;
int selectNo = 0;

char c[] = "   "; 

void setup()   {                
  Serial.begin(9600);

  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  
  display.begin(SSD1306_SWITCHCAPVCC, 60);
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

  handleButtons();

  screenPowerManagement();
}

void handleButtons(){
  bool change = false;
  if(displayState == 0){
    if (b2State == b2.CLICK_STATE){
      selectNo = (selectNo + 1)%3;
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo--;
      if(selectNo == -1){
        selectNo = 2;
      }
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = (selectNo+1);
      change = true;
    }
  }
  else if(displayState == 1){
    if (b1State == b1.CLICK_STATE){
      displayState = 0;
      change = true;
    }
  }
  else if(displayState == 2){
    if (b1State == b1.CLICK_STATE){
      displayState = 0;
      change = true;
    }
  }
  else if(displayState == 3){
    if (b1State == b1.CLICK_STATE){
      displayState = 0;
      change = true;
    }
  }

  
  if(change){
    displayAll();
  }
}

void displayButtonMenu(String str){
  display.setTextSize(1);
  display.setCursor(1,56);
  display.setTextColor(BLACK);
  
  display.fillRect(0, 55, 128, 9, WHITE);
  
  display.println(str);
}

void displayHeader(){
  display.setTextSize(1);
  display.setCursor(1,1);
  display.setTextColor(BLACK);
  
  display.fillRect(0, 0, 128, 9, WHITE);
  display.println("P1   13:15   26.2C");
}

void displayContent(){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1,10);
  if(displayState == 0){
    setChevrons();
    display.println((String)c[0]+" Manual");
    display.setCursor(1,20);
    display.println((String)c[1]+" Auto");
    display.setCursor(1,31);
    display.println((String)c[2]+" Settings");
    displayButtonMenu("      PREV  NEXT     ");
  }
  else if(displayState == 1){
    display.println("Manual");
  }
  else if(displayState == 2){
    display.println("Auto");
  }
  else if(displayState == 3){
    display.println("Setttings");
  }
}

void displayAll(){
  display.clearDisplay();
  displayHeader();
  displayContent();
  display.display();
}

void setChevrons(){
  c[selectNo] = '>';
  Serial.println(sizeof(c));
  for(int i = 0; i < sizeof(c) - 1; i++){
    if (i != selectNo){
      c[i] = ' ';
    }
  }
}

void screenPowerManagement(){
  if(!displayDimmed && b1.getLastActivityTime() + SCREEN_DIM < millis() && b2.getLastActivityTime() + SCREEN_DIM < millis() && b3.getLastActivityTime() + SCREEN_DIM < millis() && b4.getLastActivityTime() + SCREEN_DIM < millis()){
    display.dim(true);
    display.display();
    displayDimmed = true;
  }
  if(displayDimmed && (b1State != 0 || b2State != 0 || b3State != 0 || b4State != 0)){
    display.dim(false);
    display.display();
    displayDimmed = false;
  }

  if(displayOn && b1.getLastActivityTime() + SCREEN_OFF < millis() && b2.getLastActivityTime() + SCREEN_OFF < millis() && b3.getLastActivityTime() + SCREEN_OFF < millis() && b4.getLastActivityTime() + SCREEN_OFF < millis()){
    //digitalWrite(12,LOW);
    display.off();
    displayOn = false;
  }
  if(!displayOn && (b1State != 0 || b2State != 0 || b3State != 0 || b4State != 0)){
    display.begin(SSD1306_SWITCHCAPVCC, 60);
    displayOn = true;
  }
}

