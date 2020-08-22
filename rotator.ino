// #include <LiquidCrystal.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <UIPEthernet.h>
#include <avr/eeprom.h>

#define AZ_STEP 1
#define EL_STEP 1
#define MAC                            \
  {                                    \
    0x00, 0x01, 0x02, 0x03, 0x04, 0x07 \
  }
#define IP          \
  {                 \
    192, 168, 1, 41 \
  }
#define PORT 41234

#define NETWORK

#define ENC_BUTTON_PIN 4 // the number of the pushbutton encoder pin
//#define LED_PIN 13     // select the pin for the LED
#define PIN_CLK 2
#define PIN_DT 3

#define PIN_CCW 7   // Поворот против часовой стрелки
#define PIN_CW 6    // Поворот по часовой стрелки
#define PIN_SPEED 8 //Скорость поворота

// #define PIN_TO_SKY 5    // В небо
// #define PIN_TO_GROUND 9 // В землю

#define PIN_BTN_MODE 5
#define PIN_BTN_MULTI 9
//Кнопки
#define BTN_AZ_EL 1

EthernetUDP udp;

// int lastPos, newPos = 180;
uint16_t currentTime, loopTime;
byte encoder, encoderPrev;
// byte elEncoder, elEncoderPrev;
uint8_t buttonPressTime = 0;

//Флаг очистки эерана на каждом скрине
bool clearFlag = false;

// Флаг для включения вращения по азимуту
bool azMove = false;
// Флаг для включения вращения по элевации
bool elMove = false;
//Флаг включения смещения
bool offsetFlag = false;
// Хранение смещения для азимута
uint16_t offsetAz = 0;
// Хранение смещения для элевации
uint16_t offsetEl = 0;

// Для азимута
float azAngleSensor = 0.0; // С сенcора угла азимута
float elAngleSensor = 0.0; // С сенcора угла элевации

uint16_t azAngle = 0;    // Угол азимута
uint16_t azTarget = 180; // Цель для поворота
uint16_t elAngle = 0;
uint16_t elTarget = 0;
uint16_t azDelta = 0;
byte deltaDirection = 1;
//Флаг переключение азимут/элевация
byte switchAzEl = 1;

//Переменная для отображения азимута  в режиме MANUAL
String strAzAngle;  // Текущее положение антенны
String strAzTarget; // Цель для перемещения

//Переменная для отображения эливации в режиме MANUAL
String strElAngle;  // Текущее положение антенны
String strElTarget; // Цель для перемещения

//Переменная для отображения азимута и элевации в режиме PORT
String strAzTargetPort; // Цель для перемещения
String strElTargetPort; // Цель для перемещения

//Переменные для отображения смещения, в режиме SET
String strAzOffset;
String strElOffset;

int azTargetPort = 0;
int elTargetPort = 0;

struct SettingsStruct
{
  bool offsetFlag;
  byte deltaDirection;
  int azDelta;
  int offsetAz;
};
SettingsStruct newSettingsStruct;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Экран 0 - MANUAL, 1 - PORT, 2 - SETTINGS
byte appScreen = 0;
bool correctFlag = false;
uint32_t last_millis;

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
    // lcd.setCursor(15, 0);
    // lcd.print("F");
    digitalWrite(PIN_SPEED, HIGH);
  }

  if (!spd)
  {
    // lcd.setCursor(15, 0);
    // lcd.print("S");
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

uint8_t buttonEnc()
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

uint8_t buttonMode()
{
  if (digitalRead(PIN_BTN_MODE) == 0)
  {
    last_millis = millis();
    return 0;
  }
  delay(30);
  while (digitalRead(PIN_BTN_MODE) == 1)
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

uint8_t buttonMulti()
{
  if (digitalRead(PIN_BTN_MULTI) == 0)
  {
    last_millis = millis();
    return 0;
  }
  delay(30);
  while (digitalRead(PIN_BTN_MULTI) == 1)
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

int azDeltaGen(int sensorAz, int realAz)
{
  if (sensorAz == realAz)
  {
    deltaDirection = 1;
  }

  if (sensorAz < realAz)
  {
    deltaDirection = 3;
    return abs(realAz - sensorAz);
  }

  if (sensorAz > realAz)
  {
    deltaDirection = 2;
    return 359 - abs(sensorAz - realAz) + 1;
  }
}

int sensorAzToRealAz(int sensorAz, int delta)
{
  if (deltaDirection == 1)
  {
    return sensorAz;
  }

  if (deltaDirection == 2)
  {
    if (sensorAz + delta > 359)
    {
      return (sensorAz + delta - 359) - 1;
    }
    if (sensorAz + delta > 0)
    {
      return sensorAz + delta;
    }
  }

  if (deltaDirection == 3)
  {
    if (sensorAz - delta > 0)
    {
      return sensorAz - delta;
    }
    if (sensorAz - delta < 0)
    {
      return 359 - abs(sensorAz - delta);
    }
    return sensorAz - delta;
  }
}

int offsetFilter(bool offsetFlag, int sensorAz)
{
  if (offsetFlag)
  {
    return sensorAzToRealAz(sensorAz, azDelta);
  }
  else
  {
    return sensorAz;
  }
}

void offsetSwitchIndicator()
{
  if (offsetFlag)
  {
    lcd.setCursor(15, 1);
    lcd.print("#");
  }
  else
  {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
}

int Az_El()
{
  if (switchAzEl == 1)
  {
    clearFlag = true;
    switchAzEl = 2;
  }
  else if (switchAzEl == 2)
  {
    clearFlag = true;
    switchAzEl = 1;
  }

  return switchAzEl;
}

int AppScreen()
{
  if (appScreen == 0)
  {
    appScreen = 1;
    clearFlag = true;
  }
  else if (appScreen == 1)
  {
    appScreen = 2;
    clearFlag = true;
  }
  else if (appScreen == 2)
  {
    appScreen = 0;
    clearFlag = true;
  }

  return appScreen;
}

void SwitchOffset()
{
  if (offsetFlag)
  {
    offsetFlag = false;
    if (newSettingsStruct.offsetFlag != false)
    {
      newSettingsStruct.offsetFlag = false;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
    }
  }
  else
  {
    offsetFlag = true;
    if (newSettingsStruct.offsetFlag != true)
    {
      newSettingsStruct.offsetFlag = true;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
    }
  }
}

void BtnSwitchOffset(int keyAnalog)
{
  if (keyAnalog < 400 && keyAnalog > 200)
  {
    if ((millis() - buttonPressTime > 200))
    {
      if (offsetFlag)
      {
        offsetFlag = false;
        if (newSettingsStruct.offsetFlag != false)
        {
          newSettingsStruct.offsetFlag = false;
          eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
          eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
        }
      }
      else
      {
        offsetFlag = true;
        if (newSettingsStruct.offsetFlag != true)
        {
          newSettingsStruct.offsetFlag = true;
          eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
          eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
        }
      }
      buttonPressTime = millis();
    }
  }
}

void CursorAzEl(boolean switch_cursor_one, byte cursor_one, byte cursor_one_string, boolean switch_cursor_two, byte cursor_two, byte cursor_two_string)
{
  if (switchAzEl == 1)
  {
    //4
    if (switch_cursor_one)
    {
      lcd.setCursor(cursor_one, cursor_one_string);
      lcd.print(">");
    }

    //9
    if (switch_cursor_two)
    {
      lcd.setCursor(cursor_two, cursor_two_string);
      lcd.print(" ");
    }
  }
  if (switchAzEl == 2)
  {
    //4
    if (switch_cursor_one)
    {
      lcd.setCursor(cursor_one, cursor_one_string);
      lcd.print(" ");
    }
    //9
    if (switch_cursor_two)
    {
      lcd.setCursor(cursor_two, cursor_two_string);
      lcd.print(">");
    }
  }
}

void screenManualPort()
{
  lcd.setCursor(0, 0);
  lcd.print("     A    E");
  lcd.setCursor(0, 1);
  lcd.print(" ANT A    E");
}

String AzElString(uint16_t someIntVolue)
{
  if (someIntVolue >= 100)
  {
    return String(someIntVolue);
  }

  if (someIntVolue < 100)
  {
    return " " + String(someIntVolue);
  }

  if (someIntVolue < 10)
  {
    return "  " + String(someIntVolue);
  }
}

//TGT, STEP, AZ/AL - true/false
int MoveEncoder(int * tgt, byte STEP ){

      
      currentTime = millis();
      if (currentTime >= (loopTime + 15))
      {
        encoder = digitalRead(PIN_CLK);
        if ((!encoder) && (encoderPrev))
        {
          if (digitalRead(PIN_DT))
          {
            if (*tgt + AZ_STEP <= 359)
            //Serial.println(*tgt += AZ_STEP);
              *tgt += AZ_STEP;
          }
          else
          {
            if (*tgt - AZ_STEP >= 0)
             //Serial.println(*tgt -= AZ_STEP);
              *tgt -= AZ_STEP;
          }
        }
        encoderPrev = encoder;
      }

      loopTime = currentTime;
}

// void RotateEncoder()
// {
//   currentTime = millis();
//   if (currentTime >= (loopTime + 15))
//   {
//     encoder = digitalRead(PIN_CLK);
//     int encoder_B = digitalRead(PIN_DT);
//     if ((!encoder) && (encoderPrev))
//     {
//       if (encoder_B)
//       {
//         if (azTarget + AZ_STEP <= 359)
//           azTarget += AZ_STEP;
//       }
//       else
//       {
//         if (azTarget - AZ_STEP >= 0)
//           azTarget -= AZ_STEP;
//       }
//     }
//     encoderPrev = encoder;
//   }
//   loopTime = currentTime;
// }

void setup()
{
  speed(true);
  //Magic number 137
  if (eeprom_read_byte(1023) != 137)
  {
    SettingsStruct settingsStruct;
    settingsStruct.offsetFlag = false;
    settingsStruct.deltaDirection = 1;
    settingsStruct.azDelta = 0;
    settingsStruct.offsetAz = 0;
    eeprom_write_block((void *)&settingsStruct, 0, sizeof(settingsStruct));
    eeprom_write_byte(1023, 137);
  }

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
  // lcd.print("*R8CDF*");

  delay(1000);

  lcd.clear();
  clearFlag = true;
  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);
  pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIN_CCW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(PIN_SPEED, OUTPUT);
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_MULTI, INPUT_PULLUP);
  last_millis = millis();

  eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
  offsetFlag = newSettingsStruct.offsetFlag;
  deltaDirection = newSettingsStruct.deltaDirection;
  azDelta = newSettingsStruct.azDelta;
  offsetAz = newSettingsStruct.offsetAz;

#ifdef NETWORK
  getNetworkSensor();
#endif
  azAngle = int(round(azAngleSensor / 1024.0 * 360));
}

void loop()
{
#ifdef NETWORK
  getNetworkSensor();
#endif
  int _azAngle = int(round(azAngleSensor / 1024.0 * 360));
  azAngle = offsetFilter(offsetFlag, _azAngle);
  if (appScreen == 0)
  {
    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;

      screenManualPort();
      lcd.setCursor(0, 0);
      lcd.print(" MAN");
    }
    switch (buttonMode())
    {
    case 1:
      AppScreen();
      break;
    }

    switch (buttonMulti())
    {
    case 1:
      SwitchOffset();
      break;
    }

    CursorAzEl(true, 4, 0, true, 9, 0);
    offsetSwitchIndicator();
    if (switchAzEl == 1)
    {
      // AZ
      // currentTime = millis();
      // if (currentTime >= (loopTime + 15))
      // {
      //   encoder = digitalRead(PIN_CLK);
      //   int encoder_B = digitalRead(PIN_DT);
      //   if ((!encoder) && (encoderPrev))
      //   {
      //     if (encoder_B)
      //     {
      //       if (azTarget + AZ_STEP <= 359)
      //         azTarget += AZ_STEP;
      //     }
      //     else
      //     {
      //       if (azTarget - AZ_STEP >= 0)
      //         azTarget -= AZ_STEP;
      //     }
      //   }
      //   encoderPrev = encoder;
      // }
      // loopTime = currentTime;
      //RotateEncoder();
      MoveEncoder(&azTarget, AZ_STEP);
      switch (buttonEnc())
      {
      case 1:
        azMove = true;
        strAzTarget = AzElString(azTarget);
        break;
      case 2:
        Az_El();
        break;
      }
    }

    if (switchAzEl == 2)
    {
      currentTime = millis();
      if (currentTime >= (loopTime + 15))
      {
        encoder = digitalRead(PIN_CLK);
        if ((!encoder) && (encoderPrev))
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
        encoderPrev = encoder;
      }

      loopTime = currentTime;

      switch (buttonEnc())
      {
      case 1:
        elMove = true;
        strElTarget = AzElString(elTarget);
        break;
      case 2:
        Az_El();
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

    // Отображение азимута текущего положения антенны
    lcd.setCursor(6, 1);
    lcd.print(strAzAngle);
    // Отображение азимута текущей цели антенны
    lcd.setCursor(6, 0);
    lcd.print(strAzTarget);

    // Отображение данных с датчика
    strAzAngle = AzElString(azAngle);

    // Отображение цели
    strAzTarget = AzElString(azTarget);

    // Отображение элевация с датчика
    lcd.setCursor(11, 1);
    lcd.print(strElAngle);

    if (elAngle < 100)
    {
      strElAngle = " " + String(elAngle);
    }

    if (elAngle < 10)
    {
      strElAngle = "  " + String(elAngle);
    }

    // Отображение цели элевации
    lcd.setCursor(11, 0);
    lcd.print(strElTarget);

    if (elTarget < 100)
    {
      strElTarget = " " + String(elTarget);
    }

    if (elTarget < 10)
    {
      strElTarget = "  " + String(elTarget);
    }
  }

  if (appScreen == 1)
  {
    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;
      getSpeed();
      screenManualPort();
      lcd.setCursor(0, 0);
      lcd.print("PORT");
    }

    switch (buttonMode())
    {
    case 1:
      if (azMove != true && elMove != true)
      {
        AppScreen();
      }
      break;
    }

    switch (buttonMulti())
    {
    case 1:
      SwitchOffset();
      break;
    }
    offsetSwitchIndicator();

    // Отображение данных с датчика
    strAzAngle = AzElString(azAngle);

    // Отображение азимута
    lcd.setCursor(6, 1);
    lcd.print(strAzAngle);

    // Отображение цели
    strAzTargetPort = AzElString(azTargetPort);

    lcd.setCursor(6, 0);
    lcd.print(strAzTargetPort);

    if (elAngle < 100)
    {
      strElAngle = " " + String(elAngle);
    }

    if (elAngle < 10)
    {
      strElAngle = "  " + String(elAngle);
    }

    // Отображение элевация
    lcd.setCursor(11, 1);
    lcd.print(strElAngle);

    if (elTargetPort < 100)
    {
      strElTargetPort = " " + String(elTargetPort);
    }

    if (elTargetPort < 10)
    {
      strElTargetPort = "  " + String(elTargetPort);
    }
    lcd.setCursor(11, 0);
    lcd.print(strElTargetPort);
  }

  if (appScreen == 2)
  {

    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;
      //getSpeed();
      lcd.setCursor(0, 0);
      lcd.print("OSET AZ ");
      lcd.setCursor(0, 1);
      lcd.print("OSET EL ");
    }
    CursorAzEl(true, 4, 0, true, 4, 1);
    switch (buttonMode())
    {
    case 1:
      if (azMove != true && elMove != true)
      {
        AppScreen();
      }
      break;
    }
    // offsetSwitchIndicator();
    switch (buttonEnc())
    {
    case 1:
      break;
    case 2:
      Az_El();
      break;
    }

    if (buttonEnc() == 1)
    {
      if (newSettingsStruct.deltaDirection != deltaDirection)
      {
        newSettingsStruct.deltaDirection = deltaDirection;
      }
      if (newSettingsStruct.azDelta != azDelta)
      {
        newSettingsStruct.azDelta = azDelta;
      }

      if (newSettingsStruct.offsetAz != offsetAz)
      {
        newSettingsStruct.offsetAz = offsetAz;
      }
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      clearFlag = true;
      appScreen = 0;
    }

    if (switchAzEl == 1)
    {
      // AZ
      currentTime = millis();
      if (currentTime >= (loopTime + 15))
      {
        encoder = digitalRead(PIN_CLK);
        int encoder_B = digitalRead(PIN_DT);
        if ((!encoder) && (encoderPrev))
        {
          if (encoder_B)
          {
            if (offsetAz + AZ_STEP <= 359)
              offsetAz += AZ_STEP;
          }
          else
          {
            if (offsetAz - AZ_STEP >= 0)
              offsetAz -= AZ_STEP;
          }
        }
        encoderPrev = encoder;
      }
      loopTime = currentTime;
    }

    azDelta = azDeltaGen(_azAngle, offsetAz);

    if (switchAzEl == 2)
    {
      currentTime = millis();
      if (currentTime >= (loopTime + 15))
      {
        encoder = digitalRead(PIN_CLK);
        if ((!encoder) && (encoderPrev))
        {
          if (digitalRead(PIN_DT))
          {
            if (offsetEl + EL_STEP <= 90)
              offsetEl += EL_STEP;
          }
          else
          {
            if (offsetEl - EL_STEP >= 0)
              offsetEl -= EL_STEP;
          }
        }
        encoderPrev = encoder;
      }

      loopTime = currentTime;
    }

    // Отображение данных с датчика
    strAzAngle = AzElString(_azAngle);

    // Отображение азимута
    lcd.setCursor(8, 0);
    lcd.print(strAzAngle);

    // Отображение цели
    strAzOffset = AzElString(offsetAz);

    lcd.setCursor(12, 0);
    lcd.print(strAzOffset);
  }
}
