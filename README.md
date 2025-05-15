# FanGuard - Analytics
Edge-AI predictive-maintenance node for server-rack fans

## Introduction
Server-rack fans are critical to datacentre uptime, but mechanical wear gradually introduces excessive vibration and current spikes that precede outright failure. **FanGuard** sits directly on the fan power rail, sampling both vibration (MPU6050, 3-axis) and power draw (INA219).  
A lightweight anomaly-detection model runs in real-time on an **ESP32** under **FreeRTOS**, pushing alerts to the facility’s MQTT broker and main control dashboard.

![ChatGPT Image May 15, 2025, 03_28_13 PM](https://github.com/user-attachments/assets/df8f4223-56ef-4024-b31e-7067eb2008d1)

---

## Why we chose RMS?


- **Overall Vibration Level**: RMS provides a single, scalar value that represents the overall energy or intensity of the vibration signal over a given time window. This makes it easy to track changes in the general vibration level of the fan.   

- **Sensitivity to Changes**: A significant increase in the RMS value often indicates a change in the fan's operational state, which could be cause by anomalies.

- **Correlation with Energy**: RMS is proportional to the energy of the vibration. Increased vibration energy is a key indicator of potential problems in rotating machinery.   
- **Simplicity and Low Computational Cost**: Calculating RMS is straightforward and computationally inexpensive, reducing the impact on system responsiveness and energy consumption.


#### References:
    - https://dynamox.net/en/blog/the-peak-peak-to-peak-and-rms-values-in-vibration-analysis
    - https://doi.org/10.1016/j.dib.2024.110866
    - https://www.researchgate.net/figure/Anomaly-detection-velocity-RMS-and-acceleration-peak-to-peak-in-a-form-of-a-box-plots_fig7_349233935 

---

## Components & Wiring
The hardware stack is deliberately minimal: a single **ESP32** sits at the centre, sharing an I²C bus with two peripheral sensors—the tri-axis accelerometer for vibration analysis and the INA219 for power analytics—while tapping straight into the 5 V fan rail. The diagram below shows the exact pin-to-pin wiring, colour-coded for clarity (power in red, I²C in blue/orange, ground in black).


![image](https://github.com/user-attachments/assets/a7bb1680-b0de-464d-b349-77e5cd47bde8)

---
## Sampling & Communication Workflow

The diagram below shows how FanGuard balances high-rate sensor sampling with periodic WiFi/MQTT uploads:

1. **Sampling (30 s)**  
   - **Task 1** runs a tight loop at sampling frequency (Hz):  
     1. Read accelerometer  
     2. Adds values to the window
     3. Enter light-sleep

2. **Parallel Execution (≈5 s max)**  
   - **Task 1** calculate RMS, detects anomalies, pushes to communication queue
   - **Task 2** (WiFi/MQTT) kicks off while **Task 1** continues sampling.  
   - To maintain precise sampling frequency (Hz) timing during transmission, **Task 1** uses a delay rather than light-sleep.  
   - **Task 2** publishes buffered data over WiFi/MQTT.  

3. **Resume Normal Sampling**  
   - Once communication completes, **Task 1** returns to its light-sleeped sampling frequency (Hz) sampling loop.  

![WhatsApp Image 2025-05-15 at 16 07 55](https://github.com/user-attachments/assets/a50b25fd-5774-41b8-af3d-4292a05e6eda)


---

## AI / Machine-Learning Pipeline
This project blends classic condition-monitoring features with a tiny neural-network so that **FanGuard can spot abnormal vibration signatures in real-time—without cloud round-trips**. We collect raw samples from the MPU6050 ant then calculate RMS , label them as *normal* or *anomaly*, and train a compact INT8-quantised Keras model in Edge Impulse. Once flashed, the model executes inside TensorFlow Lite Micro on the ESP32.

### 1. Data acquisition & labelling  
| Parameter | Value |
|-----------|-------|
| **Sampling source** | ESP32 → MPU6050|
| **On-device pre-proc.** | calculation of **RMS** for each sample|
| **Export format** | CSV (`timestamp, rms_x, rms_y, rms_z, label`) |
| **Labels** | `normal`, `anomaly` |
| **Dataset size** | 300 samples → 80 % train / 20 % test |

> 📄 *Code reference*: [src/ml-model/dataset-builder.ino](src/ml-model/dataset-builder.ino)

### 2. Edge Impulse pipeline  
- **Features**: a **Time Series (3)** block consuming the pre-computed `rmse_x`, `rmse_y`, and `rmse_z` inputs.  
- **Learning**: a **Classification** block running our custom Keras classifier (see next section).  
- **Deployment**: export as a **Quantized (INT8)** model using the **EON™ Compiler**.  

![image](https://github.com/user-attachments/assets/741104b7-36f9-418d-8c5b-dc868c61cabc)

### 3. Model architecture & training script  

We extract per‐axis RMS features from the accelerometer and feed them into a compact, fully‐connected neural network for anomaly detection. After standard normalization, two hidden layers with regularization and dropout learn to distinguish “normal” vs. “anomalous” motion patterns. Training uses the Adam optimizer with configurable hyperparameters and early stopping to ensure robust, lightweight inference.

> 📄 *Code reference*: [src/ml-model/keras-model.py](src/ml-model/keras-model.py)

### 4. Model performance & accuracy
The INT8-quantised classifier exhibits high confidence and generalisation on the held-out validation set, making it well-suited for real-time anomaly detection on the ESP32. Below are the core metrics, confusion breakdown, and a 2D feature-space view that together demonstrate both sensitivity to early-warning vibration signatures and robustness against false positives.

![image](https://github.com/user-attachments/assets/6c57be19-5882-441f-b7fd-5f7a157d0461)

![image](https://github.com/user-attachments/assets/e671bf9f-79bd-4937-940e-1f448ccc12e2)

---

## Power consumption: measurements

Below is a snapshot of the device’s real-time power draw, captured over multiple operation cycles. The chart highlights three distinct phases: **initialization**, **active** WiFi/MQTT/sampling, and **light-sleep** sampling.


![WhatsApp Image 2025-05-15 at 15 41 33](https://github.com/user-attachments/assets/efa06ee1-41e3-44f0-9461-c42c3ed94b26)


## Power Consumption: analysis

This analysis compares two power scenarios for a device over a total duration of 35 seconds:
1.  **Full Active Mode**: A constant power consumption of 400mW for the entire 35 seconds (Sampling time + RMS Calculation + Wifi/MQTT communication ).
2.  **Hybrid Mode**: An initial period of high activity (400mW for 5 seconds), this rappresents Wifi/MQTT communication with5 seconds in the worst case scenario (NUM_MAX_RETRIES * DELAY), followed by a longer period (30 seconds), that rappresents the sampling/rms calculation, utilizing a low-power state (light sleep) with periodic, short bursts of higher power for sampling. **The sampling phase assumes 4ms active time and 1ms light sleep time per cycle (Worst case).**


This comparison helps to illustrate the energy savings we achive on our system. That garantees an life battery time of ~

### Parameters

| Parameter             | Value         |
|-----------------------|---------------|
| **Full Active Power** | 400 mW        |
| **Light Sleep Power** | 25 mW         |
| **Active Burst Power**| 200 mW        |
| **Sampling Frequency**| 200 Hz        |
| **Sampling Cycle** | 5 ms (1/200 Hz) |
| **Assumed Active Time per Sample** | **4 ms** |
| **Assumed Sleep Time per Sample** | **1 ms** (5 ms - 4 ms) |
| **Total Duration Compared** | 35 seconds |


### Calculations

#### **Scenario 1: Full Active Mode (35 seconds)**

In this scenario, the device consumes a constant 400 mW for the entire 35-second duration.

-   **Total Energy**:
    ```
    Energy = Power * Time
    Energy = 400 mW * 35 s = 14000 mJ
    ```


#### **Scenario 2: Hybrid Mode (5s Active + 30s Light Sleep/Sampling - Revised)**

This scenario consists of two distinct phases:

1.  **First 5 seconds (Initial Active Phase)**: The device consumes 400 mW for the first 5 seconds.
    ```
    Energy_initial = 400 mW * 5 s = 2000 mJ
    ```

2.  **Next 30 seconds (Sampling Phase)**: For the remaining 30 seconds, the device cycles between light sleep (25 mW) and brief active bursts (200 mW) for sampling at 200 Hz, with the revised timing (4ms active, 1ms sleep).

    * **Sampling Cycle Details**: At 200 Hz, each sampling cycle takes 5 ms. Within each 5 ms cycle, the device is active for **4 ms** and in light sleep for **1 ms**.

    * **Energy per Sampling Cycle**:
        ```
        Energy_cycle = (Active Time * Active Burst Power) + (Sleep Time * Light Sleep Power)
        Energy_cycle = (4 ms * 200 mW) + (1 ms * 25 mW) = 800 mJ + 25 mJ = 825 mJ per 5ms cycle
        ```

    * **Average Power during Sampling Phase**:
        ```
        Avg Power during Sampling = Energy per Cycle / Cycle Time
        Avg Power during Sampling = 825 mJ / 5 ms = 165 mW
        ```

    * **Energy for 30 seconds of Sampling**:
        ```
        Energy_sampling = Avg Power during Sampling * Time
        Energy_sampling = 165 mW * 30 s = 4950 mJ
        ```

3.  **Total Energy for Scenario 2 (over 35s)**: The total energy is the sum of the energy from the initial active phase and the sampling phase.
    ```
    Total Energy_hybrid = Energy_initial + Energy_sampling
    Total Energy_hybrid = 2000 mJ + 4950 mJ = 6950 mJ
    ```

### Results

| **Scenario** |Avg. Power |
|----------------------------|----------|
| **1. 35s Full Active** | 400 mW     |
| **2. 5s Active + 30s Hybrid** | **198.6 mW**|


### Key Insights

1.  **Energy Savings**: The hybrid approach offers energy savings, even when considering the worst case scenario of ripartition of active mode and light sleep mode. With these parameters (4ms active/1ms sleep), the hybrid mode consumes approximately **50.4% less energy** over 35 seconds compared to staying fully active.

---

## Latency Profile

---

## Security Implications


---

## Dashboard

---

## How to Reproduce (quick start)

---


## 👨‍💻 **Team:**  
- Massimiliano Vitale: [Linkedin](https://www.linkedin.com/in/massimiliano-vitale/)
- Luca Cornici: [Linkedin](https://www.linkedin.com/in/luca-cornici-a31a822b9/)  
- Angelo Pio Pompeo: [Linkedin](https://it.linkedin.com/in/angelo-pio-pompeo-6a2960225)

📑 **Project Presentation:** [VoltSky-Analytics Presentation Deck](https://www.canva.com/design/DAGiGgqm3vg/dWG1Gl8j_IxVZVRmSFhmMA/view?utm_content=DAGiGgqm3vg&utm_campaign=designshare&utm_medium=link2&utm_source=uniquelinks&utlId=ha00e9f673b)  
