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
bool DEBUG =false;





menwiz tree;
//init lcd library
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7,3,POSITIVE);  // Set the LCD I2C address

bool isNight = 0;
bool dosing = true;
int minValveON = 1;
int minValveOFF = 2;
unsigned long multiplier = 600;
unsigned long onTime = 2UL*60000UL;//(multiplier*(unsigned long)minValveON);
unsigned long offTime = 6UL*60000UL;//(multiplier*(unsigned long)minValveOFF);

int  list;
float bromineF;
float setpoint=2.0;
// Tracks the last time event fired
unsigned long previousMillis=0;
// Interval is how long we wait
unsigned long int interval = onTime;
void setup(){
  
  //Input signal from programmable clock
  pinMode(3, INPUT);      // sets the digital pin 7 as input
  pinMode(2, OUTPUT);      // sets the digital pin 7 as input
  //setup menu tree
  _menu *r,*s1,*s2;
  tree.begin(&lcd,20,4); //declare lcd object and screen size to menwiz lib
  
  //mount menu hierarchy
  r=tree.addMenu(MW_ROOT,NULL,F("Opcions"));
    s1=tree.addMenu(MW_VAR,r, F("Establir setpoint"));
      s1->addVar(MW_AUTO_FLOAT,&setpoint,0,10.0,0.1);  
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
 
  tree.addUsrScreen(*showSummary, 10000);
  
  tree.readEeprom();
}

void loop(){
  isNight=digitalRead(3);
  isDosing();
  if (dosing){
    digitalWrite(2,HIGH);
  } else{
    digitalWrite(2,LOW);
  }
  tree.draw(); 
  
  
  tree.writeEeprom();  
}

void showSummary (){
  char setp[5];
  //String setpoint = sp;
  dtostrf(setpoint,4,1,setp);
  int val = analogRead(0);
  int aux1 = val * 10;
  bromineF= aux1 / 1000.0;
  char bromine[5];
  dtostrf(bromineF,4,2,bromine);
  String linea1="Brom: ";
  String linea2;
  //if (DEBUG){
    //linea2="\n"+val+" "+bromine+" "+list;
  //}else{
    linea2="\n";
  //}
  String linea3;
  if (isNight==1){
    linea3="\nFranja horaria    SI";
  }else{
    linea3="\nFranja horaria    NO";
  }
  String linea4;
  if (dosing==1){
    linea4="\nDosificant        SI ";
  }else{
    linea4="\nDosificant        NO ";
  }
  
  String result =linea1+bromine+"mg/l|"+setp+linea2+linea3+linea4;
  char buf[80];
  result.toCharArray(buf,80);
  delay(1000);
  tree.drawUsrScreen( buf);

}
void isDosing (){
  if (list == 0){
    dosing=true;
  }
  if (list == 1){
    dosing=false;
  }
  if (list == 2){
    if ( (isNight) && (setpoint > bromineF)){
      // Grab snapshot of current time, this keeps all timing
      // consistent, regardless of how much code is inside the next if-statement
      unsigned long currentMillis = millis();
   
      // Compare to previous capture to see if enough time has passed
      if ((unsigned long)(currentMillis - previousMillis) >= interval) {
        // Change wait interval, based on current LED state
        if (dosing) {
          // LED is currently on, set time to stay off
          //offTime=(multiplier*(unsigned long)minValveOFF);
          interval = offTime;
        } else {
          // LED is currently off, set time to stay on
          //onTime=(multiplier*(unsigned long)minValveON);
          interval = onTime;
        }
        // Toggle the LED's state, Fancy, eh!?
        dosing = !(dosing);
   
        // Save the current time to compare "later"
        previousMillis = currentMillis;
      }
    }
    else{
      dosing=false;
    }
    
  }
  

  
}

