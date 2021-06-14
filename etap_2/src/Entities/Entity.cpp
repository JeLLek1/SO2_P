#include "Entities/Entity.hpp"
#include "Scheduler.hpp"
#include "settings.hpp"

Entity::Entity(size_t index, Vector2<size_t> pos, Scheduler* scheduler){
    _pos = pos;
    _scheduler = scheduler;
    _index = index;
}

Vector2<size_t> Entity::getPosition(){
    std::shared_lock<std::shared_mutex> lockPos(_mPos);
    return Vector2<size_t>(_pos);
}

void Entity::setPosition(Vector2<size_t> pos){
    std::unique_lock<std::shared_mutex> lockPos(_mPos);
    _pos = pos;
}

void Entity::wait(int time){
    _progress = 0;
    int waitTime = _scheduler->getRandom(static_cast<int>(time - time * 0.2f), static_cast<int>(time + time * 0.2f));
    int totalTime = waitTime;
    while(waitTime>0){
        int delta = (waitTime>REFRESH_TIME)? REFRESH_TIME : waitTime;
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(delta));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end-start;
        waitTime-=static_cast<int>(elapsed.count());
        int tmpProgress = (((totalTime-waitTime)*100)/totalTime);
        _progress = (tmpProgress>100)? 100: tmpProgress;
    }
}

void Entity::respawn(int time){
    wait(time);
    bool tmpMap[MAP_HEIGHT][MAP_WIDTH];
    std::copy(&MAP_DATA[0][0], &MAP_DATA[0][0]+MAP_HEIGHT*MAP_WIDTH,&tmpMap[0][0]);
    std::vector<Vector2<size_t>> poses = _scheduler->getPreyPos({PreyStates::RUN_AWAY});
    for(const auto &pos: poses){
        tmpMap[pos.y][pos.x] = true;
    }
    poses = _scheduler->getHuntersPos({HunterStates::HAS_PRAY, HunterStates::SEEK});
    for(const auto &pos: poses){
        tmpMap[pos.y][pos.x] = true;
    }

    std::vector<Vector2<size_t>> emptySpaces;
    for(int y = 0; y<MAP_HEIGHT; y++){
        for(int x = 0; x<MAP_WIDTH; x++){
            if(!MAP_DATA[y][x]){
                emptySpaces.push_back(Vector2<size_t>(x, y));
            }
        }
    }
    setPosition(emptySpaces[_scheduler->getRandom(static_cast<size_t>(0), emptySpaces.size()-1)]);
}

void Entity::start(){
    if(!_thread.joinable())
        _thread = std::thread(&Entity::run, this);
}

Entity::~Entity(){
}
