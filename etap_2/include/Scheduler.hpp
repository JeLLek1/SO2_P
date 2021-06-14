#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <Display.hpp>
#include "Random.hpp"
#include "Entities/Hunter.hpp"
#include "Entities/Prey.hpp"

class Scheduler{
private:
    std::atomic<bool> _isRunning{ true };
    std::mutex _mRandom;
    Random<size_t> _random;
    Display _display;
    std::vector<std::unique_ptr<Hunter>> _huntersList;
    std::vector<std::unique_ptr<Prey>> _preyList;
    void init();
    void loop();
    void loadEntities();
public:
    Scheduler();
    template<typename T>
    T getRandom(T min, T max);
    static void join(std::thread& thread);
    std::vector<Vector2<size_t>> getHuntersPos(const std::vector<HunterStates>& states = {});
    std::vector<Vector2<size_t>> getPreyPos(const std::vector<PreyStates>& states = {});
    std::vector<HunterData> getHuntersData(const std::vector<HunterStates>& states = {});
    std::vector<PreyData> getPreysData(const std::vector<PreyStates>& states = {});
    void setPreyCaught(size_t index, size_t hunterIndex);
    void setHunterRespawn(size_t index);
    void notifyCaughtPrey(size_t index);
    bool isRunning();
    static unsigned int countDigits(unsigned int number);
    ~Scheduler();
};

#include "Scheduler.inl"
