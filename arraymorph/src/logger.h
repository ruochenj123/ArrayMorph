#include <iostream>
#include <string>
using namespace std;
#include "constants.h"
#ifndef __LOGGER__
#define __LOGGER__
class Logger
{
public:
    static void log(string message, string value = "")
    {
        #ifdef LOG_ENABLE
        cout << message << value << endl;
        #endif
    }

    static void log(string message, double value)
    {
        #ifdef LOG_ENABLE
        cout << message << value << endl;
        #endif
    }

};
#endif
