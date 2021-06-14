#pragma once
#include <shared_mutex>
#include <map>
#include <atomic>
#include "Entity.hpp"
#include "Vector2.hpp"

enum class HunterStates{
    SEEK = 0,
    HAS_PRAY = 1,
    RESPAWN = 2,
};
struct HunterData{
    Vector2<size_t> pos;
    HunterStates state;
    int progress;
    size_t index;
    size_t preyIndex;
    unsigned int points;
};
class Scheduler;
class Hunter : public Entity{
    enum class TileType: char{
        GROUND,
        WALL,
        ENTITY,
    };
    HunterStates _state = HunterStates::SEEK;
    std::shared_mutex _mState;
    Vector2<size_t> bestMove(std::vector<std::vector<TileType>>& tmpMap, Vector2<size_t> from);
    std::atomic<size_t> _preyIndex;
    std::atomic<unsigned int> _points = {0};
    std::mutex _mHasPrey;
    void seekPrey();
    void returnPrey();
    void setState(HunterStates state);
    virtual void run();
public:
    static const std::map<HunterStates, std::string> stateNames;
    Hunter(size_t index, Vector2<size_t> pos, Scheduler* scheduler);
    virtual void stop();
    void setRespawn();
    HunterStates getState();
    HunterData getHunterData();
    ~Hunter();
};
