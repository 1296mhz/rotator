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
#define BTN_UP   1
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_SELECT 5
#define BTN_NONE 10
#define VERSION "v19.7.20 - 20:51"
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int target = 0;
int calibration = 0;
int correctDelta = 0;
bool buttonState;         // variable for reading the pushbutton status
bool buttonEncoder = false;
bool buttonEncoderLong = false;
bool buttonCalEncoder = false;
bool buttonCalEncoderLong = false;

int flag = 0;                  // флаг состояния
int regim = 0;    

uint32_t ms_button = 0;
bool hold;
// 1 - Main, 2 - Calibration
int subMenu;
float sensorValue = 0;  // variable to store the value coming from the sensor
int preset = 180 ; // preset encoder;
//iz fla encoder
int currentTime, loopTime, currentTimeCal, loopTimeCal ;

int encoder_A, encoder_A_prev;
String s_angle;
String s_target;
String s_pr;
String s_calibration;
int angle;

void clearLine(int line){
  lcd.setCursor(0, line);
  lcd.print("                ");
}

void printDisplay(String message){
  Serial.println(message);
  lcd.setCursor(0, 1);
  lcd.print(message);
  delay(1000);
  clearLine(1);
}
void cw() {
  digitalWrite(PIN_RIGHT, LOW);
  lcd.setCursor(9, 0);
  lcd.print("-->"); 
  digitalWrite(PIN_LEFT, HIGH);
}

void ccw() {
  digitalWrite(PIN_LEFT, LOW);
  lcd.setCursor(9, 0);
  lcd.print("<--"); 
  digitalWrite(PIN_RIGHT, HIGH);
}

int detectButton() {
  int keyAnalog = analogRead(A0);
  if (keyAnalog < 100) {
    // Значение меньше 100 – нажата кнопка right
    return BTN_RIGHT;
  } else if (keyAnalog < 200) {
    // Значение больше 100 (иначе мы бы вошли в предыдущий блок результата сравнения, но меньше 200 – нажата кнопка UP
    return BTN_UP;
  } else if (keyAnalog < 400) {
    // Значение больше 200, но меньше 400 – нажата кнопка DOWN
    return BTN_DOWN;
  } else if (keyAnalog < 600) {
    // Значение больше 400, но меньше 600 – нажата кнопка LEFT
    return BTN_LEFT;
  } else if (keyAnalog < 800) {
    // Значение больше 600, но меньше 800 – нажата кнопка SELECT
    return BTN_SELECT;
  } else {
    // Все остальные значения (до 1023) будут означать, что нажатий не было
    return BTN_NONE;
  }
}

void setup() {
  // Serial.begin(9600);
  // Serial.println("R8CDF!");
  // Serial.println(VERSION);
  subMenu = 0;
  lcd.begin(NUMCOLS, NUMROWS);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R8CDF Rotator");
  lcd.setCursor(0, 1);
  lcd.print(VERSION);
  delay(2000);
  clearLine(0);
  clearLine(1);
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
  lcd.setCursor(15, 0);
  // read the value from the sensor:
  sensorValue = analogRead(SENSOR_PIN);
  //angle=sensorValue/2.8;
  //angle=sensorValue/1024.0 * 360;
  angle = int(round(sensorValue /2.8));
  
  int button = detectButton();
  switch (button) {
    case BTN_UP:
      printDisplay("UP");
      break;
    case BTN_DOWN:
      printDisplay("DOWN");
      break;
    case BTN_LEFT:
    if(target >= 1) {
      ccw();
      target = angle;
    }
      break;
    case BTN_RIGHT:
    if(target <= 359) {
      cw();
      target = angle;
    }
      break;
    case BTN_SELECT:
      printDisplay("SELECT");
      break;
    default:
      //printDisplay("Press any key");
      break;
  }

    uint32_t ms = millis();

    buttonState = digitalRead(BUTTON_PIN);
    // Фиксируем нажатие кнопки   
    if (buttonState == LOW &&  flag == 0 && ( ms - ms_button ) > 2000) {

       ms_button = ms;
       lcd.setCursor(15, 0);
       regim ++;
       flag = 1;
      
       if(regim > 7)                     // Если номер режима превышает требуемого
       {                               // то отсчет начинается с нуля
          regim = 0;
        }
    }

    if (buttonState == HIGH && flag == 1 && ( ms - ms_button ) > 50) {

       ms_button = ms;
       flag = 0;
    }


  if(regim == 0)
    {
    lcd.setCursor(0, 0);
    lcd.print("Main");
    }
    
// РЕЖИМ 1: R
  if(regim == 1)
    {
    lcd.setCursor(0, 0);
    lcd.print("Calibration");
    }
  // Calibration
  if(subMenu == 2) {
    lcd.print("C");
    lcd.setCursor(0, 0);
    lcd.print("Calibration");
   // lcd.setCursor(0, 1);
   // lcd.print("                ");
    uint32_t ms = millis();

    buttonState = digitalRead(BUTTON_PIN);
    // Фиксируем нажатие кнопки   
    if (buttonState == LOW && !buttonCalEncoderLong && ( ms - ms_button ) > 2000) {
       buttonCalEncoder = true;
       buttonCalEncoderLong = false;
       ms_button = ms;
       lcd.setCursor(15, 0);
       lcd.print("*");
       delay(2000);
       clearLine(0);
       clearLine(1);
       // subMenu = 1;
    }

    // Фиксируем отпускание кнопки   
    // if (buttonState == HIGH && buttonCalEncoder && ( ms - ms_button ) > 50) {
    //   // turn LED off:
    //   buttonCalEncoder = false;
    //   ms_button = ms;  
    // }

  currentTimeCal = millis();
  if (currentTimeCal >= (loopTimeCal + 5)) {
    encoder_A = digitalRead(PIN_A);
    if ((!encoder_A) && (encoder_A_prev)) {
      if (digitalRead(PIN_B)) {
        if (calibration + STEP <= 360) calibration += STEP;
      }
      else {
        if (calibration - STEP >= 0) calibration -= STEP;
      }
    }
    encoder_A_prev = encoder_A;
  }
  
  loopTimeCal = currentTimeCal;
  if (calibration >= 100) {
    s_calibration = String(calibration);
  }
  if (calibration < 100) {
    s_calibration = " " + String(calibration);
  }
  if (calibration < 10) {
    s_calibration = "  " + String(calibration);
  }

    lcd.setCursor(0, 1);
    lcd.print("CAL AZ ");
    lcd.setCursor(8, 1);
    lcd.print(s_calibration);
}


  // ***********************
  // Основной цикл
  if(subMenu == 0) {
  lcd.print("M");
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
    clearLine(0);
    clearLine(1);
    delay(500);
    subMenu = 2;
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
    lcd.setCursor(15, 0);
    lcd.print(" ");
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
    cw();
  }

  if (angle - target > (hold ? HYSTERESIS_HOLD : HYSTERESIS))
  {
    ccw();
  }

  if ( abs(target - angle) < (hold ? HYSTERESIS_HOLD : HYSTERESIS))
  {
    hold = true;
    digitalWrite(PIN_RIGHT, HIGH);
    digitalWrite(PIN_LEFT, HIGH);
    lcd.setCursor(9, 0);
    lcd.print("   ");
  }
  }
 
}
