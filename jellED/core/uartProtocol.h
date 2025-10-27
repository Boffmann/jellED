#ifndef __UART_PROTOCOL_H__
#define __UART_PROTOCOL_H__

#include <stdint.h>

namespace jellED {

constexpr uint8_t UART_BEAT_DETECTED = 8;
constexpr uint8_t UART_BUTTON_PRESSED = 255;

} // namespace jellED

#endif