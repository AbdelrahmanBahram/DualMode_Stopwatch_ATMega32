# Dual-Mode Stopwatch using ATmega32

This project is a digital stopwatch built on the ATmega32 microcontroller with two modes:
- **Increment mode**: Counts up from 00:00:00
- **Countdown mode**: Counts down from a user-set time

##  Features
- 6 multiplexed seven-segment displays (HH:MM:SS)
- 10 push-button controls
- Mode toggle, reset, pause, and resume functionality
- LED indicators for current mode
- Buzzer alarm when countdown ends
- Timer1 in CTC mode for timekeeping
- External interrupts for button actions (INT0, INT1, INT2)


##  Folder Structure
- `code/` – C and header files for microcontroller
- `simulation/` – Proteus or simulation files
