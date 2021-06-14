#pragma once

template <typename T>
class Vector2 {
 public:

    T x;
    T y;

    Vector2();
    Vector2(T initX, T initY);
    Vector2(const Vector2<T>& cpy);

    Vector2<T>& operator =(const Vector2<T>& rigth);
    Vector2<T>& operator +=(const Vector2<T>& rigth);
    Vector2<T>& operator *=(const Vector2<T>& rigth);
    Vector2<T>& operator /=(const T& rigth);
    Vector2<T>& operator *=(const T& rigth);
    float euclideanDistance(const Vector2<T>& to);
};

template <typename T>
Vector2<T> operator -(const Vector2<T>& left, const Vector2<T>& right);

template <typename T>
Vector2<T> operator +(const Vector2<T>& left, const Vector2<T>& right);

template <typename T>
bool operator ==(const Vector2<T>& left, const Vector2<T>& right);

#include "Vector2.inl"
