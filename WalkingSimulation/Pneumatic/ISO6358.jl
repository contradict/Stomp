module ISO6358

using IfElse: ifelse
using ModelingToolkit

function get_model()
    @variables pᵢₙ Tᵢₙ pₒᵤₜ
    # @parameters C = 0.5, ρ₀ = 1.185, T₀ = 273.15, bₗₐₘ = 0.999, bcᵣ = 0.3, m = 0.5
    @parameters C ρ₀ T₀ bₗₐₘ bcᵣ m

    # [:C=>C, :ρ₀=>ρ₀, :T₀=>T₀, :bₗₐₘ=>bₗₐₘ, :bcᵣ=>bcᵣ, :m=>m]

    pᵣ = pₒᵤₜ / pᵢₙ

    ṁ_ch = C * ρ₀ * pᵢₙ * √(T₀ / Tᵢₙ)
    ṁₜᵤᵣ = C * ρ₀ * pᵢₙ * √(T₀ / Tᵢₙ) * (1 - ((pᵣ - bcᵣ) / (1 - bcᵣ))^2)^m
    ṁₗₐₘ =
        C *
        ρ₀ *
        ((pₒᵤₜ - pᵢₙ) / (1 - bₗₐₘ)) *
        √(T₀ / Tᵢₙ) *
        (1 - ((bₗₐₘ - bcᵣ) / (1 - bcᵣ))^2)^m

    ṁ = ifelse(
        signbit(1 - pᵣ),
        0, # greater than 1 - flow is backwards
        ifelse(
            signbit(bₗₐₘ - pᵣ),
            ṁₗₐₘ, # within bₗₐₘ of 1, laminar flow
            ifelse(
                signbit(bcᵣ - pᵣ),
                ṁₜᵤᵣ, # less than laminar, not yet choked
                ṁ_ch, # choked flow
            ),
        ),
    )
    # build_function(ṁ, (pᵢₙ, Tᵢₙ, pₒᵤₜ), expression = Val{false})
end

end
