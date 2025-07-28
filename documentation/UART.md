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