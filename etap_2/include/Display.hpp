#pragma once
#include <atomic>
#include <vector>
#include <thread>
#include "Vector2.hpp"

enum Colors: short{
    COLOR_BORDER = 1,
    COLOR_GROUND = 2,
    COLOR_PREY = 3,
    COLOR_HUNTER = 4,
    COLOR_HUNTER_WITH_PREY = 5,
    COLOR_TEXT = 6,
    COLOR_DONATION = 7,
};

class Scheduler;
class Display{
private:
    Scheduler* _scheduler;
    std::atomic<bool> _isRunning = {true};
    std::thread _thread;
    void displayHUD();
    void displayMap();
    void displayEntities();
    void endProcess();
    void run();
public:
    Display(Scheduler* scheduler);
    void start();
    unsigned findSamePos(std::vector<std::pair<Vector2<size_t>, unsigned>>& poses, Vector2<size_t> search);
    ~Display();
};
