#include <LiquidCrystal.h>

/*
  Analog Input
*/
#define HYSTERESIS 2
#define HYSTERESIS_HOLD 5
#define STEP 5
#define NUMROWS 2
#define NUMCOLS 16
#define SENSOR_PIN A0 // select the input pin for the antenna potentiometer
#define BUTTON_PIN 7   // the number of the pushbutton encoder pin
#define LED_PIN 13     // select the pin for the LED
#define PIN_A 9    
#define PIN_B 8  
#define PIN_LEFT 6
#define PIN_RIGHT 10

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int set=0;
int buttonState = 0;         // variable for reading the pushbutton status
bool hold;
int sensorValue = 0;  // variable to store the value coming from the sensor
int preset=180 ;  // preset encoder;
//iz fla encoder
int currentTime,loopTime;

int encoder_A, encoder_A_prev;
String s_angle;
String s_set;
String s_pr;
//float volt=0;
int angle;

void setup() {
lcd.begin(NUMCOLS, NUMROWS);

  pinMode(6, OUTPUT);
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIN_LEFT, OUTPUT);
  pinMode(PIN_RIGHT, OUTPUT);
  //pinMode(Motor, OUTPUT);
  currentTime = millis();
  loopTime = currentTime;
  sensorValue = analogRead(SENSOR_PIN);
  //volt=float(sensorValue)/202;
  angle=sensorValue/2.8;
  //brightness=angle;
  set=angle;

  //digitalWrite(PIN_RIGHT, LOW);
  //digitalWrite(PIN_LEFT, LOW);
  encoder_A_prev = digitalRead(PIN_A);
}


void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(SENSOR_PIN);
  //volt=float(sensorValue)/202;
  angle=sensorValue/2.8;

// encoder
currentTime = millis();
  if(currentTime >= (loopTime + 5)){ 
    encoder_A = digitalRead(PIN_A);     
    if((!encoder_A) && (encoder_A_prev)){    
      if(digitalRead(PIN_B)) {
        if(preset + STEP <= 360) preset += STEP;
      }
      else {
        if(preset - STEP >= 0) preset -= STEP;
      }
    }
    encoder_A_prev = encoder_A;  

  }
  loopTime = currentTime;
if (preset>=100){ s_pr=String(preset);}
  if (preset<100) {s_pr=" "+String(preset);}
  if (preset<10) {s_pr="  "+String(preset);}
  if (angle>=100){ s_angle=String(angle);}
  if (angle<100) {s_angle=" "+String(angle);}
  if (angle<10) {s_angle="  "+String(angle);}

  
// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
    buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    // turn LED off:
    digitalWrite(LED_PIN, LOW);
     }
    else {
    // turn LED on:
     digitalWrite(LED_PIN, HIGH);
     
  set=preset;
  hold = false;
 if (set>=100){ s_set=String(set);}
  if (set<100) {s_set=" "+String(set);}
  if (set<10) {s_set="  "+String(set);}

    }
 
//set the cursor to column 0, line 1
lcd.setCursor(0,0);
lcd.print("Az ");
lcd.setCursor(4,0);
lcd.print(s_angle);
lcd.setCursor(0,1);
lcd.print("Set ");
lcd.setCursor(4,1);
lcd.print(s_set);
lcd.setCursor(8,1);
lcd.print("PreS ");
lcd.setCursor(13,1);
lcd.print(s_pr);

 if  (set - angle > (hold?HYSTERESIS_HOLD:HYSTERESIS))
      {
  digitalWrite(PIN_RIGHT,LOW);
  lcd.setCursor(9,0);
  lcd.print("-->");
  digitalWrite(PIN_LEFT, HIGH);
}

   if (angle - set > (hold?HYSTERESIS_HOLD:HYSTERESIS))
  {
  digitalWrite(PIN_LEFT, LOW);
  lcd.setCursor(9,0);
  lcd.print("<--");
  digitalWrite(PIN_RIGHT,HIGH);
  }

//  if (set == angle){
//digitalWrite(PIN_RIGHT, HIGH);
// }


  if ( abs(set - angle) <= (hold?HYSTERESIS_HOLD:HYSTERESIS)) {
  hold = true;
  digitalWrite(PIN_RIGHT, HIGH);
  digitalWrite(PIN_LEFT, HIGH);
  //digitalWrite(LED_PIN, HIGH);
  lcd.setCursor(9,0);
  lcd.print("   ");
  }
}