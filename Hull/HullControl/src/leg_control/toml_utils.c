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


