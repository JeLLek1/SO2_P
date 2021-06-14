#include <queue>
#include "Entities/Hunter.hpp"
#include "Scheduler.hpp"
#include "settings.hpp"

const std::map<HunterStates, std::string> Hunter::stateNames = {
	{HunterStates::SEEK, "Szuka zdobyczy"},
	{HunterStates::HAS_PRAY, "Odnosi zdobycz"},
	{HunterStates::RESPAWN, "Czeka na respawn"},
};

Hunter::Hunter(size_t index, Vector2<size_t> pos, Scheduler* scheduler): Entity(index, pos, scheduler){
}

void Hunter::run(){
    while(_scheduler->isRunning()){
        switch (getState())
        {
        case HunterStates::SEEK:
            seekPrey();
            break;
        case HunterStates::HAS_PRAY:
            returnPrey();
            break;
        case HunterStates::RESPAWN:
            respawn(HUNTER_RESPAWN_TIME);
            setState(HunterStates::SEEK);
            break;
        }
    }
}

Vector2<size_t> Hunter::bestMove(std::vector<std::vector<TileType>>& tmpMap, Vector2<size_t> from){
    std::vector<std::vector<long>> weights(MAP_HEIGHT, std::vector<long>(MAP_WIDTH, -1));
    std::queue<Vector2<size_t>> posQueue;
    posQueue.push(from);
    weights[from.y][from.x] = 0;
    while (!posQueue.empty()) {
        for(size_t i = 0; i< sizeof(ALLOWED_MOVEMENT)/sizeof(ALLOWED_MOVEMENT[0]); i++){
            Vector2<size_t> tmpMov = posQueue.front() + ALLOWED_MOVEMENT[i];
            if(tmpMov.x > MAP_WIDTH || tmpMov.y > MAP_HEIGHT || weights[tmpMov.y][tmpMov.x] != -1) continue;
            if(tmpMap[tmpMov.y][tmpMov.x] == TileType::ENTITY){
                weights[tmpMov.y][tmpMov.x] = weights[posQueue.front().y][posQueue.front().x] + 1;
                while (weights[tmpMov.y][tmpMov.x] != 1){
                    int j = 0;
                    Vector2<size_t> newPos = tmpMov + ALLOWED_MOVEMENT[j];
                    while(newPos.x > MAP_WIDTH - 1 || newPos.y > MAP_HEIGHT - 1 || weights[newPos.y][newPos.x] != weights[tmpMov.y][tmpMov.x]-1){
                        j++;
                        newPos = tmpMov + ALLOWED_MOVEMENT[j];
                    }
                    tmpMov = newPos;
                } 
                return tmpMov;
            }else if(tmpMap[tmpMov.y][tmpMov.x] == TileType::GROUND){
                weights[tmpMov.y][tmpMov.x] = weights[posQueue.front().y][posQueue.front().x] + 1;
                posQueue.push(tmpMov);
            }
        }
        posQueue.pop();
    }

    return from;
}

void Hunter::returnPrey(){
    wait(HUNTER_IDLE_PREY);
    std::vector<std::vector<Hunter::TileType>> tmpMap(MAP_HEIGHT, std::vector<Hunter::TileType>(MAP_WIDTH, TileType::GROUND));
    for(size_t y = 0; y<MAP_HEIGHT; y++){
        for(size_t x = 0; x<MAP_WIDTH; x++){
            if(MAP_DATA[y][x]){
                tmpMap[y][x] = TileType::WALL;
            }
        }
    }
    for(size_t i = 0; i<sizeof(DONATION_POINTS)/sizeof(DONATION_POINTS[0]); i++){
        tmpMap[DONATION_POINTS[i].y][DONATION_POINTS[i].x] = TileType::ENTITY;
    }
    Vector2<size_t> currentPos = getPosition();
    Vector2<size_t> newPos = bestMove(tmpMap, currentPos);
    setPosition(newPos);
    for(size_t i = 0; i<sizeof(DONATION_POINTS)/sizeof(DONATION_POINTS[0]); i++){
        if(DONATION_POINTS[i] == newPos){
            _points++;
            setState(HunterStates::RESPAWN);
            _scheduler->notifyCaughtPrey(_preyIndex.load());
        }
    }
}

void Hunter::seekPrey(){
    wait(HUNTER_IDLE);

    std::vector<std::vector<Hunter::TileType>> tmpMap(MAP_HEIGHT, std::vector<Hunter::TileType>(MAP_WIDTH, TileType::GROUND));
    for(size_t y = 0; y<MAP_HEIGHT; y++){
        for(size_t x = 0; x<MAP_WIDTH; x++){
            if(MAP_DATA[y][x]){
                tmpMap[y][x] = TileType::WALL;
            }
        }
    }
    std::vector<PreyData> preys = _scheduler->getPreysData({PreyStates::RUN_AWAY});
    for(const auto &i: preys){
        tmpMap[i.pos.y][i.pos.x] = TileType::ENTITY;
    }
    Vector2<size_t> currentPos = getPosition();
    Vector2<size_t> newPos = bestMove(tmpMap, currentPos);
    setPosition(newPos);
    std::vector<HunterData> hunters = _scheduler->getHuntersData({HunterStates::SEEK, HunterStates::HAS_PRAY});
    bool foundAnother = false;
    for(const auto &i: hunters){
        if(i.index != _index && i.pos == newPos){
            foundAnother = true;
            _scheduler->setHunterRespawn(i.index);
        }
    }
    if(foundAnother){
        setRespawn();
        return;
    }
    for(const auto &i: preys){
        if(i.pos == newPos){
            std::unique_lock<std::mutex> lock(_mHasPrey);
            setState(HunterStates::HAS_PRAY);
            _preyIndex = i.index;
            _scheduler->setPreyCaught(i.index, _index.load());
            return;
        }
    }

}

void Hunter::setRespawn(){
    std::unique_lock<std::mutex> lock(_mHasPrey);
    HunterStates state = getState();
    if(state == HunterStates::SEEK){
        setState(HunterStates::RESPAWN);
    }else if(state == HunterStates::HAS_PRAY){
        setState(HunterStates::RESPAWN);
         _scheduler->notifyCaughtPrey(_preyIndex.load());
    }
}

HunterStates Hunter::getState(){
    std::shared_lock<std::shared_mutex> lockState(_mState);
    return _state;
}

void Hunter::setState(HunterStates state){
    std::unique_lock<std::shared_mutex> lockState(_mState);
    _state = state;
}

HunterData Hunter::getHunterData(){
    HunterData data;
    std::shared_lock<std::shared_mutex> posLock(_mPos, std::defer_lock);
    std::shared_lock<std::shared_mutex> stateLock(_mState, std::defer_lock);
    std::lock(posLock, stateLock);
    data.pos = _pos;
    data.state = _state;
    data.progress = _progress.load();
    data.index = _index.load();
    data.preyIndex = _preyIndex.load();
    data.points = _points.load();
    return data;
}

void Hunter::stop(){
    _scheduler->join(_thread);
}

Hunter::~Hunter(){

}
