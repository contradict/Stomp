#include <stdio.h>

#include "realtimer.h"

int main(int argc, char *argv)
{
    (void)argc;
    (void)argv;

    struct RealTimer tau;

    float period = 1.0;
    float total = 10.0;
    float elapsed;

    for(start_time(&tau); (elapsed = elapsed_time(&tau)) < total; sleep_period(&tau, period))
    {
        printf("elapsed: %f\n", elapsed);
    }

    return 0;
}
