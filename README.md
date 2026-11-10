# Arduino Fanning Thermometer

This is an Arduino UNO thermometer that has a 7 segment and LCD display. A servo hand fan swings according to temperature and a piezo beeps when it gets over 40 degrees.

# Build steps
## Components

1 Arduino Uno

1 LCD 16x2

1 Potentiometer (B50K)

8 1 kΩ resistors

1 8-Bit Shift Register

1 Anode 7 Segment Display

1 Positional Micro Servo with a fan attached

1 Piezo buzzer

1 LM35 Temperature Sensor

1 Breadboard

Jumper wires

## Build
Build the circuit using the schematic in the repository and the information below.

Open the project's .ino file in the Arduino IDE.

Connect the Arduino Uno to the computer using USB and upload code on the Arduino.

Serial Monitor should show current temperature every 3 seconds.

# Timer config
Timer 2 is configured in CTC mode, used to generate an interrupt every 0.016 seconds. Prescaler - 1024, OCR1A - 249, interrupt - TIMER2_COMPA_vect.
Interrupt sets flag for sensor input processing, display and servo updates almost every second. It also updates a 3 state counter for different actions.

# ISR roles
The Timer2 ISR counts to a second to set a sensor input processing, display and servo update flag and updates the phase counter.
Sets the sensor-reading flag.

# EEPROM layout
magic (uint16_t) – to identify valid data belonging to this application. Value 123 is used.

version (byte) – to identify the saved data format version. Value 1 is used.

temperature (int) – to save the last measured temperature.

These EEPROM fields are processed by methods in the Pet class, stats are verified to be in usable intervals.


hunger (int) – hunger stat.
