using ModelingToolkit, OrdinaryDiffEq

const γ = 1.4 # = cₚ / cᵥ for air

# const R = 8.31446261815324 # Ideal gas constant, J/(mol K)
const Rₛ = 287.058   # Specific gas constant for dry air, (molar mass) / R

const Patm = 101325  # Pa

# Choked flow
const bcᵣ = 0.528     # Pd / Pu < (2 / (γ + 1))^(γ / (γ - 1))

H(x) = 0.5 * (sign(x) + 1)


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

function deadbandTanh(command, deadband)
    tanhmin = -one(command)
    tanhmax = one(command)
    s = (command - deadband) / (one(command) - deadband) * (tanhmax - tanhmin) + tanhmin
    H(command-deadband) * (tanh(s) - tanh(tanhmin) ) / (tanh(tanhmax) - tanh(tanhmin))
end

# const Amax = 25e-6 # 25mm^2 from LS-V25s data sheet
# const Vdb = 0.07 # Valve deadband on a 0-1 scale

valveArea(command, Amax, db) = Amax * deadbandTanh(abs(command), db)

massFlow(Pu, Pd, T, Cd, a) = (
    H(Pu/Pd - bcᵣ) * ṁ_choked(Pu, T, Cd, a) *
    (1-H(Pu/Pd - bcᵣ)) * ṁ_subsonic(Pd, Pu, T, Cd, a))

function rodFlow(command, P, Pin, T, Cd, Amax, db)
    a = valveArea(command, Amax, db)
    (1-H(command)) * massFlow(Pin, P, T, Cd, a)
end

function baseFlow(command, P, Pin, T, Cd, Amax, db)
    a = valveArea(command, Amax, db)
    H(command) * massFlow(Pin, P, T, Cd, a)
end


function makeSystem()
    @parameters t Apiston r Lcylinder Temp σs σd vmin Pin Vdb Amax Cd

    @variables x(t) v(t) pr(t) pb(t) f(t) u(t)

    @derivatives D'~t

    eqs = [
        D(x) ~ v,
        D(v) ~ pb * Apiston - pr * r * Apiston + f,
        f ~ -σd * v - σs * sign(D(v)) * H(vmin - abs(v)),
        D(pr) ~ rodFlow(u, pr, Pin, Temp, Cd, Amax, Vdb) / (Lcylinder-x) / Apiston / r,
        D(pb) ~ baseFlow(u, pb, Pin, Temp, Cd, Amax, Vdb) / x / Apiston,
    ]

    cylinder = ODESystem(eqs)
end
