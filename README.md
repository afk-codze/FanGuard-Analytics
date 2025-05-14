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

> 📄 *Code reference*: TODO

### 2. Edge Impulse pipeline  
- **Features**: a **Time Series (3)** block consuming the pre-computed `rmse_x`, `rmse_y`, and `rmse_z` inputs.  
- **Learning**: a **Classification** block running our custom Keras classifier (see next section).  
- **Deployment**: export as a **Quantized (INT8)** model using the **EON™ Compiler**.  


### 3. Model architecture & training script  

```python
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, InputLayer, Dropout, Conv1D, Conv2D, Flatten, Reshape, MaxPooling1D, MaxPooling2D, AveragePooling2D, BatchNormalization, Permute, ReLU, Softmax
from tensorflow.keras.optimizers.legacy import Adam

EPOCHS = args.epochs or 50
LEARNING_RATE = args.learning_rate or 0.001
# If True, non-deterministic functions (e.g. shuffling batches) are not used.
# This is False by default.
ENSURE_DETERMINISM = args.ensure_determinism
# this controls the batch size, or you can manipulate the tf.data.Dataset objects yourself
BATCH_SIZE = args.batch_size or 32

# Add normalization layer for preprocessing
# Standard scaling (zero mean, unit variance)
normalizer = tf.keras.layers.Normalization(axis=-1)
normalizer.adapt(train_dataset.map(lambda x, y: x))

if not ENSURE_DETERMINISM:
    train_dataset = train_dataset.shuffle(buffer_size=BATCH_SIZE*4)
train_dataset = train_dataset.batch(BATCH_SIZE, drop_remainder=False)
validation_dataset = validation_dataset.batch(BATCH_SIZE, drop_remainder=False)

# model architecture
model = Sequential()
# Input layer (explicit)
model.add(InputLayer(input_shape=(3,)))  # 3 features: rmse_x, rmse_y, rmse_z
model.add(normalizer)

# Hidden layers with recommended architecture
model.add(Dense(16, activation='relu',
    activity_regularizer=tf.keras.regularizers.l2(0.0001)))
model.add(Dropout(0.2))

model.add(Dense(8, activation='relu',
    activity_regularizer=tf.keras.regularizers.l2(0.0001)))
model.add(Dropout(0.2))

# Output layer - using sigmoid for binary classification
model.add(Dense(classes, name='y_pred', activation='sigmoid' if classes == 1 else 'softmax'))

# this controls the learning rate
opt = Adam(learning_rate=LEARNING_RATE, beta_1=0.9, beta_2=0.999)

callbacks.append(BatchLoggerCallback(BATCH_SIZE, train_sample_count, epochs=EPOCHS, ensure_determinism=ENSURE_DETERMINISM))

# Early stopping to prevent overfitting
early_stopping = tf.keras.callbacks.EarlyStopping(
    monitor='val_loss',
    patience=10,
    restore_best_weights=True
)
callbacks.append(early_stopping)

# train the neural network
# Use binary_crossentropy for binary classification
loss_function = 'binary_crossentropy' if classes == 1 else 'categorical_crossentropy'
model.compile(loss=loss_function, optimizer=opt, metrics=['accuracy'])
model.fit(train_dataset, epochs=EPOCHS, validation_data=validation_dataset, 
          verbose=2, callbacks=callbacks, class_weight=ei_tensorflow.training.get_class_weights(Y_train))

# Use this flag to disable per-channel quantization for a model.
# This can reduce RAM usage for convolutional models, but may have
# an impact on accuracy.
disable_per_channel_quantization = False
```

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
