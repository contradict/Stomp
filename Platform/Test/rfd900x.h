/*
    rfd900x.h
    Driver library for RFD900x radio
*/
#pragma once

void rfd900x_init();

void rfd900x_write(const char* data, size_t size);