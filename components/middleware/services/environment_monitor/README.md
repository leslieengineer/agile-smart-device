# Environment Monitor Service

`EnvironmentMonitor` is Layer 4 reusable logic. It simulates a temperature in the inclusive range 23 to 27 C, drives an `IGpio` LED on when temperature is above 25 C, accepts `help` and `status` through `IUart`, and emits telemetry every five seconds.

It depends only on `IUart`, `IGpio`, and `IClock`. It does not include STM32, ESP-IDF, FreeRTOS, CMSIS, or board headers.

The product composition root injects concrete adapters. Host tests inject fake adapters.
