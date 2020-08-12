// #include <LiquidCrystal.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RotaryEncoder.h"
#include <UIPEthernet.h>

#define HYSTERESIS 1
#define HYSTERESIS_HOLD 5
#define AZ_STEP 1

#define AZ_P3022_V1_CW360_SENSOR_PIN A0 // select the input pin for the antenna potentiometer

#define MAC {0x00,0x01,0x02,0x03,0x04,0x07}
#define IP {192,168,1,41}

#define ANALOG
#undef  ANALOG
#define NETWORK

#define ENC_BUTTON_PIN 4   // the number of the pushbutton encoder pin
//#define LED_PIN 13     // select the pin for the LED
#define PIN_CLK 2
#define PIN_DT 3

#define PIN_CCW 7 // Поворот против часовой стрелки
#define PIN_CW 6 // Поворот по часовой стрелки
#define PIN_SPEED 8 //Скорость поворота
#define VERSION "v12.8.20 - 20:29"
#define DMESG "R8CDF Rotator"
// задаем шаг энкодера и макс./мин. значение в главном меню
#define STEPS  6
#define POSMIN 0
#define POSMAX 12

// Меню калибровки
#define CAL_STEPS  6
#define CAL_POSMIN 0
#define CAL_POSMAX 12

// Меню включение выключения
#define TURN_STEPS  6
#define TURN_POSMIN 0
#define TURN_POSMAX 12

EthernetUDP udp;

int lastPos, newPos = 180;
int currentTime, loopTime;
int azEncoder, azEncoderPrev, azCalibrate, azCalibratePrev;
bool buttonEncoder = false;
bool buttonEncoderLong = false;
bool buttonState;
bool buttonWasUp = true;
bool clearFlag = false;
bool onOffFlag = false;
int azimuth_calibration_to[] = {};
int buttonPin = 2; // Кнопка 0 нажата 1 нет
int calibrate = 0;
// Для азимута
bool azHold;
int azAngleSensor = 0; // С сенcора угла азимута
int azAngle = 0; // Угол азимута
int azTarget = 0; // Цель для поворота
int azPreset = 180;
int prevAz;
int prevStopFlag;
String strAzAngle;
String strAzTarget;
String strAzPres;
String strAzCal;
// LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LiquidCrystal_I2C lcd(0x27,16,2);
RotaryEncoder encoder(PIN_CLK, PIN_DT); // пины подключение энкодера (DT, CLK)
byte w = 0;
bool correctFlag = false;
uint32_t last_millis; // переменные: последний  millis

int correct(bool correctFlag, int az, int cal) {
  if(correctFlag) {
     if(az < cal) {
       return 360 - ((360 - (cal - az)) - az);
     }
   
     if(az > cal) {
       return az - (az - cal);
     }
   
     if (az == cal) {
        return az;
     }
  }
  return az;
}

void clearDisplay() {
    lcd.clear();
};

void cw() {
  digitalWrite(PIN_CW, LOW);
  lcd.setCursor(9, 0);
  lcd.print("-->"); 
  digitalWrite(PIN_CCW, HIGH);
}

void ccw() {
  digitalWrite(PIN_CCW, LOW);
  lcd.setCursor(9, 0);
  lcd.print("<--"); 
  digitalWrite(PIN_CW, HIGH);
}

void speed(bool spd) {
  if(spd){
    lcd.setCursor(15, 0);
    lcd.print("F"); 
    digitalWrite(PIN_SPEED, HIGH);
  }

  if(!spd){
    lcd.setCursor(15, 0);
    lcd.print("S"); 
    digitalWrite(PIN_SPEED, LOW);
  }
}

void getSpeed() {
    if(digitalRead(PIN_SPEED)){
    lcd.setCursor(15, 0);
    lcd.print("F"); 

  }

  if(!digitalRead(PIN_SPEED)){
    lcd.setCursor(15, 0);
    lcd.print("S"); 
  }
}

uint8_t button(){
  if (digitalRead(ENC_BUTTON_PIN) == 1){ // кнопка не нажата     
     last_millis = millis();
     return 0;}
   delay(30);
   while (digitalRead(ENC_BUTTON_PIN) == 0);
   delay(30);
   if (last_millis+65 > millis()){ // ложное срабатывание
     //Serial.println(millis()-last_millis);
     last_millis = millis();
     return 0;}
   if (last_millis+300 > millis()){ // короткое нажатие меньше 0.30 сек
     //Serial.println(millis()-last_millis);
     last_millis = millis();
     return 1;}
   //Serial.println(millis()-last_millis);
   last_millis = millis(); // длинное нажатие больше 0.30 сек
   return 2;
};


void generateAzimuthMap(int azAngle, int calibrate) {
      for (int x = 0; x < 359 + 1; x++) {
      if (azAngle + calibrate + x <= 360) {
        azimuth_calibration_to[x] = azAngle + calibrate + x;
      }
      if(calibrate + azAngle + x > 360) {
        azimuth_calibration_to[x] = abs(360 - (calibrate + azAngle + x));
      }
    }
}

int azimuthSubstitutionMap(bool correctFlag, int azAngle, int azimuth_calibration_to){
  if(correctFlag) {
    if(sizeof(azimuth_calibration_to) > 0) {
      return azAngle;
    }
  }
  return azAngle;
}

void setup() {
  speed(true);
  #ifdef NETWORK
    uint8_t mac[6] = MAC;
    uint8_t ip[4] = IP;
    Ethernet.begin(mac,IPAddress(ip));
    int success = udp.begin(41234);
  #endif
  Serial.begin(9600);
  lcd.init();                     
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(DMESG);
  Serial.println(DMESG);

  delay(1000);

  lcd.clear();
  clearFlag = true;
  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);
  pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIN_CCW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(PIN_SPEED, OUTPUT);
  last_millis = millis();  
  // Читаем данные с сенсора и обновляем цель
  #ifdef ANALOG
   azAngleSensor = analogRead(AZ_P3022_V1_CW360_SENSOR_PIN);
  //azAngle = int(round(azAngleSensor / 2.8));
  #endif
#ifdef NETWORK
  int size = udp.parsePacket();
  int i = 0;
  char buffer[100];
  int az;
  int el;
  if (size > 0) {
    do
      {
        char* msg = (char*)malloc(size+1);
        int len = udp.read(msg,size+1);
        msg[len]=0;
        sscanf(msg, "%d %d", &az, &el);
        
        if(prevAz != az) {
          azAngleSensor = az;
        }
        prevAz = az;
        
        free(msg);
      }
    while ((size = udp.available())>0);
    udp.flush();

    int success;
    do
      {
        //Serial.println(udp.remoteIP());
        success = udp.beginPacket(udp.remoteIP(),udp.remotePort());
      }
    while (!success);
    success = udp.endPacket();
    udp.stop();
    udp.begin(41234);
  }
#endif
  azAngle = int(round(azAngleSensor / 2.8));
  // azAngle = int(round(azAngleSensor / 1024 * 360));
  azTarget = azAngle;
}

void loop(){
  while (w == 0) {
  if(clearFlag) {
      clearDisplay();
      clearFlag = false;
      getSpeed();
     lcd.setCursor(0, 0);
     lcd.print("AZ ");
     lcd.setCursor(0, 1);
     lcd.print("TGT ");
     lcd.setCursor(8, 1);
     lcd.print("PRS ");
  }

#ifdef ANALOG
  azAngleSensor = analogRead(AZ_P3022_V1_CW360_SENSOR_PIN);
#endif

#ifdef NETWORK
  int size = udp.parsePacket();
  int i = 0;
  char buffer[100];
  int az;
  int el;

  if (size > 0) {
    do
      {
        char* msg = (char*)malloc(size+1);
        int len = udp.read(msg,size+1);
        msg[len]=0;
        sscanf(msg, "%d %d", &az, &el);

        if(prevAz != az) {
          azAngleSensor = az;
        }
        prevAz = az;
        free(msg);
      }
    while ((size = udp.available())>0);
    udp.flush();

    int success;
    do
      {
        success = udp.beginPacket(udp.remoteIP(),udp.remotePort());
      }
    while (!success);
    success = udp.endPacket();
    udp.stop();
    udp.begin(41234);
  }
#endif

  azAngle = int(round(azAngleSensor / 2.8));
  // azAngle = int(round(azAngleSensor / 1024 * 360));

  if(prevAz != az) {
      Serial.println("AZ: " + String(azAngle) + " EL: " + String(el));
  }
 

  currentTime = millis();
  if (currentTime >= (loopTime + 2)) {
    azEncoder = digitalRead(PIN_CLK);
    if ((!azEncoder) && (azEncoderPrev)) {
      if (digitalRead(PIN_DT)) {
        if (azPreset + AZ_STEP <= 360) azPreset += AZ_STEP;
      }
      else {
        if (azPreset - AZ_STEP >= 0) azPreset -= AZ_STEP;
      }
    }
    azEncoderPrev = azEncoder;
    
  }

  loopTime = currentTime;

    switch (button()) {
      case 1:  
          azTarget = azPreset;
          Serial.print("AZ ");
          Serial.println(azAngle);
          Serial.print("Target ");
          Serial.println(azTarget);
          azHold = false;
          if (azTarget >= 100){ strAzTarget=String(azTarget);}
          if (azTarget < 100) {strAzTarget=" "+String(azTarget);}
          if (azTarget < 10) {strAzTarget="  "+String(azTarget);}
         break;
      case 2:
         delay(200);
         clearFlag = true;
         w = 1;
         break;
    } 

     lcd.setCursor(4, 0);
     lcd.print(strAzAngle);
     lcd.setCursor(4, 1);
     lcd.print(strAzTarget);
     lcd.setCursor(13, 1);
     lcd.print(strAzPres);

    if (azTarget != azAngle) {
  


    //для перемещения в перделах 1 градуса
    // if ( azTarget - azAngle <= 1) {
    //    speed(false);
    //    Serial.print("CW ");
    //    Serial.println(azTarget - azAngle);
    //    cw();
    // }

    // if ( azTarget - azAngle <= -1) {
    //    speed(false);
    //    Serial.print("CW ");
    //    Serial.println(azTarget - azAngle);
    //    ccw();
    // }

    if (azTarget - azAngle > (azHold ? HYSTERESIS_HOLD : HYSTERESIS)) {
       Serial.print("CW ");
       Serial.println(azTarget - azAngle);
      if(abs(azTarget - azAngle) > 20){
         speed(true);
       }
       if(abs(azTarget - azAngle) < 20){
         speed(false);
       }
       cw();
     }

    if (azAngle - azTarget > (azHold ? HYSTERESIS_HOLD : HYSTERESIS)) {
       Serial.print("CCW ");
       Serial.println(azAngle - azTarget);
        if(abs(azAngle - azTarget) > 20){
         speed(true);
       }
       if(abs(azAngle - azTarget) < 20){
         speed(false);
       }
       ccw();
     }
    }

    if ( abs(azTarget - azAngle) < (azHold ? HYSTERESIS_HOLD : HYSTERESIS)) {
      if (azTarget == azAngle) { 
       azHold = true;
       digitalWrite(PIN_CW, HIGH);
       digitalWrite(PIN_CCW, HIGH);
       lcd.setCursor(9, 0);
       lcd.print("   ");
      }
  

    }


     if (azPreset >= 100) {
       strAzPres = String(azPreset);
     }
     if (azPreset < 100) {
       strAzPres = " " + String(azPreset);
     }
     if (azPreset < 10) {
       strAzPres = "  " + String(azPreset);
     }
     if (azAngle >= 100) {
       strAzAngle = String(azAngle);
     }
     if (azAngle < 100) {
      strAzAngle = " " + String(azAngle);
     }
     if (azAngle < 10) {
       strAzAngle = "  " + String(azAngle);
     }
  }

  while (w == 1) {
    if(clearFlag) {
       clearDisplay();
       clearFlag = false;
       lcd.setCursor(0, 0);
       lcd.print("CAL   MODE  EXIT");
    }

    // проверяем положение ручки энкодера
    encoder.tick();
    newPos = encoder.getPosition() * STEPS;
    if (newPos < POSMIN) {
      encoder.setPosition(POSMIN / STEPS);
      newPos = POSMIN;
    }
    else if (newPos > POSMAX) {
      encoder.setPosition(POSMAX / STEPS);
      newPos = POSMAX;
    }

    if (lastPos != newPos) {
      lcd.setCursor(lastPos, 1);
      lcd.print("    ");
      lcd.setCursor(newPos, 1);
      lcd.print("====");
      lastPos = newPos;
    }

    bool buttonIsUp = digitalRead(ENC_BUTTON_PIN);
    if (buttonWasUp && !buttonIsUp) {
      delay(10);
      buttonIsUp = digitalRead(ENC_BUTTON_PIN);
      if (!buttonIsUp  && newPos == 0)  { lcd.clear(); delay(500); clearFlag = true; w = 2; }
      if (!buttonIsUp  && newPos == 6)  { lcd.clear(); delay(500); clearFlag = true; w = 3; }
      if (!buttonIsUp  && newPos == 12) { lcd.clear(); delay(500); clearFlag = true; w = 0; }
    }
  }
  // Меню калибровка
   while (w == 2) {
    if(clearFlag) {
       clearDisplay();
       clearFlag = false;
       lcd.setCursor(0, 0);
       lcd.print("ANGL  TURN  EXIT");
    }
  
    encoder.tick();
    newPos = encoder.getPosition() * CAL_STEPS;
    if (newPos < CAL_POSMIN) {
      encoder.setPosition(CAL_POSMIN / CAL_STEPS);
      newPos = CAL_POSMIN;
    }
    else if (newPos > CAL_POSMAX) {
      encoder.setPosition(CAL_POSMAX / CAL_STEPS);
      newPos = CAL_POSMAX;
    }

    if (lastPos != newPos) {
      lcd.setCursor(lastPos, 1);
      lcd.print("    ");
      lcd.setCursor(newPos, 1);
      lcd.print("====");
      lastPos = newPos;
    }

    bool buttonIsUp = digitalRead(ENC_BUTTON_PIN);
    if (buttonWasUp && !buttonIsUp) {
      delay(10);
      buttonIsUp = digitalRead(ENC_BUTTON_PIN);
      if (!buttonIsUp  && newPos == 0)  { lcd.clear(); delay(500); clearFlag = true; w = 21; }
      if (!buttonIsUp  && newPos == 6)  { lcd.clear(); delay(500); clearFlag = true; w = 22; }
      if (!buttonIsUp  && newPos == 12)  { lcd.clear(); delay(500); clearFlag = true; w = 23; }
    }

   }
  // Меню изменения угла коррекции
  while (w == 21) {
        if(clearFlag) {
          clearDisplay();
          clearFlag = false;
          lcd.setCursor(0, 1);
          lcd.print("ANGLE ");
        }
        currentTime = millis();
        if (currentTime >= (loopTime + 1)) {
          azCalibrate = digitalRead(PIN_CLK);
          if ((!azCalibrate) && (azCalibratePrev)) {
            if (digitalRead(PIN_DT)) {
              if (calibrate + AZ_STEP <= 360) calibrate += AZ_STEP;
            }
            else {
              if (calibrate - AZ_STEP >= 0) calibrate -= AZ_STEP;
            }
          }
          azCalibratePrev = azCalibrate;
        }
        loopTime = currentTime;
      
        if (calibrate >= 100) {
          strAzCal = String(calibrate);
        }
        if (calibrate < 100) {
          strAzCal = " " + String(calibrate);
        }
        if (calibrate < 10) {
          strAzCal = "  " + String(calibrate);
        }

        lcd.setCursor(6, 1);
        lcd.print(strAzCal);

        if(button() == 2) {
           generateAzimuthMap(azAngle, calibrate);
           delay(1000);
           clearFlag = true;
           w = 2;
        } 
  }
      // Меню включение и выключени коррекции
    while (w == 22) {
    if(clearFlag) {
        clearDisplay();
        clearFlag = false;
        lcd.setCursor(0, 0);
        lcd.print("ON    OFF   EXIT");
    }

    encoder.tick();
    newPos = encoder.getPosition() * TURN_STEPS;
    if (newPos < TURN_POSMIN) {
      encoder.setPosition(TURN_POSMIN / TURN_STEPS);
      newPos = TURN_POSMIN;
    }
    else if (newPos > TURN_POSMAX) {
      encoder.setPosition(TURN_POSMAX / TURN_STEPS);
      newPos = TURN_POSMAX;
    }

    if (lastPos != newPos) {
      lcd.setCursor(lastPos, 1);
      lcd.print("    ");
      lcd.setCursor(newPos, 1);
      lcd.print("====");
      lastPos = newPos;
    }

    if (correctFlag) {
        lcd.setCursor(2, 0);
        lcd.print("@");
    }

    if (!correctFlag) {
        lcd.setCursor(9, 0);
        lcd.print("@");
    }

    bool buttonIsUp = digitalRead(ENC_BUTTON_PIN);
    if (buttonWasUp && !buttonIsUp) {
      buttonIsUp = digitalRead(ENC_BUTTON_PIN);

      if (!buttonIsUp && newPos == 0) { 
            correctFlag = true;
            lcd.setCursor(9, 0);
            lcd.print(" ");
      }

      if (!buttonIsUp && newPos == 6) { 
            lcd.setCursor(2, 0);
            lcd.print(" ");
            correctFlag = false;
      }

      if (!buttonIsUp && newPos == 12) { 
            delay(50); 
            clearFlag = true; 
            w = 2; 
      }
    }  
   }

  //Exit
  while (w == 23) {
       if(clearFlag) {
         clearDisplay();
         clearFlag = false;
       }
   lcd.setCursor(1, 0);
   clearFlag = true;
   delay(1000);
   w = 1;
  }

  while (w == 3) {
    if(clearFlag) {
      clearDisplay();
      clearFlag = false;
    }
   lcd.setCursor(1, 0);
   lcd.print("MODE  ");
  }
}
