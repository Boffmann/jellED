#!/bin/bash

# Configuration
RASPI_USER="hendrik"
RASPI_HOST="192.168.178.36"
REPO_PATH="/home/hendrik/repos/jellED"
RASPI_PATH="${REPO_PATH}/jellED/test/test_serial"

function build_raspi_file() {
    g++ -std=c++17 -Wall -Wextra -pedantic \
    -I../../raspi/include \
    -I../../core \
    -I../../core/serial \
    -I../../core/beatDetection/include \
    -I../../core/patternEngine \
    -I../../core/patternEngine/config \
    -I../../core/patternEngine/pattern \
    -I/usr/local/include \
    -I/opt/homebrew/include \
    -L/usr/local/lib \
    -L/opt/homebrew/lib \
    raspi.cpp \
    ../../raspi/src/serial/RaspiUart.cpp \
    ../../raspi/src/utils/raspicrono.cpp \
    ../../raspi/src/utils/raspilogger.cpp \
    ../../raspi/src/utils/raspiutils.cpp \
    -lpthread
}

function read_raspberry_pi_output() {
    echo "Connecting to Raspberry Pi to read stdout..."
    
    # SSH into Raspberry Pi and monitor the output
    ssh -t ${RASPI_USER}@${RASPI_HOST} << EOF
        echo "Connected to Raspberry Pi for output monitoring"
        
        # Set paths on Raspberry Pi
        REPO_PATH="${REPO_PATH}"
        RASPI_PATH="${RASPI_PATH}"
        
        # Navigate to the test directory
        cd \${RASPI_PATH}
        if [ \$? -ne 0 ]; then
            echo "Error: Could not navigate to \${RASPI_PATH}"
            exit 1
        fi
        
        echo "Monitoring output in directory: \$(pwd)"
        echo "Available files:"
        ls -la
        
        # Check if raspi_test is running
        if pgrep -f "raspi_test" > /dev/null; then
            echo "raspi_test is currently running. Process info:"
            ps aux | grep raspi_test | grep -v grep
            
            echo "Monitoring output (press Ctrl+C to stop):"
            # Use tail to follow the output if it's being written to a file
            # or use strace to monitor system calls if needed
            tail -f /dev/null  # Placeholder - will be replaced with actual monitoring
        else
            echo "raspi_test is not currently running."
            echo "Would you like to:"
            echo "1. Start raspi_test in background"
            echo "2. View recent output from a log file"
            echo "3. Exit"
            read -p "Enter choice (1-3): " choice
            
            case \$choice in
                1)
                    echo "Starting raspi_test in background..."
                    nohup ./raspi_test > raspi_test.log 2>&1 &
                    echo "raspi_test started with PID: \$!"
                    echo "Monitoring output:"
                    tail -f raspi_test.log
                    ;;
                2)
                    if [ -f "raspi_test.log" ]; then
                        echo "Recent output from raspi_test.log:"
                        tail -20 raspi_test.log
                    else
                        echo "No log file found."
                    fi
                    ;;
                3)
                    echo "Exiting..."
                    exit 0
                    ;;
                *)
                    echo "Invalid choice. Exiting..."
                    exit 1
                    ;;
            esac
        fi
EOF
}

function build_raspi_program_remote() {
    echo "Connecting to Raspberry Pi to run and monitor..."
    
    # SSH into Raspberry Pi and execute commands with output monitoring
    ssh -t ${RASPI_USER}@${RASPI_HOST} << EOF
        echo "Connected to Raspberry Pi"
        
        # Set paths on Raspberry Pi
        REPO_PATH="${REPO_PATH}"
        RASPI_PATH="${RASPI_PATH}"
        
        # Navigate to the repo directory
        echo "Changing to \${REPO_PATH}"
        cd \${REPO_PATH}
        if [ \$? -ne 0 ]; then
            echo "Error: Could not navigate to \${REPO_PATH}"
            exit 1
        fi
        echo "Changed to directory: \$(pwd)"
        
        # Perform git pull
        echo "Performing git pull..."
        git pull
        if [ \$? -ne 0 ]; then
            echo "Warning: git pull failed, but continuing..."
        fi

        # Navigate to the test directory
        echo "Changing to \${RASPI_PATH}"
        cd \${RASPI_PATH}
        if [ \$? -ne 0 ]; then
            echo "Error: Could not navigate to \${RASPI_PATH}"
            exit 1
        fi
        echo "Changed to directory: \$(pwd)"
        
        # Build the raspi.cpp file
        echo "Building raspi.cpp..."
        g++ -std=c++17 -Wall -Wextra -pedantic \
            -I../../raspi/include \
            -I../../core \
            -I../../core/serial \
            -I../../core/beatDetection/include \
            -I../../core/patternEngine \
            -I../../core/patternEngine/config \
            -I../../core/patternEngine/pattern \
            -I/usr/local/include \
            -I/opt/homebrew/include \
            -L/usr/local/lib \
            -L/opt/homebrew/lib \
            -o raspi_test \
            raspi.cpp \
            ../../raspi/src/serial/RaspiUart.cpp \
            ../../raspi/src/utils/raspicrono.cpp \
            ../../raspi/src/utils/raspilogger.cpp \
            ../../raspi/src/utils/raspiutils.cpp \
            -lpthread
        
        if [ \$? -ne 0 ]; then
            echo "Error: Build failed"
            exit 1
        fi
        echo "Build successful"
EOF
}

function run_and_monitor_raspberry_pi() {
    echo "Connecting to Raspberry Pi to run and monitor..."
    
    # SSH into Raspberry Pi and execute commands with output monitoring
    ssh -t ${RASPI_USER}@${RASPI_HOST} << EOF
        echo "Connected to Raspberry Pi"
        
        # Set paths on Raspberry Pi
        RASPI_PATH="${RASPI_PATH}"

        # Navigate to the test directory
        echo "Changing to \${RASPI_PATH}"
        cd \${RASPI_PATH}
        if [ \$? -ne 0 ]; then
            echo "Error: Could not navigate to \${RASPI_PATH}"
            exit 1
        fi
        echo "Changed to directory: \$(pwd)"
        
        # Execute the test with output monitoring
        echo "Executing raspi_test with output monitoring..."
        echo "Press Ctrl+C to stop the test"
        ./raspi_test 2>&1 > raspi_test.log &
        sleep 5
        cat raspi_test.log | grep "Written to uart: Hello"
        if [ $? -eq 0 ]; then
            echo OK
        else
            echo FAIL
        fi
        rm raspi_test.log
        PID=\$(ps aux | grep raspi_test | grep -v grep | awk '{print \$2}')
        kill -9 \$PID
EOF
# ./raspi_test 2>&1 | tee raspi_test.log
}

# Main execution
if [ "$1" = "remote" ]; then
    #build_raspi_program_remote
    run_and_monitor_raspberry_pi
elif [ "$1" = "local" ]; then
    echo "Building locally..."
    build_raspi_file
    if [ $? -eq 0 ]; then
        echo "Build successful. Executing a.out..."
        ./a.out
    else
        echo "Build failed"
        exit 1
    fi
elif [ "$1" = "monitor" ]; then
    read_raspberry_pi_output
else
    echo "Usage: $0 {local|remote|monitor|run-monitor}"
    echo "  local       - Build and run locally"
    echo "  remote      - Build and run on Raspberry Pi"
    echo "  monitor     - Connect to Raspberry Pi and monitor existing output"
    echo "  run-monitor - Build, run, and monitor output on Raspberry Pi"
    exit 1
fi
