#pragma once
#include "Monitor.hpp"

class Sheduler;

class Philosopher{
private:
    Scheduler* _scheduler;
    size_t _no;
    int eatingCount = 0;
public:
    Philosopher(Scheduler* scheduler, size_t no);
    void wait(int time, States status);
    void run(); // operator wywołania funkcyjnego
    int getEatingCount();
    ~Philosopher();
};
