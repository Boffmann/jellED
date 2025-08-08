#!/usr/bin/env python3

import os
import shutil
from pathlib import Path

Import("env")

# Get the project directory
project_dir = env.get('PROJECT_DIR', '')
esp_src_dir = os.path.join(project_dir, "..", "..", "esp", "src")
esp_includes_dir = os.path.join(project_dir, "..", "..", "esp", "includes")
esp_test_dir = os.path.join(project_dir, "esp_src")

print(f"Copying ESP serial files from {esp_src_dir} to {esp_test_dir}")

# Copy only the serial directory from src (excluding jellED.cpp)
serial_src_dir = os.path.join(esp_src_dir, "serial")
serial_test_dir = os.path.join(esp_test_dir, "serial")

if os.path.exists(serial_src_dir):
    # Create directory if it doesn't exist
    os.makedirs(serial_test_dir, exist_ok=True)
    
    # Copy all files from serial directory
    for file in os.listdir(serial_src_dir):
        src_file = os.path.join(serial_src_dir, file)
        dst_file = os.path.join(serial_test_dir, file)
        
        if os.path.isfile(src_file):
            shutil.copy2(src_file, dst_file)
            print(f"Copied: serial/{file}")
            
            # Fix include path in EspUart.cpp
            if file == "EspUart.cpp":
                with open(dst_file, 'r') as f:
                    content = f.read()
                
                # Replace the include path
                content = content.replace('#include "esplogger.h"', '#include "../utils/esplogger.h"')
                
                with open(dst_file, 'w') as f:
                    f.write(content)
                print("Fixed include path in EspUart.cpp")

# Copy the header files from includes
serial_includes_dir = os.path.join(esp_includes_dir, "serial")
utils_includes_dir = os.path.join(esp_includes_dir, "utils")

# Copy serial headers
if os.path.exists(serial_includes_dir):
    # Create directory if it doesn't exist
    os.makedirs(serial_test_dir, exist_ok=True)
    
    # Copy header files from includes
    for file in os.listdir(serial_includes_dir):
        if file.endswith('.h'):
            src_file = os.path.join(serial_includes_dir, file)
            dst_file = os.path.join(serial_test_dir, file)
            
            if os.path.isfile(src_file):
                shutil.copy2(src_file, dst_file)
                print(f"Copied: serial/{file}")

# Copy utils headers
utils_test_dir = os.path.join(esp_test_dir, "utils")
if os.path.exists(utils_includes_dir):
    # Create directory if it doesn't exist
    os.makedirs(utils_test_dir, exist_ok=True)
    
    # Copy header files from includes
    for file in os.listdir(utils_includes_dir):
        if file.endswith('.h'):
            src_file = os.path.join(utils_includes_dir, file)
            dst_file = os.path.join(utils_test_dir, file)
            
            if os.path.isfile(src_file):
                shutil.copy2(src_file, dst_file)
                print(f"Copied: utils/{file}")

print("ESP serial files copied successfully!") 