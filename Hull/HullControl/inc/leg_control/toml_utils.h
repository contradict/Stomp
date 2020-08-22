#pragma once
#include <toml.h>

int toml_vector_float(toml_array_t *a, float dest[3]);
int get_float(toml_table_t *tab, char *name, float *f);
