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
#include "leg_control/modbus_utils.h"
#include "lcm/stomp_telemetry_leg.h"
#include "lcm/stomp_modbus.h"

const float deg2rad = M_PI / 180.0f;

enum leg_control_mode {
    mode_init,
    mode_ping,
    mode_get_position_walk,
    mode_get_position_disable,
    mode_get_position_reenable,
    mode_gain_zero,
    mode_gain_set_walk,
    mode_gain_set_free,
    mode_air_vent_init,
    mode_air_vent_disable,
    mode_air_on,
    mode_get_start,
    mode_move_start,
    mode_walk,
    mode_lock,
    mode_free
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
    struct rate_timer *timer;
    enum leg_control_mode mode;
    float (*commanded_toe_positions)[3];
    float (*initial_toe_positions)[3];
    float turning_width;
    float walk_phase;
    float observed_period;
};


static int ping_all_legs(modbus_t *ctx, struct leg_description *legs, int nlegs)
{
    /* Ping all the legs */
    int err = 0, ret = 0;
    for(int leg=0; leg<nlegs; leg++)
    {
        err = ping_leg(ctx, legs[leg].address);
        if(err == -1)
        {
            logm(SL4C_ERROR, "Unable to communicate with leg %d(0x%02x): %s\n",
                    leg, legs[leg].address, modbus_strerror(errno));
            ret = err;
        }
    }
    return ret;
}

static int set_gains(struct leg_thread_state* state, struct joint_gains* gain)
{
    int ret = 0;
    uint32_t sec, saved_timeout;
    modbus_get_response_timeout(state->ctx, &sec, &saved_timeout);
    modbus_set_response_timeout(state->ctx, 0, 100000);
    for(int leg=0; leg<state->nlegs; leg++)
    {
        int err = set_servo_gains(state->ctx, state->legs[leg].address, &gain->proportional_gain, &gain->force_damping);
        if(err == -1)
        {
            logm(SL4C_ERROR, "Failed to set servo gain for leg %d(0x%02x): %s.\n",
                   leg, state->legs[leg].address, modbus_strerror(errno));
            ret = err;
        }
    }
    modbus_set_response_timeout(state->ctx, 0, saved_timeout);
    return ret;
}

static int zero_gains(struct leg_thread_state *state)
{
    struct joint_gains zerogain[state->nlegs];
    bzero(&zerogain, sizeof(zerogain));
    return set_gains(state, (struct joint_gains*)&zerogain);
}

static int equalize_joint_command(struct leg_thread_state* state)
{
    int ret = 0;
    uint16_t joint_feedback[3];
    for(int l=0; l<state->nlegs; l++)
    {
        int err;
        modbus_set_slave(state->ctx, state->legs[l].address);
        err = modbus_read_input_registers(state->ctx, CURL_BASE + ICachedFeedbackPosition, 1, &joint_feedback[JOINT_CURL]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error reading leg %d(0x%d) curl feedback: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
        err = modbus_read_input_registers(state->ctx, SWING_BASE + ICachedFeedbackPosition, 1, &joint_feedback[JOINT_SWING]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error reading leg %d(0x%d) swing feedback: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
        err = modbus_read_input_registers(state->ctx, LIFT_BASE + ICachedFeedbackPosition, 1, &joint_feedback[JOINT_LIFT]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error reading leg %d(0x%d) lift feedback: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
        err = modbus_write_registers(state->ctx, CURL_BASE + HCachedDigitalCommand, 1, &joint_feedback[JOINT_CURL]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error writing leg %d(0x%d) curl command: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
        err = modbus_write_registers(state->ctx, SWING_BASE + HCachedDigitalCommand, 1, &joint_feedback[JOINT_SWING]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error writing leg %d(0x%d) swing command: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
        err = modbus_write_registers(state->ctx, LIFT_BASE + HCachedDigitalCommand, 1, &joint_feedback[JOINT_LIFT]);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Error writing leg %d(0x%d) lift command: %s",
                    l, state->legs[l].address, modbus_strerror(errno));
        }
    }
    return ret;
}

static int find_interpolation_index(const float *nodes, size_t length, float x)
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

static void interpolate_value(const float *nodes, const float *values, ssize_t length, int index, float x, float *y)
{
    int next_node = index+1;
    int next_value = next_node==length ? 0 : next_node;
    *y = (x - nodes[index]) * (values[next_value] - values[index]) / (nodes[next_node] - nodes[index]) + values[index];
}

static int compute_leg_position(struct leg_thread_state* state, int leg_index, float phase, float scale, float (*toe_position)[3])
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

static int retrieve_leg_positions(struct leg_thread_state* state)
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

static int ramp_position_step(struct leg_thread_state* state, float elapsed)
{
    float phase = MIN(1.0f, elapsed / state->position_ramp_time);
    int ret = 0;
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
            logm(SL4C_ERROR, "Unable to set position for leg %d(0x%02x): %s",
                 leg, state->legs[leg].address, modbus_strerror(errno));
            ret = err;
        }
    }
    if(ret != -1 && phase >= 1.0f)
        ret = 1;
    return ret;
}

static int compute_walk_parameters(struct leg_thread_state *state, struct leg_control_parameters *p, float *frequency, float *leg_scale)
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
    for(int l=0;l<state->nlegs / 2;l++)
    {
        leg_scale[l] = right_scale;
    }
    for(int l=state->nlegs/2;l<state->nlegs;l++)
    {
        leg_scale[l] = left_scale;
    }
    *frequency = (fabs(p->angular_velocity) * state->turning_width + fabs(p->forward_velocity)) / state->steps[state->gaits[state->current_gait].step_index].length;
    return 0;
}

static int walk_step(struct leg_thread_state* state, struct leg_control_parameters *p, float dt)
{
    float frequency;
    float leg_scale[state->nlegs];
    compute_walk_parameters(state, p, &frequency, leg_scale);

    float dummy;
    state->walk_phase = MAX(modff(state->walk_phase + frequency * dt, &dummy), 0.0f);

    int ret = 0;
    for(int leg=0; leg<state->nlegs; leg++)
    {
        float toe_position[3];
        compute_leg_position(state, leg, state->walk_phase, leg_scale[leg], &toe_position);
        int err = set_toe_postion(state->ctx, state->legs[leg].address, &toe_position);
        if(err == -1)
        {
            ret = err;
            logm(SL4C_ERROR, "Unable to set toe position for leg %d(0x%02x): %s",
                    leg, state->legs[leg].address, modbus_strerror(errno));
        }
        memcpy(state->commanded_toe_positions[leg], toe_position, sizeof(toe_position));
    }
    return ret;
}

static int read_parameters(struct queue *pq, struct leg_control_parameters *p)
{
    size_t offset, sp;
    sp = sizeof(struct leg_control_parameters);
    size_t s = ringbuf_consume(pq->ringbuf, &offset);
    if(s >= sp)
    {
        memcpy(p, pq->buffer + offset + s - sp, sp);
        ringbuf_release(pq->ringbuf, s);
    }
    return s;
}

static void air_vent()
{
}

static void air_on()
{
}

static void run_leg_thread_once(struct leg_thread_state* state, struct leg_control_parameters *parameters, float elapsed, float dt)
{
    int res;
    switch(state->mode)
    {
        case mode_init:
            state->mode = mode_air_vent_init;
            logm(SL4C_INFO, "init->vent.");
            break;
        case mode_air_vent_init:
            air_vent();
            state->mode = mode_ping;
            logm(SL4C_INFO, "vent->ping.");
            break;
        case mode_ping:
            // Check comms with all legs, zero gains on success
            // stay here on fail
            if(0 == ping_all_legs(state->ctx, state->legs, state->nlegs))
            {
                logm(SL4C_INFO, "Ping all legs succeeded.");
                switch(parameters->enable)
                {
                    case ENABLE_WALK:
                        state->mode = mode_get_position_walk;
                        logm(SL4C_INFO, "ping->getpos_walk.");
                        break;
                    case ENABLE_DISABLE:
                        state->mode = mode_get_position_disable;
                        logm(SL4C_INFO, "ping->getpos_disable.");
                        break;
                    default:
                        logm(SL4C_ERROR, "Unhandled enable mode");
                        break;
                }
            }
            break;
        case mode_get_position_walk:
            res = equalize_joint_command(state);
            if(res == 0)
            {
                state->mode = mode_gain_set_walk;
                logm(SL4C_INFO, "getpos_walk->gainset_walk.");
            }
            else
            {
                logm(SL4C_ERROR, "Retrieve position failed.");
                state->mode = mode_ping;
            }
            break;
        case mode_get_position_disable:
            res = equalize_joint_command(state);
            if(res == 0)
            {
                switch(parameters->lock)
                {
                    case LOCK_FREE:
                        state->mode = mode_gain_set_free;
                        logm(SL4C_INFO, "getpos_disable->gainset_disable.");
                        break;
                    case LOCK_LOCK:
                        state->mode = mode_gain_zero;
                        logm(SL4C_INFO, "getpos_disable->gainset_disable.");
                        break;
                    default:
                        logm(SL4C_ERROR, "getpos_disable Unhandled lock setting.");
                        break;
                }
            }
            else
            {
                logm(SL4C_ERROR, "Retrieve position failed.");
                state->mode = mode_ping;
            }
            break;
        case mode_gain_set_free:
            res = set_gains(state, state->joint_gains);
            if(res == 0)
            {
                state->mode = mode_free;
                logm(SL4C_INFO, "gainset_free->free.");
            }
            else
            {
                state->mode = mode_ping;
                logm(SL4C_INFO, "gainset_free->ping.");
            }
            break;
        case mode_gain_set_walk:
            res = set_gains(state, state->joint_gains);
            if(res == 0)
            {
                state->mode = mode_air_on;
                logm(SL4C_INFO, "gainset_walk->airon.");
            }
            else
            {
                state->mode = mode_ping;
                logm(SL4C_INFO, "gainset_walk->ping.");
            }
            break;
        case mode_air_on:
            air_on();
            state->mode = mode_get_start;
            restart_rate_timer(state->timer);
            logm(SL4C_INFO, "airon->get_start.");
            break;
        case mode_get_start:
            res = retrieve_leg_positions(state);
            if(res == 0)
            {
                state->mode = mode_move_start;
                logm(SL4C_INFO, "get_start->move_start");
            }
            else
            {
                state->mode = mode_air_vent_init;
                logm(SL4C_INFO, "get_start->vent_init");
            }
            break;
        case mode_move_start:
            // move legs to start of cycle
            // on error go to vent
            res = ramp_position_step(state, elapsed);
            if(res == -1)
            {
                state->mode = mode_air_vent_init;
                logm(SL4C_ERROR, "Position ramp failed.");
                logm(SL4C_INFO, "move_start->vent_init");
            }
            else if(res == 1)
            {
                state->mode = mode_walk;
                logm(SL4C_INFO, "move_start->walk");
            }
            break;
        case mode_walk:
            // run gait generator
            // on error, go back to init
            // on command, back to stop
            switch(parameters->enable)
            {
                case ENABLE_WALK:
                    walk_step(state, parameters, dt);
                    break;
                case ENABLE_DISABLE:
                    state->mode = mode_air_vent_disable;
                    logm(SL4C_INFO, "walk->vent_disable");
                    break;
            }
            break;
        case mode_air_vent_disable:
            air_vent();
            switch(parameters->lock)
            {
                case LOCK_FREE:
                    state->mode = mode_free;
                    logm(SL4C_INFO, "vent_disable->free");
                    break;
                case LOCK_LOCK:
                    state->mode = mode_gain_zero;
                    logm(SL4C_INFO, "vent_disable->zero");
                    break;
            }
            break;
        case mode_free:
            switch(parameters->enable)
            {
                case ENABLE_WALK:
                    state->mode = mode_get_position_reenable;
                    logm(SL4C_INFO, "free->getpos_reenable.");
                    break;
                case ENABLE_DISABLE:
                    switch(parameters->lock)
                    {
                        case LOCK_FREE:
                            break;
                        case LOCK_LOCK:
                            state->mode = mode_get_position_disable;
                            logm(SL4C_INFO, "free->getpos_disable.");
                            break;
                    }
                    break;
            }
            break;
        case mode_gain_zero:
            // set all gains to zero, transition to ready on success
            // transition to init on fail
            if(0 == zero_gains(state))
            {
                state->mode = mode_lock;
                logm(SL4C_INFO, "zero->lock.");
            }
            else
            {
                state->mode = mode_init;
                logm(SL4C_INFO, "Gain zero failed.");
            }
            break;
        case mode_get_position_reenable:
            res = equalize_joint_command(state);
            if(res == 0)
            {
                state->mode = mode_air_on;
                logm(SL4C_INFO, "getpos_reenable->airon.");
            }
            else
            {
                logm(SL4C_ERROR, "Retrieve position failed.");
                logm(SL4C_INFO, "getpos_reenable->ping.");
                state->mode = mode_ping;
            }
            break;
        case mode_lock:
            switch(parameters->enable)
            {
                case ENABLE_WALK:
                    state->mode = mode_get_position_walk;
                    logm(SL4C_INFO, "lock->getpos_walk.");
                    break;
                case ENABLE_DISABLE:
                    switch(parameters->lock)
                    {
                        case LOCK_FREE:
                            state->mode = mode_get_position_disable;
                            logm(SL4C_INFO, "lock->getpos_disable.");
                            break;
                        case LOCK_LOCK:
                            break;
                    }
                    break;
            }
            break;
    }
}

static int send_telemetry(struct leg_thread_state* state)
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

static int check_command_queue(struct leg_thread_state* state)
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

static void *run_leg_thread(void *ptr)
{
    struct leg_thread_state *state = (struct leg_thread_state*)ptr;
    state->telemetry_worker = ringbuf_register(state->definition->telemetry_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        logm(SL4C_FATAL, "Unable to register worker for telemetry queue\n");
        return (void *)-1;
    }

    state->response_worker = ringbuf_register(state->definition->response_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        logm(SL4C_FATAL, "Unable to register worker for command response queue\n");
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
        .enable = ENABLE_DISABLE,
        .lock = LOCK_FREE
    };
    bool loop_phase = false;

    state->mode = mode_init;
    state->timer = create_rate_timer(state->definition->frequency);
    logm(SL4C_DEBUG, "Create timer with period %f",state->definition->frequency);
    float elapsed = 0, dt=0;;
    while(state->shouldrun)
    {
        if(loop_phase)
        {
            read_parameters(state->definition->parameter_queue, &parameters);
            run_leg_thread_once(state, &parameters, elapsed, dt);
        } else {
            send_telemetry(state);
            check_command_queue(state);
        }
        state->observed_period *= 0.9;
        state->observed_period += 0.1 * dt;
        sleep_rate(state->timer, &elapsed, &dt);
        logm(SL4C_FINE, "lp: %s el: %6.3f dt: %5.3f", loop_phase ? "walk" : "tlem", elapsed, dt);
        loop_phase ^= true;
    }
    destroy_rate_timer(&state->timer);
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
        logm(SL4C_FATAL, "Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", progname);
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
    logm(SL4C_INFO, "run_leg_thread=%zu\n", (size_t)res);
    free(*state);
    *state = NULL;
}
