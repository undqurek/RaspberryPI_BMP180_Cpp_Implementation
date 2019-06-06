#ifndef RPIBAROMETER_H
#define RPIBAROMETER_H

#include "i2c.h"

/*

http://www.jarzebski.pl/arduino/czujniki-i-sensory/czujniki-cisnienia-bmp085-bmp180.html
https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

*/
class RPiBarometer
{
private:
    I2C _device;

    // zmienne kalibrujace
    int16_t _ac1;
    int16_t _ac2;
    int16_t _ac3;
    uint16_t _ac4;
    uint16_t _ac5;
    uint16_t _ac6;
    int16_t _b1;
    int16_t _b2;
    int16_t _mb;
    int16_t _mc;
    int16_t _md;

private:
    uint16_t readUncompensatedTemperature()
    {
        if(this->_device.writeUInt8(0xF4, 0x2E))
        {
            ::usleep(4500);

            uint16_t result;

            if(this->_device.readUInt16(0xF6, result))
                return result;
        }

        return (-1);
    }

    uint32_t readUncompensatedPressure(uint8_t oss)
    {
        uint8_t address = 0x34 + (oss << 6);

        if(this->_device.writeUInt8(0xF4, address))
        {
            ::usleep(4500);

            uint32_t result;

            if(this->_device.readUInt24(0xF6, result))
                return result >> (8 - oss);
        }

        return (-1);
    }

public:
    RPiBarometer(const char *path = "/dev/i2c-1")
        : _device(I2C(path))
    {

    }

    virtual ~RPiBarometer()
    {
        this->close();
    }

    bool open(int address = 0x77)
    {
        if(this->_device.open(address))
        {
            // calibration coefficients

            this->_device.readInt16(0xAA, this->_ac1);
            this->_device.readInt16(0xAC, this->_ac2);
            this->_device.readInt16(0xAE, this->_ac3);
            this->_device.readUInt16(0xB0, this->_ac4);
            this->_device.readUInt16(0xB2, this->_ac5);
            this->_device.readUInt16(0xB4, this->_ac6);
            this->_device.readInt16(0xB6, this->_b1);
            this->_device.readInt16(0xB8, this->_b2);
            this->_device.readInt16(0xBA, this->_mb);
            this->_device.readInt16(0xBC, this->_mc);
            this->_device.readInt16(0xBE, this->_md);


//            this->_ac1 = 408;
//            this->_ac2 = -72;
//            this->_ac3 = -14383;
//            this->_ac4 = 32741;
//            this->_ac5 = 32757;
//            this->_ac6 = 23153;
//            this->_b1 = 6190;
//            this->_b2 = 4;
//            this->_mb = -32764;
//            this->_mc = -8711;
//            this->_md = 2868;

            return true;
        }

        return false;
    }

    void close()
    {
        this->_device.close();
    }

    bool resetDevice()
    {
        if(this->_device.isOpened())
        {
            if(this->_device.writeUInt8(0xE0, 0xB6))
            {
                // calibration coefficients

                this->_device.readInt16(0xAA, this->_ac1);
                this->_device.readInt16(0xAC, this->_ac2);
                this->_device.readInt16(0xAE, this->_ac3);
                this->_device.readUInt16(0xB0, this->_ac4);
                this->_device.readUInt16(0xB2, this->_ac5);
                this->_device.readUInt16(0xB4, this->_ac6);
                this->_device.readInt16(0xB6, this->_b1);
                this->_device.readInt16(0xB8, this->_b2);
                this->_device.readInt16(0xBA, this->_mb);
                this->_device.readInt16(0xBC, this->_mc);
                this->_device.readInt16(0xBE, this->_md);

                return true;
            }
        }

        return false;
    }

    /**
     * @brief Reads temperature in celcius degrees.
     *
     * @return temperature in celcius degrees
     */
    double readTemperature()
    {
        if(this->_device.isOpened())
        {
            // read uncompensated temperature value

            uint16_t ut = this->readUncompensatedTemperature();

            // calculate true temperature

            int64_t x1 = ((ut - this->_ac6) * this->_ac5) >> 15;
            int64_t x2 = (this->_mc << 11) / (x1 + this->_md);
            int64_t b5 = x1 + x2;

            return ((b5 + 8) >> 4) * 0.1;
        }

        return (0.0 / 0.0);
    }

    double readPressure()
    {
		//TODO: verification
		
        if(this->_device.isOpened())
        {
            uint8_t oss = 0; // ultra low power mode // 0..3 <- oversampling setting

            // read uncompensated temperature value

            uint16_t ut = this->readUncompensatedTemperature();

            // calculate true temperature

            int64_t x1 = ((ut - this->_ac6) * this->_ac5) >> 15;
            int64_t x2 = (this->_mc << 11) / (x1 + this->_md);
            int64_t b5 = x1 + x2;

            // read uncompensated pressure value

            int64_t up = this->readUncompensatedPressure(oss);

            // calculate true pressure

            int64_t b6 = b5 - 4000;
             x1 = (this->_b2 * ((b6 * b6) >> 12)) >> 11;
             x2 = (this->_ac2 * b6) >> 11;
            int64_t x3 = x1 + x2;
            int64_t b3 = (((this->_ac1 * 4 + x3) << oss) + 2) / 4;

            x1 = (this->_ac3 * b6) >> 13;
            x2 = (this->_b1 * ((b6 * b6) >> 12)) >> 16;
            x3 = ((x1 + x2) + 2) >> 2;

            uint64_t b4 = (this->_ac4 * (uint64_t)(x3 + 32764)) >> 15;
            uint64_t b7 = ((uint64_t)up - b3) * (50000 >> oss);

            int64_t p = (b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2);

            x1 = (p >> 8) * (p >> 8);
            x1 = (x1 * 3038) >> 16;
            x2 = (-7357 * p) >> 16;

            return p + ((x1 + x2 + 3791) >> 4);
        }

        return (0.0 / 0.0);
    }
};

#endif // RPIBAROMETER_H
