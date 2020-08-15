#pragma once

struct rate_timer;

struct rate_timer* create_rate_timer(float rate);
void restart_rate_timer(struct rate_timer* timer);
void sleep_rate(struct rate_timer* timer, float *elapsed, float *dt);
void destroy_rate_timer(struct rate_timer** timer);
