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

#define PIN_TO_SKY 5    // В небо
#define PIN_TO_GROUND 9 // В землю

//Кнопки
#define BTN_AZ_EL 1
#define CTRL_KEYS A0

EthernetUDP udp;

// int lastPos, newPos = 180;
int currentTime, loopTime;
byte azEncoder, azEncoderPrev;
byte elEncoder, elEncoderPrev;

bool clearFlag = false;
int buttonPressTime;
bool azMove = false;
bool elMove = false;
int azimuth_calibration_to[360] = {};
int offsetAz = 0;
int offsetEl = 0;
// Для азимута

float azAngleSensor = 0.0; // С сенcора угла азимута
float elAngleSensor = 0.0; // С сенcора угла элевации
int azAngle = 0;           // Угол азимута
int azTarget = 180;        // Цель для поворота
int elAngle = 0;
int elTarget = 0;
int azDelta = 0;
byte deltaDirection = 1;
bool offsetFlag = false;
byte switchAzEl = 1;
String strAzAngle;
String strAzTarget;

String strElAngle;
String strElTarget;

String strAzOffset;
String strElOffset;
uint16_t dataB_read = 0;

// variables for serial comm
String Azimuth = "";
String Elevation = "";
String ComputerRead;
String ComputerWrite;

int ComAzim = 0;     // commanded azimuth value
int ComElev = 0;     // commanded elevation value
int TruAzim = 0;     // calculated real azimuth value
int TruElev = 0;     // calculated real elevation value
byte symbolOffset[8] = {
    0b11001,
    0b10101,
    0b11001,
    0b10001,
    0b10001,
    0b10011,
    0b10101,
    0b10011};

struct SettingsStruct
{
  bool offsetFlag;
  byte deltaDirection;
  int azDelta;
  int offsetAz;
};
SettingsStruct newSettingsStruct;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Экран 0 - рабочий, 1 - калибровка
byte appScreen = 0;
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

void clearOffset()
{
  lcd.setCursor(10, 1);
  lcd.print(" ");
  lcd.setCursor(10, 0);
  lcd.print(" ");
}
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
    lcd.print(char(1));
  }
  else
  {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
  lcd.setCursor(15, 0);
  lcd.print("S");
}



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
  lcd.print("*R8CDF ROTATOR*");
  lcd.createChar(1, symbolOffset);
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

  eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
  offsetFlag = newSettingsStruct.offsetFlag;
  deltaDirection = newSettingsStruct.deltaDirection;
  azDelta = newSettingsStruct.azDelta;
  offsetAz = newSettingsStruct.offsetAz;

  Serial.print("Restore offsetFlag: ");
  Serial.println(newSettingsStruct.offsetFlag);
  Serial.print("Restore deltaDirection: ");
  Serial.println(newSettingsStruct.deltaDirection);
  Serial.print("Restore azDelta: ");
  Serial.println(newSettingsStruct.azDelta);
  Serial.print("Restore offsetAz: ");
  Serial.println(newSettingsStruct.offsetAz);
  //dataB_read = 380;
  //eeprom_update_word(0, dataB_read);
#ifdef NETWORK
  getNetworkSensor();
#endif
  azAngle = int(round(azAngleSensor / 1024.0 * 360));
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

int AppScreen()
{
  if (appScreen == 1)
  {
    appScreen = 2;
  }
  else if (appScreen == 2)
  {
    appScreen = 1;
  }

  return appScreen;
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
  while (appScreen == 0)
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
      clearOffset();
    }
    offsetSwitchIndicator();
    // ReadSerial();
    if(Serial.available()){
         ComputerRead = Serial.readString();

        lcd.setCursor(0, 1);
        lcd.print(ComputerRead);
    }
#ifdef NETWORK
    getNetworkSensor();
#endif
    
    int keyAnalog = analogRead(CTRL_KEYS);
    if (keyAnalog < 100)
    {
      if ((millis() - buttonPressTime > 1500))
      {
        Az_El();
        buttonPressTime = millis();
      }
    }

    if (keyAnalog < 200 && keyAnalog > 100)
    {
      if ((millis() - buttonPressTime > 1500))
      {
        if (azMove != true && elMove != true)
        {
          appScreen = 1;
        }
        buttonPressTime = millis();
      }
    }

    if (keyAnalog < 400 && keyAnalog > 200)
    {
      if ((millis() - buttonPressTime > 1500))
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

    CursorAzEl();
    int _azAngle = int(round(azAngleSensor / 1024.0 * 360));
    azAngle = offsetFilter(offsetFlag, _azAngle);

    if (switchAzEl == 1)
    {
      // AZ
      currentTime = millis();
      if (currentTime >= (loopTime + 15))
      {
        azEncoder = digitalRead(PIN_CLK);
        int encoder_B = digitalRead(PIN_DT);
        if ((!azEncoder) && (azEncoderPrev))
        {
          if (encoder_B)
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

    // Отображение цели
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
    // Отображение данных с датчика
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
    // lcd.setCursor(2, 1);
    // lcd.print(strElAngle);
    // lcd.setCursor(6, 1);
    // lcd.print(strElTarget);

    // if (elTarget < 100)
    // {
    //   strElTarget = " " + String(elTarget);
    // }

    // if (elTarget < 10)
    // {
    //   strElTarget = "  " + String(elTarget);
    // }

    // if (elAngle < 100)
    // {
    //   strElAngle = " " + String(elAngle);
    // }

    // if (elAngle < 10)
    // {
    //   strElAngle = "  " + String(elAngle);
    // }
  }

  while (appScreen == 1)
  {
    offsetSwitchIndicator();
    if (button() == 1)
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
      Serial.print("Saved offsetFlag: ");
      Serial.println(newSettingsStruct.offsetFlag);
      Serial.print("Saved deltaDirection: ");
      Serial.println(newSettingsStruct.deltaDirection);
      Serial.print("Saved azDelta: ");
      Serial.println(newSettingsStruct.azDelta);
      Serial.print("Saved offsetAz: ");
      Serial.println(newSettingsStruct.offsetAz);
      clearFlag = true;
      appScreen = 0;
    }

    if (switchAzEl == 1)
    {
      lcd.setCursor(10, 0);
      lcd.print("@");
      currentTime = millis();
      if (currentTime - (loopTime + 5))
      {

        azEncoder = digitalRead(PIN_CLK);
        int pinDt = digitalRead(PIN_DT);
        if ((!azEncoder) && (azEncoderPrev))
        {
          ;
          if (pinDt)
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
        azEncoderPrev = azEncoder;
      }

      loopTime = currentTime;
      azDelta = azDeltaGen(azAngle, offsetAz);
    }

    if (switchAzEl == 2)
    {
      lcd.setCursor(10, 1);
      lcd.print("@");
      currentTime = millis();
      if (currentTime >= (loopTime + 5))
      {
        elEncoder = digitalRead(PIN_CLK);
        if ((!elEncoder) && (elEncoderPrev))
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
        elEncoderPrev = elEncoder;
      }
      loopTime = currentTime;
    }

    lcd.setCursor(11, 0);
    lcd.print(offsetAz);
    lcd.setCursor(11, 1);
    lcd.print(offsetEl);

    if (offsetAz >= 100)
    {
      strAzOffset = String(offsetAz);
    }

    if (offsetAz < 100)
    {
      strAzOffset = " " + String(offsetAz);
    }

    if (offsetAz < 10)
    {
      strAzOffset = "  " + String(offsetAz);
    }

    if (offsetEl < 100)
    {
      strElOffset = " " + String(offsetEl);
    }

    if (offsetEl < 10)
    {
      strElOffset = "  " + String(offsetEl);
    }
  }

  // while (appScreen == 2)
  // {
  //   if (clearFlag)
  //   {
  //     clearDisplay();
  //     clearFlag = false;
  //     getSpeed();
  //     clearOffset();
  //   }

  // while (Serial.available())
  // {
  //   int keyAnalog = analogRead(CTRL_KEYS);
  //   if (keyAnalog < 100)
  //   {
  //     if ((millis() - buttonPressTime > 1500))
  //     {
  //       // Az_El();
  //       appScreen = 1;
  //       buttonPressTime = millis();
  //     }
  //   }
  //   ComputerRead = Serial.readString(); // read the incoming data as string
  //                                       //    Serial.println(ComputerRead);   //if i want to see what is coming via the serial com
  //   if ((ComputerRead.charAt(0) == 'A') && (ComputerRead.charAt(1) == 'Z'))
  //   {               // if read AZ
  //     Azimuth = ""; // initialize Azimuth
  //     for (int i = 2; i <= ComputerRead.length(); i++)
  //     {
  //       if (ComputerRead.charAt(i) == ' ')
  //       { // if pause detected, see if read EL
  //         if ((ComputerRead.charAt(i + 1) == 'E') && (ComputerRead.charAt(i + 2) == 'L'))
  //         {
  //           Elevation = ""; // initialize Elevation
  //           for (int j = i + 3; j <= ComputerRead.length(); j++)
  //           {
  //             if (ComputerRead.charAt(j) == ' ')
  //             {
  //               break;
  //             }
  //             Elevation = Elevation + ComputerRead.charAt(j);
  //           }
  //           break; // exit
  //         }
  //         break;
  //       }
  //       Azimuth = Azimuth + ComputerRead.charAt(i);
  //     }
  //   }
  //   ComAzim = Azimuth.toInt();
  //   ComElev = Elevation.toInt();
  //   // keeping comand between limits
  //   ComAzim = (ComAzim + 360) % 360;
  //   if (ComElev < 0)
  //   {
  //     ComElev = 0;
  //   }
  //   if (ComElev > 90)
  //   {
  //     ComElev = 90;
  //   }

  //   ComputerWrite = "ANTAZ" + String(ComAzim) + " ANTEL" + String(ComElev);
  //   Serial.println(ComputerWrite);
  // }
  // }
}
