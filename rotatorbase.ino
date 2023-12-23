// Rotorbase.ino
// (C) DF5RF 2023
// A rotor controller with ILI9341 TFT with analog touch input, Poti and keys to drive Left/right motor
// Serial DCU1 emulation
#include <TouchScreen.h>
#include <math.h>
#include <SPI.h> 

#include <Adafruit_GFX.h>    // Adafruit Grafik-Bibliothek wird benötigt
#include "Adafruit_ILI9341.h"

// Pins for Rotor
// A4/ADC4 => Rotor Poti
// A5/ADC5 => Stell-Poti
// PD0 Mot Left
// PD1 Mot Rigth
// PD2 Key1
// PD3 Key2

#define TFT_PIN_CS   5 // Arduino-Pin an Display CS   
#define TFT_PIN_DC   6  // Arduino-Pin an 
#define TFT_PIN_RST  8  // Arduino Reset-Pin
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_PIN_CS, TFT_PIN_DC); //, TFT_PIN_RST); //Adafruit_ILI9431(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);

#define TS_XP A3              
#define TS_YP A2                
#define TS_XM A1              
#define TS_YM A0             
#define TS_MINX 140
#define TS_MAXX 900
#define TS_MINY 120
#define TS_MAXY 940
TouchScreen ts(TS_XP, TS_YP, TS_XM, TS_YM, 300);

enum {UIModeOps, UIModeSettings, UIModeChangeToOps,UIModeChangeToSettings} UIMode;
 
bool sdebug=false;
int xcenter=190;
int ycenter=119;
int rradius=96;
int setdeg=0;
int currdeg=0;
bool destreached=false;
bool destlocked=false;
bool refreshInput;
int lastcurrdeg;  
int lastsetdeg;

float pi = 3.1415926;
bool demo=true;  
bool connMsgReceived;
bool isConnected;

#define CUSTOM_MARKER   255, 12,65
#define CUSTOM_LABEL    120, 120,65

int yheight=16;

void setup() {
    connect_setup();
    tft.begin();
    tft.setRotation(tft.getRotation()+1);
    showMainScreen();
    setdeg=322;
    currdeg=0;
    lastsetdeg=-1;
    lastcurrdeg=-1;
    UIMode=UIModeChangeToOps;
}


void loop(void) {
  connect_loop();
  switch (UIMode) {
    case UIModeChangeToOps:
      tft.fillScreen(ILI9341_BLACK);
      showMainScreen();  
      UIMode=UIModeOps;
    break;
    case UIModeChangeToSettings:
      tft.fillScreen(ILI9341_BLACK);  
      UIMode=UIModeSettings;
    break;
    case UIModeOps:
      if ((refreshInput=processInput())) {
        demo=false;
      }
      else {
        demo=true;
      }
      updateValues(refreshInput);
      delay(200);
      if (demo) {
        if (!destreached) {
          currdeg+=2;
          currdeg%=360;
        }
      }
      break;
    case UIModeSettings:  
       processInput();
       showInfoScreen();
       delay(500);
  }
}

bool processInput() {
bool isNewInput=false;
  
  // Poti lesen
  int poti=analogRead(A4);
  // tasten lesen
  int keyleft=digitalRead(PD2);
  int keyright=digitalRead(PD3);
  // request from PC
  if (connMsgReceived) {
    setdeg=connect_value();
    isNewInput=true;
  }
  // touch screen lesen
  int x,y;
  TSPoint p = ts.getPoint();

  
  if (p.z > ts.pressureThreshhold) 
  {
     //Serial.print("Touch.X:");Serial.print(p.x);Serial.print("Touch.Y:");Serial.println(p.y);
     y = map(p.x, TS_MINX, TS_MAXX, 0, 240);
     x = map(p.y, TS_MINY, TS_MAXY, 320, 0);
     //Serial.print("X:");Serial.print(x);Serial.print("Y:");Serial.println(y);
     if (x>50) {
        // calculate angle relative to the center
        setdeg= calculateAngle(xcenter, ycenter, x,y);
        if (setdeg%2==1)
          setdeg=setdeg+1;
        if (sdebug) {Serial.print("Angle:");Serial.println(setdeg);}
        isNewInput=true;
     }
     else if (x<50 && y <110) {
        if (UIMode==UIModeSettings) {
          UIMode=UIModeChangeToOps;
        }
        else {
          UIMode=UIModeChangeToSettings;
        }
        isNewInput=true;
     }
  }
  return isNewInput;
}

void updateValues(bool refreshInput) {
  // A/D lesen
  int rotpot=analogRead(A5);
  if (!demo) {
    //currdeg=rotpot*4/10;
  }
  if (sdebug) {Serial.print("Set.X:");Serial.print(setdeg);Serial.print("Curr:");Serial.println(currdeg);}
  // anzeigen
  showCurrArrow(false);
  destreached=(abs(setdeg-currdeg)==0);
  if (destreached) {
    if (!destlocked) {
        showSetArrow(false);
        showCurrArrow(true);
        writeValue(7, currdeg);
        destlocked=true;
    }
    if (sdebug) {Serial.println("Reached:");}
  }
  else { // !destreached
    writeValue(7, currdeg);
  }
  if (refreshInput) {
    writeValue(10, setdeg);
    showSetArrow(true);
    destlocked=false;
  }
  int statval=0;
  statval+=(!destreached?1:0);
  statval+=(destlocked?10:0);
  statval+=(connMsgReceived?100:0);
  writeValue(14, statval);
}



void showCurrArrow(bool forceRefresh) {
    if (sdebug) {Serial.print("Curr:");Serial.print(currdeg);Serial.print("Last:");Serial.println(lastcurrdeg);}
    if (lastcurrdeg!=currdeg || forceRefresh) { 
      drawArrow(lastcurrdeg, rradius-11, 5, ILI9341_BLACK);
      drawArrow(currdeg, rradius-11, 5, ILI9341_GREEN);
      lastcurrdeg=currdeg;
    }
}
void showSetArrow(bool appear) {
      drawSetMarker(lastsetdeg, 6, 5, ILI9341_BLACK);
      if (appear) {
        drawSetMarker(setdeg, 6, 5, ILI9341_RED);
      }
      lastsetdeg=setdeg;
}

void showMainScreen() {
  tft.fillScreen(ILI9341_BLACK);  // Färbt Hintergund Schwarz
  tft.drawCircle(xcenter, ycenter, (rradius+2), ILI9341_YELLOW);
  tft.drawCircle(xcenter, ycenter, (rradius+1), ILI9341_YELLOW);
  tft.drawCircle(xcenter, ycenter, 10, ILI9341_YELLOW);
  writeText(0,"Antenna");
  writeText(1,"Rotor");
  writeText(2,"DF5RF");
  writeText(6,"Ist:");
  writeText(9,"Soll:");
  writeText(13,"Status:");
  drawDegreeMarkings();
  
  // Draw N/E/S/W label
  drawLabel(0,"0",tft.color565(CUSTOM_MARKER));
  drawLabel(90,"90",tft.color565(CUSTOM_MARKER));
  drawLabel(180,"180",tft.color565(CUSTOM_MARKER));
  drawLabel(270,"270",tft.color565(CUSTOM_MARKER));
  // DXCC labels
  drawLabel(35,"JA",tft.color565(CUSTOM_LABEL));
  drawLabel(60,"VK",tft.color565(CUSTOM_LABEL));
  drawLabel(150,"FR",tft.color565(CUSTOM_LABEL));
  drawLabel(220,"PY",tft.color565(CUSTOM_LABEL));
  drawLabel(310,"K",tft.color565(CUSTOM_LABEL));
  drawLabel(340,"KL7",tft.color565(CUSTOM_LABEL));
}

void showInfoScreen() {
  writeText(0,"Antenna Rotor (C) DF5RF");
  writeText(1,"Rotor-Prot: DCU1");
  writeText(2,"COM-Param:  38400,8N1");
  writeText(3,(isConnected?"*OK*":" ?  "));
  writeText(4,"Rotor-Pot pin:  A4 ");
  writeText(5,"Set-Pot pin:    A5 ");
  writeText(6,"Motor L/R pins: PD0,1");
}
