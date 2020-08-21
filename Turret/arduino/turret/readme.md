# Table Of Contents <!-- omit in toc --> #

- [Design Overview](#design-overview)
- [Controller Hierarchy](#controller-hierarchy)
- [Main Program (turret.ino)](#main-program-turretino)
- [State Machine Pattern](#state-machine-pattern)
- [Controller Pattern](#controller-pattern)
- [Notes about changes to original Chomp code](#notes-about-changes-to-original-chomp-code)

# Design Overview #

Most logic is implemented c++ classes that use the Chomp controller pattern ([see below](#controller-pattern)).  Each controller implements a state machine ([see state machine pattern below](#state-machine-pattern)).

There are 4 global controllers

- Turret Controller
- Target Tracking Controller
- Radio Controller
- Telemetry Controller

Each of these are a global instances and they are all declared in turret.ino.  Then an <code>extern</code> deceleration is provided in each of those controller's class header file.

The Turret Controller and the TargetTrackingController will consturct new private instances of the controller types they need to do thier job.  However, each of these private controllers are owned by a global controller, which should not expose access to those controllers.

# Controller Hierarchy #

The current controller hierachy looks like this

    Global Objects                Sub Objects (instantiated in global object init())
    --------------                --------------------------------------------------

    TurretController -------------+--- TurretRotationController --------+--- AutoAimController
                                  |
                                  +--- HammerController
                                  |
                                  +--- FlameThrowerController
                                  |
                                  +--- IMUController
                                  |
                                  + --- AutoFireController

    TargetTrackingController -----+--- TargetAcquisitionController ------+--- LeddarController

    RadioController

    TelemetryController

# Main Program (turret.ino) #

The main Arduino file is turret.ino.  This file is where the 4 global controllers are declared.

    TurretController Turret;
    TargetTrackingController TargetTracking;
    RadioController Radio;
    TelemetryController Telem;

In the <code>setup()</code> method, that is called when the Arduino is (re)started, the <code>Init()</code> method of the 4 global controllers is called to get everything started.  Then the <code>RestoreParas()</code> call is issed to the global controllers

    void setup()
    {
        Telem.Init();
        Radio.Init();
        TargetTracking.Init();
        Turret.Init();

        Telem.RestoreParams();
        TargetTracking.RestoreParams();
        Turret.RestoreParams();
    }

Then each update that the Arduino issues by calling <code>loop()</code> will call the <code>Update()</code> method on each of the 4 global controllers.

    void loop()
    {
        Radio.Update();
        TargetTracking.Update();
        Turret.Update();
        Telem.Update();
    }

# State Machine Pattern #

Each controller will define its states in an <code>enum</code> in the header file.  For example this is what the flameThrowerController defines:

    enum controllerState 
    {
        EInit,
        ESafe,
        
        EDisabled,
        EReadyToFire,

        EPulseFlameOn,
        EManualFlameOn,
 
        EInvalid = -1
    };

The state machine pattern used by all controllers basically consists of code in the main <code>Init()</code> method, the <code>Update()</code> method and a private <code>setState(controllerState p_state)</code> method.

The <code>Init()</code> method will ensure the state is set and initialize the <code>m_lastUpdateTime</code> private member variable.  Finally, the state will be set to the initial state, which should be name <code>EInit</code>.  All other initilization should happen in the code triggered by transitioning into the <code>EInit</code> state.

    void FlameThrowerController::Init()
    {
        m_state = EInvalid;    
        m_lastUpdateTime = micros();

        setState(EInit);
    }

The <code>Update()</code> will first do any update calculations needed to detect state transitions, then the current state of the controller is stablized by running through all valid state transitions.  Once the state has stablized, then any other update code can run.

    void FlameThrowerController::Update()
    {
        m_lastUpdateTime = micros();

        //  Do any processing necessary for being able to detect state transitions

        ...

        //  Stabilize our state

        while(true)
        {
            controllerState prevState = m_state;

            switch (m_state)
            {
                case EInit:
                {
                    setState(ESafe);
                }
                break;

                case ESafe:
                {
                    //  Stay in safe mode for a minimum of k_safeStateMinDt

                    if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
                    {
                        setState(EDisabled);
                    }
                }
                break;

                case EDisabled:
                {
                    if (!Radio.IsNominal())
                    {
                        setState(ESafe);
                    }
                    else if (Radio.IsFlameEnabled() || Radio.IsFlamePulseEnabled())
                    {
                        setState(EReadyToFire);
                    }
                }
                break;

                ...

                default:
                break;
            }

            //  No more state changes, move on
            
            if (m_state == prevState)
            {
                break;
            }
        }

        //  Now that the state is stable, perform any other update processing

        ...
    }

# Controller Pattern #

# Notes about changes to original Chomp code #

Files that were just removed because they were no longer necessary

- autodrive.cpp
- drive.cpp
- hold_down.cpp
- rc_pwm.cpp
- selfright.cpp
- telem_message_stream.h

Files that had there contents become a contoller object

    New File                                Old File
    ------------------------------          ------------------------------
    turretController.cpp                    chomp_main.cpp
    autoFireController.cpp                  autofire.cpp
    autoAimController.cpp                   autoaim.cpp
    radioController.cpp                     sbus.cpp
    telemetryController.cpp                 telem.cpp & xbee.cpp
    imuController.cpp                       imu.cpp
    leddarController.cpp                    leddar_io.cpp
    target.cpp                              object.cpp
    targetTrackingController.cpp            track.cpp
    targetAcquisitionController.cpp         targeting.cpp

