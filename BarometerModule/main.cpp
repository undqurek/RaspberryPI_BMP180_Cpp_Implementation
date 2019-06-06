#include <iostream>

#include "rpibarometer.h"

using namespace std;


int main()
{
    RPiBarometer barometer;

    if(barometer.open())
    {
        while(true)
        {
            barometer.resetDevice();

            double temp = barometer.readTemperature();
            double pressure = barometer.readPressure();

            cout << "temp: " << temp << " oC" << endl;
            cout << "pressure: " << (pressure * 0.01) << " hPa" << endl;
        }
    }

    return 0;
}
