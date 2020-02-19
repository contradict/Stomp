#include <string.h>

extern int _sstdata;
extern int _estdata;
extern int _ldstdata;

void Storage_Init(void)
{
    memcpy(&_sstdata, &_ldstdata, &_estdata - &_sstdata);
}

void Storage_Save(void)
{
}
