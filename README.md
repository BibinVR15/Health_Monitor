# 💙 IoT Health Monitoring Dashboard

An **IoT-based real-time health monitoring system** that measures **Heart Rate (BPM)**, **SpO₂ (Oxygen Saturation)**, **Body Temperature**, **GSR (Stress Level)**, and a derived **Glucose Factor** — displaying them on a web dashboard with detailed health reports.  

Data is collected using an **ESP8266 microcontroller** and stored in **Firebase Realtime Database**, then visualized on a modern **web dashboard** built with **Chart.js**.

---

## 🚀 Features

✅ Real-time measurement of:
- ❤️ **Heart Rate (BPM)**
- 🌬️ **Oxygen Saturation (SpO₂)**
- 🌡️ **Body Temperature**
- 🤲 **GSR (Galvanic Skin Response / Stress Level)**
- 🍬 **Glucose Factor (calculated from HR & SpO₂)**  

✅ **LCD Display** on device for instant feedback  
✅ **Firebase Integration** for cloud data storage  
✅ **Interactive Dashboard** using Chart.js  
✅ **Auto-generated Health Report** comparing your readings with a healthy baseline  
✅ **Easy to Deploy** (just flash ESP8266 & run the web dashboard)

---

## 🛠️ Hardware Requirements

- 🔌 **ESP8266 NodeMCU** (ESP-12E or Wemos D1 Mini recommended)  
- ❤️ **MAX30100 Pulse Oximeter Sensor** (for BPM & SpO₂)  
- 🌡️ **DS18B20 Temperature Sensor** (1-Wire)  
- ✋ **GSR Sensor** (connected to A0 pin)  
- 📟 **16x2 I2C LCD Display** (address: `0x27`)  
- 📶 WiFi Connection  

---

## 📦 Software Requirements

- Arduino IDE (latest recommended)  
- **Board Manager:** ESP8266 by ESP8266 Community  
- **Libraries (Install from Arduino Library Manager)**  
  - `ESP8266WiFi`  
  - `Firebase_ESP_Client`  
  - `Wire`  
  - `LiquidCrystal_I2C`  
  - `MAX30100_PulseOximeter`  
  - `OneWire`  
  - `DallasTemperature`  

- **Firebase Setup**
  - Create a Firebase Project
  - Enable **Realtime Database**
  - Set **Database Rules** to allow read/write (or use Auth if needed)
  - Copy your **Database URL** & **Secret Key**

---

## ⚙️ Arduino Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/YourUsername/IoT-Health-Monitor.git
