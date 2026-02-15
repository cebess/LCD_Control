#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <vector>

class LCD_Control {
public:
    static const int DEFAULT_ROWS = 2;
    static const int DEFAULT_COLS = 16;
    
    enum class Align : uint8_t { Left, Center, Right };
    
    // Device class definition - wrapper for hd44780_I2Cexp with additional properties
    class Device : public hd44780_I2Cexp {
    private:
        uint8_t rows;
        uint8_t cols;
        
    public:
        Device(hd44780_I2Cexp* lcd, uint8_t c, uint8_t r) 
            : hd44780_I2Cexp(*lcd), rows(r), cols(c) {}
        
        uint8_t getRows() const { return rows; }
        uint8_t getCols() const { return cols; }
        
        bool printAligned(uint8_t row, const String &text, Align align);
        int getProp(I2CexpProp prop) const { return const_cast<Device*>(this)->hd44780_I2Cexp::getProp(prop); }
    };
    
    // Constructor/Destructor
    LCD_Control();
    ~LCD_Control();
    
    // Public methods
    void clearAll();
    Device* getLcdByAddress(uint8_t address);
    void labelDisplays();
    std::vector<Device> getLcds();
    
private:
    void initDevices();
    
    std::vector<Device> lcds;
    std::vector<hd44780_I2Cexp *> devices;
};

void fatalError(int ecode);

#endif