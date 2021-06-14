#include <ncurses.h>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "Scheduler.hpp"
#include "Vector2.hpp"
#include "settings.hpp"

Scheduler::Scheduler(): _display(this), _preyList(PREY_COUNT), _huntersList(HUNTER_COUNT){
    init();
    loop();
}

void Scheduler::init(){
    loadEntities();
}

void Scheduler::loadEntities(){
    std::vector<Vector2<size_t>> emptySpaces;
    for(int y = 0; y<MAP_HEIGHT; y++){
        for(int x = 0; x<MAP_WIDTH; x++){
            if(!MAP_DATA[y][x]){
                emptySpaces.push_back(Vector2<size_t>(x, y));
            }
        }
    }
    assert(((void)"Za mało miejsca dla elementów na mapie", emptySpaces.size() >= HUNTER_COUNT+PREY_COUNT));
    for(size_t i = 0; i<HUNTER_COUNT; i++){
        size_t index = getRandom(static_cast<size_t>(0), emptySpaces.size());
        _huntersList[i] = std::make_unique<Hunter>(i, emptySpaces[index], this);
        emptySpaces.erase(emptySpaces.begin() + index);
    }
    for(size_t i = 0; i<PREY_COUNT; i++){
        size_t index = getRandom(static_cast<size_t>(0), emptySpaces.size());
        _preyList[i] =  std::make_unique<Prey>(i, emptySpaces[index], this);
        emptySpaces.erase(emptySpaces.begin() + index);
    }

    for(auto const& i: _huntersList){
        i->start();
    }
    for(auto const& i: _preyList){
        i->start();
    }
    _display.start();
}

void Scheduler::loop(){
    char c = 0;
    c = getch();
    while(c != 113){
        c = getch();
    }
    _isRunning = false;
}

bool Scheduler::isRunning(){
    return _isRunning.load();
}

std::vector<Vector2<size_t>> Scheduler::getHuntersPos(const std::vector<HunterStates>& states){
    std::vector<Vector2<size_t>> pos;
    for(auto const& i : _huntersList){
        if(states.empty() || std::find(states.begin(), states.end(), i->getState()) != states.end()){
            pos.push_back(i->getPosition());
        }
    }

    return pos;
}
std::vector<Vector2<size_t>> Scheduler::getPreyPos(const std::vector<PreyStates>& states){
    std::vector<Vector2<size_t>> pos;
    for(auto const& i : _preyList){
        if(states.empty() || std::find(states.begin(), states.end(), i->getState()) != states.end()){
            pos.push_back(i->getPosition());
        }
    }

    return pos;
}
std::vector<HunterData> Scheduler::getHuntersData(const std::vector<HunterStates>& states){
    std::vector<HunterData> data;
    for(auto const& i : _huntersList){
        if(states.empty() || std::find(states.begin(), states.end(), i->getState()) != states.end()){
            data.push_back(i->getHunterData());
        }
    }
    return data;
}

std::vector<PreyData> Scheduler::getPreysData(const std::vector<PreyStates>& states){
    std::vector<PreyData> data;
    for(auto const& i : _preyList){
        if(states.empty() || std::find(states.begin(), states.end(), i->getState()) != states.end()){
            data.push_back(i->getPreyData());
        }
    }
    return data;
}

void Scheduler::setPreyCaught(size_t index, size_t hunterIndex){
    _preyList[index]->setCaught(hunterIndex);
}

void Scheduler::setHunterRespawn(size_t index){
    _huntersList[index]->setRespawn();
}

void Scheduler::notifyCaughtPrey(size_t index){
    _preyList[index]->notifyCaughtPrey();
}

unsigned int Scheduler::countDigits(unsigned int number)
{
	unsigned int digits = 0;
	do{
		digits++;
		number /= 10;
	}while (number);

	return digits;
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

Scheduler::~Scheduler(){
    for(auto const& i: _huntersList){
        i->stop();
    }
    for(auto const& i: _preyList){
        i->stop();
    }
}
