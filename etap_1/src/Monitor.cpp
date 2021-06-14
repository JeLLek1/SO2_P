#include "Monitor.hpp"
#include "Scheduler.hpp"
#include "settings.hpp"
#include <ncurses.h>
#include <string>

PhilosopherData::PhilosopherData(States satate, int progress, int eatingCount){
    this->satate = satate;
    this->progress = progress;
    this->eatingCount = eatingCount;
}

const std::map<States, std::string> Monitor::_stateNames = {
	{States::EAT, "Je"},
	{States::SLEEP, "Mysli"},
	{States::WAIT_L, "Czeka na lewy widelec"},
	{States::WAIT_R, "Czeka na prawy widelec"},
	{States::WAIT_HUNGRY, "Czeka na wyglodzonego filozofa"},
};

Monitor::Monitor(Scheduler* scheduler)
    : _philosophersData(philosophersCount, PhilosopherData(States::WAIT_L, 0, 0)),
    _forksData(philosophersCount, -1){
    initscr();			// uruchomienie trybu curses
	raw();				// wyłączenie buforowania linii (wyłącza równieć ctrl+c i ctrl+z, cbreak() na nie pozwala)
    noecho();           // wyłączenie wyświetlania wpisywanych znaków
    start_color();      // obsługa kolorów
    init_pair(static_cast<short>(Monitor::Colors::TITLE), COLOR_RED, COLOR_WHITE);
    init_pair(static_cast<short>(Monitor::Colors::NORMAL), COLOR_WHITE, COLOR_BLACK);
    init_pair(static_cast<short>(Monitor::Colors::GREEN), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<short>(Monitor::Colors::SUBTITLE), COLOR_BLUE, COLOR_WHITE);
    init_pair(static_cast<short>(Monitor::Colors::YELOW), COLOR_YELLOW, COLOR_BLACK);
    _scheduler = scheduler;
}

void Monitor::updateMonitor(size_t index, States state, int progress, int eatingCount){
    std::unique_lock<std::mutex> dataLock(_mData);
    _philosophersData[index]=PhilosopherData(state, progress, eatingCount);
}
void Monitor::updateForkData(size_t forkNo, int philosopherNo){
    std::unique_lock<std::mutex> dataLock(_mFrokData);
    _forksData[forkNo] = philosopherNo;
}

void Monitor::run(){
    unsigned int digits = _scheduler->countDigits(philosophersCount);
    while(_isRunning){
        clear();
        move(0,0);
        attron(COLOR_PAIR(Monitor::Colors::TITLE));
        attron(A_BOLD);
        printw("Problem filozofow ( nacisnij q aby wyjsc )");
        move(2,0);
        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        printw("ustawienia:");
        attroff(A_BOLD);
        move(3,0);
        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        printw("Ilosc filozofow: ");
        attron(COLOR_PAIR(Monitor::Colors::GREEN));
        printw("%d", philosophersCount);
        move(4,0);
        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        printw("Czas trwania jedzenia: ");
        attron(COLOR_PAIR(Monitor::Colors::GREEN));
        printw("%dms", eatMs);
        move(5,0);
        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        printw("Czas trwania myslenia: ");
        attron(COLOR_PAIR(Monitor::Colors::GREEN));
        printw("%dms", sleepMs);

        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        attron(A_BOLD);
        move(7,0);
        printw("Filozofowie:");
        attroff(A_BOLD);
        for(size_t i=0; i<philosophersCount; i++){
            move(8+i,0);
            attron(COLOR_PAIR(Monitor::Colors::SUBTITLE));
            printw("Filozof %d ", i+1);
            attron(COLOR_PAIR(Monitor::Colors::NORMAL));
            printw(": ");
            move(8+i, digits+11);
            _mData.lock();
            PhilosopherData data = _philosophersData[i];
            _mData.unlock();
            printw("%s...", _stateNames.at(data.satate).c_str());
            move(8+i, digits+44);
            printw(" | postep: ");
            attron(COLOR_PAIR(Monitor::Colors::GREEN));
            if(data.satate==States::EAT || data.satate==States::SLEEP){
                printw("%d%%", data.progress);
            }else{
                printw("---");
            }
            attron(COLOR_PAIR(Monitor::Colors::NORMAL));
            move(8+i, digits+64);
            printw(" | ilosc posilkow: ");
            attron(COLOR_PAIR(Monitor::Colors::GREEN));
                printw("%d", data.eatingCount);
        }

        attron(COLOR_PAIR(Monitor::Colors::NORMAL));
        attron(A_BOLD);
        move(9+philosophersCount,0);
        printw("Widelce:");
        attroff(A_BOLD);
        for(size_t i=0; i<philosophersCount; i++){
            move(10+philosophersCount+i,0);
            attron(COLOR_PAIR(Monitor::Colors::SUBTITLE));
            printw("Widelec %d ", i+1);
            attron(COLOR_PAIR(Monitor::Colors::NORMAL));
            printw(": ");
            move(10+philosophersCount+i, digits+11);
            _mFrokData.lock();
            int data = _forksData[i];
            _mFrokData.unlock();
            if(data>=0){
                printw("u filozofa %d", data+1);
            }else{
                printw("brak przydzialu");
            }
        }

        move(2*philosophersCount + 11,0);
        if(!_scheduler->getIsRunning()){
            attron(COLOR_PAIR(Monitor::Colors::TITLE));
            printw("Trwa zamykanie...");
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(refreshTime));
    }
}

void Monitor::endMonitor(){
    _isRunning = false;
}

Monitor::~Monitor(){
}

