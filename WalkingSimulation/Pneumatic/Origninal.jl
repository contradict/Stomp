const blam = 0.999
const C1=.0404184
const tem2 = 997.9166666
const Patm = 101325  # Pa
const sqrtT = sqrt(300)

function valveFlow(P1, P2, u)
    if u > 5
        if P2 > Patm
            Pu = P2
            Pd = Patm
            dir = -1
        else
            Pu = Patm
            Pd = P2
            dir = 1
        end
    else
        if (P1 > P2)
            Pu = P1
            Pd = P2
            dir = 1
        else
            Pu = P2
            Pd = P1
            dir = -1
        end
    end
    A = valveArea(u)
    tem = Pu / sqrtT
    Pr = Pd / Pu
    if Pr < bcᵣ # choked flow
        flow = dir * A * C1 * tem
    elseif Pr < blam # subsonic flow
        flow = dir * A * C1 * tem * sqrt(1 - (Pr - bcᵣ) * (Pr - bcᵣ) / bcᵣ^2)
    else # laminar flow
        tem = (Pu - Pd) / sqrtT
        flow = dir * A * C1 * tem * (1 - Pr) * tem2
    end
    return flow
end

function InverseValveFlow(P1, P2, q, active)
    if P2 >= Patm  # Muscle pressure is higher than Atm
        if q > 0      # Incharge from source
            Pu = P1
            Pd = P2
            dir = 1
        else        # Outcharge to Atm
            Pu = P2
            Pd = Patm
            dir = -1
        end
    elseif q > 0    # only incharge process with two different ways
        dir = 1       # active incharge from source
        Pu = P1
        Pd = P2
        if !active  # passive incharge from Atm
            Pu = Patm
            Pd = P2
        else          # no negative pressure source, impossible!!!!!just for assignment
            Pu = P2
            Pd = 0
            dir = -1
        end
    end
    tem = Pu / sqrtT
    Pr = Pd / Pu
    if (Pr < bcᵣ)       # choked flow  y=dir*A*C1*tem
        A = q / (dir * C1 * tem)
    elseif Pr < blam  # subsonic flow   y=dir*A*C1*tem*sqrt(1-((Pr-bcᵣ)/(1-bcᵣ))^2);
        A = q / (dir * C1 * tem * sqrt(1 - (Pr - bcᵣ) * (Pr - bcᵣ) / bcᵣ^2))
    else            # laminar flow    y=dir*A*Cd*C1*tem*((1-Pr)/(1-blam))*sqrt(1-((blam-bcᵣ)/(1-bcᵣ))^2);
        tem = (Pu - Pd) / sqrtT
        if (tem == 0)
            A = 0
        else
            A = q / (dir * C1 * tem * (1 - Pr) * tem2)
        end
    end
    vol = InverseValveArea(A, dir)
    return vol
end

function valveArea(u)
    if (u > 5)
        u = 10 - u
    end
    return ((3e-06) / (1 + exp(6 * (u - 4.15))) + 1e-07)
end

function InverseValveArea(area, indir)
    y = 0
    if area < 1.18e-7
        return 5
    end
    if area > 3.09999e-6
        area = 3.09999e-6
    end
    y = 4.15 + log((3e-6) / (area - (1e-7)) - 1) / 6.0f
    if (indir < 0)
        y = 10 - y
    end
    return y
end
