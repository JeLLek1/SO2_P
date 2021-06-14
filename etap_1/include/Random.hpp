#pragma once

#include <random>

template<typename T>
class Random{
private:
    std::random_device rd;
    std::uniform_real_distribution<>distribution(T min, T max){
		return  std::uniform_real_distribution<>(min, max);
	}
public:
	T rand(T min, T max){
		std::mt19937 mt(rd());
		auto dis = distribution(min, max);
		return dis(mt);
	}
};
