#include <Arduino.h>
#include <arduinoFFT.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "shared_defs.h"

#define NOISE_THRESHOLD = 30.0;
#define TEMP_SENSOR_PIN;
#define WATER_SENSOR_PIN;

int g_window_size = 0;
float window[];


/// @brief Real component buffer for FFT input
float g_samples_real[NUM_SAMPLES] = {0};

/// @brief Imaginary component buffer for FFT input
float g_samples_imag[NUM_SAMPLES] = {0};

/// @brief Current system sampling frequency (Hz)
int g_sampling_frequency = INIT_SAMPLE_RATE;

/// @brief ArduionFFT instance configured with buffers
ArduinoFFT<float> FFT = ArduinoFFT<float>(
    g_samples_real, 
    g_samples_imag, 
    NUM_SAMPLES, 
    g_sampling_frequency
);

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
  for (uint16_t i = 1; i < (samples >> 1); i++) {
    // Check if the current bin is a local maximum and above the noise floor
    if (vReal[i] > vReal[i-1] && vReal[i] > vReal[i+1] && vReal[i] > NOISE_THRESHOLD) {
      double currentFreq = (i * samplingFrequency) / samples;
      // Update maxFrequency if this peak has a higher frequency
      if (currentFreq > maxFrequency) {
        maxFrequency = currentFreq;
      }
    }
  }
  return maxFrequency;
}

void adjust_window_size(){
  window_size = RATIO_SAMPLES_SECONDS * SECONDS_OF_AVG;
  window[window_size] = {0};
}


/* System Configuration ---------------------------------------------------- */
/**
 * @brief Adapt sampling rate based on Nyquist-Shannon criteria
 * @param max_freq Max detected frequency component
 * @note Implements safety factor of 2.5× maximum frequency
 */
void fft_adjust_sampling_rate(float max_freq) {
    const int new_rate = (int)(NYQUIST_MULTIPLIER * max_freq);
    g_sampling_frequency = (g_sampling_frequency > new_rate) ? new_rate : g_sampling_frequency;
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
    Serial.printf("[FFT] Optimal sampling rate: %d Hz\n", g_sampling_frequency);
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
  float sample = 0.0f,avg = 0,0f;
  int i=0;
  bool anomaly = false;
  fft_init(TEMP_SENSOR_PIN);
  Serial.printf("[FFT] Starting sampling at %d Hz\n", g_sampling_frequency_temp);
  Serial.println("--------------------------------");

  while(1){
    
    //sample_water = analogRead(WATER_SENSOR_PIN) * 3.3f / 4095.0f;
    temp_sample = analogRead(TEMP_SENSOR_PIN) * 3.3f / 4095.0f;
    
    if(anomaly_detection(temp_sample)){
      send_temp_anomaly();
      Serial.printf("Anomaly detected! restarting Nyquist phase ...\n");
      fft_init(TEMP_SENSOR_PIN);
      Serial.printf("[FFT] Starting sampling at %d Hz\n", g_sampling_frequency_temp);
    }

    Serial.printf("[FFT] Sample temp %d: %.2f\n", i, temp_sample);
    
    add_to_window(temp_sample);
    
    if(i == window_size-1){
      avg = calc_rolling_avg();
      send_avg_temp(avg);
      i=0;
    }else{
      i++;
    }

    vTaskDelay(pdMS_TO_TICKS(1000/g_sampling_frequency_temp));
  }
}
