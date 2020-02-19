#include <string.h>

extern void _sstdata;
extern void _estdata;
extern void _ldstdata;

void Storage_Init(void)
{
    memcpy(&_sstdata, &_ldstdata, &_estdata - &_sstdata);
}

void Storage_Save(void)
{
}
