# SSV-Goudzwaard

SSV Goudzwaard is a wireless model submarine controlled by an application on an android phone. In this repository the source code for compiling the firmware of the ESP32 in the submarine can be found.

To compile and flash:

1. Install PlatformIO Core
2. Clone the repository
3. Run these commands:

```bash
# Build project
platformio run

# Upload firmware
platformio run --target upload
```

