#include "profiler.h"

void Profiler::start_timer(){
    gettimeofday(&start_time, NULL);
}

void Profiler::stop_timer(){

    struct timeval end;
    
    gettimeofday(&end, NULL);
    double t = (1000000 * ( end.tv_sec - start_time.tv_sec )
            + end.tv_usec -start_time.tv_usec) /1000000.0;
    
    time += t;
}

void Profiler::add_io_size(uint64_t size){
    io_size += size;
    io_num ++;
}

void Profiler::clear(){
    time = 0;
    io_size = 0;
    io_num = 0;
}

void Profiler::add(const Profiler& b){
    this->time += b.time;
    this->io_size += b.io_size;
    this->io_num += b.io_num;
}
