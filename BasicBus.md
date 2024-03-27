BasicBus Specification
======================

(c) Six Mile Creek Systems 2004-2018.  Designed by Timothy Weber.

A simple serial protocol, based loosely on the original BasicBus design from 2004.
Comfortably supports a handful of low-data-rate slaves controlled by a master.
Power can be supplied from the bus as well.

Core purpose is to distribute sensors and actuators near the main processor,
e.g. in the same room, providing a little power, over cheap, light, commonly-available cable.

## Concepts

There is one Master, and one or more Slaves.

At most one Slave is selected at a time.  Other Slaves stay connected to the bus,
but do not respond until they are selected.

Data types used:

* **Byte**: An unsigned value, 0-255.
* **Word**: An unsigned value, 0-65535.
* **Short**: A signed value, -32768 - 32767.
* **Float**: A floating-point value, represented in decimal ASCII.

The Master has a Status, consisting of one unsigned byte (0-255).
Its meaning is application-defined.

The Slaves have:

* A Status, one byte, application-defined
* A set of Parameters, each with a short value, used for mostly static configuration
* A set of Variables, each with a short value, used for primary readings and actuation.

## Limits

The limit of the number of Slaves depends on the source impedance or 
fan-out of the Master's serial driver (MOSI).  5 is definitely feasible; 10 is probably
at the limit.

Response time for that many slaves is probably around 1 second latency, if you're polling
each slave for 50-100 ms.

Variables are two bytes, so only individual data points are supported, not streaming data.

## Physical

Connection is via flat, 4-conductor, 28 AWG cable, commonly called "telephone line cord".
A maximum resistance of 1.1 Ohm/meter is recommended.

Connectors are RJ11, 6-position, 4-conductor (6P4C), though 6P6C (aka RJ14) are also compatible. 
These are American "plain old telephone service" (POTS) connectors.

If 6P6C/RJ14 connectors and 6-pin cables are used, two extra lines are
available, which can be used e.g. for controlling a programming bootloader.
See the BasicBus Transceiver project for a reference design.

### Straight-wired cables

Cables MUST be wired "straight" - meaning the RJ11 plugs have opposite orientations on either end.
Available cables seem to be inconsistent about this, because the POTS usage didn't care.

One way to specify and check this is: When assembling cables, ensure that the plug tab faces
the cable seam at one end, and the plug on the other end faces away from the cable seam.

This results in cables looking like this, viewing the plugs end-on:
    
     _==_              ____
    |    |============|    |
     ----              -__-
     1234              4321

Another test for straight wiring: If you line up the two plugs at opposite ends of the cable
so that the conductors are visible and parallel, like this:

     _______    _______
    |1|2|3|4|  |1|2|3|4|
    |1|2|3|4|  |1|2|3|4|  <-- 4 copper conductors
    |-------|  |-------|
    | ##### |  | ##### |
    |_______|  |_______|
      |   |      |   |

you should get continuity between the numbered pairs as shown (and no others).

## Electrical

Wiring to the jacks are as follows - with mnemonic color coding that matches some common jacks:

    Pin | Color  | Meaning   
    ----| ------ | ----------
    1   | black  | GND
    2   | red    | V+
    3   | green  | MOSI
    4   | yellow | MISO

Notes:

* **GND** is the common ground.
* **V+** is nominally 5 V at the Master, but will go down near 3.3 V at 100' of cable.  
    It can source 500 mA combined across all slaves (of course Masters could alter that).
* **MOSI** is Master Out, Slave In.
* **MISO** is Master In, Slave Out.

### Transmission

Transport is via full-duplex asynchronous serial at 9600 baud, 8 data bits, 1 stop bit.

The Master only transmits on MOSI, and receives on MISO.

The Slaves all receive on MOSI, and transmit when selected on MISO.  When a slave is not selected,
it must leave MISO floating.

### Termination

The Master is responsible for pulling MISO up to V+ using a 20K resistor to
prevent noise while Slaves pass off control of the MISO line. If it does not
provide this pullup, it must take responsibility to wait 100 ms after
selecting a Slave before it listens to the MISO line.  (This may result in
missing the response, so it may require selecting the Slave again.)

TBD: 
* pullups on Slaves?
* capacitance

## Protocol

Commands and responses must be on a line (newline-delimited) that starts with '~'.

Commands and responses may be grouped together on a line, separated by spaces.

Examples first, with newline represented as `\n`:

"Master status is 5" (good to report periodically)
    ~S=5\n
"Set the connected slave's index to 1" (when provisioning a new device, set its index to a known value)
    ~?=* #=1\n
"Read all parameters, all variable values, and a specific additional one"
(the slave may hide some variables except on request)
    ~?P ?* ?V\n
"Set parameter 0 to 37"
    ~P0=37\n
"Set variable T to 43.22"
    ~T=43.22\n
"Request variable V"
    ~?V\n
"Regular status exchange"
    ~S=4 ?=* ?*\n

### Master commands

The following commands are sent from the master:

* S=b  
    Reports the master status, b.
* ?=b  
    Selects the slave with index b.  All other slaves should disconnect from the MISO line within 30 ms.
    The slave responds with its Status, within 100 ms.
* ?=*  
    Selects whatever slave is currently connected (should be only one).
    The slave responds with its Status, within 100 ms.

The following commands from the master are to be executed only by the currently-selected slave:

* #=b  
    Sets the slave's index to b, permanently.  
    The slave responds with "#=b".  
    Commonly used to set the index of a single slave, the only one attached to the bus,
    during setup, like "~?=* #=1".
* Pn=w  
    Tells the slave to set parameter n (0-relative byte)'s to the new value w (word).
* P!
    Tells the slave to reset all its parameters to their defaults.
    The slave responds as for ?*.
* ?S  
    Requests the slave's status, as an S=n response.
* ?P  
    Requests a report of all parameters.
* ?A  
    Requests a reading for variable code A.  
    A is any ASCII code (but traditionally capital or lowercase letters), except P.
* ?*
    Requests a reading of all variables and all changed parameters.
    When all have been sent, the slave will send the "." command.
* A=x  
    Tells the slave to set variable code A (typically an actuator) with value x.  
    x is a decimal short or floating-point value.
    Only implemented by the slave for actuators, not sensors.

### Slave responses

The slave sends these responses back to the master, when requested as above:

* Pn=w  
    Reports that parameter n (0-relative byte)'s new value is w (word).
* S=n  
    Reports a change in the slave's status, as a byte value.
* A=x  
    Sends a reading for variable code A with value x.  
    x is a decimal short or floating-point value.
* .
    Indicates that all pending responses to past commands have now been sent.
    Follows ?* and ?P.
    If the Master only sends one ?P or ?* command at a time and waits for this,
    it will definitively indicate that everything's been sent.
