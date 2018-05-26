To resolve CAN challenges I always used an Arduino with a module TJA1050 (It is a little board for Arduino with a MCP2515 and can be bought for approximately 1€). 

To control the MCP I used this library  [MCP_CAN_lib](https://github.com/coryjfowler/MCP_CAN_lib)

We need to put a jumper in J1 enable the 120 ohm resistor.

The connections between TJA1050 and Arduino are:
```
Arduino   TJA1050
PIN  2 ---- INT (Configurable)
PIN 13 ---- SCK
PIN 11 ----  SI
PIN 12 ----  SO
PIN 10 ----  CS (Configurable)
GND    ---- GND
VCC    ---- VCC
```

The connections between TJA1050 and challenge board are:
```
TJA1050  Arduino
H   ---- CAN H  
L   ---- CAN L
```