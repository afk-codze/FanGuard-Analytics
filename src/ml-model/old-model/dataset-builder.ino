#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// I2C pins (change if your board differs)
#define ADXL345_SDA 41
#define ADXL345_SCL 42

// window length (samples)
#define WINDOW_SIZE 300
// conversion from m/s² to g
const float MPS2_TO_G = 1.0 / 9.80665;

// ADXL345 object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

// circular buffers for X, Y, Z
float bufX[WINDOW_SIZE];
float bufY[WINDOW_SIZE];
float bufZ[WINDOW_SIZE];
int  bufIndex = 0;
bool bufferFull = false;

// running sums of squares
float sumSqX = 0, sumSqY = 0, sumSqZ = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(1);

  // initialize I²C and sensor
  Wire.begin(ADXL345_SDA, ADXL345_SCL);
  if (!accel.begin()) {
    Serial.println("ERROR: ADXL345 not found!");
    while (1) delay(10);
  }
  accel.setRange(ADXL345_RANGE_2_G);
  accel.setDataRate(ADXL345_DATARATE_100_HZ);

  // zero out buffers
  for (int i = 0; i < WINDOW_SIZE; i++) {
    bufX[i] = bufY[i] = bufZ[i] = 0.0;
  }

  // CSV header
  Serial.println("rmse_x,rmse_y,rmse_z");
}

void loop() {
  sensors_event_t evt;
  accel.getEvent(&evt);

  // convert to g
  float gx = evt.acceleration.x * MPS2_TO_G;
  float gy = evt.acceleration.y * MPS2_TO_G;
  float gz = evt.acceleration.z * MPS2_TO_G;

  // update running sums of squares (remove old, add new)
  sumSqX += gx * gx - bufX[bufIndex] * bufX[bufIndex];
  sumSqY += gy * gy - bufY[bufIndex] * bufY[bufIndex];
  sumSqZ += gz * gz - bufZ[bufIndex] * bufZ[bufIndex];

  // store new sample in circular buffer
  bufX[bufIndex] = gx;
  bufY[bufIndex] = gy;
  bufZ[bufIndex] = gz;

  bufIndex++;
  if (bufIndex >= WINDOW_SIZE) {
    bufIndex = 0;
    bufferFull = true;
  }

  // once we have a full window, compute & print RMS
  if (bufferFull) {
    float rmse_x = sqrt(sumSqX / WINDOW_SIZE);
    float rmse_y = sqrt(sumSqY / WINDOW_SIZE);
    float rmse_z = sqrt(sumSqZ / WINDOW_SIZE);

    // CSV output
    Serial.print(rmse_x, 4); Serial.print(",");
    Serial.print(rmse_y, 4); Serial.print(",");
    Serial.println(rmse_z, 4);
  }

  delay(10);
}
