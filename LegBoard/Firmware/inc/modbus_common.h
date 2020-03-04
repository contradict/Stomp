#include <stdint.h>
#include <stdbool.h>
    
uint16_t return_context(void *context);
bool return_context_bool(void *context);
void save_to_context(void *context, uint16_t value);
void save_to_context_bool(void *context, bool value);
