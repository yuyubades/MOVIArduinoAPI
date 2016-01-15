# MOVIArduinoAPI
This is the official repository of the Arduino API for MOVI.
For more information check out http://www.audeme.com/MOVI

# Changes and instructions
This version is for the Huzzah ESP8266 board.

The Huzzah ESP8266 board has no software serial, also it will be programmed over the RX/TX UART. 
The board has one full functional UART RX/TX which will be used for communication between MOVI and the board. The problem is that we then do not have a good way to get debug messages, because the normal UART is already in use, luck the Huzzah ESP8266 has one additional TX pin labeld has pin #2.
The serial UART is defined within the code as Serial and the debug TX pin (#2) as Serial1.

The library will initialize Serial1 after "init" call. If you want to have it earlier please just comment the Serial1.begin command out and call this in your required location. 

The wiring is simple.
OPEN Jumper 1, else the ESP8266 get destroyed, because it uses 3.3V logic, it is written that an internal logic level shifter is used to also support 5V for RX pin, but I recommend to use 3.3V

CLOSE Jumper 2 and 3, in that case the RX/TX pins are on D11/D10 on the MOVI Shield.
1. Connect the RX pin from Huzzah ESP8266 to the TX (D10) pin on the MOVI Shield.
2. Connect the TX pin from Huzzah ESP8266 to the RX (D11) pin on the MOVI Shield.
3. Connect the 3.3V labeled pin from Huzzah ESP8266 to the IOREF labeled pin on the MOVI Shield.
4. Connect the 3.3V labeled pin from Huzzah ESP8266 to the IOREF labeled pin on the MOVI Shield.

That's all for the connections between MOVI and the Huzzah ESP8266.
You also need to get power so connect the V+ from the external power supply to the Vbat pin on the Huzzah ESP8266 and also to the MOVI VIn pin.
Connect the GND from the power supply to one of the GND on the Huzzah ESP8266 and also to the GND labeled pin on the MOVI Shield (I used the one beside the VIN pin).

To eliminate current peaks (for example when the power supply goes on) place a 1000µF between the power supply and the first device (Huzzah ESP8266 or MOVI Shield).

# How to debug
Connect the RX pin from a serial adapter (which you use for programming the ESP8266) to the #2 pin (Serial1 TX pin) on the Huzzah ESP8266 and connect the GND pin from the serial adapter to any GND pin on the Huzzah ESP8266.

The baudrate for Serial1 is set to 115200 but you can change this within the header at "DEBUG_SERIAL_BAUDRATE"
