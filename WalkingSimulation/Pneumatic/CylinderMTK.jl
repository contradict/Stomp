using ModelingToolkit, DifferentialEquations, IfElse, CSV, Unitful

const γ = 1.4 # = cₚ / cᵥ for air

const R = 8.31446261815324u"J/mol/K" # Ideal gas constant, [J/(mol K)]

const Rₛ = 287.058u"J/kg/K"   # Specific gas constant for dry air, R / (molar mass), [J/(kg K)]

const Patmospheric = 101325u"Pa"  # Pa

# Choked flow
const bcᵣ = 0.528     # Pd / Pu < (2 / (γ + 1))^(γ / (γ - 1))

tsgn(x) = tanh(50*x)

Ht(x) = 0.5 * (tsgn(x) + one(x))

# H(x) = 0.5 * (sign(x) + one(x))

# https://math.stackexchange.com/questions/728094/approximate-x-with-a-smooth-function
# tabs(x; k=100) = 2 * log(1 + exp(k*x)) / k - x - 2 * log(2) / k

# approximate density of air
ρair(P, T) = P / (Rₛ * T)

# approximate viscosity of air
# νair(T) = 2.791e-7 * T^0.7355

# Enfield valve Cd
# const Cd = 0.796

# choked flow
ṁ_choked(P, T, Cd, a) = Cd * a * sqrt(γ * ρair(P, T) * P * bcᵣ)

# Subsonic flow
# https://en.wikipedia.org/wiki/Orifice_plate#Compressible_flow
# Expansibility factor, κ has been approximated as γ
# ϵ(Pr, β) = 1.0 - (0.351 + 0.256β^4 + 0.93β^8)(1.0 - Pr^(1.0/γ))
# For β < 0.25
# [sqrt(ρ P)] = sqrt([kg] [m]^-3 [kg] [m] [s]^-2 [m]^-2) = sqrt([kg]^2 [m]^-4 [s]^-2) = [kg] [m]^-2 [s]^-1
ṁ_subsonic(Pd, Pu, T, Cd, a) = 1.255*(
    Cd * a * sqrt(2 * ρair(Pu, T) * Pu * γ / (γ - 1) * ((Pd/Pu)^(2/γ) - (Pd/Pu)^((γ+1)/γ))))

# Switch between choked and subsonic flow
massFlow(Pu, Pd, T, Cd, a) = IfElse.ifelse(
    signbit(bcᵣ - Pd/Pu),
    ṁ_subsonic(Pd, Pu, T, Cd, a),
    ṁ_choked(Pu, T, Cd, a))

# A smooth deadband function, used to approximate valve response
# 0 < command < 1.0
function deadbandTanh(command, deadband)
    tanhmin = -one(command)
    tanhmax = one(command)
    s = (command - deadband) / (one(command) - deadband) * (tanhmax - tanhmin) + tanhmin
    IfElse.ifelse(signbit(deadband - command), (tanh(s) - tanh(tanhmin) ) / (tanh(tanhmax) - tanh(tanhmin)), 0)
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
function valveFlow(command, P, T, f, Pin, Tin, fin, Patm, Tatm, fatm, Cd, Amax, db)
    a = valveArea(command, Amax, db)
    # it would be nice to do these switches with a tuple!
    Pv = IfElse.ifelse(signbit(command), Patm, Pin)
    Tv = IfElse.ifelse(signbit(command), Tatm, Tin)
    fv = IfElse.ifelse(signbit(command), fatm, fin)
    dir = IfElse.ifelse(signbit(P - Pv), 1, -1)
    Pu = IfElse.ifelse(signbit(P - Pv),  Pv, P)
    Pd = IfElse.ifelse(signbit(P - Pv),  P, Pv)
    Tu = IfElse.ifelse(signbit(P - Pv),  Tv, T)
    ff = IfElse.ifelse(signbit(P - Pv), fv, f)
    dir * ff * massFlow(Pu, Pd, Tu, Cd, a)
end

# https://www.researchgate.net/publication/238185949_Heat_transfer_evaluation_of_industrial_pneumatic_cylinders
function makeSystem()

    @parameters Apiston Cpiston r Vdb Amax Cd Tamb λ t Mass Tin σs σd vmin Pin Patm Tatm tpulse dpulse apulse
    @variables x(t) v(t) Pb(t) Tb(t) Pr(t) Tr(t)
    @derivatives D'~t

    # force on base - force on rod face of piston - force on rod end
    force = (Pb * Apiston - Pr * r * Apiston - Apiston * (1-r) * Patm)
    # A control pulse to make things move
    u = apulse * (Ht(t-tpulse) - 2*Ht(t-(tpulse + dpulse)) + Ht(t-(tpulse + 2*dpulse)))
    eqs = [
        D(x) ~ v,
        # static friction below vmin, columb friction above
        D(v) ~ IfElse.ifelse(signbit(vmin - abs(v)),
                             (force - σd * tsgn(v)),
                             IfElse.ifelse(signbit(σs - force),
                                           (force - σd * tsgn(force)),
                                           0)) / Mass,
        # Eqn 1 at rod side of piston
        D(Pr) ~ (-γ * Pr / (Apiston * r * (1-x)) * (Apiston * r * -v) +
                 γ * Rₛ / (Apiston * r * (1-x)) * valveFlow(-u, Pr, Tr, Tr, Pin, Tin, Tin, Patm, Tatm, Tatm, Cd, Amax, Vdb) +
                 (γ - 1) / (Apiston * r * (1-x)) * λ * ((1-x) * Cpiston) * (Tamb - Tr)),
        # Eqn 2 at rod side of piston
        D(Tr) ~ (Tr / (Apiston * r * (1-x)) * (Apiston * r * -v) * (1 - γ) +
                 Rₛ * Tr / (Apiston * r * (1-x)) / Pr * valveFlow(-u, Pr, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm - Tr), Cd, Amax, Vdb) +
                 (γ - 1) * Tr / Pr / (Apiston * r * (1-x))* λ * ((1-x) * Cpiston) * (Tamb - Tr)),
        # Eqn 1 at base side of piston
        D(Pb) ~ (-γ * Pb / (Apiston * x) * (Apiston * v) +
                 γ * Rₛ / (Apiston * x) * valveFlow(u, Pb, Tb, Tb, Pin, Tin, Tin, Patm, Tatm, Tatm, Cd, Amax, Vdb) +
                 (γ - 1) / (Apiston * x) * λ * (x * Cpiston) * (Tamb - Tb)),
        # Eqn 2 at base side of piston
        D(Tb) ~ (Tb / (Apiston * x) * (Apiston * v) * (1 - γ) +
                 Rₛ * Tb / (Apiston * x) / Pb * valveFlow(u, Pb, Tr, Tr*(γ-1), Pin, Tin, (γ*Tin - Tr), Patm, Tatm, (γ*Tatm-Tr), Cd, Amax, Vdb) +
                 (γ - 1) * Tb / Pb / (Apiston * x)* λ * (x * Cpiston) * (Tamb - Tb)),
    ]
    ODESystem(eqs)
end

function makeParameterMap(sys)
    Ta = 300.0u"K"     # K ambient temperature
    Pa = 101325u"Pa"    # Pa atmospheric pressure
    Rp = 0.025u"m"     # m piston radius
    Ap = π*Rp^2    # m^2 piston area
    rod_r = 0.8    # unitless area ratio of piston to rod

    parmap = Dict(p.name => p for p in parameters(sys))

    p = [
        parmap[:Apiston] => Ap,         # m^2
        parmap[:Cpiston] => π * 2 * Rp,     # m
        parmap[:r] => rod_r,
        parmap[:Vdb] => 0.07,           # unitless command deadband
        parmap[:Amax] => 25e-6u"m^2",         # m^2
        parmap[:Cd] => 0.796,           #
        parmap[:Tamb] => Ta,            # K
        parmap[:λ] => 0.0,              # heat transfer coefficient
        parmap[:Mass] => 0.10u"kg",          # kg piston + rod + load mass
        parmap[:σs] => 0.01u"kg*m/s^2",            # kg m / s^2 static friction
        parmap[:σd] => 0.001u"kg*m/s^2",           # kg m / s^2 dynamic friction
        parmap[:vmin] => 0.001u"m/s",         # m/s minimum speed for static friction
        parmap[:Pin] => 900e3u"Pa",          # Pa ≈ ~135psia valve inlet pressure
        parmap[:Tin] => 273.0u"K",          # K valve inlet temperature
        parmap[:Patm] => Pa,            # Pa
        parmap[:Tatm] => 300.0u"K",         # K
        parmap[:tpulse] => 0.1u"s",         # s pulse start time
        parmap[:dpulse] => 0.3u"s",         # s pulse duration
        parmap[:apulse] => 0.2u"s",         # unitless valve command, pulse amplitude
    ]
end

function makeProblem(sys)
    Ta = 300.0u"K"     # K ambient temperature
    Pa = 101325u"Pa"    # Pa atmospheric pressure
    L = 0.10u"m"       # m cylinder length

    varmap = Dict(st.f.name=>st for st in states(sys))

    u0 = [
        varmap[:x] => L / 2,     # m
        varmap[:v] => 0.0u"m/s",       # m/s
        varmap[:Pr] => Pa,       # Pa
        varmap[:Tr] => Ta,       # K
        varmap[:Pb] => Pa,       # Pa
        varmap[:Tb] => Ta,       # K
    ]

    p = makeParameterMap(sys)

    tspan = (0.0, 1.0)

    ODEProblem(sys, u0, tspan, p, jac=true)
end

function solveProblem(prob)
    solve(prob, adaptive=false, dt=1e-3)
end


function testf(Pr, Apiston, r, x, v, u, Tr, Pin, Tin, Patm, Tatm, Cd, Amax, Vdb, Cpiston, Tamb, λ)
    @show -γ * Pr / (Apiston * r * (1-x)) * (Apiston * r * -v)

    @show  γ * Rₛ / (Apiston * r * (1-x)) * valveFlow(-u, Pr, Tr, Tr, Pin, Tin, Tin, Patm, Tatm, Tatm, Cd, Amax, Vdb)
    @show valveFlow(-u, Pr, Tr, Tr, Pin, Tin, Tin, Patm, Tatm, Tatm, Cd, Amax, Vdb)

    @show  (γ - 1) / (Apiston * r * (1-x)) * λ * ((1-x) * Cpiston) * (Tamb - Tr)
end

function stepProblem(prob, u, t; dt=1e-3)
    u = u + prob.f(u, prob.p, t)*dt
    t += dt
    u, t
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
