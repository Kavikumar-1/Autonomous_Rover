# Hardware Wiring & Pinout 🔧

## 1. Main ESP32 Controller

| Component | Pin | Function |
|---|---|---|
| **Ultrasonic (HC-SR04)** | GPIO 5 | TRIG |
| **Ultrasonic (HC-SR04)** | GPIO 18 | ECHO |
| **Buzzer** | GPIO 4 | Warning Sound |
| **Relay (Mist Maker)** | GPIO 23 | Active LOW Switch |
| **LCD (16x2 I2C)** | GPIO 21 | SDA |
| **LCD (16x2 I2C)** | GPIO 22 | SCL |
| **UART 2 (to Cam)** | GPIO 16 | RX (Connect to Cam TX) |
| **UART 2 (to Cam)** | GPIO 17 | TX (Connect to Cam RX) |

### L298N Motor Driver
| L298N Pin | ESP32 Pin | Function |
|---|---|---|
| **ENA** | GPIO 13 | Left Speed (PWM) |
| **IN1** | GPIO 14 | Left Forward |
| **IN2** | GPIO 27 | Left Backward |
| **IN3** | GPIO 26 | Right Forward |
| **IN4** | GPIO 25 | Right Backward |
| **ENB** | GPIO 33 | Right Speed (PWM) |

---

## 2. ESP32-CAM

| Pin | Function |
|---|---|
| **GPIO 4** | Flash LED (PWM Controlled) |
| **GPIO 1** | TX (Connect to Main RX) |
| **GPIO 3** | RX (Connect to Main TX) |
| **5V / GND** | Power Supply |

---

## 3. Power System
- **Battery**: 11.1V (3S Li-ion) recommended.
- **Powering Motors**: Connect 11.1V directly to L298N `12V` input.
- **Powering ESP32**: Use a Buck Converter to step down 11.1V to 5V, or use the 5V output from the L298N (if using < 12V).
- **Common Ground**: **CRITICAL!** All GND pins from the battery, ESP32, ESP32-CAM, and L298N must be connected together.

## 4. Mist Relay Wiring
- **VCC**: Connect to **5V (VIN)**.
- **JD_VCC**: Must be connected to **5V** (remove jumper if using separate power).
- **GND**: Shared GND.
- **IN1**: Connect to **GPIO 23**.
- **Load (Mist Maker)**: Connect the Mist Maker's positive wire through the **COM** and **NO** (Normally Open) terminals of the relay.
