#pragma once
#include <mutex>
#include <atomic>
#include <map>
#include <vector>
#include <utility>

class Scheduler;

enum class States : size_t{
    EAT = 0,
    SLEEP = 1,
    WAIT_L = 2,
    WAIT_R = 3,
    WAIT_HUNGRY = 4,
};

struct PhilosopherData{
    States satate;
    int progress;
    int eatingCount;

    PhilosopherData(States satate, int progress, int eatingCount);
};

class Monitor{
private:
    Scheduler* _scheduler;
    std::atomic<bool> _isRunning{ true };
    std::mutex _mData;
    std::mutex _mFrokData;
    enum class Colors : short{
        TITLE = 1,
        NORMAL = 2,
        GREEN = 3,
        SUBTITLE = 4,
        YELOW = 5,
    };
	static const std::map<States, std::string> _stateNames;
    std::vector<PhilosopherData> _philosophersData;
    std::vector<int> _forksData;
public:
    Monitor(Scheduler* scheduler);
    void run();
    void endMonitor();
    void updateMonitor(size_t index, States state, int progress, int eatingCount);
    void updateForkData(size_t forkNo, int philosopherNo);
    ~Monitor();
};
