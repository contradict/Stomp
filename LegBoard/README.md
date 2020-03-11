# Stomp Leg Board

Each leg of the robot needs a control board to interface to the servos valves
and position sensors. The servos communicate with proprietary serial protocol
and the position sensors measure joint angle which needs to be converted to
cylinder length to use the built-in feedback loop.

## Board Layout

I added a representative connector for each connection this board will need and
picked a pinout. The layout of the connectors is being designed in parallel with
the rest of the leg, hopefully that will be settled soon. The pinout can be
freely changed, please update the Block Diagram schematic if it does.

## Electrical Specifications

## Power

Power is distrubeted from a central power distribution block on the robot
directly from a battery pack. The supplied voltage will be in the 18-28V range.
Each leg board will be supplied through a fuse at the distribution block.

### Enfield S2 servo valve output

| Signal         | Description                                      |
|----------------|--------------------------------------------------|
| Output Voltage | 24V nominal, the data sheet specs 10-28v         |
| Output Current | 1A per valve                                     |
| RXD            | 5V TTL asynchronous serial data from servo valve |
| TXD            | 5V TTL asynchronous serial data to servo valve   |
| Feedback       | 0-10V cylinder position measurement              |

The feedback signal should have an analog bandwidth of at least 1kHz.

### Angular Position Sensor

| Signal          | Description |
|-----------------|-------------|
| +5V             | Power to angle sensor, 10mA |
| Vsense          | Measured angle, 0.25 to 4.75 over specified angle range |

The measurement circuit for Vsense should have at least 1kHz bandwidth.

### Communications Bus

The communication bus connects all the Leg Boards to the platform
microcontroller. The two connectors pass through A and B lines to make wiring
simple.

| Signal | Description   |
|------------|---------------|
| A          | RS-485 A line |
| B          | RS-485 B line |
| GND        | Reference voltage for RS485 transcievers |
| Upstream   | Connected to the Leg Board closer to the microcontroller or the |
|            | microcontroller |
| Downstream | Connected to the Leg Board farter from the microcontroller |

I'm not sure how the grounding should work, it may be that ground should not be
wired in the harness on these connetors to avoid a ground loop, but I would like
to provide it in case we have signal integrity problems.

The `Upstream` and `Downstream` pins should each be connected to a processor
GPIO pin.  I would like to use them to avoid having to hard code addresses on
each board.

## Signal Processing

Since the position sensors we are installing measure angle and the servo valves
need to know cylinder length, each channela $i$  needs to calculate

```math
\theta_i =  as_i * (V_{sense} + ao_i)
l_{cylinder} = a_i + b_i \cos(\theta_i + \phi_i )
V_{feedback} = fo_i + fs_i * l_{cylinder}
```

Where $as_i$ and $ao_i$ define the conversion from sensor voltage to angle in
radians, $a_i$, $b_i$, and $phi_i$ define the geometry of the link, and $fo_i$
and $fs_i$ define the conversion from cylinder length back to voltage.

This calculation should take at most 20us per channel to ensure low latency and
CPU usage for the full calculation. This should not be hard for a
microcontroller with a clock rate of at least 50MHz.

## Communications Protocols

### Enfield S2

The processor will need to implement a serial port interface to the
Enfield S2 for delivering command positions and monitoring status. The
reverse-engineered protocol is described in the
[README](../../EnfieldProtocol/README.md). 

The pressure and feedback registers should be continously monitored while
sending the current position command.

### Leg Bus (RS-485)

The commnuication between the Platform microcontroller and the Leg Controllers
over the RS-485 bus should implement the MODBUS protocol.

TODO: MODBUS Register Map.
