/*
  LCD_Control class

  Manages one or more HD44780-compatible LCDs attached via I2C expander
  devices. The implementation lives in LCD_Control.cpp and provides the
  following public behaviors and device helpers:

    - LCD_Control():
        Initializes the I2C bus and probes known expander address ranges
        (0x20-0x27 and 0x38-0x3F). Discovered displays are initialized and
        wrapped in `Device` entries.

    - ~LCD_Control():
        Frees allocated `hd44780_I2Cexp` objects and clears internal lists.

    - initDevices():
        Internal: performs the I2C scan and constructs `Device` wrappers for
        successfully initialized LCDs.

    - getLcds():
        Returns a copy of the discovered `Device` wrappers.

    - clearAll():
        Calls `clear()` on every managed `Device`.

    - getLcdByAddress(uint8_t):
        Returns a pointer to the `Device` whose expander address matches the
        provided value, or `nullptr` if not found.

    - labelDisplays():
        Writes an identifying label (index and I2C address) to each discovered
        LCD; useful for debugging and verification.

  Device helper methods (defined in LCD_Control.cpp):

    - Device::clear():
        Calls `hd44780_I2Cexp::clear()` on the underlying display.

    - Device::setCursor(uint8_t col, uint8_t row):
        Sets the cursor position on the underlying display.

    - Device::print(...):
        Overloads for `String`, `const char*`, `char`, and `unsigned long`
        (with an optional `base`) forward to the underlying driver's
        `print()` implementation.

    - Device::println(const String&) / Device::println(const char*):
        The hd44780 I2C expander does not support a hardware newline. These
        helpers emulate `println()` by padding the provided text to the
        display width and calling `print()` so the full row is overwritten.

    - Device::getCursor(uint8_t &col, uint8_t &row) const:
        Returns the internally tracked cursor position so client code can
        find out where the next write will occur.

    - Device::getText(uint8_t line) const:
        Returns the current text contents of a buffered display row,
        trimming trailing spaces from the stored line.

    - Device::getProp(hd44780_I2Cexp::I2CexpProp):
        Forwards to the underlying `hd44780_I2Cexp::getProp()` to read driver
        properties (for example the expander address).

    - Device::printAligned(uint8_t row, const String &text, Align align):
        Clears the target row and prints `text` left/center/right aligned.
        Returns `false` if `text.length() > cols`.

    - fatalError(int):
        Prints an error code to Serial and halts execution (infinite loop).
*/
#pragma once

#include <Wire.h>
#include <vector>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#define DEFAULT_COLS 16
#define DEFAULT_ROWS 2

// LCD_Control manages one or more HD44780-compatible LCDs attached via I2C.
// It probes known expander address ranges, initializes discovered LCDs,
// and provides a lightweight wrapper type for printing and row-aligned output.
class LCD_Control {
public:
    enum Align { Left, Center, Right };

    struct Device {
        hd44780_I2Cexp* lcd;
        uint8_t cols, rows;
        uint8_t curCol, curRow;
        std::vector<String> textBuffer;

        // Construct a device wrapper around an initialized hd44780_I2Cexp instance.
        Device(hd44780_I2Cexp* l, uint8_t c, uint8_t r)
            : lcd(l), cols(c), rows(r), curCol(0), curRow(0)
        {
            String blank;
            blank.reserve(c);
            for (uint8_t i = 0; i < c; ++i)
                blank += ' ';
            textBuffer.assign(rows, blank);
        }

        // Clear the LCD and prepare for new text.
        void clear();

        // Move the cursor before printing text.
        void setCursor(uint8_t col, uint8_t row);

        // Print text or values to the current cursor position.
        void print(const String& s);
        void print(const char* s);
        void print(char c);
        void print(unsigned long n, int base = DEC);
        void println(const String& s);
        void println(const char* c);

        // Get the current cursor location.
        void getCursor(uint8_t &col, uint8_t &row) const;

        // Get the tracked text for a row from the display buffer.
        String getText(uint8_t line) const;

        // Read a property from the underlying LCD driver.
        int getProp(hd44780_I2Cexp::I2CexpProp prop);

        // Print text on the specified row, aligned left/center/right.
        // The row is cleared first so the full line is overwritten.
        bool printAligned(uint8_t row, const String &text, Align align);
    };

    // Scan the I2C bus for supported LCD expander addresses and initialize devices.
    LCD_Control();

    // Free allocated LCD objects and clear internal device lists.
    ~LCD_Control();

    // Return a copy of the discovered device wrappers.
    std::vector<Device> getLcds();

    // Clear every managed LCD display.
    void clearAll();

    // Find a managed LCD by its I2C address.
    Device* getLcdByAddress(uint8_t address);

    // Write a label to each discovered LCD showing index and I2C address.
    void labelDisplays();

private:
    // Internal device detection and initialization routine.
    void initDevices();

    std::vector<Device> lcds;
    std::vector<hd44780_I2Cexp*> devices;
};
