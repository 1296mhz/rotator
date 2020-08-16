// #include <LiquidCrystal.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// #include "RotaryEncoder.h"
#include <UIPEthernet.h>

#define AZ_STEP 1
#define EL_STEP 1
#define AZ_P3022_V1_CW360_SENSOR_PIN A0 // select the input pin for the antenna potentiometer
#undef AZ_P3022_V1_CW360_SENSOR_PIN
#define MAC                            \
  {                                    \
    0x00, 0x01, 0x02, 0x03, 0x04, 0x07 \
  }
#define IP          \
  {                 \
    192, 168, 1, 41 \
  }
#define PORT 41234
#define ANALOG
#undef ANALOG
#define NETWORK

#define ENC_BUTTON_PIN 4 // the number of the pushbutton encoder pin
//#define LED_PIN 13     // select the pin for the LED
#define PIN_CLK 2
#define PIN_DT 3

#define PIN_CCW 7   // Поворот против часовой стрелки
#define PIN_CW 6    // Поворот по часовой стрелки
#define PIN_SPEED 8 //Скорость поворота

#define PIN_TO_SKY 5    // В небо
#define PIN_TO_GROUND 9 // В землю

//Кнопки
#define BTN_AZ_EL 1
#define CTRL_KEYS A0

EthernetUDP udp;

// int lastPos, newPos = 180;
int currentTime, loopTime;
int azEncoder, azEncoderPrev;
int elEncoder, elEncoderPrev;
bool buttonEncoder = false;
bool buttonEncoderLong = false;
bool buttonState;
bool buttonWasUp = true;
bool clearFlag = false;
bool onOffFlag = false;
bool azMove = false;
bool elMove = false;
int azimuth_calibration_to[360] = {};
int buttonPin = 2;

// Для азимута

float azAngleSensor = 0.0; // С сенcора угла азимута
float elAngleSensor = 0.0; // С сенcора угла элевации
int azAngle = 0;           // Угол азимута
int azTarget = 180;        // Цель для поворота
int elAngle = 0;
int elTarget = 0;

int prevStopFlag;
byte switchAzEl = 1;

String strAzAngle;
String strAzTarget;

String strElAngle;
String strElTarget;

LiquidCrystal_I2C lcd(0x27, 16, 2);
// Экран 0 - рабочий, 1 - калибровка
int w = 0;
bool correctFlag = false;
uint32_t last_millis;
// int currentValue, prevValue;
void getNetworkSensor()
{
  int size = udp.parsePacket();
  int i = 0;

  char az[7], el[7];
  if (size > 0)
  {
    do
    {
      char *msg = (char *)malloc(size + 1);
      int len = udp.read(msg, size + 1);
      msg[len] = 0;
      sscanf(msg, "%s %s", &az, &el);
      azAngleSensor = atof(az);
      elAngleSensor = atof(el);
      free(msg);
    } while ((size = udp.available()) > 0);
    udp.flush();

    int success;
    do
    {
      success = udp.beginPacket(udp.remoteIP(), udp.remotePort());
    } while (!success);
    success = udp.endPacket();
    udp.stop();
    udp.begin(PORT);
  }
}
void clearDisplay()
{
  lcd.clear();
};

void cw()
{
  digitalWrite(PIN_CW, LOW);
  lcd.setCursor(14, 0);
  lcd.print(">");
  digitalWrite(PIN_CCW, HIGH);
}

void ccw()
{
  digitalWrite(PIN_CCW, LOW);
  lcd.setCursor(14, 0);
  lcd.print("<");
  digitalWrite(PIN_CW, HIGH);
}

void speed(bool spd)
{
  if (spd)
  {
    lcd.setCursor(15, 0);
    lcd.print("F");
    digitalWrite(PIN_SPEED, HIGH);
  }

  if (!spd)
  {
    lcd.setCursor(15, 0);
    lcd.print("S");
    digitalWrite(PIN_SPEED, LOW);
  }
}

void getSpeed()
{
  if (digitalRead(PIN_SPEED))
  {
    lcd.setCursor(15, 0);
    lcd.print("F");
  }

  if (!digitalRead(PIN_SPEED))
  {
    lcd.setCursor(15, 0);
    lcd.print("S");
  }
}

void sky()
{
  // dummy func
  Serial.println("To sky");
  elAngle++;
}

void ground()
{
  // dummy func
  Serial.println("To ground");
  elAngle--;
}

uint8_t button()
{
  if (digitalRead(ENC_BUTTON_PIN) == 1)
  {
    last_millis = millis();
    return 0;
  }
  delay(30);
  while (digitalRead(ENC_BUTTON_PIN) == 0)
    ;
  delay(30);
  if (last_millis + 65 > millis())
  {
    last_millis = millis();
    return 0;
  }
  if (last_millis + 300 > millis())
  {
    last_millis = millis();
    return 1;
  }
  last_millis = millis();
  return 2;
};

int generateAzimuthMap(int azAngle, int calibrate)
{
  for (int x = 0; x < 359 + 1; x++)
  {
    if (azAngle + calibrate + x <= 359)
    {
      azimuth_calibration_to[x] = azAngle + calibrate + x;
    }
    if (calibrate + azAngle + x > 359)
    {
      azimuth_calibration_to[x] = abs(359 - (calibrate + azAngle + x));
    }
  }
  return azimuth_calibration_to;
}

int azimuthSubstitutionMap(bool correctFlag, int azAngle, int azimuth_calibration_to)
{
  if (correctFlag)
  {
    if (sizeof(azimuth_calibration_to) > 0)
    {

      return azAngle + 5;
    }
  }
  return azAngle;
}

void setup()
{
  speed(true);
#ifdef NETWORK
  uint8_t mac[6] = MAC;
  uint8_t ip[4] = IP;
  Ethernet.begin(mac, IPAddress(ip));
  int success = udp.begin(PORT);
#endif
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

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

int Az_El()
{
  if (switchAzEl == 1)
  {
    switchAzEl = 2;
  }
  else if (switchAzEl == 2)
  {
    switchAzEl = 1;
  }

  return switchAzEl;
}

void CursorAzEl()
{
  if (switchAzEl == 1)
  {
    lcd.setCursor(1, 0);
    lcd.print(">");
    lcd.setCursor(1, 1);
    lcd.print(" ");
  }
  if (switchAzEl == 2)
  {
    lcd.setCursor(1, 0);
    lcd.print(" ");
    lcd.setCursor(1, 1);
    lcd.print(">");
  }
}

void loop()
{
  while (w == 0)
  {

    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;
      getSpeed();
      lcd.setCursor(0, 0);
      lcd.print("A ");
      lcd.setCursor(0, 1);
      lcd.print("E ");
    }

#ifdef NETWORK
    getNetworkSensor();
#endif
    boolean buttonState;
    int buttonPressTime;
    int keyAnalog = analogRead(CTRL_KEYS);
    if (keyAnalog < 100)
    {
      if ((millis() - buttonPressTime > 1500))
      {
        Az_El();
        buttonState = true;
        buttonPressTime = millis();
      }
      else
      {
        buttonState = false;
      }
    }
    if (keyAnalog < 200)
    {
      if ((millis() - buttonPressTime > 1500))
      {

        buttonState = true;
        buttonPressTime = millis();
      }
      else
      {
        buttonState = false;
      }
    }

    CursorAzEl();
    azAngle = int(round(azAngleSensor / 1020.0 * 360));
    if (switchAzEl == 1)
    {
      // AZ
      currentTime = millis();
      if (currentTime - (loopTime + 5))
      {

        azEncoder = digitalRead(PIN_CLK);

        if ((!azEncoder) && (azEncoderPrev))
        {
          if (digitalRead(PIN_DT))
          {
            if (azTarget + AZ_STEP <= 359)
              azTarget += AZ_STEP;
          }
          else
          {
            if (azTarget - AZ_STEP >= 0)
              azTarget -= AZ_STEP;
          }
        }
        azEncoderPrev = azEncoder;
      }

      loopTime = currentTime;

      switch (button())
      {
      case 1:
        azMove = true;
        if (azTarget >= 100)
        {
          strAzTarget = String(azTarget);
        }
        if (azTarget < 100)
        {
          strAzTarget = " " + String(azTarget);
        }
        if (azTarget < 10)
        {
          strAzTarget = "  " + String(azTarget);
        }
        break;
      }
    }

    if (switchAzEl == 2)
    {
      currentTime = millis();
      if (currentTime >= (loopTime + 5))
      {
        elEncoder = digitalRead(PIN_CLK);
        if ((!elEncoder) && (elEncoderPrev))
        {
          if (digitalRead(PIN_DT))
          {
            if (elTarget + EL_STEP <= 90)
              elTarget += EL_STEP;
          }
          else
          {
            if (elTarget - EL_STEP >= 0)
              elTarget -= EL_STEP;
          }
        }
        elEncoderPrev = elEncoder;
      }

      loopTime = currentTime;

      switch (button())
      {
      case 1:
        elMove = true;
        if (elTarget >= 100)
        {
          strElTarget = String(elTarget);
        }
        if (elTarget < 100)
        {
          strElTarget = " " + String(elTarget);
        }
        if (elTarget < 10)
        {
          strElTarget = "  " + String(elTarget);
        }
        break;
      }
    }

    if (azMove)
    {
      if (azTarget - azAngle >= 1)
      {
        if (abs(azTarget - azAngle) > 20)
        {
          speed(true);
        }
        if (abs(azTarget - azAngle) < 20)
        {
          speed(false);
        }
        cw();
      }

      if (azAngle - azTarget >= 1)
      {
        if (abs(azAngle - azTarget) > 20)
        {
          speed(true);
        }
        if (abs(azAngle - azTarget) < 20)
        {
          speed(false);
        }
        ccw();
      }

      if (azTarget == azAngle)
      {
        azMove = false;
        lcd.setCursor(14, 0);
        lcd.print(" ");
        digitalWrite(PIN_CW, HIGH);
        digitalWrite(PIN_CCW, HIGH);
      }
    }

    if (elMove)
    {
      if (elTarget - elAngle >= 1)
      {
        sky();
      }

      if (elAngle - elTarget >= 1)
      {
        ground();
      }

      if (elTarget == elAngle)
      {
        elMove = false;
        // lcd.setCursor(14, 0);
        //  lcd.print(" ");
        // digitalWrite(PIN_TO_SKY, HIGH);
        // digitalWrite(PIN_TO_GROUND, HIGH);
      }
    }

    // Отображение азимута
    lcd.setCursor(2, 0);
    lcd.print(strAzAngle);
    lcd.setCursor(6, 0);
    lcd.print(strAzTarget);

    if (azTarget >= 100)
    {
      strAzTarget = String(azTarget);
    }

    if (azTarget < 100)
    {
      strAzTarget = " " + String(azTarget);
    }

    if (azTarget < 10)
    {
      strAzTarget = "  " + String(azTarget);
    }

    if (azAngle >= 100)
    {
      strAzAngle = String(azAngle);
    }

    if (azAngle < 100)
    {
      strAzAngle = " " + String(azAngle);
    }

    if (azAngle < 10)
    {
      strAzAngle = "  " + String(azAngle);
    }

    // Отображение элевация
    lcd.setCursor(2, 1);
    lcd.print(strElAngle);
    lcd.setCursor(6, 1);
    lcd.print(strElTarget);
    if (elTarget < 100)
    {
      strElTarget = " " + String(elTarget);
    }

    if (elTarget < 10)
    {
      strElTarget = "  " + String(elTarget);
    }

    if (elAngle < 100)
    {
      strElAngle = " " + String(elAngle);
    }

    if (elAngle < 10)
    {
      strElAngle = "  " + String(elAngle);
    }
  }
}
