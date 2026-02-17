# 🧠 Firmware Architecture (ESP32 + ADS1298)

This directory contains the embedded C++ driver and application code developed in **PlatformIO** (VS Code).

## 🎯 Overview
The firmware acts as the bridge between the high-precision **ADS1298 Analog Front End (AFE)** and the visualization software. It handles the SPI register configuration, data retrieval, and serial transmission.

## 🛠️ Implementation Details

### 1. ADS1298 Driver (`ADS1298.cpp`)
A custom C++ class was implemented to manage the complex register map of the TI ADS1298.
* **Initialization:** Configures power-up sequencing and resets the device.
* **Register Configuration:** Sets up the 8 channels for normal electrode input (Lead II configuration) and configures the internal reference voltage (VREF).
* **Data Retrieval:** Reads the 24-bit status word and 8 channels of 24-bit ECG data via SPI.

### 2. Data Acquisition Loop (`main.cpp`)
* **Protocol:** SPI (Serial Peripheral Interface) Mode 1.
* **Sampling:** Uses the **DRDY** (Data Ready) hardware interrupt to trigger data reading, ensuring timing accuracy.
* **Output:** Formats the raw 24-bit integers into voltage values and streams them over **High-Speed Serial (Baud 115200)** for real-time plotting.

## 🔌 Pin Configuration
| Signal | ESP32 Pin | Function |
| :--- | :--- | :--- |
| **SCLK** | GPIO 18 | Serial Clock |
| **MISO** | GPIO 19 | Master In Slave Out |
| **MOSI** | GPIO 23 | Master Out Slave In |
| **CS** | GPIO 5 | Chip Select |
| **DRDY** | GPIO 16 | Data Ready Interrupt |
| **RESET**| GPIO 17 | Hardware Reset |

