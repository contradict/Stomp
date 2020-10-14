#pragma once
#include <Arduino.h>
#include "I2C.h"

namespace I2Cdev {
    static const uint16_t readTimeout=5;
    static int8_t readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout=I2Cdev::readTimeout)
    {
        int8_t err;
        I2c.timeOut(timeout);
        err = I2c.read(devAddr, regAddr, 1, data);
        *data &= _BV(bitNum);
        return err;
    }

    static int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout=I2Cdev::readTimeout)
    {
        int8_t err;
        I2c.timeOut(timeout);
        err = I2c.read(devAddr, regAddr, 1, data);
        *data >>= (bitStart-length+1);
        *data &= (_BV(length+1)-1);
        return err;
    }

    static int8_t readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout=I2Cdev::readTimeout)
    {
        int8_t err;
        I2c.timeOut(timeout);
        err = I2c.read(devAddr, regAddr, 1, data);
        return err;
    }

    static int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout=I2Cdev::readTimeout)
    {
        int8_t err;
        I2c.timeOut(timeout);
        err = I2c.read(devAddr, regAddr, length, data);
        return err;
    }

    static bool writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data)
    {
        uint8_t tmp;
        readByte(devAddr, regAddr, &tmp);
        tmp &= ~_BV(bitNum);
        data &= _BV(bitNum);
        tmp |= data;
        return 0==I2c.write(devAddr, regAddr, &tmp, 1);
    }

    static bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data)
    {
        uint8_t tmp, mask;
        readByte(devAddr, regAddr, &tmp);
        mask = (_BV(length+1)-1)<<(bitStart-length+1);
        data <<= (bitStart-length+1);
        data &= mask;
        tmp &= ~mask;
        tmp |= data;
        return 0==I2c.write(devAddr, regAddr, &tmp, 1);
    }

    static bool writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
    {
        return 0==I2c.write(devAddr, regAddr, &data, 1);
    }

    static bool writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
    {
        return 0==I2c.write(devAddr, regAddr, data, length);
    }

    static bool writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data)
    {
        return 0==I2c.write(devAddr, regAddr, (uint8_t *)&data, 2);
    }

}
