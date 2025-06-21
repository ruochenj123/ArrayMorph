#ifndef __TIMER__
#define __TIMER__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

class Profiler{

public:
    Profiler(): time(0), io_size(0), io_num(0) {}

    void start_timer();
    void stop_timer();
    void add_io_size(uint64_t size);

    double getTime(){return time;}
    uint64_t getIOSize(){return io_size;}
    uint64_t getIONum() {return io_num;}

    void add(const Profiler& b);

    void clear();

private:
    double time;
    uint64_t io_size;
    uint64_t io_num;
    struct timeval start_time;

};

#endif