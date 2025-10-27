
#include "drivers/lcd.h"
#include <cstdint>

namespace LCDDriver {
    void Print(const char* str, int x, int y, uint16_t color);
    void Print(int number, int x, int y, uint16_t color);
}

namespace lcd = LCDDriver;

extern "C" void kernel_main() {
    uint32_t a1_value = 0;

    // Inline assembly to read the value of the a1
    asm volatile("mv %0, a1" : "=r"(a1_value));

    lcd::initialize();
    lcd::clearScreen(0x0000);  // Black background

    lcd::Print("Hello, World!", 0, 0, 0xFFFF);
    lcd::Print("Welcome to Micro32!", 0, 16, 0xFFFF);


    char hex_buffer[11];  // "0x" + 8 hex digits + null terminator
    hex_buffer[0] = '0';
    hex_buffer[1] = 'x';

    for (int i = 0; i < 8; i++) {
        uint8_t nibble = (a1_value >> ((7 - i) * 4)) & 0xF;
        hex_buffer[i + 2] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    hex_buffer[10] = '\0';

    lcd::Print("a1 register:", 0, 32, 0xFFFF);
    lcd::Print(hex_buffer, 0, 48, 0xFFFF);

    while (true) {
        // infinite loop to keep kernel running
    }
}
