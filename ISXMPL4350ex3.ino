/*
 * IS4350-M1 - Arduino Uno Temperature Sensor Example
 * ---------------------------------------------------
 * A Modbus TCP/IP temperature sensor in a few lines:
 *   - Reads a TMP36 analog temperature sensor once per second
 *   - Writes the temperature to Holding Register 0 (HOLD_0)
 *   - Any Modbus TCP/IP client (PLC, SCADA, PC) reads it over Ethernet
 *
 * Wiring:
 *   TMP36 +Vs              -> Arduino 5V
 *   TMP36 Vout             -> Arduino A0
 *   TMP36 GND              -> Arduino GND
 *   (TMP36 flat side facing you, legs down: +Vs, Vout, GND from left to right)
 *
 *   IS4350-M1 3V3 (7, 35)  -> external 3.3 V supply (not the Uno 3.3V pin, 50 mA max)
 *   IS4350-M1 GND          -> Arduino GND and 3.3 V supply GND
 *   IS4350-M1 SDA (2)      -> Arduino A4, 4.7 kOhm pull-up to 5 V
 *   IS4350-M1 SCL (1)      -> Arduino A5, 4.7 kOhm pull-up to 5 V
 *   IS4350-M1 SPD (3)      -> GND (I2C at 100 kHz)
 *   IS4350-M1 ADR (4)      -> GND (I2C address 24)
 *   IS4350-M1 RJ45         -> network with a DHCP server
 *   All other module pins  -> leave floating
 *
 * Reading the temperature from a Modbus client:
 *   IP address : printed on the Serial Monitor (9600 baud)
 *   Port       : 502, Unit ID 255
 *   Function   : 03 - Read Holding Registers, address 0 (HOLD_0,
 *                shown as 40001 in tools that use 1-based numbering)
 *   Format     : signed 16-bit integer, in tenths of a degree
 *                (235 = 23.5 °C, -52 = -5.2 °C)
 */

#include <Wire.h>

const uint8_t  IS4350_ADDR   = 24;     // ADR pin tied to GND
const uint16_t REG_HOLD_0    = 0;      // temperature, in 0.1 °C
const uint16_t REG_IP        = 65501;  // 65501..65504
const uint16_t REG_GO_ONLINE = 65513;
const uint16_t REG_STATUS    = 65514;
const uint16_t REG_CHIP_ID   = 65522;  // always 150

const uint8_t  SENSOR_PIN    = A0;
const long     ADC_REF_MV    = 5000;   // 5000 on 5 V boards (Uno), 3300 on 3.3 V boards

// Write one 16-bit register. Address and data are sent MSB first.
void is4350Write(uint16_t reg, uint16_t value) {
  Wire.beginTransmission(IS4350_ADDR);
  Wire.write((uint8_t)(reg >> 8));
  Wire.write((uint8_t)(reg & 0xFF));
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
  delay(5);                            // a few ms between I2C operations
}

// Read one 16-bit register. Address and data are sent MSB first.
uint16_t is4350Read(uint16_t reg) {
  Wire.beginTransmission(IS4350_ADDR);
  Wire.write((uint8_t)(reg >> 8));
  Wire.write((uint8_t)(reg & 0xFF));
  Wire.endTransmission(false);         // repeated START
  Wire.requestFrom(IS4350_ADDR, (uint8_t)2);
  uint16_t value = 0;
  if (Wire.available() >= 2) {
    value = (uint16_t)Wire.read() << 8;  // MSB
    value |= Wire.read();                // LSB
  }
  delay(5);
  return value;
}

// TMP36: 500 mV at 0 °C and 10 mV/°C, so (mV - 500) is the temperature in 0.1 °C.
int16_t readTemperature() {
  long mV = analogRead(SENSOR_PIN) * ADC_REF_MV / 1024;
  return (int16_t)(mV - 500);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();                        // 100 kHz, matches SPD tied to GND
  delay(1000);                         // let the IS4350 start

  if (is4350Read(REG_CHIP_ID) != 150) {
    Serial.println("IS4350-M1 not detected. Check wiring, pull-ups, SPD and ADR.");
    while (1);
  }

  is4350Write(REG_HOLD_0, (uint16_t)readTemperature());  // valid data before going online
  is4350Write(REG_GO_ONLINE, 1);                         // start the Modbus server

  uint8_t status;
  while ((status = is4350Read(REG_STATUS) & 0xFF) != 5) {  // 5 = ready and listening
    Serial.print("Status: ");
    Serial.println(status);
    delay(500);
  }

  Serial.print("Sensor online. IP: ");
  for (uint8_t i = 0; i < 4; i++) {
    Serial.print(is4350Read(REG_IP + i));
    if (i < 3) Serial.print(".");
  }
  Serial.println(", port 502");
}

void loop() {
  int16_t temperature = readTemperature();             // in 0.1 °C
  is4350Write(REG_HOLD_0, (uint16_t)temperature);      // negative values as two's complement

  Serial.print("Temperature: ");
  Serial.print(temperature / 10.0, 1);
  Serial.println(" C");
  delay(1000);
}
