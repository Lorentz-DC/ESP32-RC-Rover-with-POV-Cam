# Project Description
This robotic project is an ESP32 rover featuring bluetooth PS4 gamepad control, and a 2-axis POV camera with stepper motors. Other key on-board modules and components feature INA219 current sensor, TB6612 Motor drivers, XL4016 buck converter, and a VS1838B IR receiver module for testing operations. 

With core features of the Bluetooth control and the 2-axis POV camera, my rover is an extension from a course project: ELE8941 Robotics and Controls, Semester 4, Electrical Engineering Technician Program, Algonquin College. 

This project proved to be an invaluable learning experience introducing me to advanced topics in embedded firmware and power electronics. Join me along this journey, as I highlight and document some key progressions such as troubleshooting Bluetooth connections, improving code architecture for motor controls, and power management. 

![Rover demo 1:](media/rover_demo1.gif)
![POV demo 1:](media/POV_demo1.gif)



**Components:** 

Power source: Two ICR18650, 3.7V rechargeable lithium Batteries with 3350mAh each. 8.2V at full capacity connected in series. 

Car movement: Four TT DC Gearbox Motors controlled by two TB6612  DC motor driver modules;

POV Camera movement: Two 28BYJ-48 stepper motors with two LN2003 driver modules;

Sensors and Low-power modules: INA219 Current sensor; VS1838B IR receiver module;

Voltage regulator: LM7805 circuit; (Optional: XL4016 buck converter;)

The POV cam is a budget wifi home camera with two 28BYJ-48 stepper motors for the the X and Y axis movement; I rewired the motors onto two LN2003 driver modules. For the camera video, the camera itself connects to Wifi, and the video feed can be streamed and downloaded from the Tuya App on mobile devices. 

Rover demo 1 full video: https://youtube.com/shorts/sIEPc9AW5L8?feature=share

POV demo 1 full video: https://youtu.be/2lEK4Y1qqqQ

**Robot Assembly:**

Lower plate assembly:
![Lower plate assembly: ](media/lower_plate.jpg)

Power circuit:
![Power circuit: ](media/power_circuit.jpg)

Upper plate assembly: 
![Upper plate assembly: ](media/upper_plate.jpg)

Complete assembly: 
![Complete assembly: ](media/complete_assembly.jpg)


**Wiring and Schematics:**
![Drawn schematics:](Drawn_Schematics_new.jpg)

DC wheel motors: 

Stepper Motors: 

Power, wiring and grounding: 


**PWM Simulation**
Multisim motor driver PWM simulation: 
This is a Multisim PWM motor driver simulation for 1 DC motor control.
I could not find the Multisim library for TB6612 Driver on Multisim 14.3, so I used the existing DRV8837 in the library to for demonstration purposes. Since for this driver chip IN1 and IN2 need to be swapped in order to reverse the direction, a AND-NOT gate combo was used to swap the input lines. the 5V source is used to simulate Arduino directional pins, and function. 
I'm running on college cloud VPN so the software does not run very smoothly, nevertheless, the simulation gets the point across, showing approximate DC voltages of about 2.5V on a 50% duty cycle from the function generator (simulating ESP32 PWM outputs), 
and about 3.5V on 70% duty cycle. 

![Motor driver PWM simulation](PWM_demo_large.gif) 


**Code Structures:**
Bluepad32 for joystick controls, IR receiver for testing;

Non-blocking code with millis();

Gate-latching DC motor controls on ESP32;

AnalogWrite() and ledcWrite() on ESP32 vs Arduino MCU;

Bluetooth drop-out problem: register thrashing and dual-core bus contention;

Modular individual motor control function with parameters;

MOSFET Shoot-through protection during direction changes; 


**Troubleshooting:** 
Bluetooth dropout: ground bounce and voltage sagging;
