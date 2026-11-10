# Arduino-Fanning-Thermometer

This is an Arduino UNO thermometer that has a 7 segment and LCD display. A servo hand fan swings according to temperature and a piezo beeps when it gets over 40 degrees.

# Build steps
## Components

1 Arduino Uno

1 LCD 16x2

1 Potentiometer (B50K)

8 1 kΩ resistors

1 8-Bit Shift Register

1 Anode 7 Segment Display

1 Positional Micro Servo

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
Timer 2 is configured in CTC mode, used to generate an interrupt every 0.016 seconds. Prescaler - 1024, OCR1A - 249, interrupt - TIMER1_COMPA_vect.
Interrupt sets flag for sensor input processing and some pet updates. Every 30 seconds another flag is set to update pet stats.

# ISR roles
The Timer1 ISR sets sensor reading and pet update flags and counts to 30 seconds.
Sets the sensor-reading flag.

External interrupts are used for D2 and D3. They register button presses by setting their flags for the main loop to process.

# EEPROM layout
magic (uint16_t) – to identify valid data belonging to this application. Value 666 is used.

version (byte) – to identify the saved data format version. Value 1 is used.

health (int) – health stat.

energy (int) – energy stat.

hunger (int) – hunger stat.
