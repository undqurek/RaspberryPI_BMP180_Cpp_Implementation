#ifndef I2C_H
#define I2C_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>

/*

https://www.kernel.org/doc/Documentation/i2c/dev-interface

*/
class I2C
{
private:
    const char *_path;
    int _file;

private:
    template<typename T> bool read8Bits(uint8_t reg, T &outValue)
    {
        if(::write(this->_file, &reg, 1) == 1)
        {
            ::usleep(3);

            if(::read(this->_file, &outValue, 1) == 1)
                return true;
        }

        return false;
    }

    template<typename T, typename msb_t> bool read16Bits(uint8_t reg, T &outValue)
    {
        msb_t msb;
        uint8_t lsb;

        if(this->read8Bits(reg + 0, msb)
            && this->read8Bits(reg + 1, lsb))
        {
            outValue = (msb << 8) + lsb;

            return true;
        }

        return false;
    }

    template<typename T, typename msb_t> bool read24Bits(uint8_t reg, T &outValue)
    {
        msb_t msb;
        uint8_t lsb1, lsb2;

        if(this->read8Bits(reg + 0, msb)
            && this->read8Bits(reg + 1, lsb1)
            && this->read8Bits(reg + 2, lsb2))
        {
            outValue = (msb << 16) + (lsb1 << 8) + lsb2;

            return true;
        }

        return false;
    }

public:
    I2C(const char *path)
    {
        this->_path = path;
    }

    virtual ~I2C()
    {
        this->close();
    }

    bool isOpened()
    {
        return this->_file != -1;
    }

    bool open(int address)
    {
        this->_file = ::open(this->_path, O_RDWR);

        if(this->_file == -1)
            return false;

        if(::ioctl(this->_file, I2C_SLAVE, address) == -1)
        {
            ::close(this->_file);
            this->_file = -1;

            return false;
        }

        return true;
    }

    void close()
    {
        if(this->isOpened())
        {
            ::close(this->_file);
            this->_file = -1;
        }
    }

    // read

    bool readInt8(uint8_t reg, int8_t &outValue)
    {
        if(this->isOpened())
            return this->read8Bits(reg, outValue);

        return false;
    }

    bool readUInt8(uint8_t reg, uint8_t &outValue)
    {
        if(this->isOpened())
            return this->read8Bits(reg, outValue);

        return false;
    }

    bool readInt16(uint8_t reg, int16_t &outValue)
    {
        if(this->isOpened())
            return this->read16Bits<int16_t, int8_t>(reg, outValue);

        return false;
    }

    bool readUInt16(uint8_t reg, uint16_t &outValue)
    {
        if(this->isOpened())
            return this->read16Bits<uint16_t, uint8_t>(reg, outValue);

        return false;
    }

    bool readInt24(uint8_t reg, int32_t &outValue)
    {
        if(this->isOpened())
            return this->read24Bits<int32_t, int8_t>(reg, outValue);

        return false;
    }

    bool readUInt24(uint8_t reg, uint32_t &outValue)
    {
        if(this->isOpened())
            return this->read24Bits<uint32_t, uint8_t>(reg, outValue);

        return false;
    }

    // write

    bool writeUInt8(uint8_t reg, uint8_t value)
    {
        if(this->isOpened())
        {
            if(::write(this->_file, &value, 1) == 1)
                return true;
        }

        return false;
    }
};

#endif // I2C_H
