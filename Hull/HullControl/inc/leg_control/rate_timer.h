#pragma once

struct rate_timer;

struct rate_timer* create_rate_timer(float rate);
void restart_rate_timer(struct rate_timer* timer);
float sleep_rate(struct rate_timer* timer);
void destroy_rate_timer(struct rate_timer** timer);
