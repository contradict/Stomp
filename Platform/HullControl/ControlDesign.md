# Notes concerning design of Hull remote control software

## Leg Control Process

This process is responsible for communication with the leg microcontrollers and
computing the leg positions.

```
  thread 1 (real-time scheduler, 100Hz)
    Odd loops
      parameters = read parameter queue
      X, Y, Z = compute leg position ( parameters )
      Send Leg positions (X, Y, Z)
    even loops
      data = Read position, pressure from each leg
      Queue data in telemetry queue
      if command in command queue
        send to hardware
        Queue response in response queue
  thread 2
    receive messages from control topic, compute parameters, enqueue to parameters queue
  thread 3 
    Receive commands from topic, queue to real-time command queue
    Receive responses from response queue, send to response topic
    Receive data from telemetry queue, send to telemetry topic
```

### Leg position generation, interpolated gait

This is the `compute leg position` subroutine mentioned above.

```
    Inputs: forward velocity V, curvature \kappa, interval \delta t
    State: phase
    Constants: Step shape, robot half-width W
    Each call
        compute frequency that results in full length step for legs on fastest side
            V_left = V * (1 - W * \kappa)
            V_right = V * (1 + W * \kappa)
            V_max = signed_max(V_left, V_right) 
            \omega = V_max / max_step_length
        compute step scale
            \alpha[left legs] = V_left / V_max
            \alpha[right legs] = V_right / V_max
        advance phase
            \phi += \omega * \delta t
        compute scaled leg positions from step shape
            for each leg l
                X[l], Y[l], Z[l] = step_shape(\phi + \leg_offset[l])
                Y[l] *= \alpha[l]
    return X, Y, Z
```

## Control radio process

Read from S.BUS serial port and produce messages for the rest of the system.

```
  forever
    Read packet from radio
    parse to axes and switches
    send to control topic
```

## Telemetry COSMOS interface

Translate internal LCM messages to COSMOS, bidirectional.

```
  Poll loop with serial port, topic file descriptors:
      forward received packets to command topic
      Forward response topic to radio
      forward telemetry topic to radio
```

## Others

 * IMU Data?
 * Pressure sensors
