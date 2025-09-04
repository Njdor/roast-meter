// VERSION 1.0.1-beta

#include <U8g2lib.h>
#include <Wire.h>

#include "MAX30105.h"

#ifdef ARDUINO_XIAO_ESP32S3
  const int I2C_SDA = D4; //can be any pin on ESP32s3
  const int I2C_SCL = D5; //can be any pin on ESP32s3
//#elif 
#elif ESP32S3_SUPERMINI
  const int I2C_SDA = 5; //can be any pin on ESP32s3
  const int I2C_SCL = 6; //can be any pin on ESP32s3

#else
  const int I2C_SDA = 5; //Set to the pins for your MCU (if not using build flags)
  const int I2C_SCL = 6; //Set to the pins for your MCU (if not using build flags)
#endif

#define PIN_RESET 9
#define DC_JUMPER 0  //probably zero address, not 1 address

MAX30105 particleSensor;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, I2C_SCL, I2C_SDA, U8X8_PIN_NONE);
long unblockedValue;  // Average IR at power up


void displayMeasurement(int rLevel) {
  u8g2.clearBuffer();
  
  int calibratedReading = (rLevel);

  if (rLevel == 0) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 40, "Please Load Sample!");
    u8g2.sendBuffer();
    return;
  }
  char buf[10];
  itoa(calibratedReading, buf, 10);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(40,35, buf);

  Serial.println("real:" + String(rLevel));
  Serial.println("agtron:" + String(calibratedReading));
  Serial.println("===========================");

  u8g2.sendBuffer();
}

int f(int x) {
  int intersectionPoint = 117;
  float deviation = 0.165;

  return round(x - (intersectionPoint - x) * deviation);
}

void setup() {
  Serial.begin(9600);

  Wire.begin(I2C_SDA, I2C_SCL);
  u8g2.begin();

  delay(100);  // Delay 100 ms

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false)  // Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }

  // The variable below calibrates the LED output on your hardware.
  byte ledBrightness = 135;

  byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, --32--
  byte ledMode = 2;        // Options: 1 = Red only, --2 = Red + IR--, 3 = Red + IR + Green
  int sampleRate = 50;     // Options: 50, 100, 200, 400, 800, 1000, 1600, --3200--
  int pulseWidth = 411;    // Options: 69, 118, 215, --411--
  int adcRange = 16384;    // Options: 2048, --4096--, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);  // Configure sensor with these settings

  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeGreen(0);

  particleSensor.disableSlots();
  particleSensor.enableSlot(2, 0x02);  // Enable only SLOT_IR_LED = 0x02

  // Update to ignore readings under 30.000
  unblockedValue = 30000;
}

void loop() {
  int rLevel = particleSensor.getIR();
  long currentDelta = rLevel - unblockedValue;
  Serial.println(rLevel);
  if (currentDelta > (long)100) {
    displayMeasurement(rLevel / 1000);
  } else {
    displayMeasurement(0);
  }
  delay(100);
}