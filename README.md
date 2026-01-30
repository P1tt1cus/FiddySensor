# Fiddy Leaf

ESP32-based plant moisture monitor with a web interface.

## Hardware

### Components
- ESP32 dev board (any variant with ADC1 pins)
- Capacitive soil moisture sensor (v1.2 or v2.0)

### Wiring

| Sensor | ESP32 |
|--------|-------|
| VCC    | 3.3V  |
| GND    | GND   |
| AOUT   | GPIO 34 |

### Calibration

The sensor outputs a voltage inversely proportional to moisture. Default calibration values in `moisture_sensor.c`:

```c
#define RAW_DRY     50      // ADC reading in dry air
#define RAW_WET     2500    // ADC reading in water
```

Adjust these for your specific sensor by checking the raw values in dry air and submerged in water.

## Setup

1. Set your WiFi credentials in `main/wifi.c`:
```c
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"
```

2. Build and flash:
```bash
idf.py build
idf.py flash monitor
```

3. Connect to the IP shown in the serial output to view the web interface.

## Web Interface

Shows real-time moisture percentage with an animated plant that reacts to moisture levels:
- **Wet** (>70%): Healthy green
- **Moist** (40-70%): Olive, getting worried
- **Dry** (20-40%): Yellow, needs water
- **Dead** (<20%): Gray, RIP

Includes a history graph showing moisture readings over time.
