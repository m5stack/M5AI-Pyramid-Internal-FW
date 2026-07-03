# M5AI Pyramid Internal FW

## Overview

This repository contains the internal EC firmware for M5Stack AI Pyramid and AI Pyramid-Pro. The EC runs on the built-in STM32 coprocessor and handles low-level device functions such as power control, power monitoring, OLED status display, user buttons, RGB LEDs, fan control, wake-up logic, and peripheral power management.

The host communicates with the EC through the Modbus protocol. It can read voltage, current, button events, fan speed, and firmware version, and can control 5V power rails, PCIe/USB/Grove power, RGB modes, OLED display modes, timed wake-up, and other EC features. See [EC protocol and register description](protocol/ax650_pyramid_ec控制协议.md) for the full register map.

## Supported Hardware

| Product | Official Documentation | Main Differences |
|:--|:--|:--|
| AI Pyramid | [docs.m5stack.com/en/ai_hardware/AI_Pyramid](https://docs.m5stack.com/en/ai_hardware/AI_Pyramid) | 4GB LPDDR4x, 2x digital audio/video outputs |
| AI Pyramid-Pro | [docs.m5stack.com/en/ai_hardware/AI_Pyramid-Pro](https://docs.m5stack.com/en/ai_hardware/AI_Pyramid-Pro) | 8GB LPDDR4x, 1x digital audio/video input + 1x output |

Both devices are based on the Axera AX8850 platform and include an STM32 coprocessor for EC expansion features. This repository only covers the STM32 EC firmware. It does not include the Linux kernel, U-Boot, system images, or upper-layer applications.

## Features

- Modbus RTU communication, default host address `0x01`, default baud rate `921600 bps`
- Internal and external 5V main power control, with delayed shutdown, button wake-up, and timed wake-up
- PCIe, USB, Grove, and other peripheral power control with persistent state support
- INA3221 voltage and current monitoring
- SSD1306 OLED status display, including logo, voltage, IP, and custom character display modes
- Top button and OLED button event detection, including press, release, and long-press events
- RGB LED display modes, including ARM mapping, gradient, blink, rainflow, download mode, and audio recording mode
- Fan speed control and RPM readback
- I2C register read/write passthrough
- Flash parameter storage and restore on boot

## Repository Structure

```text
.
├── code/
│   └── M5_650N_Pyramind_STM32/      # STM32CubeIDE project source
├── firmware/
│   └── 2026-01-10-V1/               # Released firmware hex and flashing note
├── protocol/
│   └── ax650_pyramid_ec控制协议.md   # EC Modbus protocol and register map
├── LICENSE
└── README.md
```

## Development Environment

STM32CubeIDE is recommended for opening and building this project.

- MCU: STM32G431CBUx
- Project file: `code/M5_650N_Pyramind_STM32/.project`
- CubeMX configuration: `code/M5_650N_Pyramind_STM32/M5_650N_Pyramind_STM32.ioc`
- Main application code: `code/M5_650N_Pyramind_STM32/Core`
- Custom drivers: `code/M5_650N_Pyramind_STM32/Drivers/M5_Pyrmind_Drivers`

Import steps:

1. Open STM32CubeIDE.
2. Select `File` -> `Import` -> `Existing Projects into Workspace`.
3. Select the `code/M5_650N_Pyramind_STM32` directory.
4. Import the project, then build, debug, or generate hex/bin firmware as needed.

## Firmware Flashing

Current prebuilt firmware:

```text
firmware/2026-01-10-V1/M5_650N_Pyramind_STM32G431CB_2026_01_10_V1.hex
```

Flashing notes:

1. Erase the STM32 Flash before flashing.
2. Power-cycle the device after flashing and wait for EC initialization.
3. The host can read Input Register 14 to get the firmware version.
4. Avoid frequent writes for parameters that are saved to Flash. Each save triggers a Flash erase operation and takes about 20 ms.

## EC Communication

The EC communicates with the host through Modbus:

| Item | Default / Description |
|:--|:--|
| Host address | `0x01` |
| Baud rate | `921600 bps` |
| Coil registers | Power control, peripheral power, save commands, shutdown, system ready flag |
| Discrete input registers | PCIe presence, power-good status, button events |
| Input registers | Voltage, current, fan RPM, firmware version |
| Holding registers | Shutdown delay, fan, RGB, OLED, IP, I2C, RAM mapping, and other settings |

For register addresses, value ranges, persistence behavior, and pin mapping, see [protocol/ax650_pyramid_ec控制协议.md](protocol/ax650_pyramid_ec控制协议.md).

## Related Resources

- [AI Pyramid official documentation](https://docs.m5stack.com/en/ai_hardware/AI_Pyramid)
- [AI Pyramid-Pro official documentation](https://docs.m5stack.com/en/ai_hardware/AI_Pyramid-Pro)
- [AI Pyramid EC Proxy hardware control](https://docs.m5stack.com/en/stackflow/ai_pyramid/ec_proxy)
- [AI Pyramid getting started](https://docs.m5stack.com/en/stackflow/ai_pyramid/getting_started)

## License

This project is licensed under the [MIT License](LICENSE).
