# SPIFFS
This explains how to upload files to the ESP32 board filesystem using VS Code with PlatformIO. 

The ESP32 contains s Serial Peripheral Interface Flash File System (SPIFFS) that is a lightweight filesystem created for microcontrollers with a flash chip, which are connected by SPI bus, like the ESP32 flash memory.

SPIFFS lets you access the flash memory like you would do in a normal filesystem in your computer, but simpler and more limited. You can read, write, close, and delete files. SPIFFS doesn’t support directories, so everything is saved on a flat structure.

## How to Upload Files to ESP32 SPIFFS

The files you want to upload to the ESP32 filesystem should be placed in a folder called `data` under the project folder. **IMPORTANT:** The folder must be called `data`, otherwise it won't work.

Afterwards, follow these steps:

1. Click the PIO icon at the left side bar. The project tasks should open.
2. Select the corresponding board
3. Expand the Platform menu.
4. Select Build Filesystem Image.
5. Finally, click Upload Filesystem Image.