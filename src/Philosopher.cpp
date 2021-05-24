#include "Philosopher.hpp"
#include <iostream>
#include "settings.hpp"
#include "Scheduler.hpp"

Philosopher::Philosopher(Scheduler* scheduler, size_t no): _scheduler(scheduler){
    _no=no;
}

void Philosopher::run(){
    while(_scheduler->getIsRunning()){
        this->_scheduler->checkIfAnotherHungry(_no);
        this->_scheduler->updateMonitor(this->_no, States::WAIT_L, 0, _eatingCount);
        this->_scheduler->getFork(_no, true, _no);
        this->_scheduler->updateMonitor(this->_no, States::WAIT_R, 0, _eatingCount);
        this->_scheduler->getFork((_no+1)%philosophersCount, false, _no);
        this->wait(eatMs,States::EAT);
        _eatingCount++;
        this->_scheduler->notifyEatEnd();
        this->_scheduler->returnFork(_no);
        this->_scheduler->returnFork((_no+1)%philosophersCount);
        this->wait(sleepMs,States::SLEEP);
    }
}

void Philosopher::wait(int time, States status){
    this->_scheduler->updateMonitor(this->_no, status, 0, _eatingCount);
    int waitTime = static_cast<int>(eatMs * _scheduler->getRandom(0.8f, 1.2f));
    int totalTime = waitTime;
    while(waitTime>0){
        int delta = (waitTime>refreshTime)? refreshTime : waitTime;
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(delta));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end-start;
        waitTime-=static_cast<int>(elapsed.count());
        int percent = (((totalTime-waitTime)*100)/totalTime);
        this->_scheduler->updateMonitor(this->_no, status, (percent>100)?100:percent, _eatingCount);
    }
}

int Philosopher::getEatingCount(){
    return _eatingCount;
}

Philosopher::~Philosopher(){
}

