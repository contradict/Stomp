# Hammer operation

The pneumatic hammer makes efficient use of air by allowing an initial charge of
air to expand during the throw phase. The initial charge is metered by opening
the fill valve on command and closing it once the hammer has passed a specified
angle, or by leaving the valve open for a maximum time.

** Hammer throw energy calculations here **

## Braking

During the retract phase, the same techniques is used in the retract cylinder.
To prevent the hammer from colliding with its stop, the hammer is decelerated by
compressing air in the throw cylinder. 

The energy that can be absorbed by the throw cylinder to brake the retract
motion between the angle of initial braking $\bar{\theta}$ and final
stopping angle of $\theta_s$ is

$$E_{brake} = -\int_{\bar{\theta}}^{\theta_s} P(\theta) dV(\theta)$$

Making a simplifying (and incorrect) assumption that the process is isothermal

$$P(\theta) = \frac{P_{atm} V(\bar{\theta})}{V(\theta)}$$

The volume of the throw cylinder is linear in throw angle with as the throw
chain wraps around its sprocket at $p_r$.

$$V(\theta) = \pi r_{throw}^2 p_r \theta$$

Solving

$$E_{brake} = -\int_{ \bar{\theta}}^{\theta_s}
\frac{P_{atm} \pi r_{throw}^2 p_r \bar{\theta}}{\pi r_{throw}^2 p_r \theta}
\pi r_{throw}^2 p_r d\theta$$

$$E_{brake} = -P_{atm} \pi r_{throw}^2 p_r \bar{\theta}
\int_{ \bar{\theta}}^{\theta_s}
\frac{d\theta}{\theta} $$

$$E_{brake} = -P_{atm} \pi r_{throw}^2 p_r \bar{\theta}
(\ln{\theta_s} - \ln{\bar{\theta}})$$

$$E_{brake} = P_{atm} \pi r_{throw}^2 p_r \bar{\theta}
\ln{\frac{\bar{\theta}}{\theta_s}}$$

Close the vent valve when the hammer energy becomes greater than the braking
energy with a suitable stopping angle $\theta_s$.

$$\frac{1}{2} I \omega^2 \leq E_{brake}$$


$$\frac{1}{2} I \omega^2 \leq P_{atm} \pi r_{throw}^2 p_r \theta
\ln{\frac{\theta}{\theta_s}}$$

| Description                 | Parameter      | Value  | Units    |
|-----------------------------|----------------|--------|----------|
| Atmospheric Pressure        | $P_{atm}$      | 101325 | $Pa$     |
| Hammer Moment of Inertia    | $I$            | 1.818  | $kg\,m^2$ |
| Throw Cylinder Radius       | $r_{throw}$    | 0.0762 | $m$      |
| Throw Sprocket Pitch Radius | $p_r$          | 0.0964 | $m$      |
| Hammer Angle                | $\theta$       |        | $rad$    |
| Hammer Brake Angle          | $\bar{\theta}$ |        | $rad$    |
| Hammer Stop Angle           | $\theta_s$     | 0.0873 | $rad$    |

I think the efficient way to evaluate the braking condition is

$$\frac{\frac{1}{2} I}{P_{atm} \pi r_{throw}^2 p_r } \leq
\frac{\theta}{\omega^2} \ln{\frac{\theta}{\theta_s}}$$

Checking units.

$$\frac{[kg][m]^2}{[kg][m]^{-1}[s]^{-2}[m]^2[m]} = [s]^2$$

Numerically

$$ 5.102e-3 \leq \frac{\theta}{\omega^2} \ln{\frac{\theta}{\theta_s}}$$

## Sensor calibration.


### Hammer Angle

The voltage from the hammer angle sensor should always be $0.5V \leq v \leq 4.5V$,
voltages outside this range indicate a disconnected or damaged sensor.

The hammer angle sensor reads $v_{min}=4.004V$ at $\theta=0$ and $v_{max}=1.941V$
at $\theta=\pi$.

Symbolically

$\theta(v) = \frac{v-v_{min}}{v_{max} - v_{min}} \pi$

With the values measured from our current assembly

$\theta(v) = (4.004 - v) * 6.481$ 

### Pressure Sensors

The pressure sensor on both cylinders is an SSI Technologies
[P51-500-A-B-I36-4.5V](https://www.ssi-sensors.com/perch/resources/documents/p51-pressure-sensor-p51-ds.pdf).

The pressure sensor voltages should always be $0.5V \leq v \leq 4.5V$,
voltages outside this range indicate a disconnected or damaged sensor.

This sensor has a $P_{max}=500PSIA=3.447e6Pa$ full-scale reading at
$v_{max}=4.5V$ and a voltage of $v_{min}=0.5V$ at $P_{min}=0$ absolute pressure.

The absolute pressure is

$$P(v) = \frac{(v-v_{min})(P_{max} - P_{min})}{v_{max} - v_{min}} + P_{min}$$

In $Pa$,

$$P(v) = (v-0.5) * 8.618e5$$
