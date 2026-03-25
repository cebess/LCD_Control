#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

#include <Wire.h>
#include <vector>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#define DEFAULT_COLS 16
#define DEFAULT_ROWS 2

class LCD_Control {
public:
    enum Align { Left, Center, Right };

    struct Device {
        hd44780_I2Cexp* lcd;
        uint8_t cols, rows;

        Device(hd44780_I2Cexp* l, uint8_t c, uint8_t r) : lcd(l), cols(c), rows(r) {}

        void clear() { lcd->clear(); }
        void setCursor(uint8_t col, uint8_t row) { lcd->setCursor(col, row); }
        void print(const String& s) { lcd->print(s); }
        int getProp(hd44780_I2Cexp::I2CexpProp prop) { return lcd->getProp(prop); }

        bool printAligned(uint8_t row, const String &text, Align align);
    };

    LCD_Control();
    ~LCD_Control();

    std::vector<Device> getLcds();
    void clearAll();
    Device* getLcdByAddress(uint8_t address);
    void labelDisplays();

private:
    void initDevices();

    std::vector<Device> lcds;
    std::vector<hd44780_I2Cexp*> devices;
};

#endif