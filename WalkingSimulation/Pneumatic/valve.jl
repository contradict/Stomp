# https://www.mathworks.com/help/physmod/hydro/ref/variableorificeiso6358g.html
@derived_dimension SonicConductance Unitful.𝐋^3 / ( Unitful.𝐓 * (Unitful.𝐌 * Unitful.𝐋^-1 * Unitful.𝐓^-2))
@with_kw struct ISO6358Valve
    C::SonicConductance = 4.1e-8u"s*m^4/kg"
    ρ₀::Unitful.Density = 1.2041u"kg/m^3"
    T₀::Unitful.Temperature = 293.15u"K"
    bₗₐₘ::Real = 0.999
    bcᵣ::Real = 0.35
    m::Real = 0.5
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
    dir * ff * a * valve(Pu, Tu, Pd)
end

function loadValveData(flowfile)
    flow = CSV.File(flowfile, header=1:2)
    # SCFM at 20C for this data
    SCFMtokgs=uconvert(u"kg/s", 1u"ft^3/minute"*1.2041u"kg/m^3")
    valid_inlet_pressure = 20:20:200
    outlet_pressure = Dict(
        inlet_pressure=>CSV.getcolumn(flow, Symbol("OutletPressure_$(inlet_pressure)psia")) |> skipmissing |> collect |> x->x*u"psi"
        for inlet_pressure in valid_inlet_pressure)
    outlet_flow = Dict(
        inlet_pressure=>CSV.getcolumn(flow, Symbol("Flow_$(inlet_pressure)psia")) |> skipmissing |> collect |> x->x*SCFMtokgs
        for inlet_pressure in valid_inlet_pressure)
    return (valid_inlet_pressure, outlet_pressure, outlet_flow)
end

function plotValveData(inp, outp, outf)
    plt = plot()
    for p in inp
        l = scatter!(plt, outp[p], outf[p]; label="Pᵢ=$p")
    end
    plt
end

function fitValveParameters(inp, outp, outf)
    function loss(params)
        C = params[1]*1e-8u"s*m^4/kg"
        bcᵣ = exp(params[2])
        valve = ISO6358Valve(;C=C, bcᵣ=bcᵣ)
        sum(inp) do inlet_pressure
            pᵢ = inlet_pressure*u"psi"
            Tᵢ = 293.15u"K"
            pₒ = outp[inlet_pressure]
            flow = outf[inlet_pressure]
            vflow = valve.(pᵢ, Tᵢ, pₒ)
            sum(((flow .- vflow).^2))/1u"kg^2/s^2"
        end
    end
    opt = optimize(loss, [4.1, log(0.35)], BFGS())
    @show opt
    C = opt.minimizer[1]*1e-8u"s*m^4/kg"
    bcᵣ = exp(opt.minimizer[2])
    ISO6358Valve(;C=C, bcᵣ=bcᵣ)
end

function plotValveFit(plt, v, inp, outp)
    Tᵢ = 293.15u"K"
    for Pᵢ ∈ inp
        Pₒ = outp[Pᵢ]
        flow = v.(Pᵢ*u"psi", Tᵢ, Pₒ)
        plot!(plt, Pₒ, uconvert.(u"kg/s", flow); color=:black, label="")
    end
    plt
end

function fitAperture(commandfile)
    aperture = CSV.File(commandfile, header=[:command, :fraction])
    cmd = (aperture.command .- 50) ./ 50
    loss(params) = sum((valveArea.(cmd, 1.0, params[1]) - aperture.fraction).^2)
    opt = optimize(loss, [0.1], BFGS())
    @show opt
    plt = scatter(aperture.command, aperture.fraction)
    plot!(aperture.command, valveArea.(cmd, 1.0, opt.minimizer[1]))
    display(plt)
    opt.minimizer
end


