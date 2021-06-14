#include <limits>
#include "Entities/Prey.hpp"
#include "Scheduler.hpp"
#include "settings.hpp"

const std::map<PreyStates, std::string> Prey::stateNames = {
	{PreyStates::RUN_AWAY, "Ucieka przed lowcami"},
	{PreyStates::CAUGHT, "Zostala zlapana"},
	{PreyStates::RESPAWN, "Czeka na respawn"},
};

Prey::Prey(size_t index, Vector2<size_t> pos, Scheduler* scheduler): Entity(index, pos, scheduler){
}

PreyStates Prey::getState(){
    std::shared_lock<std::shared_mutex> lockState(_mState);
    return _state;
}
void Prey::setState(PreyStates state){
    std::unique_lock<std::shared_mutex> lockState(_mState);
    _state = state;
}

void Prey::run(){
    while(_scheduler->isRunning()){
        switch (getState())
        {
        case PreyStates::RUN_AWAY:
            runAway();
            break;
        case PreyStates::CAUGHT:
            caughtWait();
            break;
        case PreyStates::RESPAWN:
            respawn(PREY_RESPAWN_TIME);
            setState(PreyStates::RUN_AWAY);
            break;
        }
    }
}

void Prey::runAway(){
    std::vector<Vector2<size_t>> huntersPos = _scheduler->getHuntersPos({HunterStates::SEEK, HunterStates::HAS_PRAY});
    Vector2<size_t> currentPos = getPosition();
    Vector2<size_t> bestPos = currentPos;
    float distance = shortestDistance(_pos, huntersPos);
    for(size_t i = 0; i< sizeof(ALLOWED_MOVEMENT)/sizeof(ALLOWED_MOVEMENT[0]); i++){
        Vector2<size_t> newPos = currentPos + ALLOWED_MOVEMENT[i];
        if(newPos.x >= MAP_WIDTH || newPos.y >= MAP_HEIGHT || MAP_DATA[newPos.y][newPos.x]) continue;

        if(MAP_DATA[newPos.y][newPos.x]) continue;
        float tmpDistance = shortestDistance(newPos, huntersPos);
        if(tmpDistance > distance){
            bestPos = newPos;
            distance = tmpDistance;
        }
    }
    wait(PREY_IDLE);
    if(getState() != PreyStates::RUN_AWAY) return;
    setPosition(bestPos);
}

void Prey::caughtWait(){
    std::unique_lock<std::mutex> lock(_mcv);
    _cvPrayCaught.wait(lock, [=]{ return this->getState()!=PreyStates::CAUGHT;});
}

void Prey::setCaught(size_t hunterIndes){
    setState(PreyStates::CAUGHT);
    _hunterIndex = hunterIndes;
}

void Prey::notifyCaughtPrey(){
    std::unique_lock<std::mutex> lock(_mcv);
    setState(PreyStates::RESPAWN);
    _cvPrayCaught.notify_one();
}

PreyData Prey::getPreyData(){
    PreyData data;
    std::shared_lock<std::shared_mutex> posLock(_mPos, std::defer_lock);
    std::shared_lock<std::shared_mutex> stateLock(_mState, std::defer_lock);
    std::lock(posLock, stateLock);
    data.pos = _pos;
    data.state = _state;
    data.progress = _progress.load();
    data.index = _index.load();
    data.hunterIndex = _hunterIndex.load();
    return data;
}

float Prey::shortestDistance(Vector2<size_t> from, std::vector<Vector2<size_t>>& to){
    float distance = std::numeric_limits<float>::max();
    for(auto& i: to){
        float tmpDistance = i.euclideanDistance(from);
        if(tmpDistance < distance){
            distance = tmpDistance;
        }
    }
    return distance;
}

void Prey::stop(){
    notifyCaughtPrey();
    _scheduler->join(_thread);
}

Prey::~Prey(){
}
