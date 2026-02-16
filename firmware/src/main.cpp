#include <Arduino.h>
#include <SPI.h>
#include "ADS1298.h"

// Buffer to store 24-bit ADC data converted to 32-bit signed integers
int32_t channelValues[8];

void setup() 
{
    // Initialize GPIO pins for ADS1298 hardware control interface
    pinMode(csPin, OUTPUT);
    pinMode(drdyPin, INPUT);
    pinMode(resetPin, OUTPUT);
    pinMode(DIN, OUTPUT); 
    pinMode(DOUT, INPUT); 
    pinMode(SCLK, OUTPUT);
    pinMode(START, OUTPUT);

    // Hardware Power-On Reset (POR) Sequence
    digitalWrite(csPin, HIGH); 
    delay(10);
    digitalWrite(resetPin, LOW); // Pull reset low to initialize the chip
    delay(10);
    digitalWrite(resetPin, HIGH); 
    delay(10);
    digitalWrite(START, HIGH);   // Pull START high to begin conversions
    delay(10);

    // SPI Configuration: 2MHz clock, MSB first, and SPI Mode 1 (CPOL=0, CPHA=1)
    SPI.begin();
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    Serial.begin(115200); 

    // Device Initialization: Set internal registers and operating modes
    ADS1298rInit();
    delay(10);

    // Configure biopotential acquisition settings (Gain, Sample Rate, MUX)
    ADS1298rSettings();
    delay(10);
    send_command(0x08); // START command to initiate data output

    // Verify communication by reading the unique Device ID register
    Serial.print("Device ID: ");
    Serial.println(readRegister(ID), HEX);
}

void readChannels() 
{
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    
    // Check if Data Ready (DRDY) is low, indicating new conversion results are available
    if (digitalRead(drdyPin) == LOW) 
    { 
        digitalWrite(csPin, LOW); // Activate chip select
        delayMicroseconds(5); 
        SPI.transfer(RDATA);      // Send Read Data Command
        
        // Read 24-bit Status Word (contains fault detection and lead-off flags)
        uint32_t status = 0;
        for (int i = 0; i < 3; i++) 
        {
            status = (status << 8) | SPI.transfer(0x00);
        }
        
        // Iterate through all 8 channels to retrieve 24-bit sample data
        for (int channel = 0; channel < 8 ; channel++) 
        {
            uint32_t value = 0;
            for (int i = 0; i < 3; i++) { 
                // Receive 3 bytes per channel (24-bit resolution)
                value = (value << 8) | SPI.transfer(0x00); 
            }
            // Convert 24-bit two's complement value to a signed 32-bit integer
            channelValues[channel] = convert24bitToSigned(value); 
        }

        // Output processed channel data via Serial for visualization/logging
        // These calculations derive the 12-lead ECG signals from the 8 physical channels
        Serial.print(channelValues[7]); Serial.print(",");
        Serial.print(channelValues[3]); Serial.print(",");
        Serial.print(channelValues[4]); Serial.print(",");
        Serial.print(channelValues[5]); Serial.print(",");
        Serial.print(channelValues[6]); Serial.print(",");
        Serial.print(channelValues[0]); Serial.print(",");
        Serial.print(channelValues[1]); Serial.print(",");
        Serial.print(channelValues[2]); Serial.print(",");
        
        // Differential lead calculations (Lead III, aVR, aVL, aVF logic)
        Serial.print(channelValues[2] - channelValues[1]); Serial.print(",");
        Serial.print(-((channelValues[1] + channelValues[2]) / 2)); Serial.print(",");
        Serial.print(channelValues[1] - channelValues[2] / 2); Serial.print(",");
        Serial.println(channelValues[2] - channelValues[1] / 2);
        
        digitalWrite(csPin, HIGH); // Deactivate chip select
    }
    SPI.endTransaction();
}

void loop() 
{
    // Continuously poll for new sensor data
    readChannels();
}
