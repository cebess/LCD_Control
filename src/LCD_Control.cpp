#include "LCD_Control.h"

LCD_Control::LCD_Control()
{
    Wire.begin();
    initDevices();
}

LCD_Control::~LCD_Control()
{
    for (auto dev : devices)
    {
        delete dev;
    }
    devices.clear();
    lcds.clear();
}

void LCD_Control::initDevices()
{
    const uint8_t ranges[][2] = {{0x20, 0x27}, {0x38, 0x3F}};
    for (const auto &range : ranges)
    {
        for (uint8_t addr = range[0]; addr <= range[1]; addr++)
        {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0)
            {
                hd44780_I2Cexp *newLcd = new hd44780_I2Cexp(addr);
                if (newLcd->begin(DEFAULT_COLS, DEFAULT_ROWS) == 0) {
                    lcds.push_back(Device(newLcd, DEFAULT_COLS, DEFAULT_ROWS));
                    devices.push_back(newLcd);
                }
                else
                {
                    delete newLcd;
                }
            }
        }
    }
}

std::vector<LCD_Control::Device> LCD_Control::getLcds()
{
    return lcds;
}

void LCD_Control::clearAll()
{
    for (auto &d : lcds)
    {
        d.clear();
    }
}

LCD_Control::Device *LCD_Control::getLcdByAddress(uint8_t address)
{
    for (auto &d : lcds)
    {
        if (d.getProp(hd44780_I2Cexp::Prop_addr) == address)
        {
            return &d;
        }
    }
    return nullptr;
}

bool LCD_Control::Device::printAligned(uint8_t row, const String &text, Align align)
{
    int len = text.length();
    if (len > cols)
        return false;
    
    int start = 0;
    if (align == Align::Center)
        start = (cols - len) / 2;
    else if (align == Align::Right)
        start = cols - len;
    
    setCursor(0, row);
    for (int i = 0; i < cols; ++i)
        print(" ");
    
    setCursor((uint8_t)start, row);
    print(text);
    return true;
}

void LCD_Control::labelDisplays()
{
    for (size_t n = 0; n < lcds.size(); ++n)
    {
        int addr = lcds[n].getProp(hd44780_I2Cexp::Prop_addr);
        lcds[n].clear();
        lcds[n].printAligned(0, "LCD #" + String(n) + " ADDR 0x" + String(addr, HEX), Align::Left);
    }
}

void fatalError(int ecode)
{
    Serial.print("Error: ");
    Serial.println(ecode);
    while (1)
        delay(100);
}