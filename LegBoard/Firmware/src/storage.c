#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f7xx_hal.h"
#include "modbus.h"

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes */
extern uint8_t _sstdata; // Start of data in RAM
extern uint8_t _estdata; // End of data in RAM
extern uint8_t _ldstdata; // Start of data in FLASH (LMA)

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7) */
  {
    sector = FLASH_SECTOR_7;
  }
  return sector;
}

void Storage_Init(void)
{
    memcpy(&_sstdata, &_ldstdata, &_estdata - &_sstdata);
}

int Storage_Save(void *context, bool dummy)
{
    (void)context;
    (void)dummy;
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t SECTORError;
    int err;

    err = HAL_FLASH_Unlock();
    if(err)
        return SLAVE_DEVICE_FAILURE;

    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInitStruct.Sector = GetSector((uint32_t)&_ldstdata);
    uint32_t ldata = &_estdata - &_sstdata;
    eraseInitStruct.NbSectors = GetSector((uint32_t)&_ldstdata + ldata) - eraseInitStruct.Sector + 1;
    err = HAL_FLASHEx_Erase(&eraseInitStruct, &SECTORError);

    for(uint32_t *prgm = (uint32_t *)&_ldstdata, *data=(uint32_t *)&_sstdata;
        data < (uint32_t *)&_estdata; prgm++, data++)
    {
        err = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)prgm, *data);
        if(err)
            return SLAVE_DEVICE_FAILURE;
    }

    err = HAL_FLASH_Lock();
    if(err)
        return SLAVE_DEVICE_FAILURE;
    else
        return 0;
}

int Storage_IsSaved(void *context, bool *state)
{
    *state = memcmp(&_sstdata, &_ldstdata, &_estdata - &_sstdata) == 0;
    return 0;
}
