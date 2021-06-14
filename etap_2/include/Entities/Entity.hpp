#pragma once
#include <vector>
#include <atomic>
#include <shared_mutex>
#include <thread>
#include "Vector2.hpp"

class Scheduler;
class Entity{
protected:
    Vector2<size_t> _pos;
    std::shared_mutex _mPos;
    std::atomic<int> _progress;
    std::atomic<size_t> _index;
    std::thread _thread;
    Scheduler* _scheduler;
    void wait(int time);
    void setPosition(Vector2<size_t> pos);
    void respawn(int time);
    virtual void run() = 0;
public:
    Entity(size_t index, Vector2<size_t> pos, Scheduler* scheduler);
    void start();
    Vector2<size_t> getPosition();
    virtual void stop() = 0;
    ~Entity();
};
