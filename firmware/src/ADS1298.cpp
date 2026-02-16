#include "ADS1298.h"

/**
 * Initializes the ADS1298 using its internal Command Opcode sequence.
 * 0x02 = WAKEUP (Exit Standby)
 * 0x06 = RESET (Reset registers to default)
 * 0x11 = SDATAC (Stop Data Continuous mode to allow register access)
 */
void ADS1298rInit() 
{
    send_command(0x02); // Wake up the device
    delay(10);
    send_command(0x06); // Software reset to ensure known state
    delay(10);
    send_command(0x11); // Stop Continuous Read mode to enable RREG/WREG commands
    delay(10);
}

/**
 * Reads a single 8-bit register from the ADS1298.
 * Follows the SPI protocol: [Command Byte] [Number of Registers - 1] [Data]
 */
uint8_t readRegister(byte reg) 
{
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    uint8_t result, result2;

    digitalWrite(csPin, LOW);
    delayMicroseconds(5);

    // Opcode 1: READ command OR'ed with register address
    SPI.transfer(READ | reg);
    // Opcode 2: Number of registers to read minus 1 (0x00 for 1 register)
    SPI.transfer(0x00);

    // Receive the register data
    result = SPI.transfer(0x00);
    result2 = SPI.transfer(0x00); 

    digitalWrite(csPin, HIGH);

    Serial.print("BYTE 1: ");
    Serial.println(result, HEX);
    
    SPI.endTransaction();
    return result;
}

/**
 * Sends a single-byte Command Opcode to the device.
 * Used for system control (RESET, START, STOP, WAKEUP, SDATAC).
 */
void send_command(uint8_t cmd) 
{
    digitalWrite(csPin, LOW);
    delayMicroseconds(5);
    SPI.transfer(cmd);
    delayMicroseconds(10); // Ensure t_SDECODE timing requirement is met
    digitalWrite(csPin, HIGH);
    delay(10);
}

/**
 * Writes an 8-bit value to a specific ADS1298 register.
 */
void writeRegister(byte reg, byte value) 
{
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    digitalWrite(csPin, LOW);
    delayMicroseconds(5);
    
    // Opcode 1: WRITE command OR'ed with register address
    SPI.transfer(WRITE | reg);
    // Opcode 2: Number of registers to write minus 1
    SPI.transfer(0x00);
    // Data byte to write
    SPI.transfer(value);
    
    digitalWrite(csPin, HIGH);
    delay(10);
    SPI.endTransaction();
}

/**
 * Configures the internal ADS1298 signal chain and front-end settings.
 * CONFIG1 (0xC6): High-Resolution mode, 500 SPS.
 * CONFIG3 (0xCC): Internal reference enabled (2.4V), RLD enabled.
 * CHnSET: Configures Gain (6x) and MUX (Normal electrode input).
 */
void ADS1298rSettings() 
{
    writeRegister(CONFIG1, 0xC6); // Mode: HR, Data Rate: 500 SPS 
    delay(10);
    writeRegister(CONFIG2, 0x20); // Test signal configuration
    delay(10);
    writeRegister(CONFIG3, 0xCC); // Power up internal reference and RLD 
    delay(10);
    writeRegister(LOFF, 0x00);    // Lead-off detection disabled
    delay(10);
    
    // Configure all 8 Channels: Gain = 6x, Normal electrode input (0x10)
    for (byte i = 0x05; i <= 0x0C; i++) {
        writeRegister(i, 0x10);
        delay(10);
    }

    // Configure Wilson Central Terminal (WCT) and Right Leg Drive (RLD) routing
    writeRegister(0x0D, 0x06); // RLD_SENS settings
    delay(10);
    
    // Remaining registers (0x0E to 0x17) set to default/grounded state
    for (byte i = 0x0E; i <= 0x17; i++) {
        writeRegister(i, 0x00);
        delay(10);
    }
    
    writeRegister(0x18, 0x0B); // GPIO Settings
    delay(10);
    writeRegister(0x19, 0xD4); // PACE Detection & WCT settings
    delay(10);
}

/**
 * Converts raw 24-bit two's complement ADC data into a signed 32-bit integer.
 * This is critical for maintaining the correct polarity of biopotential signals.
 */
int32_t convert24bitToSigned(uint32_t value) 
{
    // If the 24th bit (MSB) is 1, the number is negative.
    // Perform sign extension to 32 bits.
    if (value & 0x800000) value |= 0xFF000000;
    return (int32_t)value;
}
