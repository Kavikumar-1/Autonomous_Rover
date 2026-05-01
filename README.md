# Autonomous AI Plant-Analysis Rover 🚜🪴

An intelligent agricultural rover that uses deep learning to detect plant diseases and automatically treat them with a mist sprayer. Features autonomous scanning, manual IoT control via Blynk, and a real-time AI analysis backend.

## 🚀 Key Features
- **Autonomous Scanning**: Automatically roams the field using ultrasonic sensors to detect plants.
- **AI Disease Detection**: Uses a Custom CNN (TensorFlow) to identify Healthy vs. Diseased Chilli and Tomato plants.
- **Automatic Treatment**: Automatically sounds a buzzer warning and sprays mist for 10 seconds when a disease is detected.
- **Manual Override**: Full manual driving control via the Blynk IoT app.
- **Live Video Feed**: Real-time MJPEG stream from the ESP32-CAM for remote monitoring.
- **Data Collection**: Automatically saves all captured images to the server for future AI dataset improvements.

## 🛠️ System Architecture
1. **Main ESP32**: Controls the chassis (L298N), sensors (HC-SR04), LCD (16x2), and communicates with Blynk.
2. **ESP32-CAM**: Handles image capture and hosts the live video stream.
3. **Python Flask Server**: Processes images using TensorFlow/Keras and returns the analysis to the rover.
4. **Blynk IoT**: Provides a dashboard for manual driving, speed control, and manual misting.

## 📁 Project Structure
- `/Esp32_Main/`: Firmware for the primary rover controller.
- `/CameraWebServer/`: Optimized firmware for the ESP32-CAM.
- `/Rover_Project_BackEnd/`: Python server, AI model, and training scripts.
  - `server.py`: The live Flask backend.
  - `train_model.py`: Dataset consolidation and training pipeline.
  - `plant_model.h5`: The trained "brain" of the AI.

## ⚙️ Quick Start
### 1. Backend Setup
1. Install requirements: `pip install flask tensorflow opencv-python numpy`
2. Run the server: `python server.py`
3. Note your laptop's IP address (e.g., `10.171.245.229`).

### 2. ESP32-CAM Setup
1. Open `CameraWebServer.ino`.
2. Update `ssid`, `password`, and `serverName` with your laptop's IP.
3. Upload to ESP32-CAM.

### 3. Main ESP32 Setup
1. Open `update_code.ino`.
2. Update Blynk `Template ID`, `Auth Token`, and WiFi credentials.
3. Update `serverName` with your laptop's IP.
4. Upload to the Main ESP32.

## 🕹️ Blynk Dashboard Setup
| Virtual Pin | Widget | Function |
|---|---|---|
| **V0 - V3** | Buttons | Manual Direction (Fwd, Bwd, Left, Right) |
| **V4** | Switch | Mode Toggle (Auto / Manual) |
| **V5** | Gauge | Live Distance (cm) |
| **V6** | Slider | Motor Speed Control (0-255) |
| **V7** | Button | Manual Sprayer (Hold to Spray) |

---
*Developed for the Autonomous Plant-Analysis Optimization project.*
