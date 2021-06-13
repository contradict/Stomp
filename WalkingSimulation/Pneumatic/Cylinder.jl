using DifferentialEquations, CSV, Unitful, Parameters
using RecursiveArrayTools
using Plots, UnitfulRecipes
using Optim

const γ = 1.4 # = cₚ / cᵥ for air

const R = 8.31446261815324u"J/mol/K" # Ideal gas constant, [J/(mol K)]

const Rₛ = 287.058u"J/kg/K"   # Specific gas constant for dry air, R / (molar mass), [J/(kg K)]

const Patmospheric = 101325.0u"Pa"  # Pa

include("valve.jl")

# from fits to datasheet
const bestDeadband = 0.132
const bestValve = ISO6358Valve(;C=3.31e-8u"m^4*s*kg^-1", bcᵣ=0.527)

# https://www.researchgate.net/publication/238185949_Heat_transfer_evaluation_of_industrial_pneumatic_cylinders
function cylinder!(du, u, p, t)

    L, Apiston, Cpiston, r, Vdb, Amax, Tamb, λ, Mass, Tin, σs, σd, vmin, Pin, Patm, Tatm, command, valve = p
    x, v, Pb, Tb, Pr, Tr = u

    # force on base - force on rod face of piston - force on rod end
    force = Pb * Apiston - Pr * r * Apiston - Apiston * (1-r) * Patm

    du[1] = v
    du[2] = (force - σd*sign(v/1u"m/s")) / Mass

    # [kg] [m] [s]^-2 [kg] [m]^-2 [m]^-4 = [kg]^2 [m]^-5 [s]^-2
    Vpiston = Apiston * x
    dVpiston = Apiston * v
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

function makeProblem()
    Ta = 300.0u"K"     # K ambient temperature
    Pa = 101325.0u"Pa" # Pa atmospheric pressure
    Rp = 0.025u"m"     # m piston radius
    Ap = π*Rp^2        # m^2 piston area
    rod_r = 0.8        # unitless area ratio of piston to rod
    L = 0.10u"m"       # m cylinder length

    p = [
        L,                #L        m
        Ap,               #Apiston  m^2
        π * 2 * Rp,       #Cpiston  m
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
        Pa,               #Patm     Pa
        300.0u"K",        #Tatm     K
        0.0,              #valve command value
        bestValve,
    ]

    u0 = ArrayPartition(
        [L / 2],     # x   m
        [0.0u"m/s"], # v   m/s
        [Pa],        # Pb  Pa
        [Ta],       # Tb  K
        [Pa],       # Pr  Pa
        [Ta],       # Tr  K
    )

    tspan = (0.0u"s", 1.0u"s")

    ODEProblem(cylinder!, u0, tspan, p)
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
    solve(prob, d_discontinuities=discontinuities(pulse), callback=cb, abstol=1e-10, reltol=1e-10)
end

function solveProblem(prob)
    solve(prob, abstol=1e-10, reltol=1e-10)
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
