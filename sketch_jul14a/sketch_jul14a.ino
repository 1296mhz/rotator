#include <LiquidCrystal.h>

/*
   R8CDF Edition
  Analog Input
*/
#define HYSTERESIS 1
#define HYSTERESIS_HOLD 5
#define STEP 1
#define NUMROWS 2
#define NUMCOLS 16
#define SENSOR_PIN A1 // select the input pin for the antenna potentiometer
#define BUTTON_PIN 1   // the number of the pushbutton encoder pin
//#define LED_PIN 13     // select the pin for the LED
#define PIN_A 2
#define PIN_B 3
#define PIN_LEFT 11
#define PIN_RIGHT 13
#define VERSION "v19.7.20 - 01:13"
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int target = 0;
int calibration = 0;
int correctDelta = 0;
bool buttonState;         // variable for reading the pushbutton status
bool buttonEncoder = false;
bool buttonEncoderLong = false;
uint32_t ms_button = 0;
bool hold;
// 1 - Main, 2 - Calibration
int subMenu;
float sensorValue = 0;  // variable to store the value coming from the sensor
int preset = 180 ; // preset encoder;
//iz fla encoder
int currentTime, loopTime;

int encoder_A, encoder_A_prev;
String s_angle;
String s_target;
String s_pr;
int angle;

void setup() {
  // Serial.begin(9600);
  // Serial.println("R8CDF!");
  // Serial.println(VERSION);
  subMenu = 1;
  lcd.begin(NUMCOLS, NUMROWS);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R8CDF Rotator");
  lcd.setCursor(0, 1);
  lcd.print(VERSION);
  delay(2000);
  lcd.clear();
  // pinMode(6, OUTPUT);
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
  //angle=sensorValue/2.8;
  angle = int(round(sensorValue /2.8));
  //brightness=angle;
  target = angle;
  //digitalWrite(PIN_RIGHT, LOW);
  //digitalWrite(PIN_LEFT, LOW);
  encoder_A_prev = digitalRead(PIN_A);
}


void loop() {
  
  // read the value from the sensor:
  sensorValue = analogRead(SENSOR_PIN);
  //angle=sensorValue/2.8;
  //angle=sensorValue/1024.0 * 360;
  angle = int(round(sensorValue /2.8));
  // encoder
  if(subMenu == 2) {
 
  lcd.setCursor(0, 0);
  lcd.print("Calibration");
  }
  if(subMenu == 1) {
     currentTime = millis();
  if (currentTime >= (loopTime + 5)) {
    encoder_A = digitalRead(PIN_A);
    if ((!encoder_A) && (encoder_A_prev)) {
      if (digitalRead(PIN_B)) {
        if (preset + STEP <= 360) preset += STEP;
      }
      else {
        if (preset - STEP >= 0) preset -= STEP;
      }
    }
    encoder_A_prev = encoder_A;
  }
  
  loopTime = currentTime;
  if (preset >= 100) {
    s_pr = String(preset);
  }
  if (preset < 100) {
    s_pr = " " + String(preset);
  }
  if (preset < 10) {
    s_pr = "  " + String(preset);
  }
  if (angle >= 100) {
    s_angle = String(angle);
  }
  if (angle < 100) {
   s_angle = " " + String(angle);
  }
  if (angle < 10) {
    s_angle = "  " + String(angle);
  }


  uint32_t ms = millis();
 
  buttonState = digitalRead(BUTTON_PIN);
 // Фиксируем нажатие кнопки   
  if (buttonState == LOW && !buttonEncoder && ( ms - ms_button ) > 50) {

    buttonEncoder = true;
    buttonEncoderLong = false;
    ms_button = ms;
    lcd.setCursor(15, 0);
    lcd.print("*");

  }
// Фиксируем длинное нажатие кнопки   
  if (buttonState == LOW && !buttonEncoderLong && ( ms - ms_button ) > 2000) {
    subMenu = 2;
    lcd.clear();
    buttonEncoderLong = true;
    ms_button = ms;
  }

  // Фиксируем отпускание кнопки   
  if (buttonState == HIGH && buttonEncoder && ( ms - ms_button ) > 50) {
    // turn LED off:
        target = preset;
    hold = false;
    buttonEncoder = false;
    ms_button = ms;
   // lcd.setCursor(15, 0);
   // lcd.print(" ");
       if (target >= 100){ s_target=String(target);}
    if (target < 100) {s_target=" "+String(target);}
    if (target < 10) {s_target="  "+String(target);}
  }

  //set the cursor to column 0, line 1
  lcd.setCursor(0, 0);
  lcd.print("AZ ");
  lcd.setCursor(4, 0);
  lcd.print(s_angle);
  // Serial.println("AZ ");
  // Serial.println(s_angle);
  lcd.setCursor(0, 1);
  lcd.print("TGT ");
  lcd.setCursor(4, 1);
  lcd.print(s_target);
  lcd.setCursor(8, 1);
  lcd.print("PRS ");
  lcd.setCursor(13, 1);
  lcd.print(s_pr);

  if (target - angle > (hold ? HYSTERESIS_HOLD : HYSTERESIS))
  {
    if(digitalRead(PIN_RIGHT) == HIGH) {
      digitalWrite(PIN_RIGHT, LOW);
    }
    lcd.setCursor(9, 0);
    lcd.print("-->"); 
    if(digitalRead(PIN_LEFT) == LOW){
      digitalWrite(PIN_LEFT, HIGH);
    }
  }

  if (angle - target > (hold ? HYSTERESIS_HOLD : HYSTERESIS))
  {
    if(digitalRead(PIN_LEFT) == HIGH ){
      digitalWrite(PIN_LEFT, LOW);
    }
    lcd.setCursor(9, 0);
    lcd.print("<--"); 
    if(digitalRead(PIN_RIGHT) == LOW){
       digitalWrite(PIN_RIGHT, HIGH);
    } 
  }

  if ( abs(target - angle) < (hold ? HYSTERESIS_HOLD : HYSTERESIS))
  {
    hold = true;
    digitalWrite(PIN_RIGHT, HIGH);
    digitalWrite(PIN_LEFT, HIGH);
    //digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(9, 0);
    lcd.print("   ");
  }
  }
 
}
