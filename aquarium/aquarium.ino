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

int mode = 0;
int brightness = 100;
int noOfLed = 9;



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
  if(displayOn && !displayDimmed){
    handleButtons();
  }
  
  screenPowerManagement();
  delay(25);
}

void handleButtons(){
  bool change = false;
  if(displayState == 0){
    if (b2State == b2.CLICK_STATE){
      selectNo--;
      if(selectNo == -1){
        selectNo = 2;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%3;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = (selectNo+1);
      if(displayState == 1){
        mode = 1;
      }
      if(displayState == 2 || displayState == 3){
        selectNo = 0;
      }
      change = true;
    }
  }

  
  else if(displayState == 1){
    if (b2State == b2.CLICK_STATE){
      if(brightness >= 5){
        brightness = brightness - 5;
      }
      change = true;
    }
    if (b2State == b2.DURING_LONG_PRESS_STATE){
      if(brightness >= 5){
        brightness = brightness - 5;
      }
      delay(25);
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      if(brightness <= 95){
        brightness = brightness + 5;
      }
      change = true;
    }
    if (b3State == b3.DURING_LONG_PRESS_STATE){
      if(brightness <= 95){
        brightness = brightness + 5;
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
      selectNo--;
      if(selectNo == -1){
        selectNo = 2;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%3;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = (selectNo+20);
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
      selectNo--;
      if(selectNo == -1){
        selectNo = 1;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      selectNo = (selectNo + 1)%2;
      change = true;
    }
    if (b4State == b4.CLICK_STATE){
      displayState = (selectNo+30);
      change = true;
    }
  }

  
  else if(displayState == 10){
    if (b2State == b2.CLICK_STATE){
      if(noOfLed > 0){
        noOfLed--;
      }
      change = true;
    }
    if (b3State == b3.CLICK_STATE){
      if(noOfLed < 9){
        noOfLed++;
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
  display.setCursor(1,17);
  
  if(displayState == 0){
    setChevrons(3);
    display.println((String)c[0]+" Manual");
    display.println((String)c[1]+" Auto");
    display.println((String)c[2]+" Settings");
  }
  
  else if(displayState == 1){
    display.setTextSize(2);
    display.setCursor(45,15);
    display.setTextColor(BLACK, WHITE);
    display.print(brightness);
    display.println("%");
    display.setCursor(55,40);
    display.setTextColor(WHITE);
    display.println(noOfLed);
  }
  
  if(displayState == 2){
    setChevrons(3);
    display.println((String)c[0]+" 100%");
    //display.setCursor(1,20);
    display.println((String)c[1]+" Cloud");
    //display.setCursor(1,31);
    display.println((String)c[2]+" Eco");
  }
  
  else if(displayState == 3){
    setChevrons(2);
    display.println((String)c[0]+" Set Temp");
    display.println((String)c[1]+" On/Off Time");
  }
  
  else if(displayState == 10){
    
    display.setTextSize(2);
    display.setCursor(45,15);
    display.print(brightness);
    display.println("%");
    display.setCursor(55,40);
    display.setTextColor(BLACK, WHITE);
    display.println(noOfLed);
  }
}

void displayAll(){
  display.clearDisplay();
  displayHeader();
  displayContent();
  display.display();
}

void setChevrons(int n){
  c[selectNo] = '>';
  for(int i = 0; i < n; i++){
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
  if(displayDimmed && (b1State == 1 || b2State == 1 || b3State == 1 || b4State == 1)){
    display.dim(false);
    display.display();
    displayDimmed = false;
  }

  if(displayOn && b1.getLastActivityTime() + SCREEN_OFF < millis() && b2.getLastActivityTime() + SCREEN_OFF < millis() && b3.getLastActivityTime() + SCREEN_OFF < millis() && b4.getLastActivityTime() + SCREEN_OFF < millis()){
    //digitalWrite(12,LOW);
    display.off();
    displayOn = false;
  }
  if(!displayOn && (b1State == 1 || b2State == 1 || b3State == 1 || b4State == 1)){
    display.begin(SSD1306_SWITCHCAPVCC, 60);
    displayOn = true;
  }
}

