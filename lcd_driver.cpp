

#include <cstdint>
#include <cstdio>

// Define memory-mapped registers for SPI and GPIO
#define SPI_BASE 0x60002000  // Replace with the actual SPI base address
#define SPI_CMD_REG (SPI_BASE + 0x00)
#define SPI_DATA_REG (SPI_BASE + 0x08)
#define GPIO_BASE 0x60004000 // Replace with the actual GPIO base address
#define GPIO_OUT_REG (GPIO_BASE + 0x04)

// LCD Driver namespace
namespace LCDDriver {

    // Function to send a command to the LCD
    void sendCommand(uint8_t cmd) {
        *(volatile uint32_t*)SPI_CMD_REG = 0;  // Set to command mode
        *(volatile uint32_t*)SPI_DATA_REG = cmd;
        while (*(volatile uint32_t*)SPI_CMD_REG & (1 << 0));  // Wait for transmission to complete
    }

    // Function to send data to the LCD
    void sendData(uint8_t data) {
        *(volatile uint32_t*)SPI_CMD_REG = 1;  // Set to data mode
        *(volatile uint32_t*)SPI_DATA_REG = data;
        while (*(volatile uint32_t*)SPI_CMD_REG & (1 << 0));  // Wait for transmission to complete
    }

    // Function to initialize the LCD
    void initialize() {
        // Reset the LCD (toggle the reset pin)
        *(volatile uint32_t*)GPIO_OUT_REG &= ~(1 << 5);  // Set GPIO5 low (reset pin)
        for (volatile int i = 0; i < 100000; i++);       // Delay
        *(volatile uint32_t*)GPIO_OUT_REG |= (1 << 5);   // Set GPIO5 high

        // Send initialization commands
        sendCommand(0x01);  // Software reset
        for (volatile int i = 0; i < 120000; i++);       // Delay
        sendCommand(0x11);  // Exit sleep mode
        for (volatile int i = 0; i < 120000; i++);       // Delay
        sendCommand(0x29);  // Turn on the display
    }

    // Function to draw a pixel on the LCD
    void drawPixel(int x, int y, uint16_t color) {
        sendCommand(0x2A);  // Set column address
        sendData(x >> 8);
        sendData(x & 0xFF);
        sendCommand(0x2B);  // Set row address
        sendData(y >> 8);
        sendData(y & 0xFF);
        sendCommand(0x2C);  // Write memory
        sendData(color >> 8);
        sendData(color & 0xFF);
    }

    // Function to clear the screen
    void clearScreen(uint16_t color) {
        sendCommand(0x2C);  // Memory write
        for (int i = 0; i < 240 * 320; i++) {  // Assuming 240x320 resolution
            sendData(color >> 8);
            sendData(color & 0xFF);
        }
    }
    // Function to print a string or integer to the LCD
    void Print(const char* str, int x, int y, uint16_t color) {
        int offset = 0;
        while (*str) {
            drawPixel(x + offset, y, color);  // Draw each character
            offset += 8;  // Move to the next character position
            str++;
        }
    }

    void Print(int number, int x, int y, uint16_t color) {
        char buffer[12];  // Buffer to hold the integer as a string
        snprintf(buffer, sizeof(buffer), "%d", number);  // Convert integer to string
        Print(buffer, x, y, color);  // Reuse the string Print function
    }
}