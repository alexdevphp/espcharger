# espcharger

External ESPHome component for Li-Ion Battery Charger SouthROL ZZ-ST-20.

## Protocol

UART frame format:

- `0x11 0x55` — frame header
- byte 3 — message type
  - `0x57` write to MCU
  - `0x52` read from MCU
  - `0x42` response from MCU
  - `0x4C` request to MCU
  - `0x53` telemetry packet from MCU
- bytes 4-5 — 16-bit address
- byte 6 — payload length
- payload
- last 2 bytes — CRC-16/CMS

On startup component sends `ENABLE EDIT`:

- `11 55 4c 00 40 02 a4 5b + CRC`

MCU sends telemetry autonomously (about once per second).

`GET TELEMETRY` can still be sent manually using the button entity:

- `11 55 52 0a 01 01 18 + CRC`

## Telemetry fields (from packet type `0x53`)

- bytes 11-12: voltage (`uint16`, little endian)
- bytes 13-14: current (`uint16`, little endian)
- bytes 21-22: charging counter (`uint16`, little endian)
- bytes 23-24: temperature 1 (`uint16`, little endian)
- bytes 25-26: temperature 2 (`uint16`, little endian)
- byte 28: mode
- byte 29: charging state

## Example ESPHome YAML

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/alexdevphp/espcharger.git
    components: [ espcharger ]
    refresh: always

uart:
  id: charger_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

espcharger:
  id: charger
  uart_id: charger_uart

sensor:
  - platform: espcharger
    id: charger
    voltage:
      name: "Charger Voltage"
    current:
      name: "Charger Current"
    charge_counter:
      name: "Charge Counter"
    temperature_1:
      name: "Temperature 1"
    temperature_2:
      name: "Temperature 2"
    mode:
      name: "Charger Mode"
    charging_state:
      name: "Charging State"

number:
  - platform: espcharger
    id: charger
    voltage:
      name: "Set Voltage"
    current:
      name: "Set Current"

switch:
  - platform: espcharger
    id: charger
    charging:
      name: "Charging"
```
