#include <Arduino.h>
#include <arduinoFFT.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "shared_defs.h"
#include "fft-and-adaptive-sampling.h"

#define NOISE_THRESHOLD = 30.0;
#define TEMP_SENSOR_PIN;

int g_window_size = 0;
float* window = nullptr;

// Track cumulative stats for all samples
float global_mean = 0.0f;
float global_m2 = 0.0f; // Sum of squared differences
uint32_t global_sample_count = 0;

/// @brief Real component buffer for FFT input
float g_samples_real[NUM_SAMPLES] = {0};

/// @brief Imaginary component buffer for FFT input
float g_samples_imag[NUM_SAMPLES] = {0};

/// @brief Current system sampling frequency (Hz)
int g_sampling_frequency_temp = INIT_SAMPLE_RATE;

/// @brief ArduionFFT instance configured with buffers
ArduinoFFT<float> FFT = ArduinoFFT<float>(
    g_samples_real, 
    g_samples_imag, 
    NUM_SAMPLES, 
    g_sampling_frequency_temp
);

void send_avg_temp(float avg) {
  // Send to queue
}
void send_temp_anomaly() {
  // Trigger alert
}

bool anomaly_detection(unsigned long ts, float sample, float variance);

float calc_rolling_avg() {
  float sum = 0;
  for(int i=0; i<g_window_size; i++) {
      sum += window[i];
  }
  return sum / g_window_size;
}

void add_to_window(float sample) {
  static int index = 0;
  window[index] = sample;
  index = (index + 1) % g_window_size;
}

void update_global_stats(float new_sample) {
  global_sample_count++;
  float delta = new_sample - global_mean;
  global_mean += delta / global_sample_count;
  float delta2 = new_sample - global_mean;
  global_m2 += delta * delta2;
}

float get_global_variance() {
  if (global_sample_count < 2) return 0.0f;
  return global_m2 / (global_sample_count - 1);
}

void reset_global_stats() {
  global_mean = 0.0f;
  global_m2 = 0.0f;
  global_sample_count = 0;
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
  double maxFrequency = -1;

  // Loop through all bins (skip DC at i=0)
  for (uint16_t i = 1; i < (NUM_SAMPLES >> 1); i++) {
    // Check if the current bin is a local maximum and above the noise floor
    if (g_samples_real[i] > g_samples_real[i-1] && g_samples_real[i] > g_samples_real[i+1] && g_samples_real[i] > NOISE_THRESHOLD) {
      double currentFreq = (i * g_sampling_frequency_temp) / NUM_SAMPLES;
      // Update maxFrequency if this peak has a higher frequency
      if (currentFreq > maxFrequency) {
        maxFrequency = currentFreq;
      }
    }
  }
  return maxFrequency;
}

void adjust_window_size(){
  g_window_size = RATIO_SAMPLES_SECONDS * SECONDS_OF_AVG;
  if (window) free(window);
  window = (float*)calloc(g_window_size, sizeof(float));
}

void fft_sample_signal(int pin) {
  // Sample the signal for initial FFT analysis
  for (int i = 0; i < NUM_SAMPLES; i++) {
    g_samples_real[i] = analogRead(pin);
    delay(1000 / g_sampling_frequency_temp);
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
    g_sampling_frequency_temp = (g_sampling_frequency_temp > new_rate) ? new_rate : g_sampling_frequency_temp;
}

/**
 * @brief Initialize FFT processing module for water OR temp
 * @details Performs:
 * 1. Initial signal acquisition
 * 2. Frequency analysis
 * 3. Adaptive rate configuration
 * @note Must be called before starting sampling tasks
 */
void fft_init(int PIN) {
    Serial.println("[FFT] Initializing FFT module");
    
    // Initial analysis with default signal
    fft_sample_signal(PIN);
    
    fft_perform_analysis();
    
    // Adaptive rate adjustment
    const float max_freq = fft_get_max_frequency();

    if (max_freq > 0){ 
      Serial.printf("[FFT] Max frequency: %.2f Hz\n", max_freq);
    } else {
      Serial.printf("[ERROR] No valid peaks detected\n");
      vTaskDelete(NULL);
    }

    fft_adjust_sampling_rate(max_freq);
    Serial.printf("[FFT] Optimal sampling rate: %d Hz\n", g_sampling_frequency_temp);
    adjust_window_size(); // window size will change if the sampling frequency changes
    Serial.printf("[FFT] New window size: %d Hz\n", g_window_size);
}

/*Se ho due segnali da campionare devo campionare con due f_opt diverse? devo avere due task per segnale?*/

/**
 * @brief Main sampling task handler
 * @param pvParameters FreeRTOS task parameters (unused)
 * @details
 * 
 * @warning Depends on initialized queue (xQueue_temp and xQueue_temp_avg)
 */
void fft_sampling_temp_task(void *pvParameters) {
  float temp_sample = 0.0f,avg = 0.0f,variance = 0.0f;
  unsigned long time_stamp = 0;
  int i=0;
  fft_init(TEMP_SENSOR_PIN);
  Serial.printf("[FFT] Starting sampling at %d Hz\n", g_sampling_frequency_temp);
  Serial.println("--------------------------------");

  while(1){
    
    temp_sample = analogRead(TEMP_SENSOR_PIN) * 3.3f / 4095.0f;
    
    Serial.printf("[FFT] Sample temp %d: %.2f\n", i, temp_sample);

    
    time_stamp = millis();
    
    add_to_window(temp_sample);
    
    variance = get_global_variance();
    
    if( (i+1) % (g_window_size) == 0){
      avg = calc_rolling_avg();
      send_avg_temp(avg);
    }
    i++;
    printf ("[FFT] Variance: %.2f\n", variance);
    printf ("[FFT] Rolling avg: %.2f\n", avg);

    if(anomaly_detection(time_stamp,temp_sample,variance)) {
      send_temp_anomaly();
      Serial.printf("[ALERT] Anomaly detected! Restarting Nyquist phase...\n");
      fft_init(TEMP_SENSOR_PIN);
      Serial.printf("[FFT] Restarted sampling at %d Hz\n", g_sampling_frequency_temp);
      
      reset_global_stats();
    }
    update_global_stats(temp_sample);

    vTaskDelay(pdMS_TO_TICKS(1000/g_sampling_frequency_temp));
  }
}
