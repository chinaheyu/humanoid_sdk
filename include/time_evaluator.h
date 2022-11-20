#ifndef __TIME_EVALUATOR_HPP__
#define __TIME_EVALUATOR_HPP__

#include <chrono>

class TimeEvaluator
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> st;
    bool start_measure_flag{false};
public:
    TimeEvaluator(){}

    void start()
    {
        st = std::chrono::high_resolution_clock::now();
        start_measure_flag = true;
    }

    bool is_measurement_start()
    {
        return start_measure_flag;
    }

    double end()
    {
        start_measure_flag = false;
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - st).count();
    }
    
};

#endif
