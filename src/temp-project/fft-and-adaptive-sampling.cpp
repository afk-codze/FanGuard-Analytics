#include <Arduino.h>
#include <ArduinoFFT.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <MPU6500_WE.h>
#include <shared-defs.h>

// Number of baseline windows
#define BASELINE_WINDOW_SIZE 10
// Window size for data collection (number of samples)
#define WINDOW_SIZE     10
// Threshold multiplier for anomaly detection on each axis (adjust based on testing)
#define ANOMALY_THRESHOLD_X 1.5
#define ANOMALY_THRESHOLD_Y 1.5
#define ANOMALY_THRESHOLD_Z 1.5

// Constants for FFT
#define NOISE_THRESHOLD 5
#define NYQUIST_MULTIPLIER 2.5f // Safety factor for Nyquist rate
#define INIT_SAMPLE_RATE 1024 // Initial sampling rate (Hz)
#define SECONDS_OF_AVG 5 // Number of seconds for rolling average

// FFT phase flags
volatile RTC_DATA_ATTR bool fft_init_complete = false;

// Current system sampling frequency (Hz)
RTC_DATA_ATTR int g_sampling_frequency = INIT_SAMPLE_RATE;

float samples_real[INIT_SAMPLE_RATE];
float samples_imag[INIT_SAMPLE_RATE];

RTC_DATA_ATTR float window_rms[3][WINDOW_SIZE];
RTC_DATA_ATTR float baseline[3][BASELINE_WINDOW_SIZE];
int g_window_size = WINDOW_SIZE; // Window size for RMS calculation

RTC_DATA_ATTR int index_rms = 0;
RTC_DATA_ATTR float baseline_rms[3] = {0};
RTC_DATA_ATTR bool baseline_established = false;
RTC_DATA_ATTR int num_of_samples = 0;



// ArduinoFFT instance configured with buffers
ArduinoFFT<float> FFT = ArduinoFFT<float>(samples_real, samples_imag, INIT_SAMPLE_RATE, INIT_SAMPLE_RATE);

void send_rms(float* avg) {
  // Send to queue
}

void send_anomaly() {
  // Trigger alert
}

void read_sample(float* x, float* y, float* z) {
  xyzFloat values = myMPU6500.getGValues();
  *x = values.x;
  *y = values.y;
  *z = values.z;
}

float calculateRMS(float* data, int size) {
  float sumSquares = 0.0;
  for (int i = 0; i < size; i++) {
    sumSquares += data[i] * data[i];
  }
  return sqrt(sumSquares / size);
}

void light_sleep(int duration) {
  uart_wait_tx_idle_polling((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM);
  esp_sleep_enable_timer_wakeup(1000*1000*duration);
  esp_light_sleep_start();
}

void deep_sleep(int duration) {
  uart_wait_tx_idle_polling((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM);
  esp_sleep_enable_timer_wakeup(1000*1000*duration);
  esp_deep_sleep_start();
}

bool anomaly_detection(unsigned long ts, float* rms_values) {
  // Check if the sample is an anomaly based on the threshold and baseline
  if (!baseline_established) return false;

  if (rms_values[0] > (baseline_rms[0]) * ANOMALY_THRESHOLD_X ||
      rms_values[1] > (baseline_rms[1]) * ANOMALY_THRESHOLD_Y ||
      rms_values[2] > (baseline_rms[2]) * ANOMALY_THRESHOLD_Z) {
      Serial.printf("[ANOMALY] Anomaly detected at %lu: x: %.2f, y: %.2f, z: %.2f\n", ts, rms_values[0], rms_values[1], rms_values[2]);
      Serial.printf("[ANOMALY] Baseline: x: %.2f, y: %.2f, z: %.2f\n", baseline_rms[0], baseline_rms[1], baseline_rms[2]);
      Serial.printf("[ANOMALY] Threshold: x: %.2f, y: %.2f, z: %.2f\n", baseline_rms[0] * ANOMALY_THRESHOLD_X, baseline_rms[1] * ANOMALY_THRESHOLD_Y, baseline_rms[2] * ANOMALY_THRESHOLD_Z);
      return true;
  }
  return false;
}

void add_to_window(float sample[], float window[][WINDOW_SIZE], int size) {
  window[0][index_rms] = sample[0];
  window[1][index_rms] = sample[1];
  window[2][index_rms] = sample[2];
  index_rms = (index_rms + 1) % size;
}

/* FFT Processing Core ----------------------------------------------------- */
/**
 * @brief Execute complete FFT processing chain
 * @details Performs:
 * 1. Hamming window application
 * 2. Forward FFT computation
 * 3. Complex-to-magnitude conversion
 * @note Results stored in module buffers
 */
void fft_perform_analysis(void) {
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
}

/**
 * @brief Identify max frequency component
 * @return Frequency (Hz) of highest frequency
 * @pre Requires prior call to fft_perform_analysis()
 */
float fft_get_max_frequency(void) {
  float maxFrequency = -1;

  // Loop through all bins (skip DC at i=0)
  for (uint16_t i = 1; i < (INIT_SAMPLE_RATE >> 1); i++) {
    // Check if the current bin is a local maximum and above the noise floor
    if (samples_real[i] > samples_real[i-1] && samples_real[i] > samples_real[i+1] && samples_real[i] > NOISE_THRESHOLD) {
      float currentFreq = (i * g_sampling_frequency) / INIT_SAMPLE_RATE;
      // Update maxFrequency if this peak has a higher frequency
      if (currentFreq > maxFrequency) {
        maxFrequency = currentFreq;
      }
    }
  }
  return maxFrequency;
}

void fft_sample_signal() {
  float sample[3] = {0,0,0};
  // Sample the signal for initial FFT analysis
  for (int i = 0; i < INIT_SAMPLE_RATE; i++) {
    read_sample(&sample[0], &sample[1], &sample[2]);
    samples_real[i] = sample[0]+sample[1]+sample[2];
    light_sleep((1000 / g_sampling_frequency));
  }
}


/* System Configuration ---------------------------------------------------- */
/**
 * @brief Adapt sampling rate based on Nyquist-Shannon criteria
 * @param max_freq Max detected frequency component
 * @note Implements safety factor of 2.5× maximum frequency
 */
void fft_adjust_sampling_rate(float max_freq) {
    int new_rate = (int)(NYQUIST_MULTIPLIER * max_freq);
    if (new_rate < 1) new_rate = INIT_SAMPLE_RATE / 2; // Fallback if no valid frequency detected
    g_sampling_frequency = (g_sampling_frequency > new_rate) ? new_rate : g_sampling_frequency;
}

/**
 * @brief Initialize FFT processing module
 * @details Performs:
 * 1. Initial signal acquisition
 * 2. Frequency analysis
 * 3. Adaptive rate configuration
 * @note Must be called before starting sampling tasks
 */
void fft_init() {
  Serial.println("[FFT] Initializing FFT module");
  
  if (!fft_init_complete) {

    // Initial analysis with high frequency sampling
    g_sampling_frequency = INIT_SAMPLE_RATE;
    fft_sample_signal();
    
    fft_perform_analysis();
    
    // Adaptive rate adjustment
    float max_freq = fft_get_max_frequency();

    if (max_freq > 0) { 
      Serial.printf("[FFT] Max frequency: %.2f Hz\n", max_freq);
    } else {
      Serial.printf("[ERROR] No valid peaks detected\n");
      return; 
    }

    fft_adjust_sampling_rate(max_freq);
    Serial.printf("[FFT] Optimal sampling rate: %d Hz\n", g_sampling_frequency);
    
    // Mark initialization as complete
    fft_init_complete = true;
    
  } 
}

/**
 * @brief Main sampling task handler
 * @param pvParameters FreeRTOS task parameters (unused)
 * @details
 * 
 * @warning Depends on initialized queue (xQueue_temp and xQueue_temp_avg)
 */
void fft_sampling_task(void *pvParameters) {
  float sample[3] = {0,0,0};
  float rms_array[3] = {0,0,0};

  unsigned long time_stamp = 0;
  
  if(!fft_init_complete)
    fft_init();
  Serial.printf("[FFT] Starting sampling at %d Hz\n", g_sampling_frequency);
  Serial.println("--------------------------------");

  while(1) {
    // Read sample from sensor
    read_sample(&sample[0], &sample[1], &sample[2]);
  
    
    Serial.printf("[FFT] Sample %d = x: %.2f, y: %.2f, z: %.2f\n",num_of_samples, sample[0], sample[1], sample[2]);

    time_stamp = millis();
    
    // Handle baseline establishment
    if (!baseline_established) {

      if (num_of_samples < BASELINE_WINDOW_SIZE) {
        baseline[0][num_of_samples] += sample[0];
        baseline[1][num_of_samples] += sample[1];
        baseline[2][num_of_samples] += sample[2];
        
        if (num_of_samples == (BASELINE_WINDOW_SIZE - 1)) {
          baseline_established = true;
          baseline_rms[0] = calculateRMS(baseline[0], BASELINE_WINDOW_SIZE);
          baseline_rms[1] = calculateRMS(baseline[1], BASELINE_WINDOW_SIZE);
          baseline_rms[2] = calculateRMS(baseline[2], BASELINE_WINDOW_SIZE);
          
          if (baseline_rms[0] < 0.1) baseline_rms[0] = 0.1; // Avoid zero multiplication
          if (baseline_rms[1] < 0.1) baseline_rms[1] = 0.1; 
          if (baseline_rms[2] < 0.1) baseline_rms[2] = 0.1; 

          Serial.printf("[FFT] Baseline established rms: x: %.2f, y: %.2f, z: %.2f\n", baseline_rms[0], baseline_rms[1], baseline_rms[2]);
          Serial.printf("[ANOMALY] Threshold: x: %.2f, y: %.2f, z: %.2f\n", baseline_rms[0] * ANOMALY_THRESHOLD_X, baseline_rms[1] * ANOMALY_THRESHOLD_Y, baseline_rms[2] * ANOMALY_THRESHOLD_Z);

          num_of_samples = 0; // Reset sample count for next phase
        }
      }
    } else {
      // Add RMS values to the window
      add_to_window(sample, window_rms, g_window_size);

      // Send RMS values periodically
      if ((num_of_samples % g_window_size) == 0) {
        rms_array[0] = calculateRMS(window_rms[0], g_window_size);
        rms_array[1] = calculateRMS(window_rms[1], g_window_size);
        rms_array[2] = calculateRMS(window_rms[2], g_window_size);
        send_rms(rms_array);
        // Print RMS values
        Serial.printf("[FFT] RMS: x:%.2f y:%.2f z:%.2f\n", rms_array[0], rms_array[1], rms_array[2]);
         // Check for anomalies
        if (anomaly_detection(time_stamp, rms_array)) {
          send_anomaly();
          Serial.printf("[ALERT] Anomaly detected! \n");
          // Consider whether to reset or continue
        }
      }
    }   
    num_of_samples++;
    deep_sleep(1000/g_sampling_frequency);
  }
}
