using ModelingToolkit, DifferentialEquations, IfElse, Plots

@parameters t k M σs σd vmin
@variables x(t) v(t)
@derivatives D'~t

tsgn(x) = tanh(x*10)
Ht(x) = (1 + tsgn(x)) / 2

isign(x) = IfElse.ifelse(signbit(x), -1, 1)
Hi(x) = (1 + isign(x)) / 2

eqs_sm = [
    D(x) ~ v,
    D(v) ~ (Ht(abs(v) - vmin) * (-k * x - σd * tsgn(v)) +
            Ht(vmin - abs(v)) * Ht(k * abs(x) - σs) * (-k * x + tsgn(x) * σs)) / M
]

eqs = [
    D(x) ~ v,
    D(v) ~ IfElse.ifelse(abs(v) < vmin,
                         IfElse.ifelse(signbit(k*abs(x) - σs),
                                       0,
                                       -k * x + isign(x) * σs),
                         -k * x - σd * isign(v)) / M
]

sys_sm = ODESystem(eqs_sm, t)

sys = ODESystem(eqs, t)

u0 = [x=>0, v=>10]

p = [k=>4.0, M=>0.25, σd=>0.1, σs=>1.0, vmin=>0.01]

tspan = (0.0, 50.0)

prob_sm = ODEProblem(sys_sm, u0, tspan, p, jac=true)

prob = ODEProblem(sys, u0, tspan, p, jac=true)

sol_sm = solve(prob_sm, Rodas4P());

sol = solve(prob, Rodas4P());

plot!(sol_sm, vars=(0,2))

plot(sol, vars=(0,2))

expr = (2.5 * (1 + tanh(10 * ((-1 * abs(v⦗t⦘)) + vmin))) *
 ((k * x⦗t⦘) + (-1 * tanh(10 * x⦗t⦘) * σs)) *
 (1 + (-1 * (tanh(10 * ((k * abs(x⦗t⦘)) + (-1 * σs))) ^ 2))) *
 ifelse(signbit(x⦗t⦘), -1, 1) * k)
