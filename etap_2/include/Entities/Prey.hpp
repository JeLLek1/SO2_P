#pragma once
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <map>
#include <atomic>
#include <condition_variable>
#include "Vector2.hpp"
#include "Entity.hpp"

enum class PreyStates{
    RUN_AWAY = 0,
    CAUGHT = 1,
    RESPAWN = 2,
};
struct PreyData{
    Vector2<size_t> pos;
    PreyStates state;
    int progress;
    size_t index;
    size_t hunterIndex;
};
class Scheduler;
class Prey : public Entity{
    PreyStates _state = PreyStates::RUN_AWAY;
    std::shared_mutex _mState;
    std::atomic<size_t> _hunterIndex;
    std::mutex _mcv;
    std::condition_variable _cvPrayCaught;
    void runAway();
    void setState(PreyStates state);
    void caughtWait();
    float shortestDistance(Vector2<size_t> from, std::vector<Vector2<size_t>>& pos);
    virtual void run();
public:
    static const std::map<PreyStates, std::string> stateNames;
    Prey(size_t index, Vector2<size_t> pos, Scheduler* scheduler);
    virtual void stop();
    PreyStates getState();
    PreyData getPreyData();
    void setCaught(size_t hunterIndes);
    void notifyCaughtPrey();
    ~Prey();
};
