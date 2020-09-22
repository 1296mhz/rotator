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

// Реле
#define PIN_CCW 31   // Поворот против часовой стрелки
#define PIN_CW 29    // Поворот по часовой стрелки
#define PIN_SPEED 30 // Скорость поворота

#define PIN_UP 27   // Актуатор вверх
#define PIN_DOWN 28 // Актуатор вниз

//Кнопки
// #define BTN_CW 21      // Реле по часовой стрелке
// #define BTN_CCW 22     // Реле против часовой стрелки
#define BTN_OPERATE 19 // Включение отключение
#define BTN_AZ_EL 24   // Переключение между - азимутом и эливацией
#define BTN_MODE 25    // Кнопка вид работы MANUAL/PORT/SETTING
#define BTN_CCW 15     // AZ CCW, EL DOWN
#define BTN_CW 17      // AZ CW, EL UP

#define ENC_BUTTON_PIN 4 // Вклюбчение отключени по умолчанию ротатор в режиме OFF
#define PIN_CLK 2        // Энкодер
#define PIN_DT 3         // Энкодер

#define LED_OPER_PIN 26 // Светодиод OPER ON/OFF

EthernetUDP udp;

// int lastPos, newPos = 180;
uint16_t currentTime, loopTime;
byte encoder, encoderPrev;
// byte elEncoder, elEncoderPrev;
int buttonPressTime = 0;

bool clearFlag = false; //Флаг очистки эерана на каждом скрине

bool azMove = false; // Флаг для включения вращения по азимуту

bool elMove = false; // Флаг для включения вращения по элевации

bool operFlag = false;
// Для азимута
float azAngleSensor = 0.0; // С сенcора угла азимута

bool offsetFlagAz = false; // Флаг включения смещения AZ
bool offsetFlagEl = false; // Флаг включения смещения EL

int offsetAz = 0;   // Хранение смещения для азимута
int azAngle = 0;    // Угол азимута
int azTarget = 180; // Цель для поворота
int azDelta = 0;
int _azAngle = 0;
float elAngleSensor = 0.0; // С сенcора угла элевации
                           // Хранение смещения для элевации
int offsetEl = 0;
int elAngle = 0;
int elTarget = 0;
int elDelta = 0;
int _elAngle = 0;
byte deltaDirectionAz = 1;
byte deltaDirectionEl = 1;

byte switchAzEl = 1; //Флаг переключение азимут/элевация

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

//Секция для работы с com-портом
String PortRead;
int azPortTarget = 0;
int elPortTarget = 0;

struct SettingsStruct
{
  bool offsetFlagAz;
  bool offsetFlagEl;
  byte deltaDirectionAz;
  int azDelta;
  int offsetAz;
  byte deltaDirectionEl;
  int offsetEl;
  int elDelta;
};
SettingsStruct newSettingsStruct;
// LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C lcd(0x27, 20, 4);
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

void btnOperate()
{
  if (btn(BTN_OPERATE) == 0)
  {
    if (!operFlag)
    {
      operFlag = true;
      digitalWrite(LED_OPER_PIN, HIGH);
    }
    else
    {
      operFlag = false;
      digitalWrite(LED_OPER_PIN, LOW);
    }
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
    digitalWrite(PIN_SPEED, HIGH);
  }

  if (!spd)
  {
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
  // delay(500);
 // digitalWrite(PIN_UP, LOW);
  _elAngle++;
 // delay(500);
 // digitalWrite(PIN_UP, HIGH);
}

void ground()
{
  // dummy func
  Serial.println("To ground");
  //delay(500);
//  digitalWrite(PIN_DOWN, LOW);
  _elAngle--;
//  delay(500);
//  digitalWrite(PIN_UP, HIGH);
}

uint8_t btn(int KEY)
{
  if (digitalRead(KEY) == 1)
  {
    last_millis = millis();
    return 0;
  }
  delay(30);
  while (digitalRead(KEY) == 0)
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

uint8_t btnEnc()
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
    deltaDirectionAz = 1;
  }

  if (sensorAz < realAz)
  {
    deltaDirectionAz = 3;
    return abs(realAz - sensorAz);
  }

  if (sensorAz > realAz)
  {
    deltaDirectionAz = 2;
    return 359 - abs(sensorAz - realAz) + 1;
  }
}

int elDeltaGen(int sensorEl, int realEl)
{
  if (sensorEl == realEl)
  {
    deltaDirectionEl = 1;
  }

  if (sensorEl < realEl)
  {
    deltaDirectionEl = 3;
    return abs(realEl - sensorEl);
  }

  if (sensorEl > realEl)
  {
    deltaDirectionEl = 2;
    return 180 - abs(sensorEl - realEl) + 1;
  }
}

int sensorAzToRealAz(int sensorAz, int delta)
{
  if (deltaDirectionAz == 1)
  {
    return sensorAz;
  }

  if (deltaDirectionAz == 2)
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

  if (deltaDirectionAz == 3)
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

int sensorElToRealEl(int sensorEl, int delta)
{
  if (deltaDirectionEl == 1)
  {
    return sensorEl;
  }

  if (deltaDirectionEl == 2)
  {
    if (sensorEl + delta > 180)
    {
      return (sensorEl + delta - 180) - 1;
    }
    if (sensorEl + delta > 0)
    {
      return sensorEl + delta;
    }
  }

  if (deltaDirectionEl == 3)
  {
    if (sensorEl - delta > 0)
    {
      return sensorEl - delta;
    }
    if (sensorEl - delta < 0)
    {
      return 180 - abs(sensorEl - delta);
    }

    return sensorEl - delta;
  }
}

int offsetFilterAz(bool offsetFlagAz, int sensorAz)
{
  if (offsetFlagAz)
  {
    return sensorAzToRealAz(sensorAz, azDelta);
  }
  else
  {
    return sensorAz;
  }
}

int offsetFilterEl(bool offsetFlagEl, int sensorEl)
{
  if (offsetFlagEl)
  {
    return sensorElToRealEl(sensorEl, elDelta);
  }
  else
  {
    return sensorEl;
  }
}

void offsetSwitchIndicator()
{
  if (offsetFlagAz)
  {
    lcd.setCursor(15, 0);
    lcd.print("a");
  }
  else
  {
    lcd.setCursor(15, 0);
    lcd.print(" ");
  }

  if (offsetFlagEl)
  {
    lcd.setCursor(15, 1);
    lcd.print("e");
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

void SwitchOffsetAz()
{
  if (offsetFlagAz)
  {
    offsetFlagAz = false;
    if (newSettingsStruct.offsetFlagAz != false)
    {
      newSettingsStruct.offsetFlagAz = false;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
    }
  }
  else
  {
    offsetFlagAz = true;
    if (newSettingsStruct.offsetFlagAz != true)
    {
      newSettingsStruct.offsetFlagAz = true;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
    }
  }
}

void SwitchOffsetEl()
{
  if (offsetFlagEl)
  {
    offsetFlagEl = false;
    if (newSettingsStruct.offsetFlagEl != false)
    {
      newSettingsStruct.offsetFlagEl = false;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
    }
  }
  else
  {
    offsetFlagEl = true;
    if (newSettingsStruct.offsetFlagEl != true)
    {
      newSettingsStruct.offsetFlagEl = true;
      eeprom_write_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
      eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
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

String AzElString(int someIntVolue, bool el)
{
  if (someIntVolue < 0)
  {
    return "" + String(someIntVolue);
  }
  if (someIntVolue < 10)
  {
    return "  " + String(someIntVolue);
  }

  if (someIntVolue < 100)
  {
    return " " + String(someIntVolue);
  }

  if (someIntVolue >= 100)
  {
    return String(someIntVolue);
  }
}

//TGT, STEP, AZ/AL - true/false
int RotateEncoder(int *tgt, byte STEP, bool az_el)
{

  currentTime = millis();
  if (currentTime >= (loopTime + 15))
  {
    encoder = digitalRead(PIN_CLK);
    if ((!encoder) && (encoderPrev))
    {
      if (digitalRead(PIN_DT))
      {
        if (*tgt + STEP <= az_el ? 359 : 90)
          *tgt += STEP;
      }
      else
      {
        if (*tgt - STEP >= 0)
          *tgt -= STEP;
      }
    }
    encoderPrev = encoder;
  }

  loopTime = currentTime;
}

void setup()
{
  attachInterrupt(4, btnOperate, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.begin(20, 4);
  lcd.print("*R8CDF*");
  speed(true);

  if (!operFlag)
  {
    digitalWrite(LED_OPER_PIN, LOW);
  }
  else
  {
    digitalWrite(LED_OPER_PIN, HIGH);
  }

  //Magic number 137
  if (eeprom_read_byte(1023) != 137)
  {
    SettingsStruct settingsStruct;
    settingsStruct.offsetFlagAz = false;
    settingsStruct.deltaDirectionAz = 1;
    settingsStruct.azDelta = 0;
    settingsStruct.offsetAz = 0;
    settingsStruct.offsetFlagEl = false;
    settingsStruct.deltaDirectionEl = 1;
    settingsStruct.elDelta = 0;
    settingsStruct.offsetEl = 0;
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

  delay(1000);
  lcd.clear();
  clearFlag = true;
  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);
  pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);

  pinMode(PIN_CCW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(PIN_SPEED, OUTPUT);

  pinMode(PIN_UP, OUTPUT);
  pinMode(PIN_DOWN, OUTPUT);

  pinMode(LED_OPER_PIN, OUTPUT);

  pinMode(BTN_OPERATE, INPUT_PULLUP);
  pinMode(BTN_AZ_EL, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);

  pinMode(BTN_CW, INPUT_PULLUP);
  pinMode(BTN_CCW, INPUT_PULLUP);
  digitalWrite(PIN_CW, HIGH);
  digitalWrite(PIN_CCW, HIGH);
  digitalWrite(PIN_SPEED, HIGH);
  digitalWrite(PIN_UP, HIGH);
  digitalWrite(PIN_DOWN, HIGH);

  last_millis = millis();
  Serial.println('R8CDF RoToR');
  eeprom_read_block((void *)&newSettingsStruct, 0, sizeof(newSettingsStruct));
  offsetFlagAz = newSettingsStruct.offsetFlagAz;
  offsetFlagEl = newSettingsStruct.offsetFlagEl;
  deltaDirectionAz = newSettingsStruct.deltaDirectionAz;
  azDelta = newSettingsStruct.azDelta;
  offsetAz = newSettingsStruct.offsetAz;
  deltaDirectionEl = newSettingsStruct.deltaDirectionEl;
  elDelta = newSettingsStruct.elDelta;
  offsetEl = newSettingsStruct.offsetEl;
  int _elAngle = 0;
#ifdef NETWORK
  getNetworkSensor();
#endif
  _azAngle = int(round(azAngleSensor / 1024.0 * 360));
  // _elAngle = int(round(elAngleSensor / 1024.0 * 180));
azAngle = offsetFilterAz(false, _azAngle);

  // azAngle = offsetFilterAz(offsetFlagAz, _azAngle);
  elAngle = offsetFilterEl(offsetFlagEl, _elAngle);
}

void loop()
{
#ifdef NETWORK
  getNetworkSensor();
#endif

  _azAngle = int(round(azAngleSensor / 1024.0 * 360));
  // _elAngle = int(round(elAngleSensor / 1024.0 * 180));
  azAngle = offsetFilterAz(azAngleSensor, _azAngle);

  // azAngle = offsetFilterAz(offsetFlagAz, _azAngle);
  elAngle = offsetFilterEl(offsetFlagEl, _elAngle);

  if (btn(BTN_MODE) == 1)
  {
    //  if (azMove != true && elMove != true)
    //{
    AppScreen();
    // }
  }

  // MANUAL
  if (appScreen == 0)
  {
    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;
      getSpeed();
      screenManualPort();
      lcd.setCursor(0, 0);
      lcd.print(" MAN");
    }

    CursorAzEl(true, 4, 0, true, 9, 0);
    offsetSwitchIndicator();
    if (operFlag)
    {
      if (switchAzEl == 1)
      {

        RotateEncoder(&azTarget, AZ_STEP, true);

        if (btnEnc() == 1)
        {
          azMove = true;
          strAzTarget = AzElString(azTarget, false);
        }

        if (btn(BTN_AZ_EL) == 1)
        {
          Az_El();
        }

        // Управление кнопками
        if (digitalRead(BTN_CW) == LOW)
        {
          digitalWrite(PIN_CW, LOW);
        }
        else
        {
          digitalWrite(PIN_CW, HIGH);
        }

        if (digitalRead(BTN_CCW) == LOW)
        {
          digitalWrite(PIN_CCW, LOW);
        }
        else
        {
          digitalWrite(PIN_CCW, HIGH);
        }
      }

      if (switchAzEl == 2)
      {
        RotateEncoder(&elTarget, EL_STEP, false);
        loopTime = currentTime;

        if (btnEnc() == 1)
        {
          elMove = true;
          strElTarget = AzElString(elTarget, true);
        }

        if (btn(BTN_AZ_EL) == 1)
        {
          Az_El();
        }
        // Управление кнопками
        if (digitalRead(BTN_CW) == LOW)
        {
          digitalWrite(PIN_UP, LOW);
        }
        else
        {
          digitalWrite(PIN_UP, HIGH);
        }

        if (digitalRead(BTN_CCW) == LOW)
        {
          digitalWrite(PIN_DOWN, LOW);
        }
        else
        {
          digitalWrite(PIN_DOWN, HIGH);
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
        }
      }
    }

    // Отображение азимута текущего положения антенны
    lcd.setCursor(6, 1);
    lcd.print(strAzAngle);
    // Отображение азимута текущей цели антенны
    lcd.setCursor(6, 0);
    lcd.print(strAzTarget);

    // Отображение данных с датчика
    strAzAngle = AzElString(azAngle, false);

    // Отображение цели
    strAzTarget = AzElString(azTarget, false);

    // Отображение элевация с датчика
    lcd.setCursor(11, 1);
    lcd.print(strElAngle);

    strElAngle = AzElString(elAngle, true);

    // Отображение цели элевации
    lcd.setCursor(11, 0);
    lcd.print(strElTarget);

    strElTarget = AzElString(elTarget, true);
  }

  //PC
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

    if (btn(BTN_MODE) == 1)
    {
      if (azMove != true && elMove != true)
      {
        AppScreen();
      }
    }

    offsetSwitchIndicator();
    if (operFlag)
    {

      if (Serial.available())
      {

        PortRead = Serial.readString();
        int buffer_len = PortRead.length() + 1;
        char port_char_array[buffer_len];
        char az[7], el[7];

        sscanf(PortRead.c_str(), "%s %s", &az, &el);
        azPortTarget = int(round(atof(az)));
        elPortTarget = int(round(atof(el)));

        if (azPortTarget >= 0 && azPortTarget <= 359)
        {
          azMove = true;
        }

        if ((elPortTarget >= 0) && (elPortTarget <= 180))
        {
          elMove = true;
        }
      }

      if (azMove)
      {
        if (azPortTarget - azAngle >= 1)
        {
          if (abs(azPortTarget - azAngle) > 20)
          {
            speed(true);
          }
          if (abs(azPortTarget - azAngle) < 20)
          {
            speed(false);
          }
          cw();
        }

        if (azAngle - azPortTarget >= 1)
        {
          if (abs(azAngle - azPortTarget) > 20)
          {
            speed(true);
          }
          if (abs(azAngle - azPortTarget) < 20)
          {
            speed(false);
          }
          ccw();
        }

        if (azPortTarget == azAngle)
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
        if (elPortTarget - elAngle >= 1)
        {
          sky();
        }

        if (elAngle - elPortTarget >= 1)
        {
          ground();
        }

        if (elPortTarget == elAngle)
        {
          elMove = false;
          digitalWrite(PIN_UP, HIGH);
          digitalWrite(PIN_DOWN, HIGH);
        }
      }
    }

    // Отображение данных с датчика
    strAzAngle = AzElString(azAngle, false);

    // Отображение азимута
    lcd.setCursor(6, 1);
    lcd.print(strAzAngle);

    // Отображение цели
    strAzTargetPort = AzElString(azPortTarget, false);

    lcd.setCursor(6, 0);
    lcd.print(strAzTargetPort);

    strElAngle = AzElString(elAngle, true);

    // Отображение элевация
    lcd.setCursor(11, 1);
    lcd.print(strElAngle);

    strElTargetPort = AzElString(elPortTarget, true);

    lcd.setCursor(11, 0);
    lcd.print(strElTargetPort);
  }

  //SETTINGS
  if (appScreen == 2)
  {
    if (clearFlag)
    {
      clearDisplay();
      clearFlag = false;
      getSpeed();
      lcd.setCursor(0, 0);
      lcd.print("OSET AZ ");
      lcd.setCursor(0, 1);
      lcd.print("OSET EL ");
    }
    CursorAzEl(true, 4, 0, true, 4, 1);

    if (btn(BTN_AZ_EL) == 1)
    {
      Az_El();
    }

    if (btn(BTN_MODE) == 1)
    {
      if (azMove != true && elMove != true)
      {
        AppScreen();
      }
    }

    offsetSwitchIndicator();

    if (switchAzEl == 1)
    {
      RotateEncoder(&offsetAz, AZ_STEP, true);
      azDelta = azDeltaGen(_azAngle, offsetAz);

      if (btn(BTN_CW) == 1 && btn(BTN_CCW) == 1)
      {
        SwitchOffsetAz();
      }
    }

    if (switchAzEl == 2)
    {
      RotateEncoder(&offsetEl, EL_STEP, false);
      elDelta = elDeltaGen(_elAngle, offsetEl);
      if (btn(BTN_CW) == 1 && btn(BTN_CCW) == 1)
      {
        SwitchOffsetEl();
      }
    }

    // Отображение данных с датчика азимута
    strAzAngle = AzElString(_azAngle, false);
    // Отображение смещения азимута
    strAzOffset = AzElString(offsetAz, false);
    // Отображение азимута
    lcd.setCursor(8, 0);
    lcd.print(strAzAngle);

    lcd.setCursor(12, 0);
    lcd.print(strAzOffset);

    // Отображение данных с датчика азимута
    strElAngle = AzElString(_elAngle, true);
    // Отображение смещения элевации
    strElOffset = AzElString(offsetEl, true);

    // Отображение элевации
    lcd.setCursor(8, 1);
    lcd.print(strElAngle);

    lcd.setCursor(12, 1);
    lcd.print(strElOffset);
  }
}
