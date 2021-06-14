#pragma once
#include <iostream>

class Fork{
private:
    size_t _no;
public:
    bool isTaken;
    Fork(size_t _no);
    void getFork();
    void returnFork();
};
