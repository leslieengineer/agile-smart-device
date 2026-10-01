# UHAL Interfaces

This is the contract layer between reusable logic and hardware. Each interface describes one capability required by a device driver, protocol, or application. Logic may include files in this directory and `components/uhal/core`; platform adapters implement the interfaces with STM32 LL/CMSIS, ESP-IDF, or host fakes.

## Modification Rules (Status)

*   FROZEN: API is frozen. Modifying the current function structures is strictly prohibited. (Only comment additions are allowed if clarification is necessary).
*   DRAFT: API is currently under design or serves as a placeholder. Free to be modified.

## Current Contracts

### 1. Synchronous Communication & High-Speed Bus
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `II2c` | FROZEN | I2C read and write (Blocking) | SHT3x, EEPROM, IMU |
| `ISpi` | FROZEN | Full-duplex SPI transfer (Blocking) | Flash, ADC, display |
| `IUart` | FROZEN | UART byte transport (Blocking) | Modbus RTU, GNSS |
| `ICan` | FROZEN | CAN/CAN FD frame transport | CANopen, DroneCAN |
| `II2s` | DRAFT | Inter-IC Sound for digital audio | I2S microphone, audio DAC |
| `IUsbDevice` | DRAFT | USB Device peripheral abstraction | USB CDC, HID, MSC |
| `IQuadSpi` | DRAFT | Quad SPI for external memory | PSRAM, external flash |
| `IOspi` | DRAFT | Octal SPI for external memory | High-speed memory mapping |

### 2. Asynchronous & Interrupt-Driven Communication
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `II2cAsync` | DRAFT | Non-blocking I2C with callback/DMA | Large sensor arrays, I2C displays |
| `ISpiAsync` | DRAFT | Non-blocking SPI transfer/DMA | SD card logging, TFT framebuffers |
| `IUartAsync` | DRAFT | Non-blocking UART TX/RX | GNSS data streams, CLI consoles |
| `IAdcAsync` | DRAFT | Continuous ADC sampling (DMA) | Audio recording, power analysis |

### 3. General I/O & Analog
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `IGpio` | FROZEN | One configured digital pin | Sensor reset, chip select, RS-485 enable |
| `IGpioInterrupt`| FROZEN | Edge-triggered external interrupt (EXTI) | Push buttons, IMU data-ready pins |
| `IPwm` | FROZEN | One configured PWM channel | LED dimming, motor, servo output |
| `IAdc` | FROZEN | One configured analog channel | Battery monitor, analog sensor |
| `IDac` | DRAFT | Digital-to-Analog output | Audio generation, analog control valves |
| `IComparator` | DRAFT | Analog voltage comparison interrupt | Overcurrent/overvoltage protection |
| `IOpAmp` | DRAFT | Internal Operational Amplifier routing | Small signal amplification |
| `ITouch` | DRAFT | Capacitive touch sensing | Touchpad, UI buttons |

### 4. Time & Motion Control
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `IClock` | FROZEN | Time source and delay (Uptime) | Sensor conversion time, retry policy |
| `IRtc` | DRAFT | Real-Time Clock (Calendar time) | Log timestamping, daily alarms |
| `IPeriodicTimer`| DRAFT | Hardware timer for precise callbacks | LED matrix scanning, PID control loops |
| `IWatchdog` | FROZEN | Watchdog feed | System health service |
| `IEncoder` | DRAFT | Quadrature encoder reading | Motor feedback, rotary knobs |
| `IInputCapture` | DRAFT | Pulse width and frequency measurement | RC receivers, HC-SR04 |
| `IPulseCounter` | DRAFT | Hardware pulse counting | Flow meters, anemometers |

### 5. System, Storage & Security
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `IPower` | DRAFT | CPU sleep modes and wakeup sources | Battery-powered IoT nodes |
| `IResetReason` | DRAFT | Retrieve system reset cause | Telemetry, crash reporting |
| `IStorage` | FROZEN | Byte-addressable persistent storage | Settings, calibration, event log |
| `IFlash` | DRAFT | Raw internal flash memory control | Bootloader, OTA updates |
| `IFileSystem` | DRAFT | High-level file operations (open, read) | JSON configs, CSV logging |
| `IHardwareCrypto`| DRAFT | Hardware cryptographic accelerator | TLS/SSL, secure boot |

### 6. Wireless & Network
| Contract | Status | Capability | Example consumer |
|---|---|---|---|
| `IWifi` | DRAFT | Wi-Fi connectivity and lifecycle | Cloud connectivity, web server |
| `IBle` | DRAFT | Bluetooth Low Energy communication | Smart phone provisioning, BLE beacons |
| `IRadio` | DRAFT | 802.15.4 / Sub-GHz radio communication | Zigbee, Thread nodes |

## Design Rules

1. A contract describes a **capability**, not a vendor peripheral. Use `ICan`, not `IFdcan` or `ITwai`.
2. Each object represents a resource already configured by the board or product. `IGpio` does not receive a pin number; `IAdc` does not receive a channel number; `IPwm` does not receive a timer instance.
3. Pin mapping, baud rates, clocks, DMA streams or channels, interrupt priority, and SDK handles belong in the platform adapter or `products/<product>/board/`.
4. Extend an interface only after a real use case exists. Do not add asynchronous transfer, DMA, or configuration methods to every interface merely because the peripheral supports them.
5. `Status` must not expose `HAL_StatusTypeDef` or `esp_err_t`. An adapter maps vendor errors to `uhal::Status`.

## Example

```cpp
class DriverWithReset {
public:
    DriverWithReset(uhal::II2c& bus, uhal::IGpio& reset, uhal::IClock& clock)
        : bus_(bus), reset_(reset), clock_(clock) {}

private:
    uhal::II2c& bus_;
    uhal::IGpio& reset_;
    uhal::IClock& clock_;
};
```

`DriverWithReset` does not know whether the reset pin is GPIOA.4 or GPIO8, and it does not know whether I2C is implemented by STM32H5 or ESP32-C6. The product composition root creates the board-specific adapters and injects them through the constructor.
