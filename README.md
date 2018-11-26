# SSV-Goudzwaard

SSV Goudzwaard is a wireless model submarine controlled by an application on an android phone. 

To compile:

1. `Install PlatformIO Core <http://docs.platformio.org/page/core.html>`_
2. Download `development platform with examples <https://github.com/platformio/platform-espressif32/archive/develop.zip>`_
3. Extract ZIP archive
4. Run these commands:

.. code-block:: bash

    # Build project
    > platformio run

    # Upload firmware
    > platformio run --target upload

    # Build specific environment
    > platformio run -e esp32dev

    # Upload firmware for the specific environment
    > platformio run -e esp32dev --target upload

    # Clean build files
    > platformio run --target clean

