#!/bin/bash

# jellED ESP32 Deployment Script
# Supports: compile, upload, and compile+upload operations

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
PROJECT_DIR="."
ENVIRONMENT="az-delivery-devkit-v4"
# DEFAULT_PORT="/dev/ttyUSB0"
DEFAULT_PORT="/dev/tty.usbserial-0001"
DEFAULT_BAUD="115200"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "jellED ESP32 Deployment Script"
    echo ""
    echo "Usage: $0 [OPTIONS] COMMAND"
    echo ""
    echo "Commands:"
    echo "  compile     Compile the project"
    echo "  upload      Upload compiled firmware to ESP32"
    echo "  deploy      Compile and upload (default)"
    echo "  clean       Clean build artifacts"
    echo "  monitor     Open serial monitor"
    echo ""
    echo "Options:"
    echo "  -p, --port PORT    Serial port (default: $DEFAULT_PORT)"
    echo "  --auto-port        Automatically discover ESP32 port"
    echo "  -b, --baud BAUD    Baud rate (default: $DEFAULT_BAUD)"
    echo "  -e, --env ENV      PlatformIO environment (default: $ENVIRONMENT)"
    echo "  --list-ports       List available serial ports"
    echo "  -h, --help         Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 compile                    # Compile only"
    echo "  $0 upload                     # Upload only"
    echo "  $0 deploy                     # Compile and upload"
    echo "  $0 --auto-port deploy         # Auto-discover port and deploy"
    echo "  $0 -p /dev/ttyUSB1 upload     # Upload to specific port"
    echo "  $0 -b 921600 deploy           # Use different baud rate"
}

# Function to check if PlatformIO is installed
check_platformio() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO is not installed or not in PATH"
        echo "Please install PlatformIO: https://platformio.org/install"
        exit 1
    fi
}

# Function to check if we're in the correct directory
check_project_dir() {
    if [ ! -f "$PROJECT_DIR/platformio.ini" ]; then
        print_error "PlatformIO project not found at $PROJECT_DIR"
        echo "Please run this script from the project root directory"
        exit 1
    fi
}

# Function to find available serial ports
find_serial_ports() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        ls /dev/tty.usbserial* /dev/tty.usbmodem* /dev/tty.SLAB_USBtoUART* 2>/dev/null || true
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true
    else
        # Windows or other
        echo "Please specify port manually with -p option"
    fi
}

# Function to discover ESP32 port automatically
discover_esp32_port() {
    print_status "Discovering ESP32 port..."

    local found_port=""

    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS - check for ESP32 devices using system_profiler
        if command -v system_profiler &> /dev/null; then
            # Look for ESP32 devices in USB devices
            local esp_devices=$(system_profiler SPUSBDataType 2>/dev/null | grep -i "esp\|espressif" -A 5 -B 5 | grep "Product ID" | head -1)
            if [ -n "$esp_devices" ]; then
                print_status "ESP32 device detected via system_profiler"
                # Find corresponding serial port
                for port in /dev/tty.usbserial* /dev/tty.usbmodem* /dev/tty.SLAB_USBtoUART*; do
                    if [ -e "$port" ]; then
                        found_port="$port"
                        break
                    fi
                done
            fi
        fi

        # Fallback: try common ESP32 port patterns
        if [ -z "$found_port" ]; then
            print_status "Trying fallback port detection..."
            for port in /dev/tty.usbserial* /dev/tty.usbmodem* /dev/tty.SLAB_USBtoUART*; do
                if [ -e "$port" ]; then
                    found_port="$port"
                    break
                fi
            done
        fi

    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux - check for ESP32 devices using lsusb
        if command -v lsusb &> /dev/null; then
            # Look for ESP32 devices (common vendor IDs: 10c4, 1a86, 303a, 0403, 1d50)
            local esp_devices=$(lsusb 2>/dev/null | grep -E "(10c4|1a86|303a|0403|1d50)" | head -1)
            if [ -n "$esp_devices" ]; then
                print_status "ESP32 device detected via lsusb: $esp_devices"
                # Find corresponding serial port
                for port in /dev/ttyUSB* /dev/ttyACM*; do
                    if [ -e "$port" ]; then
                        found_port="$port"
                        break
                    fi
                done
            fi
        fi

        # Fallback: try common ESP32 port patterns
        if [ -z "$found_port" ]; then
            print_status "Trying fallback port detection..."
            for port in /dev/ttyUSB* /dev/ttyACM*; do
                if [ -e "$port" ]; then
                    found_port="$port"
                    break
                fi
            done
        fi
    else
        print_warning "Auto-discovery not supported on this OS. Please specify port manually."
        return 1
    fi

    if [ -n "$found_port" ]; then
        print_success "Found ESP32 on port: $found_port"
        echo "$found_port"
        return 0
    else
        print_warning "No ESP32 device found automatically"
        print_status "Available ports:"
        find_serial_ports
        return 1
    fi
}

# Function to compile the project
compile_project() {
    print_status "Compiling jellED project..."
    cd "$PROJECT_DIR"

    if pio run -e "$ENVIRONMENT"; then
        print_success "Compilation completed successfully"
    else
        print_error "Compilation failed"
        exit 1
    fi
}

# Function to upload to ESP32
upload_firmware() {
    print_status "Uploading firmware to ESP32..."
    cd "$PROJECT_DIR"

    # Check if firmware exists
    if [ ! -f ".pio/build/$ENVIRONMENT/firmware.bin" ]; then
        print_warning "Firmware not found. Compiling first..."
        compile_project
    fi

    echo "Uploading firmware to $PORT at ${BAUD} baud..."

    # Upload command
    if pio run -e "$ENVIRONMENT" -t upload --upload-port "$PORT"; then
        print_success "Upload completed successfully"
    else
        print_error "Upload failed"
        echo "Common solutions:"
        echo "  - Check if ESP32 is connected and in bootloader mode"
        echo "  - Verify the correct serial port with: $0 --list-ports"
        echo "  - Try holding BOOT button while uploading"
        exit 1
    fi
}

# Function to clean build artifacts
clean_project() {
    print_status "Cleaning build artifacts..."
    cd "$PROJECT_DIR"

    if pio run -e "$ENVIRONMENT" -t clean; then
        print_success "Clean completed successfully"
    else
        print_error "Clean failed"
        exit 1
    fi
}

# Function to open serial monitor
monitor_serial() {
    print_status "Opening serial monitor on $PORT at ${BAUD} baud..."
    cd "$PROJECT_DIR"

    if pio device monitor --port "$PORT" --baud "$BAUD"; then
        print_success "Serial monitor closed"
    else
        print_error "Failed to open serial monitor"
        exit 1
    fi
}

# Function to list available ports
list_ports() {
    print_status "Available serial ports:"
    find_serial_ports
}

# Parse command line arguments
COMMAND="print"  # Default command
PORT="$DEFAULT_PORT"
BAUD="$DEFAULT_BAUD"
AUTO_DISCOVER=false

while [[ $# -gt 0 ]]; do
    case $1 in
        compile|upload|deploy|clean|monitor)
            COMMAND="$1"
            shift
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -b|--baud)
            BAUD="$2"
            shift 2
            ;;
        -e|--env)
            ENVIRONMENT="$2"
            shift 2
            ;;
        --list-ports)
            list_ports
            exit 0
            ;;
        --auto-port)
            AUTO_DISCOVER=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
main() {
    print_status "jellED ESP32 Deployment Script"
    print_status "Project: $PROJECT_DIR"
    print_status "Environment: $ENVIRONMENT"

    # Handle auto-discovery if requested
    if [ "$AUTO_DISCOVER" = true ]; then
        discovered_port=$(discover_esp32_port)
        if [ $? -eq 0 ]; then
            PORT="$discovered_port"
            print_success "Using auto-discovered port: $PORT"
        else
            print_error "Failed to discover ESP32 port automatically. Please specify manually with -p."
            exit 1
        fi
    fi

    print_status "Port: $PORT"
    print_status "Baud: $BAUD"
    echo ""

    # Pre-flight checks
    check_platformio
    check_project_dir

    # Execute command
    case $COMMAND in
        compile)
            compile_project
            ;;
        upload)
            upload_firmware
            ;;
        deploy)
            print_status "Deploying (compile + upload)..."
            compile_project
            upload_firmware
            ;;
        clean)
            clean_project
            ;;
        monitor)
            monitor_serial
            ;;
        print)
            print_status "Printing only"
            ;;
        *)
            print_error "Unknown command: $COMMAND"
            show_usage
            exit 1
            ;;
    esac

    print_success "Operation completed successfully!"
}

# Run main function
main "$@"
