# EmptyBin Monitor 🗑️

Real-time waste bin level monitoring system using ESP32 and HC-SR04 ultrasonic sensor with WiFi connectivity and web dashboard.

## Hardware Components

- **ESP32 Development Board**
- **HC-SR04 Ultrasonic Sensor**
- Jumper wires
- Power supply (USB or battery)

## Wiring Diagram

```
HC-SR04          ESP32
--------         -----
VCC       -->    5V
TRIG      -->    GPIO 5
ECHO      -->    GPIO 18
GND       -->    GND
```

## Software Setup

### 1. Install PlatformIO
- Install [Visual Studio Code](https://code.visualstudio.com/)
- Install the PlatformIO IDE extension

### 2. Configure WiFi Credentials
Open `src/main.cpp` and update these lines with your WiFi details:

```cpp
const char* ssid = "YOUR_WIFI_SSID";      // Replace with your WiFi name
const char* password = "YOUR_WIFI_PASSWORD";  // Replace with your WiFi password
```

### 3. Configure Bin Dimensions
Update these values to match your actual bin:

```cpp
#define BIN_HEIGHT 100.0    // Total height of your bin in cm
#define SENSOR_OFFSET 5.0   // Distance from sensor to bin top in cm
```

**How to measure:**
- `BIN_HEIGHT`: Measure the total height of your bin from bottom to top
- `SENSOR_OFFSET`: Mount the sensor above the bin and measure the distance from the sensor to the top edge of the bin

### 4. Upload Code
1. Connect your ESP32 to your computer via USB
2. Click the PlatformIO upload button (→) or run: `pio run -t upload`
3. Open the Serial Monitor to see the connection status

## Usage

### Accessing the Dashboard

1. After uploading, open the Serial Monitor (115200 baud)
2. Wait for the WiFi connection message
3. Note the IP address displayed (e.g., `192.168.1.100`)
4. Open a web browser and navigate to: `http://[IP_ADDRESS]`

Example: `http://192.168.1.100`

### Dashboard Features

The web dashboard displays:
- **Visual bin representation** with animated fill level
- **Fill percentage** (0-100%)
- **Current distance** measurement in cm
- **Status indicator**:
  - ✓ Green: Bin has plenty of space (< 60%)
  - ⚠ Yellow: Bin getting full (60-85%)
  - ⚠ Red: Bin nearly full (> 85%)
- **Real-time updates** every 2 seconds

### API Endpoint

You can also access the raw sensor data in JSON format:

```
GET http://[IP_ADDRESS]/data
```

Response format:
```json
{
  "distance": 45.23,
  "fillPercentage": 54.77,
  "timestamp": 123456
}
```

## How It Works

### Distance to Fill Percentage Calculation

1. Sensor measures distance from itself to the waste surface
2. System calculates empty space: `emptySpace = distance - SENSOR_OFFSET`
3. Fill percentage: `fillPct = ((BIN_HEIGHT - emptySpace) / BIN_HEIGHT) × 100`

**Example:**
- Bin height: 100 cm
- Sensor offset: 5 cm
- Measured distance: 55 cm
- Empty space: 55 - 5 = 50 cm
- Fill percentage: ((100 - 50) / 100) × 100 = **50%**

### Features

- **Moving average filter**: Smooths out noisy sensor readings using 5-sample averaging
- **Auto-refresh**: Dashboard updates every 2 seconds automatically
- **Responsive design**: Works on desktop, tablet, and mobile devices
- **Visual feedback**: Color-coded status based on fill level

## Troubleshooting

### WiFi Connection Issues
- Verify your WiFi credentials are correct
- Ensure your ESP32 is within WiFi range
- Check if your network supports 2.4GHz (ESP32 doesn't support 5GHz)

### Inaccurate Readings
- Ensure the sensor is mounted securely and level
- Verify the `BIN_HEIGHT` and `SENSOR_OFFSET` values are correct
- Check that the sensor has a clear line of sight to the bin contents
- Avoid mounting near bin edges or corners

### Dashboard Not Loading
- Verify the ESP32 is connected to WiFi (check Serial Monitor)
- Ensure you're using the correct IP address
- Make sure your device is on the same network as the ESP32

### Out of Range Errors
- HC-SR04 effective range: 2cm - 400cm
- Ensure bin height + offset is within sensor range
- Check sensor wiring connections

## Customization

### Change Update Interval
In `main.cpp`, modify the delay in the loop function:
```cpp
delay(1000);  // Change from 1000ms (1 second) to your preferred interval
```

And in the HTML JavaScript section:
```javascript
setInterval(updateData, 2000);  // Change from 2000ms to match your preference
```

### Modify Alert Thresholds
In the HTML section, adjust these values:
```javascript
if (fillPct < 60) {          // Change 60 to your preferred threshold
    // "OK" status
} else if (fillPct < 85) {   // Change 85 to your preferred threshold
    // "Warning" status
} else {
    // "Full" status
}
```

## Future Enhancements

Potential improvements you could add:
- Mobile notifications when bin is full
- Data logging and historical charts
- Multiple bin monitoring
- Integration with smart home systems
- Battery power optimization with deep sleep

## License

This project is open source and available for educational and personal use.

## Support

For issues or questions, please check the troubleshooting section above or refer to:
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core Documentation](https://docs.espressif.com/projects/arduino-esp32/)
