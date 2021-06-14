#pragma once
#include "Vector2.hpp"
#include <cmath>

template <typename T>
inline Vector2<T>::Vector2() {
    this->x = this->y = 0;
}

template <typename T>
inline Vector2<T>::Vector2(T initX, T initY) {
    this->x = initX;
    this->y = initY;
}
template <typename T>
inline Vector2<T>::Vector2(const Vector2<T>& cpy): x(cpy.x), y(cpy.y){};


template<typename T>
inline Vector2<T>& Vector2<T>::operator=(const Vector2<T>& right) {
    this->x = right.x;
    this->y = right.y;
    return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& right) {
    this->x += right.x;
    this->y += right.y;

    return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator*=(const Vector2<T>& right) {
    this->x *= right.x;
    this->y *= right.y;

    return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator/=(const T& right) {
    this->x /= right;
    this->y /= right;

    return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator*=(const T& right) {
    this->x *= right;
    this->y *= right;

    return *this;
}
template<typename T>
inline float Vector2<T>::euclideanDistance(const Vector2<T>& to){
    float xTmp = static_cast<float>(x) - static_cast<float>(to.x);
    float yTmp = static_cast<float>(y) - static_cast<float>(to.y);
    return std::sqrt(xTmp*xTmp + yTmp*yTmp);
}

template<typename T>
inline Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right) {
    return Vector2<T>(left.x - right.x,
        left.y - right.y);
}

template<typename T>
inline Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right) {
    return Vector2<T>(left.x + right.x,
        left.y + right.y);
}

template <typename T>
inline bool operator ==(const Vector2<T>& left,
    const Vector2<T>& right) {
    if (left.x == right.x
        && left.y == right.y) {
        return true;
    }
    return false;
}
