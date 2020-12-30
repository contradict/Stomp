using DifferentialEquations, CSV, Unitful, Parameters, RecursiveArrayTools

const γ = 1.4 # = cₚ / cᵥ for air

const R = 8.31446261815324u"J/mol/K" # Ideal gas constant, [J/(mol K)]

const Rₛ = 287.058u"J/kg/K"   # Specific gas constant for dry air, R / (molar mass), [J/(kg K)]

const Patmospheric = 101325u"Pa"  # Pa

# https://www.mathworks.com/help/physmod/hydro/ref/variableorificeiso6358g.html
@derived_dimension SonicConductance Unitful.𝐋^3 / ( Unitful.𝐓 * (Unitful.𝐌 * Unitful.𝐋^-1 * Unitful.𝐓^-2))
@with_kw struct ISO6358Valve
    C::SonicConductance = 4.1e-8u"s*m^4/kg"
    ρ₀::Unitful.Density = 1.185u"kg/m^3"
    T₀::Unitful.Temperature = 273.15u"K"
    bₗₐₘ::Float64 = 0.999
    bcᵣ::Float64 = 0.35
    m::Float64 = 0.5
end

function (v::ISO6358Valve)(pᵢₙ, Tᵢₙ, pₒᵤₜ)

    @unpack_ISO6358Valve v

    pᵣ = pₒᵤₜ / pᵢₙ

    if pᵣ > 1
        # pₒᵤₜ > pᵢₙ valve is being used incorrectly
        zero(ρ₀ * pᵢₙ)
    elseif bₗₐₘ < pᵣ
        # laminar flow
        C * ρ₀ * ((pₒᵤₜ - pᵢₙ) / (1 - bₗₐₘ)) * √(T₀ / Tᵢₙ) * (1 - ((bₗₐₘ - bcᵣ) / (1 - bcᵣ))^2)^m
    elseif bcᵣ < pᵣ
        # subsonic flow
        C * ρ₀ * pᵢₙ * √(T₀ / Tᵢₙ) * (1 - ((pᵣ - bcᵣ) / (1 - bcᵣ))^2)^m
    else
        # choked flow
        C * ρ₀ * pᵢₙ * √(T₀ / Tᵢₙ)
    end
end

# A smooth deadband function, used to approximate valve response
# 0 < command < 1.0
function deadbandTanh(command, deadband)
    tanhmin = -one(command)
    tanhmax = one(command)
    s = (command - deadband) / (one(command) - deadband) * (tanhmax - tanhmin) + tanhmin
    if command>deadband
        (tanh(s) - tanh(tanhmin) ) / (tanh(tanhmax) - tanh(tanhmin))
    else
        0
    end
end

# Values estimated from datasheet to get area from command
# const Amax = 25e-6 # 25mm^2 from LS-V25s data sheet
# const Vdb = 0.07 # Valve deadband on a 0-1 scale
valveArea(command, Amax, db) = Amax * deadbandTanh(abs(command), db)

# Compute the flow for one port of a 3-way valve.
# If command is positive, connect cylinder to Presure Source
# If command is negative, connect cylinder to atmosphere
# Always call massflow with high pressure as first argument and correct the flow
# direction later
# Use the correct temperature corresponding to the flow direction
# f is a factor to carry along to keep track of heat flow that depends on flow
# direction
function valveFlow(command, P, T, f, Pin, Tin, fin, Patm, Tatm, fatm, Amax, db, valve)
    a = valveArea(command, 1.0, db)
    Pv, Tv, Fv = ifelse(command<0, (Patm, Tatm, fatm), (Pin, Tin, fin))
    dir, Pu, Pd, Tu, ff = ifelse((P < Pv), (1, Pv, P, Tv, Fv), (-1, P, Pv, T, f))
    #@show Pu Tu Pd
    dir * ff * a * valve(Pu, Tu, Pd)
end

# https://www.researchgate.net/publication/238185949_Heat_transfer_evaluation_of_industrial_pneumatic_cylinders
function cylinder!(du, u, p, t)

    L, Apiston, Cpiston, r, Vdb, Amax, Cd, Tamb, λ, Mass, Tin, σs, σd, vmin, Pin, Patm, Tatm, tpulse, dpulse, apulse, valve = p
    x, v, Pb, Tb, Pr, Tr = u

    # force on base - force on rod face of piston - force on rod end
    force = Pb * Apiston - Pr * r * Apiston - Apiston * (1-r) * Patm

    # A control pulse to make things move
    if tpulse<=t<tpulse+dpulse
        pulse = apulse
    elseif tpulse+dpulse<=t<tpulse+2*dpulse
        pulse = -apulse
    else
        pulse = 0
    end

    # static friction below vmin, columb friction above
    if abs(v) < vmin
        if force < σs
            du[1] = zero(du[1])
            du[2] = zero(du[2])
        else
            du[1] = v
            du[2] = (force - σd * sign(force)) / Mass
        end
    else
        du[1] = v
        du[2] = (force - σd * sign(v)) / Mass
    end

    # [kg] [m] [s]^-2 [kg] [m]^-2 [m]^-4 = [kg]^2 [m]^-5 [s]^-2
    Vpiston = Apiston * x
    dVpiston = Apiston * v
    # Eqn 1 at base side of piston
    du[3] = (-γ * Pb / Vpiston * dVpiston +
              γ * Rₛ / Vpiston * valveFlow(pulse, Pb, Tb, Tb, Pin, Tin, Tin, Patm, Tatm, Tatm, Amax, Vdb, valve) +
              (γ - 1) / Vpiston * λ * (x * Cpiston) * (Tamb - Tb))
    # Eqn 2 at base side of piston
    du[4] = (Tb / Vpiston * dVpiston * (1 - γ) +
             Rₛ * Tb / Vpiston / Pb * valveFlow(pulse, Pb, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm-Tr), Amax, Vdb, valve) +
             (γ - 1) * Tb / Pb / Vpiston * λ * (x * Cpiston) * (Tamb - Tb))

    Vrod = Apiston * r * (L - x)
    dVrod = Apiston * r * -v
    # Eqn 1 at rod side of piston
    du[5] = (-γ * Pr / Vrod * dVrod +
              γ * Rₛ / Vrod * valveFlow(-pulse, Pr, Tr, Tr, Pin, Tin, Tin, Patm, Tatm, Tatm, Amax, Vdb, valve) +
              (γ - 1) / Vrod * λ * ((L-x) * Cpiston) * (Tamb - Tr))
    # Eqn 2 at rod side of piston
    du[6] = (Tr / Vrod * dVrod * (1 - γ) +
             Rₛ * Tr / Vrod / Pr * valveFlow(-pulse, Pr, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm - Tr), Amax, Vdb, valve) +
             (γ - 1) * Tr / Pr / Vrod * λ * ((L-x) * Cpiston) * (Tamb - Tr))
    du
end

function makeProblem()
    Ta = 300.0u"K"     # K ambient temperature
    Pa = 101325.0u"Pa"    # Pa atmospheric pressure
    Rp = 0.025u"m"     # m piston radius
    Ap = π*Rp^2    # m^2 piston area
    rod_r = 0.8    # unitless area ratio of piston to rod
    L = 0.10u"m"       # m cylinder length

    p = (
        L,                #L        m
        Ap,               #Apiston  m^2
        π * 2 * Rp,       #Cpiston  m
        rod_r,            #r
        0.07,             #Vdb      unitless command deadband
        25e-6u"m^2",      #Amax     m^2
        0.796,            #Cd       Valve coefficient
        Ta,               #Tamb     K
        0.0u"W/K/m^2",              #λ        heat transfer coefficient
        0.10u"kg",        #Mass     kg piston + rod + load mass
        273.0u"K",        #Tin      K valve inlet temperature
        0.01u"kg*m/s^2",  #σs       kg m / s^2 static friction
        0.001u"kg*m/s^2", #σd       kg m / s^2 dynamic friction
        0.001u"m/s",      #vmin     m/s minimum speed for static friction
        900e3u"Pa",       #Pin      Pa ≈ ~135psia valve inlet pressure
        Pa,               #Patm     Pa
        300.0u"K",        #Tatm     K
        0.1u"s",          #tpulse   s pulse start time
        0.05u"s",          #dpulse   s pulse duration
        0.1,              #apulse   unitless valve command, pulse amplitude
        ISO6358Valve(),
    )

    u0 = ArrayPartition(
        [L / 2],     # x   m
        [0.0u"m/s"], # v   m/s
        [Pa],        # Pb  Pa
        [Ta],        # Tb  K
        [Pa],        # Pr  Pa
        [Ta],        # Tr  K
    )

    tspan = (0.0u"s", 1.0u"s")

    ODEProblem(cylinder!, u0, tspan, p)
end

function solveProblem(prob)
    solve(prob, Tsit5(), d_discontinuities=[0.1u"s", 0.2u"s", 0.3u"s"])
end


function stepProblem(prob, u, t; dt=1e-3u"s")
    du = zero(u/oneunit(t))
    prob.f(du, u, prob.p, t)*dt
    u += du * dt
    t += dt
    u, t
end

function persistent_euler(prob; dt=1e-3u"s")
    u = deepcopy(prob.u0)
    uhist = [u]
    du = zero(u / 1.0u"s")
    t = 0.0u"s"
    for s ∈ 1:1e6
        try
            u, t = stepProblem(prob, u, t; dt=dt)
        catch e
            @show e
            break
        end
        push!(uhist, u)
    end
    t, uhist
end

function fitValveParameters(commandfile, flowfile)
    aperture = CSV.File(commandfile, header=[:command, :fraction])
    flow = CSV.File(flowfile, header=1:2)
    PSIAtoPA=6894.76
    SCFMtokgs=4.9e-4
    function loss(a, db, cd)
        for inlet_pressure in 20:20:200
            outlet_pressure = CSV.getcolumn(flow, Symbol("OutletPressure_$(inlet_pressure)psia")) |> skipmissing |> collect
            outlet_pressure *= PSIAtoPA
            outlet_flow = CSV.getcolumn(flow, Symbol("Flow_$(inlet_pressure)psia")) |> skipmissing |> collect
            outlet_flow *= SCFMtokgs
            valveFlow()
        end
    end
end
