#include <stdlib.h>
#include <string.h>

#include "sclog4c/sclog4c.h"

#include "leg_control/toml_utils.h"

int toml_vector_float(toml_array_t *a, float dest[3])
{
    for(int oi=0;oi<3;oi++)
    {
        toml_raw_t tomlr = toml_raw_at(a, oi);
        if(tomlr == 0)
            return -1;
        double tmp;
        if(-1==toml_rtod(tomlr, &tmp))
            return -1;
        dest[oi] = tmp;
    }
    return 0;
}

int get_float(toml_table_t *tab, char *name, float *f)
{
    toml_raw_t tomlr = toml_raw_in(tab, name);
    if(tomlr == 0)
        return -1;
    double tmpd;
    int err = toml_rtod(tomlr, &tmpd);
    if(err == 0)
    {
        *f = tmpd;
    }
    return err;
}

struct leg_description *parse_leg_descriptions(toml_table_t *legs_config, int *nlegs)
{
    toml_raw_t tomlr = toml_raw_in(legs_config, "count");
    int64_t num_legs;
    toml_rtoi(tomlr, &num_legs);
    *nlegs = num_legs;
    toml_array_t *descriptions = toml_array_in(legs_config, "description");
    struct leg_description *legs = calloc(num_legs, sizeof(struct leg_description));

    for(int leg=0; leg<toml_array_nelem(descriptions); leg++)
    {
        toml_table_t *desc = toml_table_at(descriptions, leg);
        tomlr = toml_raw_in(desc, "index");
        int64_t index;
        toml_rtoi(tomlr, &index);
        if(index < *nlegs)
        {
            legs[index].index = index;
            tomlr = toml_raw_in(desc, "name");
            toml_rtos(tomlr, &legs[index].name);
            tomlr = toml_raw_in(desc, "address");
            int64_t addr;
            toml_rtoi(tomlr, &addr);
            legs[index].address = addr;
            toml_array_t *o= toml_array_in(desc, "orientation");
            toml_vector_float(o, legs[index].orientation);
            o= toml_array_in(desc, "origin");
            toml_vector_float(o, legs[index].origin);
        }
    }
    return legs;
}

void free_leg_description(struct leg_description *legs, int nlegs)
{
    for(int i=0; i<nlegs; i++)
    {
        free(legs[i].name);
    }
    free(legs);
}

struct joint_gains *parse_joint_gains(toml_table_t *legs_config)
{
    toml_table_t *joints = toml_table_in(legs_config, "joint_gain");
    const char *joint_names[3] = {"Curl", "Swing", "Lift"};
    struct joint_gains *gains = malloc(sizeof(struct joint_gains));
    for(int j=0; j<3; j++)
    {
        toml_table_t *joint = toml_table_in(joints, joint_names[j]);
        get_float(joint, "ProportionalGain", &gains->proportional_gain[j]);
        get_float(joint, "DerivativeGain", &gains->derivative_gain[j]);
        get_float(joint, "ForceDamping", &gains->force_damping[j]);
    }
    return gains;
}

struct step *parse_steps(toml_table_t *config, int *nsteps)
{
    toml_array_t *step_descriptions = toml_array_in(config, "steps");
    *nsteps = toml_array_nelem(step_descriptions);
    struct step *steps = calloc(*nsteps, sizeof(struct step));
    for(int s=0; s<*nsteps; s++)
    {
        toml_table_t *step = toml_table_at(step_descriptions, s);
        toml_raw_t tomlr = toml_raw_in(step, "name");
        toml_rtos(tomlr, &steps[s].name);
        get_float(step, "length", &steps[s].length);
        get_float(step, "direction_swap_tolerance", &steps[s].swap_tolerance);
        toml_array_t *swap_phase = toml_array_in(step, "direction_swap_phase");
        steps[s].nswap = toml_array_nelem(swap_phase);
        steps[s].swap_phase = calloc(steps[s].nswap, sizeof(float));
        for(int p=0;p<steps[s].nswap;p++)
        {
            tomlr = toml_raw_at(swap_phase, p);
            double tmpd;
            toml_rtod(tomlr, &tmpd);
            steps[s].swap_phase[p] = tmpd;
        }
        toml_array_t *points = toml_array_in(step, "points");
        steps[s].npoints = toml_array_nelem(points);
        steps[s].phase = calloc(steps[s].npoints + 1, sizeof(float));
        steps[s].X = calloc(steps[s].npoints, sizeof(float));
        steps[s].Y = calloc(steps[s].npoints, sizeof(float));
        steps[s].Z = calloc(steps[s].npoints, sizeof(float));
        for(int p=0; p<steps[s].npoints; p++)
        {
            toml_table_t *pt = toml_table_at(points, p);
            get_float(pt, "phase", &steps[s].phase[p]);
            get_float(pt, "X", &steps[s].X[p]);
            get_float(pt, "Y", &steps[s].Y[p]);
            get_float(pt, "Z", &steps[s].Z[p]);
        }
        steps[s].phase[steps[s].npoints] = 1.0f;
    }
    return steps;
}

void free_step_descriptions(struct step *steps, int nsteps)
{
    for(int s=0;s<nsteps;s++)
    {
        free(steps[s].name);
        free(steps[s].phase);
        free(steps[s].X);
        free(steps[s].Y);
        free(steps[s].Z);
    }
    free(steps);
}

static
int find_step(const struct step *steps, int nsteps, const char *step_name)
{
    for(int s=0;s<nsteps;s++)
    {
        if(strcmp(step_name, steps[s].name) == 0)
            return s;
    }
    return -1;
}

struct gait *parse_gaits(toml_table_t *config, int *ngaits, const struct step* steps, int nsteps)
{
    toml_array_t *gait_descriptions = toml_array_in(config, "gaits");
    *ngaits = toml_array_nelem(gait_descriptions);
    struct gait *gaits = calloc(*ngaits, sizeof(struct gait));
    for(int g=0; g<*ngaits; g++)
    {
        toml_table_t *gait = toml_table_at(gait_descriptions, g);
        toml_raw_t tomlr = toml_raw_in(gait, "name");
        toml_rtos(tomlr, &gaits[g].name);
        tomlr = toml_raw_in(gait, "step_name");
        char *step_name;
        toml_rtos(tomlr, &step_name);
        gaits[g].step_index = find_step(steps, nsteps, step_name);
        free(step_name);
        get_float(gait, "step_cycles", &gaits[g].step_cycles);
        toml_array_t *phase_offsets = toml_array_in(gait, "leg_phase");
        if(phase_offsets == 0)
        {
            logm(SL4C_FATAL, "No leg_phase for gait %s\n", gaits[g].name);
            return NULL;
        }
        gaits[g].nlegs = toml_array_nelem(phase_offsets);
        gaits[g].phase_offsets = calloc(gaits[g].nlegs, sizeof(float));
        for(int p=0; p<gaits[g].nlegs; p++)
        {
            tomlr = toml_raw_at(phase_offsets, p);
            double tmpd;
            toml_rtod(tomlr, &tmpd);
            gaits[g].phase_offsets[p] = tmpd;
        }
    }
    return gaits;
}

void free_gait_descriptions(struct gait *gaits, int ngaits)
{
    for(int g=0;g<ngaits;g++)
    {
        free(gaits[g].name);
        free(gaits[g].phase_offsets);
    }
    free(gaits);
}
