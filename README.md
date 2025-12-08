
# ELEC2645 Unit 2 Project 

Converter Design Tool – Buck / Boost / Buck-Boost / Cuk
I. Introduction.

This project is a C application that allows users(engineers or students) to design four types of DC-DC converters:
1. Buck converter
2. Boost converter
3. Buck-Boost converter
4. Cuk converter

The program takes user input parameters and automatically calculates the required component values.

This project is inspired by ELEC2501 Power Electronics.

II. Features.
1. Four converter designs
Each converter has:
- Input validation
- Components calculations
- Data analysis
- Output summary in menu form
- Save-to-file option

2. Supported inputs
- Maximum input volatge
- Minimum input voltage
- Output volatge
- Switching frequency
- Inductor ripple in %
- Voltage output ripple in %
- (Cuk only) L1 ripple %, L2 ripple %, Cn ripple %

3. Output components
- Inductance
- Capacitance
- Boundary current
- Duty cycle
- Load resistance

III. How to run
gcc main.c funcs.c -o main.exe -lm
./main.exe

IV. Author
Minh Tran Nguyen
School of Electrical & Electronic Engineering
University of Leeds - ELEC2645 Embedded Systems Project


