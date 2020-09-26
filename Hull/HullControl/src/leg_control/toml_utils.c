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

int get_float(toml_table_t *tab, char *name, float d, float *f)
{
    *f = d;
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
        get_float(joint, "ProportionalGain", 12.0f, &gains->proportional_gain[j]);
        get_float(joint, "DerivativeGain", 0.0f, &gains->derivative_gain[j]);
        get_float(joint, "ForceDamping", 20.0f, &gains->force_damping[j]);
        get_float(joint, "FeedbackLowpass", 0.0f, &gains->feedback_lowpass[j]);
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
        toml_array_t* ext = toml_array_in(step, "minimum");
        toml_vector_float(ext, steps[s].minimum);
        ext = toml_array_in(step, "maximum");
        toml_vector_float(ext, steps[s].maximum);
        get_float(step, "r_inner", 0.153, &steps[s].r_inner);
        get_float(step, "r_outer", 0.236, &steps[s].r_outer);
        get_float(step, "swing_angle_min", -1.181, &steps[s].swing_angle_min);
        get_float(step, "swing_angle_max",  1.181, &steps[s].swing_angle_max);
        get_float(step, "direction_swap_tolerance", 0.030f, &steps[s].swap_tolerance);
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
        int p=0;
        for(p=0; p<steps[s].npoints; p++)
        {
            toml_table_t *pt = toml_table_at(points, p);
            if(get_float(pt, "phase", 0.0f, &steps[s].phase[p]) < 0)
            {
                logm(SL4C_ERROR, "Unable to parse phase value for step %s",
                        steps[s].name);
                break;
            }
            if(get_float(pt, "X", 0.0f, &steps[s].X[p]))
            {
                logm(SL4C_ERROR, "Unable to parse X value for step %s",
                        steps[s].name);
                break;
            }
            if(get_float(pt, "Y", 0.0f, &steps[s].Y[p]))
            {
                logm(SL4C_ERROR, "Unable to parse Y value for step %s",
                        steps[s].name);
                break;
            }
            if(get_float(pt, "Z", 0.0f, &steps[s].Z[p]))
            {
                logm(SL4C_ERROR, "Unable to parse Z value for step %s",
                        steps[s].name);
                break;
            }
        }
        if(p<steps[s].npoints)
        {
            steps[s].npoints = 0;
        }
        else
        {
            steps[s].phase[steps[s].npoints] = 1.0f;
        }
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
        if(gaits[g].step_index < 0)
        {
            gaits[g].step_index = 0;
            logm(SL4C_ERROR, "Unable to find step \"%s\", using step \"%s\".",
                step_name, steps[0].name);
        }
        free(step_name);
        get_float(gait, "step_cycles", 1.0f, &gaits[g].step_cycles);
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

char **parse_gait_selections(toml_table_t* config, struct gait* gaits, int ngaits, char* default_gait)
{
    toml_array_t* sel = toml_array_in(config, "gait_selections");
    char **gait_selections = calloc(toml_array_nelem(sel), sizeof(char*));
    for(int s=0; s<toml_array_nelem(sel); s++)
    {
        toml_raw_t tomlr = toml_raw_at(sel, s);
        char* name;
        toml_rtos(tomlr, &name);
        for(int g=0; g<ngaits; g++)
        {
            if(strcmp(name, gaits[g].name) == 0)
                gait_selections[s] = name;
        }
        if(gait_selections[s] == 0)
        {
            logm(SL4C_ERROR, "No gait named %s, replaced with default %s",
                 name, default_gait);
            gait_selections[s] = default_gait;
        }
    }
    return gait_selections;
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
