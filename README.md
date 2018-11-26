# SSV-Goudzwaard

SSV Goudzwaard is a wireless model submarine controlled by an application on an android phone. In deze repository kan de sourcecode voor het compileren van de firmware van de ESP32 in de duikboot gevonden worden.

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

