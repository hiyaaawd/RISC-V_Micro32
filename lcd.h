


#ifndef LCD_DRIVER_H

#define LCD_DRIVER_H

#include <cstdint>

// Namespace for LCD driver functions
namespace LCDDriver {
    // Function to send a command to the LCD
    void sendCommand(uint8_t cmd);

    // Function to send data to the LCD
    void sendData(uint8_t data);

    // Function to initialize the LCD
    void initialize();

    // Function to draw a pixel on the LCD
    void drawPixel(int x, int y, uint16_t color);

    // Function to clear the screen
    void clearScreen(uint16_t color);

    // Print a null-terminated string at x,y with 16-bit color
    void Print(const char* str, int x, int y, uint16_t color);

    // Print a signed integer at x,y with 16-bit color
    void Print(int number, int x, int y, uint16_t color);
}

#endif // LCD_DRIVER_H
