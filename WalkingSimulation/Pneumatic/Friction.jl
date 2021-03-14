using DifferentialEquations
using Parameters
using Plots

struct KlaffFriction
    fₛ::Float64
    fᵥ::Float64
    force::Function
end

@with_kw struct SystemParameters
    M::Float64
    k::Float64
    A::Float64
    f::Float64
    kₑ::Float64
    fric::KlaffFriction
end

condition(f::KlaffFriction) = (out, u, t, i) -> let p=i.p, v=u[2], state=u[3]
    out[1] = (state ==  0) * ( f.force(u, p, t) - f.fₛ) # start moving pos
    out[2] = (state ==  0) * (-f.force(u, p, t) - f.fₛ) # start moving neg
    out[3] = (state ==  1) * (-v)                       # stop from positive motion
    out[4] = (state == -1) * ( v)                       # stop from negative motion
    out
end

affect(f::KlaffFriction) = (i, idx) -> let u=i.u, p=i.p, t=i.t
    @show t, idx
    if idx==1
        u[3] =  1
    elseif idx==2
        u[3] = -1
    elseif idx==3
        if p.fric.force(u, p, t) < -p.fric.fₛ
            u[3] = -1
        else
            u[3] =  0
        end
    elseif idx==4
        if p.fric.force(u, p, t) > p.fric.fₛ
            u[3] =  1
        else
            u[3] =  0
        end
    end
end

Base.length(::KlaffFriction) = 4

callback(f::KlaffFriction) = VectorContinuousCallback(condition(f), affect(f), nothing, length(f))
total_force(f::KlaffFriction, u, p, t) = ifelse(u[3]==0, 0.0, f.force(u, p, t) - f.fᵥ * sign(u[2]))

excitation(u, p, t) = p.kₑ * (p.A * sin.(2π * p.f * t) .- u[1])

function dampeddriven!(du, u, p, t)
    x, v = u
    force = total_force(p.fric, u, p, t)
    du[1] = v
    du[2] = force / p.M
    du[3] = 0
    du
end

function simulate()

    ff(u,p,t) = -p.k * u[1] + excitation(u, p, t)

    fᵥ = 0.5
    fric = KlaffFriction(2*fᵥ, fᵥ, ff)

    p_sys = SystemParameters(
        0.1,    # M::Float64
        1.0,    # k::Float64
        0.1,    # A::Float64
        1.0,    # f::Float64
        11.0,    # kₑ::Float64
        fric,
    )

    u0 = [0.0, 0.0, 0]

    tspan = (0, 5.0)

    prob = ODEProblem(dampeddriven!, u0, tspan, p_sys)
    soln = solve(
        prob,
        callback = callback(fric),
        maxiters = 2000,
    )

    pl = plot(soln, vars = [(0, 1)], label = "position", color = "green", legend = :topleft)
    # pl = plot(soln, vars = [(0, 2)], label = "velocity", color = "blue", legend = :topleft)
    plot!(
        twinx(pl),
        soln,
        vars = [(0, 2)],
        label = "velocity",
        color = "blue",
        legend = :topright,
    )
    # plot!(pl, soln, vars=[((t,x,v)->(t, ff([x, v, 0.0], p_sys, t)-fric.fᵥ), 0, 1, 2),
    #                       ((t,x,v)->(t, -ff([x, v, 0.0], p_sys, t)+fric.fᵥ), 0, 1, 2)])
    # plot!(pl, soln, vars=[#((t,x,v)->(t, condition(fric)([0.0,0.0,0.0,0.0], [x,v], t, (p=p_sys,))[1]), 0, 1, 2),
    #                       #((t,x,v)->(t, condition(fric)([0.0,0.0,0.0,0.0], [x,v], t, (p=p_sys,))[2]), 0, 1, 2)],
    #                       #((t,x,v)->(t, condition(fric)([0.0,0.0,0.0,0.0], [x,v], t, (p=p_sys,))[3]), 0, 1, 2),
    #                       ((t,x,v)->(t, condition(fric)([0.0,0.0,0.0,0.0], [x,v], t, (p=p_sys,))[4]), 0, 1, 2)],
    #       color="red")
    display(pl)
    soln
end
