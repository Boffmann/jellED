# Universal Asynchronous Receiver / Transmitter - UART
UART is a serial communication protocol that allows two devices to communicate.
UART is asynchronous which means it does not use a clock signal to synchronize the data transmission between devices.
Instead, the two devices must agree on a `baud rate` (speed of transmission).

```
RaspberryPi          ESP32
┌─────────────┐     ┌─────────────┐
│ GPIO14 (TX) ├────►│ GPIO16 (RX) │
│ GPIO15 (RX) │◄────┤ GPIO17 (TX) │
│ GND         ├─────┤ GND         │
└─────────────┘     └─────────────┘
```

## UART on ESP32
The ESP32 chip has 3 UART controllers (aka ports), each featuring an identical set of registers.
Each UART controller is independently configurable through

- baud rate
- data bit length
- bit ordering
- number of stop bits
- parity bit
- ...

### Install Drivers
The first thing to do is to install the drivers with `uart_driver_install()` and specify the following parameters:

- UART port number
- Size of RX ring buffer
- Size of TX ring buffer
- Event queue size
- Pointer to store the event queue handle
- Flags to allocate an interrupt




# Refs
- [Espressif UART Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html)
- [Raspi Setup for UART](https://www.electronicwings.com/raspberry-pi/raspberry-pi-uart-communication-using-python-and-c)