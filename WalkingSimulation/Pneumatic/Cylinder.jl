using DifferentialEquations, CSV, Unitful, Parameters
using RecursiveArrayTools
using ComponentArrays
using Plots, UnitfulRecipes
using Optim


# const R = 8.31446261815324u"J/mol/K" # Ideal gas constant, [J/(mol K)]

include("valve.jl")

# from fits to datasheet
const bestDeadband = 0.132
const bestValve = ISO6358Valve(;C=3.31e-8u"m^4*s*kg^-1", bcᵣ=0.527)

# https://www.researchgate.net/publication/238185949_Heat_transfer_evaluation_of_industrial_pneumatic_cylinders
function cylinder!(du, u, p, t)

    L, Rp, r, Vdb, Amax, Tamb, λ, Mass, Tin, σs, σd, vmin, Pin, Patm, Tatm, command, valve, γ, Rₛ = p
    x, v, Pb, Tb, Pr, Tr = u

    # force on base - force on rod face of piston - force on rod end
    Apiston = π*Rp^2
    force = Pb * Apiston - Pr * r * Apiston - Apiston * (1-r) * Patm

    du[1] = v
    du[2] = (force - σd*sign(v/oneunit(v))) / Mass

    # [kg] [m] [s]^-2 [kg] [m]^-2 [m]^-4 = [kg]^2 [m]^-5 [s]^-2
    Vpiston = Apiston * x
    dVpiston = Apiston * v
    Cpiston = 2π*Rp
    # Eqn 1 at base side of piston
    du[3] = (-γ * Pb / Vpiston * dVpiston +
              γ * Rₛ / Vpiston * valveFlow(command, Pb, Tb, Tb, Pin, Tin, Tin, Patm, Tatm, Tatm, Amax, Vdb, valve) +
              (γ - 1) / Vpiston * λ * (x * Cpiston) * (Tamb - Tb))
    # Eqn 2 at base side of piston
    du[4] = (Tb / Vpiston * dVpiston * (1 - γ) +
             Rₛ * Tb / Vpiston / Pb * valveFlow(command, Pb, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm-Tr), Amax, Vdb, valve) +
             (γ - 1) * Tb / Pb / Vpiston * λ * (x * Cpiston) * (Tamb - Tb))

    Vrod = Apiston * r * (L - x)
    dVrod = Apiston * r * -v
    # Eqn 1 at rod side of piston
    du[5] = (-γ * Pr / Vrod * dVrod +
              γ * Rₛ / Vrod * valveFlow(-command, Pr, Tr, Tr, Pin, Tin, Tin, Patm, Tatm, Tatm, Amax, Vdb, valve) +
              (γ - 1) / Vrod * λ * ((L-x) * Cpiston) * (Tamb - Tr))
    # Eqn 2 at rod side of piston
    du[6] = (Tr / Vrod * dVrod * (1 - γ) +
             Rₛ * Tr / Vrod / Pr * valveFlow(-command, Pr, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm - Tr), Amax, Vdb, valve) +
             (γ - 1) * Tr / Pr / Vrod * λ * ((L-x) * Cpiston) * (Tamb - Tr))
    du
end

function compcylinder!(du, u, p, t)

    # force on base - force on rod face of piston - force on rod end
    Apiston = π*p.Rp^2
    force = u.Pb * Apiston - u.Pr * p.r * Apiston - Apiston * (1-p.r) * p.Patm

    du[1] = u.v
    du[2] = (force - p.σd*sign(u.v/oneunit(u.v))) / p.Mass

    # [kg] [m] [s]^-2 [kg] [m]^-2 [m]^-4 = [kg]^2 [m]^-5 [s]^-2
    Vpiston = Apiston * u.x
    dVpiston = Apiston * u.v
    Cpiston = 2π*p.Rp
    # Eqn 1 at base side of piston
    du[3] = (-p.γ * u.Pb / Vpiston * dVpiston +
             p.γ * p.Rₛ / Vpiston * valveFlow(p.command, u.Pb, u.Tb, u.Tb, p.Pin, p.Tin, p.Tin, p.Patm, p.Tatm, p.Tatm, p.Amax, p.Vdb, p.valve[1]) +
              (p.γ - 1) / Vpiston * p.λ * (u.x * Cpiston) * (p.Ta - u.Tb))
    # Eqn 2 at base side of piston
    du[4] = (u.Tb / Vpiston * dVpiston * (1 - p.γ) +
             p.Rₛ * u.Tb / Vpiston / u.Pb * valveFlow(p.command, u.Pb, u.Tr, u.Tr*(p.γ-1), p.Pin, p.Tin, (p.γ*p.Tin - u.Tr), p.Patm, p.Tatm, (p.γ*p.Tatm-u.Tr), p.Amax, p.Vdb, p.valve[1]) +
             (p.γ - 1) * u.Tb / u.Pb / Vpiston * p.λ * (u.x * Cpiston) * (p.Ta - u.Tb))

    Vrod = Apiston * p.r * (p.L - u.x)
    dVrod = Apiston * p.r * -u.v
    # Eqn 1 at rod side of piston
    du[5] = (-p.γ * u.Pr / Vrod * dVrod +
             p.γ * p.Rₛ / Vrod * valveFlow(-p.command, u.Pr, u.Tr, u.Tr, p.Pin, p.Tin, p.Tin, p.Patm, p.Tatm, p.Tatm, p.Amax, p.Vdb, p.valve[1]) +
              (p.γ - 1) / Vrod * p.λ * ((p.L-u.x) * Cpiston) * (p.Ta - u.Tr))
    # Eqn 2 at rod side of piston
    du[6] = (u.Tr / Vrod * dVrod * (1 - p.γ) +
             p.Rₛ * u.Tr / Vrod / u.Pr * valveFlow(-p.command, u.Pr, u.Tr, u.Tr*(p.γ-1), p.Pin, p.Tin, (p.γ*p.Tin - u.Tr), p.Patm, p.Tatm, (p.γ*p.Tatm - u.Tr), p.Amax, p.Vdb, p.valve[1]) +
             (p.γ - 1) * u.Tr / u.Pr / Vrod * p.λ * ((p.L-u.x) * Cpiston) * (p.Ta - u.Tr))
    du
end


function makeProblem()
    γ = 1.4 # = cₚ / cᵥ for air
    Rₛ = 287.058u"J/kg/K"   # Specific gas constant for dry air, R / (molar mass), [J/(kg K)]
    Patmospheric = 101325.0u"Pa"  # Pa

    Ta = 300.0u"K"     # K ambient temperature
    Rp = 0.025u"m"     # m piston radius
    rod_r = 0.8        # unitless area ratio of piston to rod
    L = 0.10u"m"       # m cylinder length

    p = [
        L,                #L        m
        Rp,               #Apiston  m^2
        rod_r,            #r
        bestDeadband,     #Vdb      unitless command deadband
        25e-6u"m^2",      #Amax     m^2
        Ta,               #Tamb     K
        0.0u"W/K/m^2",    #λ        heat transfer coefficient
        1.00u"kg",        #Mass     kg piston + rod + load mass
        273.0u"K",        #Tin      K valve inlet temperature
        10.0u"kg*m/s^2",  #σs       kg m / s^2 static friction
        1.0u"kg*m/s^2",   #σd       kg m / s^2 dynamic friction
        0.001u"m/s",      #vmin     m/s minimum speed for static friction
        900e3u"Pa",       #Pin      Pa ≈ ~135psia valve inlet pressure
        Patmospheric,     #Patm     Pa
        300.0u"K",        #Tatm     K
        0.0,              #valve command value
        bestValve,
        γ,
        Rₛ
    ]

    u0 = ArrayPartition(
        [L / 2],     # x   m
        [0.0u"m/s"], # v   m/s
        [Patmospheric],        # Pb  Pa
        [Ta],       # Tb  K
        [Patmospheric],       # Pr  Pa
        [Ta],       # Tr  K
    )

    tspan = (0.0u"s", 1.0u"s")

    ODEProblem(cylinder!, u0, tspan, p)
end

function makeCompProblem()

    γ = 1.4 # = cₚ / cᵥ for air
    Rₛ = 287.058u"J/kg/K"   # Specific gas constant for dry air, R / (molar mass), [J/(kg K)]

    Patmospheric = 101325.0u"Pa"  # Pa

    Ta = 300.0u"K"     # K ambient temperature

    p = ComponentVector(
        L = 0.10u"m",          # m cylinder length
        Rp = 0.025u"m",        # m piston radius
        r = 0.8,               # unitless area ratio of piston to rod
        Vdb=bestDeadband,      #Vdb      unitless command deadband
        Amax=25e-6u"m^2",      #Amax     m^2
        Ta = Ta,               # K ambient temperature
        λ = 0.0u"W/K/m^2",     #λ        heat transfer coefficient
        Mass = 1.00u"kg",      #Mass     kg piston + rod + load mass
        Tin = 273.0u"K",       #Tin      K valve inlet temperature
        σs = 10.0u"kg*m/s^2",  #σs       kg m / s^2 static friction
        σd = 1.0u"kg*m/s^2",   #σd       kg m / s^2 dynamic friction
        vmin = 0.001u"m/s",    #vmin     m/s minimum speed for static friction
        Pin = 900e3u"Pa",      #Pin      Pa ≈ ~135psia valve inlet pressure
        Patm = Patmospheric,   #Patm     Pa
        Tatm = 300.0u"K",      #Tatm     K
        command = 0.0,         #valve command value
        valve = (bestValve,),
        γ = γ,
        Rₛ = Rₛ
    )

    u0 = ComponentVector(
        x = p.L / 2,     # x   m
        v = 0.0u"m/s", # v   m/s
        Pb = Patmospheric,        # Pb  Pa
        Tb = Ta,       # Tb  K
        Pr = Patmospheric,       # Pr  Pa
        Tr = Ta,       # Tr  K
    )

    tspan = (0.0u"s", 1.0u"s")

    ODEProblem(compcylinder!, u0, tspan, p)
end


struct Pulse
    start::Unitful.Time
    duration::Unitful.Time
    amplitude::Float64
end

function condition(p::Pulse)
    pulse_condition(u, t, integrator) = ustrip((t-p.start)*(t-p.start-p.duration)*(t-p.start-2*p.duration))
end

function affect(p::Pulse)
    function pulse_affect!(integrator)
        integrator.p[17] = if p.start <= integrator.t < p.start + p.duration
            p.amplitude
        elseif p.start + p.duration <= integrator.t < p.start + 2 * p.duration
            -p.amplitude
        else
            0.0
        end
    end
end

discontinuities(p::Pulse) = [p.start, p.start+p.duration, p.start+2*p.duration]

callback(p::Pulse) = PresetTimeCallback(discontinuities(pulse), affect(p))

function solveProblem(prob, pulse)
    cb = callback(pulse)
    solve(prob, d_discontinuities=discontinuities(pulse), callback=cb; hints=[:stiff])
end

function solveProblem(prob)
    #solve(prob; maxiters=1e8, dt=1e-4u"s") # , alg_hints=[:stiff])
    solve(prob; maxiters=1e9, alg_hints=[:stiff])
end

function stepProblem(prob, u, t; dt=1e-3u"s")
    du = zero(u/oneunit(t))
    prob.f(du, u, prob.p, t)
    u += du * dt
    t += dt
    u, t
end

function persistent_euler(prob; dt=1e-3u"s")
    u = deepcopy(prob.u0)
    uhist = [u]
    du = zero(u / 1.0u"s")
    t = prob.tspan[1]
    while t < prob.tspan[2]
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
