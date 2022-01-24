/*
    rfd900x.h
    Driver library for RFD900x radio
*/
#pragma once

void rfd900x_init();
int rfd900x_get_fileno();

void rfd900x_write(const char* data, size_t size);
int rfd900x_read(char* buf, size_t buf_size);
