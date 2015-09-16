#include <MENWIZ.h>
#include <buttons.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
/*
This arduino system opens a valve to overdose with bromine a water circuit during few hours
This system has two inputs:
  -Signal proceeding from a 4-20mah loop from a bromine reader
  -Digital signal from a programable clock to control the overdosing hours
The output pin switches a relay connected to the dosificating valve
During the overdose hours the valve opens intermitently until desired level of bromine is archieved
*/


// DEFINE ARDUINO PINS FOR THE NAVIGATION BUTTONS
#define UP_BOTTON_PIN       4
#define DOWN_BOTTON_PIN     5
//#define LEFT_BOTTON_PIN     7 
//#define RIGHT_BOTTON_PIN    8
#define CONFIRM_BOTTON_PIN  6
#define ESCAPE_BOTTON_PIN   7

menwiz tree;
//init lcd library
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7,3,POSITIVE);  // Set the LCD I2C address

bool isNight = 0;
bool dosing = 0;
int minValveON = 10;
int minValveOFF = 15;
int  list;
float setpoint=2.0;

void setup(){
  
  //Input signal from programmable clock
  pinMode(3, INPUT);      // sets the digital pin 7 as input
  
  //setup menu tree
  _menu *r,*s1,*s2;
  tree.begin(&lcd,20,4); //declare lcd object and screen size to menwiz lib
  
  //mount menu hierchy
  r=tree.addMenu(MW_ROOT,NULL,F("Opcions"));
    s1=tree.addMenu(MW_VAR,r, F("Establir setpoint"));
      s1->addVar(MW_AUTO_FLOAT,&setpoint,0,5.0,0.1);  
    s1=tree.addMenu(MW_VAR,r, F("Forcar dosificacio"));
      s1->addVar(MW_LIST,&list);
      s1->addItem(MW_LIST, F("Activada"));
      s1->addItem(MW_LIST, F("Desactivada"));
      s1->addItem(MW_LIST, F("Automatica"));
    s1=tree.addMenu(MW_SUBMENU,r, F("Temps dosificacio"));
      s2=tree.addMenu(MW_VAR,s1, F("Temps valvula ON"));
        s2->addVar(MW_AUTO_INT,&minValveON,0,60,1);   
      s2=tree.addMenu(MW_VAR,s1, F("Temps valvula OFF"));
        s2->addVar(MW_AUTO_INT,&minValveOFF,0,60,1);  
  tree.navButtons(UP_BOTTON_PIN,DOWN_BOTTON_PIN,ESCAPE_BOTTON_PIN,CONFIRM_BOTTON_PIN);
 
  tree.addUsrScreen(*showSummary, 20000);
  
  tree.readEeprom();
}

void loop(){
  isNight=digitalRead(3);
  dosing=isDosing();
  tree.draw(); 
  
  
  tree.writeEeprom();  
}

void showSummary (){
  char setp[4];
  //String setpoint = sp;
  dtostrf(setpoint,1,2,setp);
  String linea1="Brom: 1.75 ppm | ";
  String linea2;
  if (isNight==1){
    linea2="\n\nFranja horaria    SI";
  }else{
    linea2="\n\nFranja horaria    NO";
  }
  String linea3="\nDosificant        NO ";
  String result =linea1+setp+linea2+linea3;
  char buf[80];
  result.toCharArray(buf,80);

  tree.drawUsrScreen( buf);

}
bool isDosing (){
  if ( isNight ){
    return true;
  }
  else{
    return false;
  }
}

