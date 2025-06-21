#include <iostream>
#include <sstream>
#include "constants.h"
#ifndef __LOGGER__
#define __LOGGER__
class Logger {
public:
    template <typename... Args>
    static void log(Args&&... args) {
#ifdef LOG_ENABLE
    std::ostringstream oss;
    ((oss << std::forward<Args>(args) << ' '), ...);
    std::cout << oss.str() << std::endl;
#endif
    }
};
#endif
