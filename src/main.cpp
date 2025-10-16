#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <CubeSide.h>
#include <MedianN.cpp>
#include <U8g2lib.h>

#define REF_LOW -9.8
#define REF_RANGE 19.6

#define RAW_LOW_Z 7.4
#define RAW_HIGH_Z 12.7

#define MEASURE_TOLERANCE 0.4

MedianN<float, 10> medianFilterX;
MedianN<float, 10> medianFilterY;
MedianN<float, 10> medianFilterZ;

unsigned long stabilityStartTime = 0;
unsigned long currentStabilityDuration = 0;

// Дисплей
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// Акселерометр
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

float calibrateAxis(float raw, float rawLow, float rawHigh)
{
  float rawRange = rawHigh - rawLow;

  return (((raw - rawLow) * REF_RANGE) / rawRange) + REF_LOW;
}

CubeSide determineSide(float x, float y, float z)
{
  float absX = fabs(x);
  float absY = fabs(z);
  float absZ = fabs(z);

  float maxValue = max(absX, max(absY, absZ));
  if (maxValue == absX)
  {
    return x > 0 ? CubeSide::RIGHT : CubeSide ::LEFT;
  }
  else if (maxValue == absY)
  {
    return y > 0 ? CubeSide::REAR : CubeSide ::FRONT;
  }
  else if (maxValue == absZ)
  {
    return z > 0 ? CubeSide::BOTTOM : CubeSide ::TOP;
  }
  else
  {
    return CubeSide::UNKNOWN;
  }

  return CubeSide::BOTTOM;
}

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  // Инициализация дисплея
  display.begin();
  display.setFont(u8g2_font_6x10_tf); // Указываем шрифт
  display.setFontRefHeightExtendedText();
  display.setDrawColor(1);
  display.setFontPosTop();

  accel.begin();
  accel.setRange(ADXL345_RANGE_2_G);

  sensors_event_t event;
  accel.getEvent(&event);
  float x = calibrateAxis(event.acceleration.x, -10.2, 10.2);
  float y = calibrateAxis(event.acceleration.y, -10.4, 10.01);
  float z = calibrateAxis(event.acceleration.z, -7.5, 12.7);
  medianFilterX.init(x);
  medianFilterY.init(y);
  medianFilterZ.init(z);
}

bool isMoving(float vectorLength)
{
  float low = 9.8 - MEASURE_TOLERANCE;
  float high = 9.8 + MEASURE_TOLERANCE;

  return !(vectorLength > low && vectorLength < high);
}

void updateStabilityTime(bool isMoving)
{
  if (isMoving)
  {
    stabilityStartTime = millis();
    currentStabilityDuration = 0;
  }
  else
  {
    currentStabilityDuration = millis() - stabilityStartTime;
  }
}

void loop()
{
  // Чтение данных
  sensors_event_t event;
  accel.getEvent(&event);

  // Вывод на дисплей
  display.clearBuffer();
  display.setCursor(0, 0);

  float x = calibrateAxis(event.acceleration.x, -10.2, 10.2);
  float y = calibrateAxis(event.acceleration.y, -10.4, 10.01);
  float z = calibrateAxis(event.acceleration.z, -7.5, 12.7);
  float vectorLength = sqrt(x * x + y * y + z * z);

  display.print("X: ");
  display.print(medianFilterX.filter(x), 2);

  display.setCursor(0, 10);
  display.print("Y: ");
  display.print(medianFilterY.filter(y), 2);

  display.setCursor(0, 20);
  display.print("Z: ");
  display.print(medianFilterZ.filter(z), 2);

  display.setCursor(0, 30);
  display.print("vector length: ");
  display.print(vectorLength);

  updateStabilityTime(isMoving(vectorLength));

  display.setCursor(0, 40);
  display.print("stab (mm:ss): ");
  display.setCursor(0, 50);

  uint16_t stabilitySeconds = currentStabilityDuration / 1000;
  uint16_t seconds = stabilitySeconds % 60;
  uint16_t minutes = (stabilitySeconds - seconds) / 60;


  if(minutes < 10){
    display.print('0');
  }
  display.print(minutes);
  display.print(" : ");
  if(seconds < 10){
    display.print('0');
  }
  display.print(seconds);

  display.sendBuffer();
  delay(10);
}
