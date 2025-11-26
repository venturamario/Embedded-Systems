# Embedded-Systems
Labs of the Embedded Systems subject of MEI, UPC, 2025-2026

### AUTHORS:
Mario Ventura Burgos & Jofre Coll Vila.

# Implemented Projects
Projects implemented along all laboratory sessions.

## LAB 1 – ESP32C6
First steps with the ESP32-C6 and ESP-IDF environment. Basic project setup, GPIO control, LED blinking, UART monitoring and verification of the full build/flash workflow.

## LAB 2 – ADC Converter for heart bits
High-speed analog acquisition using the ESP32-C6 SAR ADC. Captured a simulated heartbeat signal (or real ECG front-end), applied basic digital filtering and displayed the waveform in real time through the serial port.

## LAB 3 – Oscilloscope Drawing & PWM
Generation of high-quality analog signals using two high-resolution PWM channels (approximately 250 kHz) filtered with RC low-pass networks. Demonstrated vector drawing on an oscilloscope in X-Y mode (lines, circles, Lissajous figures, text) and optimized effective PWM resolution.

## LAB 4 – CAN & Oscilloscope Game
Implementation of the ESP32-C6 CAN controller with an external transceiver. Developed a real-time two-player “Pong” (or Snake) game where paddle/ball position and score were exchanged via CAN frames and rendered simultaneously on the oscilloscope in X-Y mode.

## LAB 5 – Multitasking w. RTOS
Deep dive into FreeRTOS on ESP32-C6: preemptive scheduling, mutexes vs binary semaphores, message queues, task priorities and fair distribution. Implemented (1) parallel LED blinking with mutual exclusion, (2) producer-consumer pattern with one 16-slot queue and perfectly balanced consumption, (3) real measurement of context-switch overhead (approximately 8–12 µs at 160 MHz).
