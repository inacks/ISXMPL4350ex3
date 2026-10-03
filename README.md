# ISXMPL4350ex3 - IS4350 I2C Modbus TCP Server Simple Example for Arduino

A minimal Arduino Uno example for the **IS4350**, the I2C Modbus TCP/IP Server chip by INACKS.

The sketch brings the Modbus TCP server online and increments Holding Register 0 (HOLD_0) by 1 every second. Any Modbus TCP client (a PLC, SCADA system or PC tool) can read the value over Ethernet.

No dedicated library is needed: the sketch uses only the standard Arduino `Wire` library and two short routines, one to read a register and one to write it.

## What the sketch does

1. Writes an initial value to HOLD_0, so clients never read invalid data.
2. Brings the Modbus server online by writing 1 to the `GO_ONLINE` register.
3. Waits until the `STATUS` register reads 5 (Modbus server ready and listening).
4. Increments HOLD_0 every second.

By default, the IS4350 uses DHCP and a MAC address generated from its serial number, so no network configuration is needed.

## Hardware

- Arduino Uno
- IS4350 (or the IS4350-M1 module)
- 2 × 4.7 kΩ pull-up resistors
- Ethernet connection to a network with a DHCP server

## Wiring

| Arduino Uno | IS4350           | Notes                              |
|-------------|------------------|------------------------------------|
| A4 (SDA)    | SDA              | 4.7 kΩ pull-up to 5 V              |
| A5 (SCL)    | SCL              | 4.7 kΩ pull-up to 5 V              |
| GND         | VSS              | Common ground                      |
| —           | ADR → GND        | I2C address 24 (0x18)              |
| —           | SPD → GND        | 100 kHz (Arduino Wire default)     |

## Usage

1. Open the `.ino` file in the Arduino IDE.
2. Select **Arduino Uno** and upload.
3. Open the Serial Monitor at **9600 baud**.

Expected output:

```
STATUS: 3
STATUS: 5
Server online.
HOLD_0 = 1
HOLD_0 = 2
...
```

4. Read Holding Register 0 from any Modbus TCP client on port 502. Use the IS4350's IP address, which you can read from its IP registers.

## Key points

- Register addresses and register values are 16-bit and are sent MSB first.
- Reads use a repeated start (`Wire.endTransmission(false)`).
- Leave a few milliseconds between consecutive I2C operations.
- `GO_ONLINE` is 0 after every power-up, so the sketch sets it at startup.
- If STATUS shows an error (101 and above), check the network cable and the MAC configuration. See the datasheet for the full list of status codes.

## Documentation

- IS4350 datasheet and product page: [www.inacks.com/is4350](https://www.inacks.com/is4350)
- IS4350-M1 module: [www.inacks.com/is4350-m1](https://www.inacks.com/is4350-m1)
