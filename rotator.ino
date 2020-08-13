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
#define PORT 41234
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
bool azMove = false;
int azimuth_calibration_to[] = {};
int buttonPin = 2; // Кнопка 0 нажата 1 нет
int calibrate = 0;
// Для азимута
bool azHold;
float azAngleSensor = 0.0; // С сенcора угла азимута
int azAngle = 0; // Угол азимута
int azTarget = 0; // Цель для поворота
int azPreset = 180;
float prevAz;
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


void getNetworkSensor() {
  int size = udp.parsePacket();
  int i = 0;
  char buffer[100];
  char az[7], el[7];
  float azimuth;
  if (size > 0) {
    do
      {
        char* msg = (char*)malloc(size+1);
        int len = udp.read(msg,size+1);
        msg[len]=0;
        sscanf(msg, "%s %s", &az, &el);
        if(prevAz != azimuth) {
           Serial.println(az);
           azimuth=atof(az);
           azAngleSensor = azimuth;
        }
        prevAz = azimuth;
        
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
    udp.begin(PORT);
}
}
void clearDisplay() {
    lcd.clear();
};

void cw() {
  digitalWrite(PIN_CW, LOW);
  lcd.setCursor(14, 0);
  lcd.print(">"); 
  digitalWrite(PIN_CCW, HIGH);
}

void ccw() {
  digitalWrite(PIN_CCW, LOW);
  lcd.setCursor(14, 0);
  lcd.print("<"); 
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
    if(digitalRead(PIN_SPEED)) {
    lcd.setCursor(15, 0);
    lcd.print("F"); 
  }

  if(!digitalRead(PIN_SPEED)) {
    lcd.setCursor(15, 0);
    lcd.print("S"); 
  }
}

uint8_t button(){
  if (digitalRead(ENC_BUTTON_PIN) == 1) {  
     last_millis = millis();
     return 0;}
   delay(30);
   while (digitalRead(ENC_BUTTON_PIN) == 0);
   delay(30);
   if (last_millis+65 > millis()){
     last_millis = millis();
     return 0;}
   if (last_millis+300 > millis()){
     last_millis = millis();
     return 1;}
   last_millis = millis();
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

int azimuthSubstitutionMap(bool correctFlag, int azAngle, int azimuth_calibration_to) {
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
    int success = udp.begin(PORT);
  #endif

  Serial.begin(9600);
  lcd.init();                     
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(DMESG);
  Serial.println(DMESG);

  delay(500);

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
    getNetworkSensor();
  #endif
 //azAngle = int(round(azAngleSensor / 2.8));
 azAngle = int(round(azAngleSensor / 1020.0 * 360));

}

void loop() {
  while (w == 0) {
     if (clearFlag) {
       clearDisplay();
       clearFlag = false;
       getSpeed();
       lcd.setCursor(0, 0);
       lcd.print("AZ ");
       lcd.setCursor(0, 1);
       lcd.print("TGT ");
     }

#ifdef ANALOG
  azAngleSensor = analogRead(AZ_P3022_V1_CW360_SENSOR_PIN);
#endif

#ifdef NETWORK
   getNetworkSensor();
#endif

  //azAngle = int(round(azAngleSensor / 2.8));
  //azAngle = int(round(azAngleSensor / 1024.0 * 360));
  azAngle = int(round(azAngleSensor / 1020.0 * 360));
  currentTime = millis();
  if (currentTime >= (loopTime + 2)) {
    azEncoder = digitalRead(PIN_CLK);
    if ((!azEncoder) && (azEncoderPrev)) {
      if (digitalRead(PIN_DT)) {
        if (azTarget + AZ_STEP <= 360) azTarget += AZ_STEP;
      }
      else {
        if (azTarget - AZ_STEP >= 0) azTarget -= AZ_STEP;
      }
    }
    azEncoderPrev = azEncoder;
    
  }

  loopTime = currentTime;

    switch (button()) {
      case 1:  
          Serial.print("AZ ");
          Serial.println(azAngle);
          Serial.print("Target ");
          Serial.println(azTarget);
          azMove = true;
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

    if (azMove) {
       if (azTarget - azAngle >= 1) {
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

       if (azAngle - azTarget >= 1) {
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

       if (azTarget == azAngle) {
         azMove = false;
         lcd.setCursor(14, 0);
         lcd.print(" ");
         digitalWrite(PIN_CW, HIGH);
         digitalWrite(PIN_CCW, HIGH);
       }
    
    } 

     if (azTarget >= 100) {
       strAzTarget = String(azTarget);
     }
     if (azTarget < 100) {
       strAzTarget = " " + String(azTarget);
     }
     if (azTarget < 10) {
       strAzTarget = "  " + String(azTarget);
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

}
