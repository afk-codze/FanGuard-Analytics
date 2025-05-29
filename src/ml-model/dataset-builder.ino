/*
  DatasetSampler_1kHz_stream.ino
  ──────────────────────────────
  Continuous CSV stream:
    idx , ts_ms , x_rms_g , y_rms_g , z_rms_g , power_mW
*/
#include <Arduino.h>
#include <Wire.h>
#include <MPU6500_WE.h>
#include <Adafruit_INA219.h>

/* ---------------- user parameters (match production) --------------- */
#define SAMPLE_RATE     1000          // raw Hz (1 kHz)
#define NUM_AVG_ACC     20            // raw points per feature row
#define ALPHA           0.10f         // INA low-pass factor
#define SERIAL_BAUD     115200
#define I2C_SDA         41
#define I2C_SCL         42
#define MPU6500_ADDR    0x68
/* ------------------------------------------------------------------- */

/* xyzFloat is already defined inside the *_WE libraries */
TwoWire         I2CBus = TwoWire(0);
MPU6500_WE      mpu6500(&I2CBus, MPU6500_ADDR);
Adafruit_INA219 ina219;

/* INA219 LPF state */
static bool  first_lp = true;
static float power_lp = 0;

/* timing helpers */
const uint32_t RAW_INTERVAL_US = 1'000'000 / SAMPLE_RATE;   // 1 000 µs

/* ------------------------------------------------------------------- */
void setup()
{
  Serial.begin(SERIAL_BAUD);
  // give the host up to 2 s to open the port, then continue anyway
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) { }

  Serial.println("\nidx,ts_ms,x_rms_g,y_rms_g,z_rms_g,power_mW");

  I2CBus.begin(I2C_SDA, I2C_SCL, 400000);

  /* MPU-6500: identical settings to your production firmware */
  if (!mpu6500.init()) { Serial.println("MPU6500 missing"); while (1) {} }
  mpu6500.enableAccDLPF(true);
  mpu6500.setAccRange(MPU6500_ACC_RANGE_2G);
  mpu6500.setAccDLPF(MPU6500_DLPF_2);

  /* INA219 */
  if (!ina219.begin(&I2CBus)) { Serial.println("INA219 missing"); while (1) {} }
  ina219.setCalibration_16V_400mA();
}

/* acquire one 20-sample slice → return synchronous RMS & mean values */
void get_feature_row(float &xr, float &yr, float &zr, float &power_lp_out,
                     uint32_t &ts_ms)
{
  float sx2 = 0, sy2 = 0, sz2 = 0, sumP = 0;
  uint32_t next_tick = micros();
  ts_ms = millis();                      // timestamp at start of slice

  for (int i = 0; i < NUM_AVG_ACC; ++i)
  {
    while ((int32_t)(micros() - next_tick) < 0) { /* busy-wait to 1 kHz */ }
    next_tick += RAW_INTERVAL_US;

    xyzFloat a = mpu6500.getGValues();
    float Pw   = ina219.getPower_mW();

    sx2  += a.x * a.x;
    sy2  += a.y * a.y;
    sz2  += a.z * a.z;
    sumP += Pw;
  }

  xr = sqrtf(sx2 / NUM_AVG_ACC);
  yr = sqrtf(sy2 / NUM_AVG_ACC);
  zr = sqrtf(sz2 / NUM_AVG_ACC);

  float p_mean = sumP / NUM_AVG_ACC;
  if (first_lp) { power_lp = p_mean; first_lp = false; }
  else          { power_lp = ALPHA * p_mean + (1 - ALPHA) * power_lp; }

  power_lp_out = power_lp;
}

/* ------------------------------------------------------------------- */
void loop()
{
  static uint32_t idx = 0;
  float xr, yr, zr, p;
  uint32_t ts;

  get_feature_row(xr, yr, zr, p, ts);

  Serial.printf("%lu,%lu,%.6f,%.6f,%.6f,%.6f\n",
                idx++,        // monotonically increasing index
                ts,           // ms timestamp at start of slice
                xr, yr, zr,   // RMS acceleration
                p);           // filtered power
}
