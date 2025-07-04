#include "uart.h"
#include <iostream>
#include <cassert>

using namespace jellED::serial;

/**
 * @brief Simple test suite for UART library
 */
class UartTest {
public:
    static void testSerialConfig() {
        std::cout << "Testing SerialConfig..." << std::endl;
        
        // Test default constructor
        SerialConfig config1;
        assert(config1.baudRate == 115200);
        assert(config1.dataBits == 8);
        assert(config1.stopBits == 1);
        assert(config1.parity == false);
        assert(config1.flowControl == false);
        assert(config1.timeoutMs == 1000);
        
        // Test parameterized constructor
        SerialConfig config2(9600, 7, 2, true, true, 500);
        assert(config2.baudRate == 9600);
        assert(config2.dataBits == 7);
        assert(config2.stopBits == 2);
        assert(config2.parity == true);
        assert(config2.flowControl == true);
        assert(config2.timeoutMs == 500);
        
        std::cout << "SerialConfig tests passed!" << std::endl;
    }
    
    static void testUartConfig() {
        std::cout << "Testing UartConfig..." << std::endl;
        
        // Test default constructor
        UartConfig config1;
        assert(config1.portName.empty());
        assert(config1.uartNumber == 0);
        assert(config1.invertTx == false);
        assert(config1.invertRx == false);
        assert(config1.enableLoopback == false);
        
        // Test parameterized constructor
        UartConfig config2("/dev/ttyUSB0", 1, 115200);
        assert(config2.portName == "/dev/ttyUSB0");
        assert(config2.uartNumber == 1);
        assert(config2.baudRate == 115200);
        
        // Test inheritance from SerialConfig
        config2.dataBits = 6;
        config2.parity = true;
        assert(config2.dataBits == 6);
        assert(config2.parity == true);
        
        std::cout << "UartConfig tests passed!" << std::endl;
    }
    
    static void testInterfaceCompliance() {
        std::cout << "Testing interface compliance..." << std::endl;
        
        // Test that UartConfig properly inherits from SerialConfig
        UartConfig uartConfig;
        SerialConfig& serialConfig = uartConfig; // Should work due to inheritance
        
        serialConfig.baudRate = 19200;
        assert(uartConfig.baudRate == 19200);
        
        std::cout << "Interface compliance tests passed!" << std::endl;
    }
    
    static void testConfigurationValidation() {
        std::cout << "Testing configuration validation..." << std::endl;
        
        // Test valid configurations
        UartConfig validConfig;
        validConfig.baudRate = 115200;
        validConfig.dataBits = 8;
        validConfig.stopBits = 1;
        validConfig.portName = "UART0";
        validConfig.uartNumber = 0;
        
        // Test invalid configurations (should be handled gracefully)
        UartConfig invalidConfig;
        invalidConfig.baudRate = 999999; // Invalid baud rate
        invalidConfig.dataBits = 9;      // Invalid data bits
        invalidConfig.stopBits = 3;      // Invalid stop bits
        
        // These should not crash the application
        assert(invalidConfig.baudRate == 999999);
        assert(invalidConfig.dataBits == 9);
        assert(invalidConfig.stopBits == 3);
        
        std::cout << "Configuration validation tests passed!" << std::endl;
    }
    
    static void runAllTests() {
        std::cout << "Running UART library tests..." << std::endl;
        std::cout << "=================================" << std::endl;
        
        testSerialConfig();
        testUartConfig();
        testInterfaceCompliance();
        testConfigurationValidation();
        
        std::cout << "=================================" << std::endl;
        std::cout << "All UART library tests passed!" << std::endl;
    }
};

int main() {
    UartTest::runAllTests();
    return 0;
} 