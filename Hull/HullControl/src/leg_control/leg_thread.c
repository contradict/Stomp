#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>

#include "joint.h"
#include "modbus_register_map.h"
#include "sclog4c/sclog4c.h"

#include "modbus_device.h"
#include "leg_control/leg_thread.h"
#include "leg_control/toml_utils.h"
#include "leg_control/rate_timer.h"
#include "lcm/stomp_telemetry_leg.h"
#include "lcm/stomp_modbus.h"

const float deg2rad = M_PI / 180.0f;

struct leg_description {
    char *name;
    int index;
    uint8_t address;
    float orientation[3];
    float origin[3];
};

struct joint_gains {
    float gain_ramp_time;
    float proportional_gain[3];
    float force_damping[3];
};

struct step {
    char *name;
    float length;
    int npoints;
    float *phase, *X, *Y, *Z;
};

struct gait {
    char *name;
    int step_index;
    float step_cycles;
    int nlegs;
    float *phase_offsets;
};

enum leg_control_mode {
    mode_init,
    mode_zero_gain,
    mode_ready,
    mode_gain_ramp,
    mode_gain_set,
    mode_get_position,
    mode_pos_ramp,
    mode_stop,
    mode_walk
};

struct leg_thread_state {
    struct leg_thread_definition *definition;
    pthread_t thread;
    modbus_t *ctx;
    ringbuf_worker_t *telemetry_worker;
    ringbuf_worker_t *response_worker;
    int nlegs;
    struct leg_description* legs;
    struct joint_gains *joint_gains;
    int nsteps;
    struct step *steps;
    int ngaits;
    struct gait *gaits;
    int current_gait;
    float position_ramp_time;
    atomic_bool shouldrun;
    enum leg_control_mode mode;
    float (*commanded_toe_positions)[3];
    float (*initial_toe_positions)[3];
    float turning_width;
    float walk_phase;
    float observed_period;
};

struct leg_description *parse_leg_descriptions(toml_table_t *legs_config, int *nlegs)
{
    toml_raw_t tomlr = toml_raw_in(legs_config, "count");
    int64_t num_legs;
    toml_rtoi(tomlr, &num_legs);
    *nlegs = num_legs;
    toml_array_t *descriptions = toml_array_in(legs_config, "description");
    struct leg_description *legs = calloc(num_legs, sizeof(struct leg_description));

    for(int leg=0; leg<num_legs; leg++)
    {
        toml_table_t *desc = toml_table_at(descriptions, leg);
        tomlr = toml_raw_in(desc, "index");
        int64_t index;
        toml_rtoi(tomlr, &index);
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
    get_float(legs_config, "gain_ramp_time", &gains->gain_ramp_time);
    for(int j=0; j<3; j++)
    {
        toml_table_t *joint = toml_table_in(joints, joint_names[j]);
        get_float(joint, "ProportionalGain", &gains->proportional_gain[j]);
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
        toml_array_t *points = toml_array_in(step, "points");
        steps[s].npoints = toml_array_nelem(points);
        steps[s].phase = calloc(steps[s].npoints, sizeof(float));
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
    toml_array_t *gait_descriptions = toml_array_in(config, "gait");
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
            printf("No leg_phase for gait %s\n", gaits[g].name);
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

bool ping_all_legs(modbus_t *ctx, struct leg_description *legs, int nlegs)
{
    /* Ping all the legs */
    int err = 0;
    for(int leg=0; leg<nlegs; leg++)
    {
        modbus_set_slave(ctx, legs[leg].address);
        uint16_t dummy;
        err = modbus_read_registers(ctx, 0x55, 1, &dummy);
        if(err == -1)
        {
            printf("Unable to communicate with leg %d(0x%02x): %s\n",
                    leg, legs[leg].address, modbus_strerror(errno));
            break;
        }
    }
    return err;
}

int set_servo_gains(modbus_t *ctx, uint8_t address, const float (*gain)[3], const float (*damping)[3])
{
    int err;
    uint16_t gain_value, damping_value;
    modbus_set_slave(ctx, address);
    gain_value = 10.0f * (*gain)[JOINT_CURL];
    damping_value = 10.0f * (*damping)[JOINT_CURL];
    do {
        err = modbus_write_registers(ctx, CURL_BASE + HProportionalGain, 1, &gain_value);
        if(err != -1)
            err = modbus_write_registers(ctx, CURL_BASE + HForceDamping, 1, &damping_value);
        gain_value = 10.0f * (*gain)[JOINT_SWING];
        damping_value = 10.0f * (*damping)[JOINT_SWING];
        if(err != -1)
            err = modbus_write_registers(ctx, SWING_BASE + HProportionalGain, 1, &gain_value);
        if(err != -1)
            err = modbus_write_registers(ctx, SWING_BASE + HForceDamping, 1, &damping_value);
        gain_value = 10.0f * (*gain)[JOINT_LIFT];
        damping_value = 10.0f * (*damping)[JOINT_LIFT];
        if(err != -1)
            err = modbus_write_registers(ctx, LIFT_BASE + HForceDamping, 1, &damping_value);
        if(err != -1)
            err = modbus_write_registers(ctx, LIFT_BASE + HProportionalGain, 1, &gain_value);
    } while(err == -1 && errno == EMBXSFAIL);
    return err;
}

int zero_gains(modbus_t *ctx, struct leg_description *legs, int nlegs)
{
    float zero_gain[3] = {0.0f, 0.0f, 0.0f};
    float zero_damping[3] = {0.0f, 0.0f, 0.0f};
    for(int l=0; l<nlegs; l++)
    {
        int err = set_servo_gains(ctx, legs[l].address, &zero_gain, &zero_damping);
        if(0 != err)
            return err;
    }
    return 0;
}

int ramp_gain_step(struct leg_thread_state* state, float elapsed)
{
    struct joint_gains *gain = state->joint_gains;
    float phase = MIN(1.0f, elapsed / gain->gain_ramp_time);
    float current_gain[3];
    float current_damping[3];
    for(int joint=0;joint<JOINT_COUNT;joint++)
    {
        current_gain[joint] = gain->proportional_gain[joint] * phase;
        current_damping[joint] = gain->force_damping[joint] * phase;
    }
    for(int leg=0; leg<state->nlegs; leg++)
    {
        int err = set_servo_gains(state->ctx, state->legs[leg].address, &current_gain, &current_damping);
        if(err == -1)
        {
            printf("Failed to set servo gain for leg %d(0x%02x): %s.\n",
                   leg, state->legs[leg].address, modbus_strerror(errno));
            return -1;
        }
    }
    if(elapsed > gain->gain_ramp_time)
        return 1;
    else
        return 0;
}

int find_interpolation_index(const float *nodes, size_t length, float x)
{
    size_t i;
    for(i=0; i<length; i++)
    {
        if((nodes[i] <= x) && (nodes[i+1] >= x))
            break;
    }
    if(i==length)
        return -1;
    return i;
}

void interpolate_value(const float *nodes, const float *values, ssize_t length, int index, float x, float *y)
{
    int next_node = index+1;
    int next_value = next_node==length ? 0 : next_node;
    *y = (x - nodes[index]) * (values[next_value] - values[index]) / (nodes[next_node] - nodes[index]) + values[index];
}

int compute_leg_position(struct leg_thread_state* state, int leg_index, float phase, float scale, float (*toe_position)[3])
{
    struct gait *gait = (state->gaits+state->current_gait);
    struct step *step = (state->steps+gait->step_index);
    float discard;
    float leg_phase = modff(phase + gait->phase_offsets[leg_index], &discard);
    int index = find_interpolation_index(step->phase, step->npoints, leg_phase);
    if(index<0)
        return index;
    interpolate_value(step->phase, step->X, step->npoints, index, leg_phase, &((*toe_position)[0]));
    interpolate_value(step->phase, step->Y, step->npoints, index, leg_phase, &((*toe_position)[1]));
    interpolate_value(step->phase, step->Z, step->npoints, index, leg_phase, &((*toe_position)[2]));
    // TODO Make this correct for angles other than 0, 180.
    (*toe_position)[1] = copysignf((*toe_position)[1], cosf(state->legs[leg_index].orientation[2] * deg2rad)) * scale;
    return 0;
}

int get_toe_feedback(modbus_t *ctx, uint8_t address, float (*toe_position)[3], float (*cylinder_pressure)[6])
{
    int err;
    uint16_t toe_value[3];
    uint16_t cylinder_value[6];
    modbus_set_slave(ctx, address);
    err = modbus_read_registers(ctx, ToeXPosition, 3, toe_value);
    if(err != -1)
        for(int i=0;i<3;i++)
            (*toe_position)[i] = ((int16_t *)toe_value)[i] / 100.0f;
    if(err != -1)
        err = modbus_read_registers(ctx, CURL_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[4]));
    if(err != -1)
        err = modbus_read_registers(ctx, SWING_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[0]));
    if(err != -1)
        err = modbus_read_registers(ctx, LIFT_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[2]));
    if(err != -1)
        for(int i=0;i<6;i++)
        {
            (*cylinder_pressure)[i] = cylinder_value[i] / 100.0f;
        }
    return err == -1 ? -1 : 0;
}

int retrieve_leg_positions(struct leg_thread_state* state)
{
    float discard_pressures[6];
    for(int leg=0; leg<state->nlegs; leg++)
    {
        int err = get_toe_feedback(state->ctx, state->legs[leg].address, &(state->commanded_toe_positions[leg]), &discard_pressures);
        if(err!=0)
            return err;
        compute_leg_position(state, leg, 0.0f, 0.0f, &(state->initial_toe_positions[leg]));
    }
    return 0;
}

int set_toe_postion(modbus_t *ctx, uint8_t address, float (*toe_position)[3])
{
    uint16_t toe_values[3];

    for(int axis=0; axis < 3; axis++)
    {
        ((int16_t *)toe_values)[axis] = roundf((*toe_position)[axis] * 100.0f);
    }
    modbus_set_slave(ctx, address);
    return modbus_write_registers(ctx, ToeXPosition, 3, toe_values);
}

int ramp_position_step(struct leg_thread_state* state, float elapsed)
{
    float phase = MIN(1.0f, elapsed / state->position_ramp_time);
    for(int leg=0; leg<state->nlegs; leg++)
    {
        float ramp_position[3];
        for(int axis=0;axis<3;axis++)
        {
            ramp_position[axis] = phase * state->initial_toe_positions[leg][axis] + (1.0 - phase) * state->commanded_toe_positions[leg][axis];
        }
        int err = set_toe_postion(state->ctx, state->legs[leg].address, &ramp_position);
        if(err == -1)
        {
            return err;
        }
    }
    return 0;
}

int compute_walk_parameters(struct leg_thread_state *state, struct leg_control_parameters *p, float *frequency, float *leg_scale)
{
    float left_scale, right_scale;
    if(p->angular_velocity > 0)
    {
        right_scale = copysignf(p->forward_velocity, 1.0);
        left_scale = p->angular_velocity * state->turning_width / p->forward_velocity;
    }
    else
    {
        right_scale = -p->angular_velocity * state->turning_width / p->forward_velocity;
        left_scale = copysignf(p->forward_velocity, 1.0);
    }
    leg_scale[0] = leg_scale[1] = leg_scale[2] = right_scale;
    leg_scale[3] = leg_scale[4] = leg_scale[5] = left_scale;
    *frequency = (fabs(p->angular_velocity) * state->turning_width + fabs(p->forward_velocity)) / state->steps[state->gaits[state->current_gait].step_index].length;
    return 0;
}

int walk_step(struct leg_thread_state* state, struct leg_control_parameters *p, float dt)
{
    float frequency;
    float leg_scale[state->nlegs];
    compute_walk_parameters(state, p, &frequency, leg_scale);

    float dummy;
    state->walk_phase = MAX(modff(state->walk_phase + frequency * dt, &dummy), 0.0f);

    for(int leg=0; leg<state->nlegs; leg++)
    {
        float toe_position[3];
        compute_leg_position(state, leg, state->walk_phase, leg_scale[leg], &toe_position);
        int err = set_toe_postion(state->ctx, state->legs[leg].address, &toe_position);
        if(err != 0)
            return err;
        memcpy(state->commanded_toe_positions[leg], toe_position, sizeof(toe_position));
    }
    return 0;
}

int read_parameters(struct queue *pq, struct leg_control_parameters *p)
{
    size_t offset;
    size_t s = ringbuf_consume(pq->ringbuf, &offset);
    if(s == sizeof(struct leg_control_parameters))
    {
        memcpy(p, pq->buffer + offset, s);
        ringbuf_release(pq->ringbuf, s);
    }
    return s;
}

bool run_leg_thread_once(struct leg_thread_state* state, struct leg_control_parameters *parameters, float elapsed, float dt)
{
    int res;
    bool restart_timer = false;
    switch(state->mode)
    {
        case mode_init:
            // Check comms with all legs, zero gains on success
            // stay here on fail
            if(parameters->mode != command_init &&
               0 == ping_all_legs(state->ctx, state->legs, state->nlegs))
            {
                state->mode = mode_zero_gain;
                logm(SL4C_INFO, "Ping all legs succeeded.");
            }
            break;
        case mode_zero_gain:
            // set all gains to zero, transition to ready on success
            // transition to init on fail
            if(0 == zero_gains(state->ctx, state->legs, state->nlegs))
            {
                state->mode = mode_ready;
                logm(SL4C_INFO, "Gain zeroed.");
            } else {
                state->mode = mode_init;
            }
            break;
        case mode_ready:
            // Wait for command to ramp
            switch(parameters->mode)
            {
                case command_init:
                    state->mode = mode_init;
                    break;
                case command_zero_gain:
                    break;
                case command_gain_set:
                case command_stop:
                case command_walk:
                    restart_timer = true;
                    state->mode = mode_gain_ramp;
                    logm(SL4C_INFO, "Gain ramp started.");
                    break;
            }
            break;
        case mode_gain_ramp:
            // ramp gain to full value
            // on error go to init
            res = ramp_gain_step(state, elapsed);
            if(res==-1)
            {
                state->mode = mode_init;
            }
            else if(res == 1)
            {
                state->mode = mode_gain_set;
                logm(SL4C_INFO, "Gain set.");
            }
            break;
        case mode_gain_set:
            // wait for command to position
            switch(parameters->mode)
            {
                case command_init:
                    state->mode = mode_init;
                    break;
                case command_zero_gain:
                    state->mode = mode_zero_gain;
                    break;
                case command_gain_set:
                    break;
                case command_stop:
                case command_walk:
                    state->mode = mode_get_position;
                    logm(SL4C_INFO, "Positioning started.");
                    break;
            }
            break;
        case mode_get_position:
            res = retrieve_leg_positions(state);
            if(res == 0)
            {
                restart_timer = true;
                state->mode = mode_pos_ramp;
            }
            else
            {
                state->mode = mode_init;
            }
            break;
        case mode_pos_ramp:
            // move legs to start of cycle
            // on error go to init
            res = ramp_position_step(state, elapsed);
            if(res == -1)
            {
                state->mode = mode_init;
            }
            else if(res == 1)
            {
                state->mode = mode_stop;
                logm(SL4C_INFO, "Legs positioned.");
            }
            break;
        case mode_stop:
            // Wait for command
            // walk or zero
            switch(parameters->mode)
            {
                case command_init:
                    state->mode = mode_init;
                    break;
                case command_zero_gain:
                    state->mode = mode_zero_gain;
                    break;
                case command_gain_set:
                    restart_timer = true;
                    state->mode = mode_gain_ramp;
                    break;
                case command_stop:
                    break;
                case command_walk:
                    restart_timer = true;
                    state->mode = mode_walk;
                    logm(SL4C_INFO, "Walk mode.");
                    break;
            }
            break;
        case mode_walk:
            // run gait generator
            // on error, go back to init
            // on command, back to stop
            switch(parameters->mode)
            {
                case command_init:
                    state->mode = mode_init;
                    break;
                case command_zero_gain:
                    state->mode = mode_zero_gain;
                    break;
                case command_gain_set:
                    restart_timer = true;
                    state->mode = mode_gain_ramp;
                    break;
                case command_stop:
                    restart_timer = true;
                    state->mode = mode_stop;
                    break;
                case command_walk:
                    walk_step(state, parameters, dt);
                    break;
            }
            break;
    }
    return restart_timer;
}

int send_telemetry(struct leg_thread_state* state)
{
    float position[state->nlegs][3], pressure[state->nlegs][6];
    int err;
    for(int leg=0;leg<state->nlegs;leg++)
    {
        err = get_toe_feedback(state->ctx, state->legs[leg].address, &(position[leg]), &(pressure[leg]));
        if(err)
            return err;
    }
    ssize_t offset = ringbuf_acquire(state->definition->telemetry_queue->ringbuf, state->telemetry_worker, sizeof(stomp_telemetry_leg));
    if(offset > 0)
    {
        stomp_telemetry_leg *telem = (stomp_telemetry_leg *)(state->definition->telemetry_queue->buffer + offset);
        for(int leg=0;leg<state->nlegs;leg++)
        {
            for(int joint=0;joint<JOINT_COUNT;joint++)
            {
                telem->base_end_pressure[JOINT_COUNT * leg + joint] = pressure[leg][2*joint];
                telem->rod_end_pressure[JOINT_COUNT * leg + joint] = pressure[leg][2*joint + 1];
            }
            telem->toe_position_measured_X[leg] = position[leg][0];
            telem->toe_position_measured_Y[leg] = position[leg][1];
            telem->toe_position_measured_Z[leg] = position[leg][2];
            telem->toe_position_commanded_X[leg] = state->commanded_toe_positions[leg][0];
            telem->toe_position_commanded_Y[leg] = state->commanded_toe_positions[leg][1];
            telem->toe_position_commanded_Z[leg] = state->commanded_toe_positions[leg][2];
        }
        telem->observed_period = state->observed_period;
        ringbuf_produce(state->definition->telemetry_queue->ringbuf, state->telemetry_worker);
    }
    return 0;
}

int check_command_queue(struct leg_thread_state* state)
{
    size_t offset;
    size_t s = ringbuf_consume(state->definition->command_queue->ringbuf, &offset);
    if(s >= sizeof(stomp_modbus))
    {
        size_t rs = s;
        while(s >= sizeof(stomp_modbus))
        {
            stomp_modbus* request = (stomp_modbus*)(state->definition->command_queue->buffer + offset);
            switch(request->command)
            {
                //TODO:
                //write_register
                //write_registers
                //write_bits
                //read_registers
                //read_input_registers
                //read_bits
            }
            ssize_t offset = ringbuf_acquire(state->definition->response_queue->ringbuf, state->response_worker, sizeof(stomp_modbus));
            if(offset > 0)
            {
                stomp_modbus *response = (stomp_modbus *)(state->definition->response_queue->buffer + offset);
                memcpy(response, request, sizeof(stomp_modbus));
                ringbuf_produce(state->definition->response_queue->ringbuf, state->response_worker);
            }
            s -= sizeof(stomp_modbus);
            offset += sizeof(stomp_modbus);
        }
        ringbuf_release(state->definition->command_queue->ringbuf, rs);
    }
    return 0;
}

void *run_leg_thread(void *ptr)
{
    struct leg_thread_state *state = (struct leg_thread_state*)ptr;
    state->telemetry_worker = ringbuf_register(state->definition->telemetry_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        printf("Unable to register worker for telemetry queue\n");
        return (void *)-1;
    }

    state->response_worker = ringbuf_register(state->definition->response_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        printf("Unable to register worker for command response queue\n");
        return (void *)-1;
    }


    toml_table_t *legs_config = toml_table_in(state->definition->config,
                                             "legs");
    state->legs = parse_leg_descriptions(legs_config, &state->nlegs);
    state->joint_gains = parse_joint_gains(legs_config);
    get_float(legs_config, "position_ramp_time", &state->position_ramp_time);
    state->commanded_toe_positions = malloc(sizeof(float[state->nlegs][3]));
    state->initial_toe_positions = malloc(sizeof(float[state->nlegs][3]));

    state->steps = parse_steps(state->definition->config, &state->nsteps);

    state->gaits = parse_gaits(state->definition->config, &state->ngaits, state->steps, state->nsteps);

    toml_table_t *geometry = toml_table_in(state->definition->config, "geometry");
    get_float(geometry, "width", &state->turning_width);

    struct leg_control_parameters parameters = {
        .forward_velocity = 0.0f,
        .angular_velocity = 0.0f,
        .mode = command_init
    };
    bool loop_phase = false;

    struct rate_timer *timer = create_rate_timer(state->definition->frequency);
    logm(SL4C_DEBUG, "Create timer with period %f",state->definition->frequency);
    float elapsed = 0, dt=0;;
    while(state->shouldrun)
    {
        if(loop_phase)
        {
            read_parameters(state->definition->parameter_queue, &parameters);
            if(run_leg_thread_once(state, &parameters, elapsed, dt))
                restart_rate_timer(timer);
        } else {
            send_telemetry(state);
            check_command_queue(state);
        }
        state->observed_period *= 0.9;
        state->observed_period += 0.1 * dt;
        sleep_rate(timer, &elapsed, &dt);
        logm(SL4C_FINE, "lp: %s el: %6.3f dt: %5.3f", loop_phase ? "walk" : "tlem", elapsed, dt);
        loop_phase ^= true;
    }
    destroy_rate_timer(&timer);
    free_leg_description(state->legs, state->nlegs);
    free(state->joint_gains);
    free(state->commanded_toe_positions);
    free(state->initial_toe_positions);
    free_step_descriptions(state->steps, state->nsteps);
    free_gait_descriptions(state->gaits, state->ngaits);
    return NULL;
}

struct leg_thread_state* create_leg_thread(struct leg_thread_definition *leg_thread, const char * progname)
{
    struct leg_thread_state *state = malloc(sizeof(struct leg_thread_state));
    state->definition = leg_thread;
    if(create_modbus_interface(leg_thread->devname, leg_thread->baud, leg_thread->response_timeout, &state->ctx))
    {
        free(state);
        return 0;
    }

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        printf("Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", progname);
        free(state);
        return 0;
    }

    state->shouldrun = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&state->thread, &attr, &run_leg_thread, state);
    return state;
}

void terminate_leg_thread(struct leg_thread_state **state)
{
    (*state)->shouldrun = false;
    void *res;
    pthread_join((*state)->thread, &res);
    printf("run_leg_thread=%zu\n", (size_t)res);
    free(*state);
    *state = NULL;
}
