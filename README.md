# FanGuard  
**Edge-AI predictive-maintenance node for server-rack fans**  

## Introduction
Server-rack fans are critical to datacentre uptime, but mechanical wear gradually introduces excessive vibration and current spikes that precede outright failure. **FanGuard** sits directly on the fan power rail, sampling both vibration (MPU6050, 3-axis) and power draw (INA219).  
A lightweight anomaly-detection model runs in real-time on an **ESP32** under **FreeRTOS**, pushing alerts to the facility’s MQTT broker and main control dashboard.

TODO: why RMS?

---

## Components & Wiring
The hardware stack is deliberately minimal: a single **ESP32** sits at the centre, sharing an I²C bus with two peripheral sensors—the tri-axis accelerometer for vibration analysis and the INA219 for power analytics—while tapping straight into the 5 V fan rail. The diagram below shows the exact pin-to-pin wiring, colour-coded for clarity (power in red, I²C in blue/orange, ground in black).


![image](https://github.com/user-attachments/assets/a7bb1680-b0de-464d-b349-77e5cd47bde8)

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

## Energy Consumption

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
