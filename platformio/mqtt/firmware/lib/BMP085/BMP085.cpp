#include "BMP085.h"
#include "I2CDevice.h"

Adafruit_BMP085::Adafruit_BMP085() { i2c_dev = nullptr; }

bool Adafruit_BMP085::begin(uint8_t mode, TwoWire *wire) {
  if (mode > BMP085_ULTRAHIGHRES)
    mode = BMP085_ULTRAHIGHRES;
  oversampling = mode;

  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(BMP085_I2CADDR, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  if (read8(0xD0) != 0x55)
    return false;

  /* read calibration data */
  ac1 = read16(BMP085_CAL_AC1);
  ac2 = read16(BMP085_CAL_AC2);
  ac3 = read16(BMP085_CAL_AC3);
  ac4 = read16(BMP085_CAL_AC4);
  ac5 = read16(BMP085_CAL_AC5);
  ac6 = read16(BMP085_CAL_AC6);

  b1 = read16(BMP085_CAL_B1);
  b2 = read16(BMP085_CAL_B2);

  mb = read16(BMP085_CAL_MB);
  mc = read16(BMP085_CAL_MC);
  md = read16(BMP085_CAL_MD);
#if (BMP085_DEBUG == 1)
  Serial.print("ac1 = ");
  Serial.println(ac1, DEC);
  Serial.print("ac2 = ");
  Serial.println(ac2, DEC);
  Serial.print("ac3 = ");
  Serial.println(ac3, DEC);
  Serial.print("ac4 = ");
  Serial.println(ac4, DEC);
  Serial.print("ac5 = ");
  Serial.println(ac5, DEC);
  Serial.print("ac6 = ");
  Serial.println(ac6, DEC);

  Serial.print("b1 = ");
  Serial.println(b1, DEC);
  Serial.print("b2 = ");
  Serial.println(b2, DEC);

  Serial.print("mb = ");
  Serial.println(mb, DEC);
  Serial.print("mc = ");
  Serial.println(mc, DEC);
  Serial.print("md = ");
  Serial.println(md, DEC);
#endif

  return true;
}

int32_t Adafruit_BMP085::computeB5(int32_t UT) {
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
  return X1 + X2;
}

uint16_t Adafruit_BMP085::readRawTemperature(void) {
  write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
#if BMP085_DEBUG == 1
  Serial.print("Raw temp: ");
  Serial.println(read16(BMP085_TEMPDATA));
#endif
  return read16(BMP085_TEMPDATA);
}

uint32_t Adafruit_BMP085::readRawPressure(void) {
  uint32_t raw;

  write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  if (oversampling == BMP085_ULTRALOWPOWER)
    delay(5);
  else if (oversampling == BMP085_STANDARD)
    delay(8);
  else if (oversampling == BMP085_HIGHRES)
    delay(14);
  else
    delay(26);

  raw = read16(BMP085_PRESSUREDATA);

  raw <<= 8;
  raw |= read8(BMP085_PRESSUREDATA + 2);
  raw >>= (8 - oversampling);

  /* this pull broke stuff, look at it later?
   if (oversampling==0) {
     raw <<= 8;
     raw |= read8(BMP085_PRESSUREDATA+2);
     raw >>= (8 - oversampling);
   }
  */

#if BMP085_DEBUG == 1
  Serial.print("Raw pressure: ");
  Serial.println(raw);
#endif
  return raw;
}

int32_t Adafruit_BMP085::readPressure(void) {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = readRawTemperature();
  UP = readRawPressure();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;
  oversampling = 0;
#endif

  B5 = computeB5(UT);

#if BMP085_DEBUG == 1
  Serial.print("X1 = ");
  Serial.println(X1);
  Serial.print("X2 = ");
  Serial.println(X2);
  Serial.print("B5 = ");
  Serial.println(B5);
#endif

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4;

#if BMP085_DEBUG == 1
  Serial.print("B6 = ");
  Serial.println(B6);
  Serial.print("X1 = ");
  Serial.println(X1);
  Serial.print("X2 = ");
  Serial.println(X2);
  Serial.print("B3 = ");
  Serial.println(B3);
#endif

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> oversampling);

#if BMP085_DEBUG == 1
  Serial.print("X1 = ");
  Serial.println(X1);
  Serial.print("X2 = ");
  Serial.println(X2);
  Serial.print("B4 = ");
  Serial.println(B4);
  Serial.print("B7 = ");
  Serial.println(B7);
#endif

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

#if BMP085_DEBUG == 1
  Serial.print("p = ");
  Serial.println(p);
  Serial.print("X1 = ");
  Serial.println(X1);
  Serial.print("X2 = ");
  Serial.println(X2);
#endif

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
#if BMP085_DEBUG == 1
  Serial.print("p = ");
  Serial.println(p);
#endif
  return p;
}

int32_t Adafruit_BMP085::readSealevelPressure(float altitude_meters) {
  float pressure = readPressure();
  return (int32_t)(pressure / pow(1.0 - altitude_meters / 44330, 5.255));
}

float Adafruit_BMP085::readTemperature(void) {
  int32_t UT, B5; // following ds convention
  float temp;

  UT = readRawTemperature();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
#endif

  B5 = computeB5(UT);
  temp = (B5 + 8) >> 4;
  temp /= 10;

  return temp;
}

float Adafruit_BMP085::readAltitude(float sealevelPressure) {
  float altitude;

  float pressure = readPressure();

  altitude = 44330 * (1.0 - pow(pressure / sealevelPressure, 0.1903));

  return altitude;
}

/*********************************************************************/

uint8_t Adafruit_BMP085::read8(uint8_t a) {
  uint8_t ret;

  // send 1 byte, reset i2c, read 1 byte
  i2c_dev->write_then_read(&a, 1, &ret, 1, true);

  return ret;
}

uint16_t Adafruit_BMP085::read16(uint8_t a) {
  uint8_t retbuf[2];
  uint16_t ret;

  // send 1 byte, reset i2c, read 2 bytes
  // we could typecast uint16_t as uint8_t array but would need to ensure proper
  // endianness
  i2c_dev->write_then_read(&a, 1, retbuf, 2, true);

  // write_then_read uses uint8_t array
  ret = retbuf[1] | (retbuf[0] << 8);

  return ret;
}

void Adafruit_BMP085::write8(uint8_t a, uint8_t d) {
  // send d prefixed with a (a d [stop])
  i2c_dev->write(&d, 1, true, &a, 1);
}
