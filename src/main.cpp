#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <CubeSide.h>

#define REF_LOW -9.8
#define REF_RANGE 19.6

#define RAW_LOW_Z 7.4
#define RAW_HIGH_Z 12.7




// Дисплей
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Акселерометр
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

float calibrateAxis(float raw, float rawLow, float rawHigh){
  float rawRange = rawHigh - rawLow;

  return (((raw - rawLow) * REF_RANGE) / rawRange) + REF_LOW;
}

CubeSide determineSide(float x, float y, float z){
  float absX = fabs(x);
  float absY = fabs(z);
  float absZ = fabs(z);

  float maxValue = max(absX, max(absY, absZ));
  if(maxValue == absX){
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
  else{
    return CubeSide::UNKNOWN;
  }
  
  
  return CubeSide::BOTTOM;
}

void setup() {
  Wire.begin();
  
  // Инициализация дисплея
  display.begin(0x3C, false);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  accel.begin();
  accel.setRange(ADXL345_RANGE_2_G);
}

void loop() {
  // Чтение данных
  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Вывод на дисплей
  display.clearDisplay();
  display.setCursor(0, 0);
  
  float x = calibrateAxis(event.acceleration.x, -10.2, 10.2);
  float y = calibrateAxis(event.acceleration.y, -10.4, 10.01);
  float z = calibrateAxis(event.acceleration.z, -7.5, 12.7);

  display.print("X: ");
  display.println(x, 2);
  
  display.print("Y: ");
  display.println(y, 2);
  
  display.print("Z: ");
  display.println(z, 2);

  display.print("vector length: ");
  display.println(sqrt(x*x + y*y + z*z));

  display.print("raw side:");
  display.println("" + (int)determineSide(x, y, z));
  
  display.display();
  delay(100);
}

