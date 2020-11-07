using ModelingToolkit, OrdinaryDiffEq

const γ = 1.4 # = cₚ / cᵥ for air

# const R = 8.31446261815324 # Ideal gas constant, J/(mol K)
const Rₛ = 287.058   # Specific gas constant for dry air, R / (molar mass)

const Patm = 101325  # Pa

# Choked flow
const bcᵣ = 0.528     # Pd / Pu < (2 / (γ + 1))^(γ / (γ - 1))

tsgn(x) = tanh(100*x)

Ht(x) = 0.5 * (tsgn(x) + 1)

H(x) = 0.5 * (sign(x) + one(x))


# density of air
ρair(P, T) = P / (Rₛ * T)

# viscosity of air
νair(T) = 2.791e-7 * T^0.7355

# Enfield valve Cd
# const Cd = 0.796

ṁ_choked(P, T, Cd, a) = Cd * a * sqrt(γ * ρair(P, T) * P * bcᵣ)

# Subsonic flow
# https://en.wikipedia.org/wiki/Orifice_plate#Compressible_flow
# Expansibility factor, κ has been approximated as γ
# ϵ(Pr, β) = 1.0 - (0.351 + 0.256β^4 + 0.93β^8)(1.0 - Pr^(1.0/γ))
# For β < 0.25
# [sqrt(ρ P)] = sqrt([kg] [m]^-3 [kg] [m] [s]^-2 [m]^-2) = sqrt([kg]^2 [m]^-4 [s]^-2) = [kg] [m]^-2 [s]^-1
ṁ_subsonic(Pd, Pu, T, Cd, a) = Cd * a * sqrt(2 * ρair(Pu, T) * Pu * γ / (γ - 1) * ((Pd/Pu)^(2/γ) - (Pd/Pu)^((γ+1)/γ)))

# Switch between choked and subsonic flow
massFlow(Pu, Pd, T, Cd, a) = (
    Ht(Pu/Pd - bcᵣ) * ṁ_choked(Pu, T, Cd, a) +
    (1-Ht(Pu/Pd - bcᵣ)) * ṁ_subsonic(Pd, Pu, T, Cd, a))

# A smooth deadband function, used to approximate valve response
function deadbandTanh(command, deadband)
    tanhmin = -one(command)
    tanhmax = one(command)
    s = (command - deadband) / (one(command) - deadband) * (tanhmax - tanhmin) + tanhmin
    Ht(command-deadband) * (tanh(s) - tanh(tanhmin) ) / (tanh(tanhmax) - tanh(tanhmin))
end

# Values estimated from datasheet to get area from command
# const Amax = 25e-6 # 25mm^2 from LS-V25s data sheet
# const Vdb = 0.07 # Valve deadband on a 0-1 scale
valveArea(command, Amax, db) = Amax * deadbandTanh(abs(command), db)

# Estimate flow for one port of a 3-way valve.
# If command is positive, flow from pressure source to cylinder
# If command is negative, flow from cylinder to atmosphere
function valveFlow(command, P, Pin, T, Cd, Amax, db)
    a = valveArea(command, Amax, db)
    (Ht(command) * massFlow(Pin, P, T, Cd, a) -
     (1-Ht(command)) * massFlow(P, Patm, T, Cd, a))
end

# Base end of cylinder is connected to pressure with postive command
baseFlow(command, P, Pin, T, Cd, Amax, db) = valveFlow(command, P, Pin, T, Cd, Amax, db)
# Rod end of cylinder is connected to pressrure with negative command
rodFlow(command, P, Pin, T, Cd, Amax, db) = valveFlow(1.0 - command, P, Pin, T, Cd, Amax, db)

function makeSystem()

    Apiston = π*0.025^2
    r = 0.8
    Lcylinder = 0.10
    Vdb = 0.07
    Amax = 25e-6
    Cd = 0.796
    Tinit = 300.0

    @parameters t Mass Tin σs σd vmin Pin
    @variables x(t) v(t) a(t) f(t) Pb(t) Tb(t) Pr(t) Tr(t) u(t)
    @derivatives D'~t

    eqs = [
        D(x) ~ v,
        D(v) ~ a,
        0 ~ Mass * a - (Pb * Apiston - Pr * r * Apiston + f - Apiston * (1-r) * Patm), # F=ma
        0 ~ f + -σd * v - σs * tsgn(a) * Ht(vmin - abs(v)), # friction = dynamic + static
        D(Pr) ~ -γ/(Apiston * r * (1-x))*(-P * (Apiston * r * v) + Rₛ*rodFlow(u, Pr, Pin, Tin, Cd, Amax, Vdb)
        D(Mb) ~ baseFlow(u, Pb, Pin, Tin, Cd, Amax, Vdb),   # mass of air on base
        0 ~ D(Pb * Apiston * x / (Tb * Mr * Rₛ)),           # Ideal gas law on base
        0 ~ D(Pb^(γ-1) / Tb^γ),                             # isentropic on base
        0 ~ -u + Ht(t-0.1) * 0.1 + Ht(t-0.2) * -0.2 + Ht(t-0.3) * 0.1 # A control pulse to make things move
    ]
    cylinder = ODESystem(eqs)

    u0 = [
        x => Lcylinder / 2,
        v => 0.0,
        a => 0.0,
        Mr => Patm * (Apiston * r * (Lcylinder / 2)) * Rₛ / Tinit,
        Pr => Patm,
        Tr => Tinit,
        Mb => Patm * (Apiston * (Lcylinder / 2)) * Rₛ / Tinit,
        Pb => Patm,
        Tb => Tinit,
        f => 0,
        u => 0,
    ]

    p = [
        Mass => 1.0,
        Tin => 273.0,
        σs => 0.01,
        σd => 0.001,
        vmin => 0.001,
        Pin => 900e3, # ~135psia
    ]

    tspan = (0.0, 1.0)

    prob = ODEProblem(cylinder, u0, tspan, p, jac=true)

    sol = solve(prob, Rodas4P())
end
