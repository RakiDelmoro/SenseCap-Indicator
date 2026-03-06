# SenseCAP Indicator - Home Assistant Controller

A clean, modular ESP32 application for the Seeed Studio SenseCAP Indicator device that provides a touchscreen control panel with MQTT-based Home Assistant integration.

This project is built with ESP-IDF and uses LVGL for the UI.

## Features

- **Touchscreen UI** with LVGL graphics library
- **WiFi Connectivity** with automatic retry and IP acquisition
- **MQTT Integration** with Home Assistant broker (supports hostname resolution)
- **Real-time Control** of switches and sensors via MQTT topics
- **Modular Architecture** for easy maintenance and testing

## Hardware Requirements

- Seeed Studio SenseCAP Indicator (ESP32-S3)
- WiFi network access
- Home Assistant MQTT broker

## Project Structure

```
main/
├── main.c              # Application entry point
├── app_config.h        # Centralized configuration (WiFi, MQTT)
├── wifi_manager.h/.c   # WiFi connectivity module
├── mqtt_manager.h/.c   # MQTT client module with message handlers
├── ui_handlers.h/.c    # UI event callbacks
├── lv_port.c/h         # LVGL display/touch porting
└── ui/                 # SquareLine Studio UI files
    └── screens/
        └── ui_Screen_1.c  # Main screen UI

components/             # ESP-IDF components
├── bsp/               # Board Support Package
├── lvgl/              # LVGL Graphics Library
└── ...
```

## Configuration

Edit `main/app_config.h` to customize settings:

```c
/* WiFi Settings */
#define WIFI_SSID           "your-ssid"
#define WIFI_PASSWORD       "your-password"

/* MQTT Broker - Supports hostname or IP */
#define MQTT_BROKER_URL     "mqtt://homeassistant.local"
#define MQTT_USERNAME       "your-mqtt-username"
#define MQTT_PASSWORD       "your-mqtt-password"
```

## MQTT Topics

| Topic | Direction | Description |
|-------|-----------|-------------|
| `sensecap/bright_switch/state` | Subscribe | Bright switch state from HA |
| `sensecap/bright_switch` | Publish | Send bright switch commands to HA |
| `sensecap/relax_switch/state` | Subscribe | Relax switch state from HA |
| `sensecap/relax_switch` | Publish | Send relax switch commands to HA |
| `esp/sensors` | Subscribe | Sensor data (water level JSON) |

### JSON Format for Sensors
```json
{"water_tank_2": 800}
```

## Building

### Prerequisites
- ESP-IDF 5.x installed
- USB cable connected to SenseCAP Indicator

### Build Commands

```bash
# Set ESP-IDF environment
get_idf

# Build the project
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py monitor

# Combined: build, flash, and monitor
idf.py -p /dev/ttyUSB0 build flash monitor
```

## Architecture

### WiFi Manager (`wifi_manager.h/.c`)
- Self-contained WiFi initialization
- Event-driven state management
- Automatic retry with configurable max attempts
- Blocking API: `wifi_manager_wait_for_ip()`

### MQTT Manager (`mqtt_manager.h/.c`)
- Encapsulated client with wrapper functions
- Automatic reconnection handling
- Topic-specific message processors
- Clean error handling with `esp_err_t`

### UI Handlers (`ui_handlers.h/.c`)
- Switch toggle callbacks with MQTT publishing
- Sensor data processing and visualization
- State management for UI widgets

## Application Flow

1. **System Init**: Initialize NVS, board, and LVGL
2. **UI Init**: Create LVGL tick task and initialize UI
3. **WiFi Init**: Start WiFi and wait for IP address
4. **MQTT Init**: Connect to broker and subscribe to topics
5. **Main Loop**: UI updates and MQTT message handling

## Key Design Decisions

### Modular Structure
- Each module has single responsibility
- No global variables between modules
- Clear public APIs via headers
- Error codes for proper error handling

### Blocking vs Async
- WiFi initialization uses blocking wait
- MQTT runs async via event handlers
- UI callbacks use user_data for client handle

### Memory Safety
- All dynamic memory checked
- Event groups protected
- Client handle validation before use

## Troubleshooting

### WiFi Connection Fails
- Check SSID and password in `app_config.h`
- Verify router DHCP is enabled
- Check signal strength (shown in logs)

### MQTT Connection Fails
- Verify broker hostname/IP is reachable
- Check username/password authentication
- Ensure broker is running on Home Assistant

### UI Not Updating
- Check LVGL tick task is running
- Verify event callbacks registered
- Look for semaphore timeout errors

## Dependencies

- ESP-IDF 5.x
- LVGL 8.x
- cJSON
- SenseCAP Indicator BSP

## Development

### Adding New MQTT Topics
1. Define topic in `app_config.h`
2. Add handler in `mqtt_manager.c`
3. Update topic subscription in `MQTT_EVENT_CONNECTED`

### Adding New UI Widgets
1. Create widget in SquareLine Studio
2. Export UI files to `main/ui/`
3. Add callback in `ui_handlers.c`

## License

MIT License

## Acknowledgments

- Seeed Studio for SenseCAP Indicator hardware
- Espressif for ESP-IDF framework
- LVGL team for graphics library
