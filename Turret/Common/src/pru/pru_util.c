#include <stdint.h>
#include <string.h>

#include "pru_util.h"

// ----------------------------------------------------------------------------
// file scope consts
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// file scope variables
// ----------------------------------------------------------------------------

static char s_itoa_buf[32];

// ----------------------------------------------------------------------------
// public methods
// ----------------------------------------------------------------------------

char* pru_util_itoa(int32_t val, uint32_t base)
{
    int negative = 0;

    memset(s_itoa_buf, 0, 32);

    if (val < 0)
    {
        negative= 1;
        val *= -1;
    }

    int i = 30;
    do
    {
        s_itoa_buf[i] = "0123456789abcdef"[val % base];
        val /= base;
        i--;
    }
    while (val > 0);

    if (negative)
    {
        s_itoa_buf[i] = '-';
        i--;
    }
	
    return &s_itoa_buf[i+1];
}
