using Unitful
using DifferentialEquations
using Parameters
using ComponentArrays
using Plots, UnitfulRecipes

struct KlaffFriction
    fₛ::Unitful.Force
    fᵥ::Unitful.Force
    force::Function
end

@derived_dimension SpringConstant Unitful.𝐌 * Unitful.𝐋 / Unitful.𝐓^2 / Unitful.𝐋
@with_kw struct SystemParameters
    M::Unitful.Mass
    k::SpringConstant
    A::Unitful.Length
    f::Unitful.Frequency
    kₑ::SpringConstant
    fric::KlaffFriction
end

condition(f::KlaffFriction) = (out, u, t, i) -> let p=i.p, v=u[2], state=u[3]
    out[1] = (state ==  0u"st") * ustrip( f.force(u, p, t) - f.fₛ) # start moving pos
    out[2] = (state ==  0u"st") * ustrip(-f.force(u, p, t) - f.fₛ) # start moving neg
    out[3] = (state ==  1u"st") * ustrip(-v)                       # stop from positive motion
    out[4] = (state == -1u"st") * ustrip( v)                       # stop from negative motion
    out
end

affect(f::KlaffFriction) = (i, idx) -> let u=i.u, p=i.p, t=i.t
    if idx==1
        u[3] =  1u"st"
    elseif idx==2
        u[3] = -1u"st"
    elseif idx==3
        if p.fric.force(u, p ,t) < -p.fric.fₛ
            u[3] = -1u"st"
        else
            u[3] = 0u"st"
        end
    elseif idx==4
        if p.fric.force(u, p, t) > p.fric.fₛ
            u[3] = 1u"st"
        else
            u[3] = 0u"st"
        end
    end
end

Base.length(::KlaffFriction) = 4

callback(f::KlaffFriction) = VectorContinuousCallback(condition(f), affect(f), nothing, length(f))
total_force(f::KlaffFriction, u, p, t) = ifelse(u[3]==0.0, 0.0u"kg*m/s^2", f.force(u, p, t) - f.fᵥ * sign(u[2]))

excitation(u, p, t) = @. p.kₑ * (p.A * sin(2π * p.f * t) - u[1])

function dampeddriven!(du, u, p, t)
    x, v, moving = u
    force = total_force(p.fric, u, p, t)
    du[1] = v
    du[2] = force / p.M
    du[3] = 0 * (oneunit(u[3])/oneunit(t))
    du
end

@unit st "st" State 1 false
Unitful.register(@__MODULE__)

function simulate()
    ff(u,p,t) = -p.k*u[1] + excitation(u,p,t)
    fᵥ = 0.5u"N"
    fric = KlaffFriction(
        2*fᵥ,
        fᵥ,
        ff
    )

    p_sys = SystemParameters(
        0.2u"kg",
        11.0u"kg/s^2",
        0.1u"m",
        4.0u"Hz",
        11.0u"kg/s^2",
        fric,
    )

    u0 = ComponentArray(x=0.0u"m", v=0.0u"m/s", state=0u"st")

    tspan = (0u"s", 2.0u"s")

    prob = ODEProblem(dampeddriven!, u0, tspan, p_sys)
    soln = solve(prob, callback=callback(fric))

    p = plot(soln, vars=[(0,1)], label="position", color="green", legend=:topleft)
    plot!(twinx(), soln, vars=[(0,2)], label="velocity", color="blue", legend=:topright)
    display(p)
    soln
end
