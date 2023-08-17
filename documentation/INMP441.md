# INMP441
The INMP441 is a digital-output, omnidirectional MEMS microphone. The microphone features a digital I2S interface.

The I2S peripheral supports Direct Memory Access (DMA) meaning it can stream sample data without requiring each sample to be read or written by the CPU. When using DMA, the CPU initiates a transfer between memory and the peripheral. The transfer is all taken care of by the DMA controller and the CPU is free to go off and do other work.

# Interface
The I2S interface requires three connected pins, namely the Word Select (WS), the Serial Clock (SCK), and Serial Data (SD) pin. The audio channel is configured via the WS pin. A logical high indicates that the transmitted values are for the right-channel audio, a logical low indicates left-channel audio. Data is transmitted in a serial manner via the SD pin in most siginficant bit first format.

A new INMP441 instance is created via the constructor where the corresponding pins are specified:

```
INMP441(uint8_t ws_pin, uint8_t sd_pin, uint8_t sck_pin);
```

The actual initialization happens within the `initialize()` method. Here, the I2S config is created, the I2S driver is installed and the pin is configured. We set the data output pin `data_out_num` in the pin configuration to `-1` because the INMP441 is dedicated to be input only. An ESP32 features two I2S ports, namely `I2S_NUM_0` and `I2S_NUM_1`. Because we only use one microphone at the monent, the port is hard coded to `I2S_NUM_0`. The ESP's I2S mode is set to master and receiving mode. Other important configuration values are the sampling rate, the sampling bit count, the DMA buffer length, and the DMA buffer count. These values depend on the desired sampling resolution and available bandwidth.

## DMA Buffer Length and Count
The DMA Buffer length specifies how many samples it requires to fill the buffer. Only filled buffers can be read. Setting this value low means that the buffer fills quickly and hence needs to be read quickly to not cause congestion. Setting the length value too high can limit resolution.

The DMA Buffer count specifies how many DMA buffers the I2S has available. Because we can only read a buffer when the DMA controller has transmitted enough data to fill the buffer, we need at least two buffers so that one can be read while the other is being filled. Otherwise, it will again cause congestion.

# Reading Data
Reading data from the DMA is as simple as calling

```
i2s_read(I2S_PORT, &buffer->buffer, I2S_DMA_BUF_LEN, &buffer->buffer_bytes, portMAX_DELAY);
```

The call requires the I2S port to read from, a pointer to a buffer to write the samples into, the length of the DMA buffer, a pointer to a value to store the number of bytes read from the DMA and a max delay of RTOS ticks to wait until enough bytes are available in the DMA.

