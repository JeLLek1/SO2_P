#include "Scheduler.hpp"
#include <iostream>
#include <ncurses.h>
#include "Philosopher.hpp"
#include "settings.hpp"

Scheduler::Scheduler() : _monitor(this){
    init();
    loop();
}

void Scheduler::init(){
    for (size_t i = 0; i < philosophersCount; i++){
        _forksList.push_back(Fork(i));
    }
    for (size_t i = 0; i < philosophersCount; ++i){
        _philosophers.push_back(new Philosopher(this, i));
    }
    for (size_t i = 0; i < philosophersCount; ++i){
        _philosopherThreads.push_back(std::thread(&Philosopher::run, _philosophers[i]));
    }
    _tMonitor = std::thread(&Monitor::run, &_monitor);
}

void Scheduler::loop(){
    char c = 0;
    c = getch();
    while(c != 113){
        c = getch();
    }
    _isRunning = false;
}

bool Scheduler::getIsRunning(){
    return _isRunning;
}

float Scheduler::getRandom(float min, float max){
    std::unique_lock<std::mutex> lock(_mRandom);
    return _random.rand(min, max);
}

void Scheduler::join(std::thread& thread){
    try
	{
		if (thread.joinable()) // czy wątek jest jeszcze uruchominoy
			thread.join(); // oczekiwanie na zakończenie wątku
	}
	catch (std::system_error &e)
	{
		std::cerr << e.what() << std::endl; // błąd na standardowy strumień błędów
	}
}

//zwraca liczbę cyfr w liczbie
unsigned int Scheduler::countDigits(unsigned int number)
{
	unsigned int digits = 0;
	do{
		digits++;
		number /= 10;
	}while (number);

	return digits;
}
void Scheduler::checkIfAnotherHungry(size_t no){
    std::unique_lock<std::mutex> hungryLock(_mHungry);
    while(true){
        int mealCount = _philosophers[0]->getEatingCount();
        size_t hungerMaxIndex = 0;
        for(size_t i = 1; i < philosophersCount; i++){
            mealCount+=_philosophers[i]->getEatingCount();
            if(_philosophers[i]->getEatingCount()<_philosophers[hungerMaxIndex]->getEatingCount()){
                hungerMaxIndex = i;
            }
        }
        int eatingCounts = _philosophers[hungerMaxIndex]->getEatingCount()*3;
        if(no==hungerMaxIndex || (mealCount/philosophersCount)<=eatingCounts){
            break;
        }
        updateMonitor(no, States::WAIT_HUNGRY, 0, _philosophers[no]->getEatingCount());
        _hungryCondition.wait(hungryLock);
    }
}

void Scheduler::notifyEatEnd(){
    _hungryCondition.notify_all();
}

void Scheduler::getFork(size_t no, bool isFirst, size_t phNo){
    std::unique_lock<std::mutex> waiterLock(_mWaiter);
    while (_forksList[no].isTaken || (isFirst && _forksTaken>=philosophersCount-1)){
        _waiterCondition.wait(waiterLock);
    }
    _forksList[no].isTaken = true;
    _forksTaken++;
    _monitor.updateForkData(no, static_cast<int>(phNo));
}

void Scheduler::returnFork(size_t no){
    std::unique_lock<std::mutex> waiterLock(_mWaiter);
    _forksList[no].isTaken = false;
    _forksTaken--;
    _waiterCondition.notify_all();
    _monitor.updateForkData(no, -1);
}

void Scheduler::updateMonitor(size_t index, States state, int progress, int eatingCount){
    _monitor.updateMonitor(index, state, progress, eatingCount);
}

std::condition_variable& Scheduler::getWaiterCondition(){
    return _waiterCondition;
}

Scheduler::~Scheduler(){
    for (std::thread &i : _philosopherThreads)
	{
		join(i);
	}
    _monitor.endMonitor();
    join(_tMonitor);
    for (Philosopher* &i : _philosophers)
	{
		delete i;
	}
}
