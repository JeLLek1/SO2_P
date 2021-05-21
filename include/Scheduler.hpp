#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "Monitor.hpp"
#include "Random.hpp"
#include "Fork.hpp"
#include "Philosopher.hpp"

class Scheduler{
private:
    std::atomic<bool> _isRunning{ true };
    std::vector<std::thread> _philosopherThreads;
    std::vector<Philosopher> _philosophers;
    std::vector<Fork> _forksList;
    Random<float> _random;
    Monitor _monitor;
    std::thread _tMonitor;
    std::mutex _mRandom;
    std::mutex _mWaiter;
    std::mutex _mHungry;
    std::condition_variable _waiterCondition;
    std::condition_variable _hungryCondition;
    int _forksTaken = 0;
public:
    Scheduler();
    void loop();
    bool getIsRunning();
    float getRandom(float min, float max);
    void init();
    void join(std::thread& thread);
    void updateMonitor(size_t index, States state, int progress, int eatingCount);
    void getFork(size_t no, bool isFirst, size_t phNo);
    void returnFork(size_t no);
    void checkIfAnotherHungry(size_t no);
    unsigned int countDigits(unsigned int number);
    void notifyEatEnd();
    std::condition_variable& getWaiterCondition();
    ~Scheduler();
};
