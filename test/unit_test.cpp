#include <Arduino.h>
#include <unity.h>
#include <LCD_Control.h>

// Pointer to the controller instance
LCD_Control *lcdctrl = nullptr;

// Test: Constructor initializes and discovers LCDs
void test_finding_LCDs() {
    lcdctrl = new LCD_Control();
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    TEST_ASSERT_TRUE(lcds.size() > 0);
}

// Test: getLcds returns the device list
void test_getLcds() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    TEST_ASSERT_TRUE(lcds.size() > 0);
    // Verify we can iterate through devices
    for (const auto &dev : lcds) {
        TEST_ASSERT_NOT_NULL(dev.lcd);
    }
}

// Test: clearAll executes without error
void test_clearAll() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    lcdctrl->clearAll();
    // If we reach here, clearAll completed successfully
}

// Test: getLcdByAddress returns valid device or nullptr
void test_getLcdByAddress() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        // Get address of first LCD
        uint8_t addr = lcds[0].getProp(hd44780_I2Cexp::Prop_addr);
        
        // Find by valid address
        auto *dev = lcdctrl->getLcdByAddress(addr);
        TEST_ASSERT_NOT_NULL(dev);
        TEST_ASSERT_EQUAL(addr, dev->getProp(hd44780_I2Cexp::Prop_addr));
        
        // Find by invalid address should return nullptr
        auto *nullDev = lcdctrl->getLcdByAddress(0xFF);
        TEST_ASSERT_NULL(nullDev);
    }
}

// Test: Device clear
void test_device_clear() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].clear();
        // If we reach here, clear completed successfully
    }
}

// Test: Device setCursor
void test_device_setCursor() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].setCursor(0, 0);
        lcds[0].setCursor(lcds[0].cols - 1, lcds[0].rows - 1);
        // If we reach here, setCursor completed successfully
    }
}

// Test: Device print with various types
void test_device_print() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].clear();
        lcds[0].setCursor(0, 0);
        
        // Test print(String)
        lcds[0].print(String("Hello"));
        delay(500);        
        // Test print(const char*)
        lcds[0].print(" ");
        lcds[0].print("World");
        delay(500);
        // Test print(char)
        lcds[0].print('!');
        delay(500);
        
        // Test print(unsigned long)
        lcds[0].print((unsigned long)123);
        delay(500);        
        // Test print(unsigned long) with base
        lcds[0].setCursor(0, 1);
        lcds[0].print((unsigned long)255, HEX);
        delay(500);
    }
}

// Test: Device println with various types
void test_device_println() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].clear();
        lcds[0].setCursor(0, 0);
        
        // Test println(String)
        lcds[0].println(String("long line test"));
        delay(500);
        // Test println(const char*)
        lcds[0].println("short"); // should cover the long line test and clear it
        delay(500);
        // Test scroll behavior when on the last line
        uint8_t lastRow = lcds[0].rows - 1;
        lcds[0].clear();
        lcds[0].setCursor(0, lastRow);
        lcds[0].println("Bottom");
        uint8_t col, row;
        lcds[0].getCursor(col, row);
        TEST_ASSERT_EQUAL_UINT8(0, col);
        TEST_ASSERT_EQUAL_UINT8(lastRow, row);
        lcds[0].println("New Bottom");
        String secondLine = lcds[0].getText(1);
        TEST_ASSERT_EQUAL_STRING("New Bottom", secondLine.c_str());
        delay(500);
        lcds[0].println("Newest Bottom");
        secondLine = lcds[0].getText(1);
        TEST_ASSERT_EQUAL_STRING("Newest Bottom", secondLine.c_str());
    }
}

void test_device_println_scrolling() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    if (lcds.size() > 0) {
        lcds[0].clear();
        lcds[0].setCursor(0, 0);
        lcds[0].println("line1");
        String firstLine = lcds[0].getText(0);
        TEST_ASSERT_EQUAL_STRING("line1", firstLine.c_str());
        lcds[0].println("line2");
        String secondLine = lcds[0].getText(1);
        TEST_ASSERT_EQUAL_STRING("line2", secondLine.c_str());
        lcds[0].println("line3");
        firstLine = lcds[0].getText(0);
        secondLine = lcds[0].getText(1);
        TEST_ASSERT_EQUAL_STRING("line2", firstLine.c_str());  
        TEST_ASSERT_EQUAL_STRING("line3", secondLine.c_str());
    }
}

// Test: Device getText from buffered display rows
void test_device_getText() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].clear();
        lcds[0].print("Hello");
        lcds[0].setCursor(0, 1);
        lcds[0].print("World!");

        String firstLine = lcds[0].getText(0);
        String secondLine = lcds[0].getText(1);

        TEST_ASSERT_EQUAL_UINT8(5, firstLine.length());
        TEST_ASSERT_EQUAL_UINT8(6, secondLine.length());
        TEST_ASSERT_EQUAL_STRING("Hello", firstLine.c_str());
        TEST_ASSERT_EQUAL_STRING("World!", secondLine.c_str());
        TEST_ASSERT_EQUAL_STRING("", lcds[0].getText(2).c_str());
    }
}

// Test: Device printAligned with various alignments
void test_device_printAligned() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    auto lcds = lcdctrl->getLcds();
    
    if (lcds.size() > 0) {
        lcds[0].clear();
        
        // Test left align
        bool result = lcds[0].printAligned(0, String("Left"), LCD_Control::Align::Left);
        TEST_ASSERT_TRUE(result);
        String firstLine = lcds[0].getText(0);
        TEST_ASSERT_EQUAL_STRING("Left", firstLine.c_str());
        
        // Test center align
        result = lcds[0].printAligned(1, String("Center"), LCD_Control::Align::Center);
        TEST_ASSERT_TRUE(result);
        String secondLine = lcds[0].getText(1);
        int expectedSpaces = (lcds[0].cols - String("Center").length()) / 2;
        // Serial.print("Expected spaces for center align: ");
        // Serial.println(expectedSpaces);
        // String expectedLine = "";
        // for (int i = 0; i < expectedSpaces; i++) {
        //     expectedLine += " ";
        // }
        std::string expectedLine(expectedSpaces, ' ');
        expectedLine += "Center";
        // Serial.print("Expected line: '");
        // Serial.print(expectedLine.c_str()); 
        // Serial.println("'");
        // Serial.print("Actual line: '"); 
        // Serial.print(secondLine); 
        // Serial.println("'");    
        TEST_ASSERT_EQUAL_STRING(expectedLine.c_str(), secondLine.c_str());
        delay(500);
        result = lcds[0].printAligned(0, String("Right"), LCD_Control::Align::Right);
        TEST_ASSERT_TRUE(result);
        secondLine = lcds[0].getText(0);
        expectedSpaces = lcds[0].cols - String("Right").length();
        expectedLine = std::string(expectedSpaces, ' ') + "Right";  
        TEST_ASSERT_EQUAL_STRING(expectedLine.c_str(), secondLine.c_str());
        
        // Test with text too long (should fail)
        String longText = "";
        for (int i = 0; i < lcds[0].cols + 5; i++) {
            longText += "X";
        }
        result = lcds[0].printAligned(1, longText, LCD_Control::Align::Left);
        TEST_ASSERT_FALSE(result);
    }
}

// Test: labelDisplays
void test_labelDisplays() {
    TEST_ASSERT_NOT_NULL(lcdctrl);
    lcdctrl->labelDisplays();
    // If we reach here, labelDisplays completed successfully
}

// Test: Destructor cleanup
void test_destructor() {
    if (lcdctrl != nullptr) {
        delete lcdctrl;
        lcdctrl = nullptr;
        // If we reach here, destructor completed successfully
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
        Serial.println("Waiting for Serial...");
    };
    randomSeed(12345);

    UNITY_BEGIN();
    
    RUN_TEST(test_finding_LCDs);
    RUN_TEST(test_getLcds);
    RUN_TEST(test_labelDisplays);
    delay(500);
    RUN_TEST(test_clearAll);
    delay(500);
    RUN_TEST(test_getLcdByAddress);
    RUN_TEST(test_device_clear);
    delay(500);
    RUN_TEST(test_device_setCursor);
    delay(500);
    RUN_TEST(test_device_print);
    delay(500);
    RUN_TEST(test_device_println);
    delay(500);
    RUN_TEST(test_device_getText);
    delay(500);
    RUN_TEST(test_device_printAligned);
    delay(500);
    RUN_TEST(test_device_println_scrolling);
    RUN_TEST(test_destructor);

    UNITY_END();
}

void loop() {
    // not used
}
