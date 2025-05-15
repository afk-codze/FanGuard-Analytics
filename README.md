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

### Why Monitor Server Rack Fans Every 30 Seconds?

This section documents our analysis and reasoning for implementing a 30-second monitoring frequency for server rack fan systems. Our analysis show this is a good balance between detection reliability and system resources.

This system provides early detection of fan anomalies through accelerometer-based vibration monitoring, alerting operators before thermal damage occurs.

#### The Problem: Rapid Temperature Rise

Server racks experience alarmingly quick temperature increases after cooling failure:

- A server rack starting at 20-23°C can reaches critical temperatures in **~5 minutes** after fans failure.

This narrow intervention window requires a carefully calculated monitoring frequency.

####  Response Timeline Components


| Component | Time Required | Description |
|-----------|---------------|-------------|
| Alert processing | 5 seconds | System processing and alert distribution |
| Human response | 120 seconds | Staff receiving alert and beginning intervention |
| Intervention time | 60 seconds | Fix the problem |
| Safety buffer | 25 seconds | Additional buffer for unexpected delays |
| **Total response time** | **210 seconds** | From detection to completed intervention |

With temperatures reaching critical levels 5 minutes, this leaves a maximum allowable detection delay of **90 seconds**.

#### Monitoring Frequency Analysis

We evaluated multiple monitoring intervals:

| Monitoring Frequency | Worst-Case Detection Delay | Safety margin | Viable? |
|--------------------:|---------------------------:|------------------:|:-------:|
|              5 sec  |                    5.0 sec |           85 sec  |   YES   |
|             10 sec  |                   10.0 sec |           80 sec  |   YES   |
|             15 sec  |                   15.0 sec |           75 sec  |   YES   |
|           **30 sec**|                **30.0 sec**|         **60 sec**| **YES** |
|             60 sec  |                   60.0 sec |           30 sec  |   YES   |
|            120 sec  |                  120.0 sec |           none  |    NO   |
|            180 sec  |                  180.0 sec |           none  |    NO   |
|            300 sec  |                  300.0 sec |           none  |    NO   |

#### Why 30 Seconds is Optimal

While intervals up to 60 seconds would technically work, we chose **30 seconds** for these reasons:

- **Safety margin**: Provides additional buffer if response is delayed 
- **Variable loads**: System load fluctuations could accelerate temperature rise
- **Balance**: Strikes optimal balance between detection reliability and system resource utilization

#### References

- ASHRAE TC 9.9 Thermal Guidelines for Data Processing Environments


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

## Security Implications: TODO



---

## Cloud Application

The last component of our system is the cloud, this is the final destination of the data gathered and computed by the sensors in order to make this data usable and readable by human operators through a web application. 
Thanks to this tool an operator can achieve  a global view of the servers fans health status also they can  monitor the situation in realtime and respond quickly to anomalous situations. 

![image](https://github.com/user-attachments/assets/78cf1217-ee41-4ce5-a18b-50fab6d62c78)

### The Architecture

In this section we give an overview of the structure and the technologies used for building the application running on the cloud, the application is called "Monitoring System". 

* __MQTT broker__: This is the junction point between the esp32 and the cloud, it lets transfer data from the IoT device to the cloud application.
* __Java__: The programming language used to build the web application running on the cloud, this object-oriented language is largely used to build web applications thanks to the huge amount of support and libraries it offers.
* __Java Spring__: Web development framework that supports many integrated modules that speed up the process of web development by integrating many aspects like: Dabates handling, front-end templating, MVC code structure etc.
* __PostgresSQL__ : Database Management System used to store the data coming from the esp32 and provide historical view of all the data.
* __Html - javascript__: Languages used to build the presentation level look and functionalities.

### Presentation Level

In the presentation video that you can find at this link: link, you can see how the application works from the presentation level:

There are 2 pages that can be access through the navbar placed on the top, the first page that is also the index of the application is called __realtime graphs__ in this page you can select an IoT device and then click the Submit button, at this point 4 graphs will be generated for you, these graphs are realtime so they are periodically updated with the new data coming from the IoT devices. \
The graphs generated show the power levels measured in mW and the vibration on the 3 axes measured in g, each value is timestamped with the time it was received by the cloud application, in order to make it more readable and easy to understand this timestamp has been processed in order to be shown in terms hours,minutes and seconds.\
On the right side of the page there is also a table that shows  the last 10 most recent anomalies detected by the system. \
The second page called __log__ is very similar to the first one, the functionalities are the same, but here the graphs are not realtime and by using time selectors the operator can choose a time interval of interest of which to generate the graphs and the anomalies table.

### Under the hood: the back end

The back end of the application as stated in the previous paragraph uses Java Spring as development framework, so the code is structured following the MVC pattern:

  * __Model__: ORM Model connected to the db models the information coming from the IoT devices: Anomaly, Power and RMS.
  * __View__: From a api call to the proper url it returns the html and javascript code to render the web page.
  * __Controller__: Manages interaction between Model and View, there is a controller that manages API calls from javscript code to retrieve information from the DB and a controller that manages the web pages routing.

At startup the application connects to the MQTT broker and subscribes to the interested topics, when a new message written in json fromat comes from the IoT devices into the broker, the application consumes it from it, this message is then parsed into a DTO (Data Transfer Object) using a JSON serializer and then stored on the database. \
As stated before the application uses javascript at the presentation level to handle the graph generation and other graphicacl functionalities, the data needed to fill the graphs is retrieved from the backend through an ajax request that is handled by a back end controller which processes the request, interacts with the DB to retrieve the data , process it into a service class and finally returns it as response.\
Notice that before storing the data retrieved from the MQTT broker the system verifies the hmac contained in the message, if it is not valid the message is dropped, this security mechanism is used to prevent possible replay attacks.

---

## How to install and run (quick start)

### Cloud application

The following steps explain the set up needed to make this application work:

#### Dependencies:

* Install the open-jdk-17 and postgresql on your machine:
  ```
  sudo apt install openjdk-17-jdk postgresql -Y
  ```
* Create a new user in the psql console environment with a password
  ```
  $ sudo -i -u postgres
  $ psql
  CREATE USER your_username WITH PASSWORD 'your_password';
  CREATE DATABASE your_dbname;
  GRANT ALL PRIVILEGES ON DATABASE your_dbname TO your_username;
  \q
  $ psql -U your_username -d your_dbname -h localhost

  ```
 * Clone from github this repository, application files can be found in `cloud-application`
 
#### Environment Variables:

  * Navigate to `cloud-application/src/main/resources/application.properties` 
    ```
    spring.datasource.url=jdbc:postgresql://localhost:5432/your_dbname
    spring.datasource.username=your_username
    spring.datasource.password=your_password
    spring.datasource.driver-class-name=org.postgresql.Driver

    ```
    modify these variables with your database credentials and name.

#### Running:

* Start an MQTT broker or use a public one
* Build and start the application:
  ```
  $ cd ./cloud-application
  $ ./mwnw spring-boot:run
  ```
* You can now access the application at url: `http://localhost:8080/`


### Arduino sketch: TODO


---


## 👨‍💻 **Team:**  
- Massimiliano Vitale: [Linkedin](https://www.linkedin.com/in/massimiliano-vitale/)
- Luca Cornici: [Linkedin](https://www.linkedin.com/in/luca-cornici-a31a822b9/)  
- Angelo Pio Pompeo: [Linkedin](https://it.linkedin.com/in/angelo-pio-pompeo-6a2960225)

📑 **Project Presentation:** [VoltSky-Analytics Presentation Deck](https://www.canva.com/design/DAGiGgqm3vg/dWG1Gl8j_IxVZVRmSFhmMA/view?utm_content=DAGiGgqm3vg&utm_campaign=designshare&utm_medium=link2&utm_source=uniquelinks&utlId=ha00e9f673b)  
