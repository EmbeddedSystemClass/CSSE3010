# CSSE3010
Firmware written for Embedded Systems Design and Interfacing (CSSE3010) in Semester 1, 2018 at the University of Queensland. This consisted of:
+ Weekly in-class exercises
+ Weekly take-home demonstrations
+ Two projects

## Weekly Demonstrations
Projects were developed focussing on various embedded systems concepts. 

|        | Concepts                                           |
| ------ | ---------------------------------------------------|
| Week 1 | Pushbuttons and LEDs                               |
| Week 2 | Pan tilt servo, PWM                                |
| Week 3 | Joystick, ADC, infrared TX/RX, timer input capture |
| Week 4 | Interfacing with radio transceiver, FSMs           |
| Week 5 | Real time operating systems, CLI                   |
| Week 6 | Additional RTOS features                           |


## Projects
### Project 1: Duplex Radio/IR Communications 
This project implemented duplex communications between two STM32F4 dev boards over radio, with an acknowledgement scheme over infrared. The radio packets used a (7, 4) hamming encoding with a parity bit for error detection and correction. The acknowledgement scheme implemented ACK, ERR and timeouts, and employed manchester encoding. 

### Project 2: Remote Plotter Controller
This project wirelessly controls a Computer Numerical Controlled (CNC) plotter and focusses on using RTOS in an embedded design. The design receives commands over a CLI or IR and transmits packets to the remote plotter for drawing.  
