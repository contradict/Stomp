#pragma once
#include <toml.h>

struct leg_description {
    char *name;
    int index;
    uint8_t address;
    float orientation[3];
    float origin[3];
};

struct joint_gains {
    float proportional_gain[3];
    float derivative_gain[3];
    float force_damping[3];
    float feedback_lowpass[3];
};

struct step {
    char *name;
    float length;
    int npoints;
    float swap_tolerance;
    int nswap;
    float* swap_phase;
    float *phase, *X, *Y, *Z;
};

struct gait {
    char *name;
    int step_index;
    float step_cycles;
    int nlegs;
    float *phase_offsets;
};

int toml_vector_float(toml_array_t *a, float dest[3]);
int get_float(toml_table_t *tab, char *name, float default_value, float *f);
struct leg_description *parse_leg_descriptions(toml_table_t *legs_config, int *nlegs);
void free_leg_description(struct leg_description *legs, int nlegs);
struct joint_gains *parse_joint_gains(toml_table_t *legs_config);
struct step *parse_steps(toml_table_t *config, int *nsteps);
void free_step_descriptions(struct step *steps, int nsteps);
struct gait *parse_gaits(toml_table_t *config, int *ngaits, const struct step* steps, int nsteps);
char **parse_gait_selections(toml_table_t* config, struct gait* gaits, int ngaits, char* default_gait);
void free_gait_descriptions(struct gait *gaits, int ngaits);
