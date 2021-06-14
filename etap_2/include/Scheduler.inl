#pragma once
#include "Scheduler.hpp"

template<typename T>
T Scheduler::getRandom(T min, T max){
    std::unique_lock<std::mutex> lock(_mRandom);
    return _random.rand(min, max);
}
