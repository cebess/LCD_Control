/*
  LCD_Control.cpp

  This module provides multi-LCD management for HD44780-compatible displays
  on I2C expander addresses. 
*/

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

static String padLine(const String &line, uint8_t cols)
{
    String result = line;
    while (result.length() < cols)
        result += ' ';
    return result;
}

static String makeBlankLine(uint8_t cols)
{
    String blank;
    blank.reserve(cols);
    for (uint8_t i = 0; i < cols; ++i)
        blank += ' ';
    return blank;
}

static void scrollDisplayBuffer(std::vector<String> &buffer, uint8_t cols)
{
    if (buffer.empty())
        return;

    for (size_t i = 1; i < buffer.size(); ++i)
        buffer[i - 1] = buffer[i];
    buffer.back() = makeBlankLine(cols);
}

static void refreshDisplayFromBuffer(hd44780_I2Cexp *lcd, const std::vector<String> &buffer)
{
    lcd->clear();
    for (uint8_t r = 0; r < buffer.size(); ++r)
    {
        lcd->setCursor(0, r);
        lcd->print(buffer[r]);
    }
}

static void advanceCursor(uint8_t &col, uint8_t &row, uint8_t cols, uint8_t rows, size_t count)
{
    if (count == 0)
        return;

    uint16_t newCol = col + count;
    if (newCol < cols)
    {
        col = newCol;
        return;
    }

    uint16_t overflow = newCol / cols;
    col = newCol % cols;
    row = (row + overflow) % rows;
}

static void writeTextToBuffer(std::vector<String> &buffer, uint8_t &col, uint8_t &row, uint8_t cols, uint8_t rows, const String &text)
{
    uint8_t currentCol = col;
    uint8_t currentRow = row;

    for (size_t i = 0; i < text.length(); ++i)
    {
        if (currentRow >= rows)
            break;
        buffer[currentRow][currentCol] = text[i];
        currentCol++;
        if (currentCol >= cols)
        {
            currentCol = 0;
            currentRow = (currentRow + 1) % rows;
        }
    }

    col = currentCol;
    row = currentRow;
}

void LCD_Control::Device::clear()
{
    lcd->clear();
    curCol = 0;
    curRow = 0;
    textBuffer.assign(rows, makeBlankLine(cols));
}

void LCD_Control::Device::setCursor(uint8_t col, uint8_t row)
{
    lcd->setCursor(col, row);
    curCol = col;
    curRow = row;
}

void LCD_Control::Device::print(const String& s)
{
    lcd->print(s);
    writeTextToBuffer(textBuffer, curCol, curRow, cols, rows, s);
}

void LCD_Control::Device::println(const String& s)
{
    if (curRow == rows - 1)
    {
        scrollDisplayBuffer(textBuffer, cols);
        refreshDisplayFromBuffer(lcd, textBuffer);
        curCol = 0;
        curRow = rows - 1;
    }

    lcd->setCursor(curCol, curRow);
    size_t available = cols - curCol;
    String out = s;
    if (out.length() > available)
        out = out.substring(0, available);

    out = padLine(out, available);
    lcd->print(out);

    for (size_t i = 0; i < out.length(); ++i)
        textBuffer[curRow][curCol + i] = out[i];

    if (curRow < rows - 1)
    {
        curCol = 0;
        curRow++;
    }
    else
    {
        curCol = 0;
    }
}

void LCD_Control::Device::print(const char* s)
{
    lcd->print(s);
    writeTextToBuffer(textBuffer, curCol, curRow, cols, rows, String(s));
}

void LCD_Control::Device::println(const char* s)
{
    if (curRow == rows - 1)
    {
        scrollDisplayBuffer(textBuffer, cols);
        refreshDisplayFromBuffer(lcd, textBuffer);
        curCol = 0;
        curRow = rows - 1;
    }

    lcd->setCursor(curCol, curRow);
    String out = String(s);
    size_t available = cols - curCol;
    if (out.length() > available)
        out = out.substring(0, available);

    out = padLine(out, available);
    lcd->print(out);

    for (size_t i = 0; i < out.length(); ++i)
        textBuffer[curRow][curCol + i] = out[i];

    if (curRow < rows - 1)
    {
        curCol = 0;
        curRow++;
    }
    else
    {
        curCol = 0;
    }
}

void LCD_Control::Device::print(char c)
{
    lcd->print(c);
    writeTextToBuffer(textBuffer, curCol, curRow, cols, rows, String(c));
}

void LCD_Control::Device::print(unsigned long n, int base)
{
    String text = String(n, base);
    lcd->print(text);
    writeTextToBuffer(textBuffer, curCol, curRow, cols, rows, text);
}

void LCD_Control::Device::getCursor(uint8_t &col, uint8_t &row) const
{
    col = curCol;
    row = curRow;
}

String LCD_Control::Device::getText(uint8_t line) const
{
    if (line >= rows)
        return String("");

    String result = textBuffer[line];
    int len = result.length();
    while (len > 0 && result.charAt(len - 1) == ' ')
        len--;

    String trimmed;
    for (int i = 0; i < len; ++i)
        trimmed += result.charAt(i);

    return trimmed;
}

int LCD_Control::Device::getProp(hd44780_I2Cexp::I2CexpProp prop)
{
    return lcd->getProp(prop);
}

bool LCD_Control::Device::printAligned(uint8_t row, const String &text, Align align)
{
    int len = text.length();
    if (len > cols)
        return false;
    setCursor(0, row); // Move to the start of the row to clear it
    int start = 0; // Default to left align
    // Serial.println("text to print: " + text);
    // Serial.print("align: ");
    // Serial.println(align);
    if (align == Align::Center)
        start = (cols - len) / 2;
    else if (align == Align::Right)
        start = cols - len;
    // Serial.print("start at: ");
    // Serial.println(start);
    for (int i = 0; i < start; ++i)
        print(" ");
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