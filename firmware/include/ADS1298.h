#ifndef ADS1298_H
#define ADS1298_H

#include <Arduino.h>
#include <SPI.h>

/** * ADS1298 Opcode and Register Definitions
 * Based on the TI ADS1298 Datasheet
 */
#define ID         0x00    // Device ID Register
#define READ       0x20    // Read Register Opcode (001nnnnn)
#define WRITE      0x40    // Write Register Opcode (010nnnnn)
#define CONFIG1    0x01    // Configuration Register 1 (SPS / Power Mode)
#define CONFIG2    0x02    // Configuration Register 2 (Test Signal)
#define CONFIG3    0x03    // Configuration Register 3 (Reference / RLD)
#define LOFF       0x04    // Lead-Off Control Register
#define RDATA      0x12    // Read Data by Command Opcode

/**
 * Hardware Pin Mapping
 * Configured for the ESP32 platform [cite: 17, 38]
 */
const int csPin = 10;       // Chip Select (Active Low)
const int drdyPin = 4;     // Data Ready Output (Active Low)
const int resetPin = 5;   // Hardware Reset
const int DIN = 11;        // MOSI
const int DOUT = 13;       // MISO
const int SCLK = 12;       // Serial Clock
const int START = 6;      // Start Conversion Control

/**
 * Function Prototypes for ADS1298 Interface
 */

// Handles the power-up and reset sequence of the AFE
void ADS1298rInit();

// Configures internal MUX, Gain, and Power registers for 12-lead ECG acquisition [cite: 12]
void ADS1298rSettings();

// Converts the 24-bit raw ADC output into a 32-bit signed integer for processing
int32_t convert24bitToSigned(uint32_t value);

// Low-level SPI abstraction for reading a specific register
uint8_t readRegister(byte reg);

// Sends a single-byte command (e.g., START, STOP, SDATAC) to the device
void send_command(uint8_t cmd);

// Low-level SPI abstraction for writing to a specific register
void writeRegister(byte reg, byte value);

#endif
