#ifndef MOLESIM_VECTOR2_HPP
#define MOLESIM_VECTOR2_HPP

#include "Scalar.hpp"
#include <ostream>

namespace Molesim
{
/**
 * 2D vector.
 */
class Vector2
{
public:
    Vector2(Scalar s = 0) : x(s), y(s)
    {
    }

    Vector2(Scalar cx, Scalar cy) : x(cx), y(cy)
    {
    }

    static Vector2 PositiveInfinity()
    {
        return Vector2(ScalarPositiveInfinity);
    }

    static Vector2 NegativeInfinity()
    {
        return Vector2(ScalarNegativeInfinity);
    }

    Vector2 abs() const
    {
        return Vector2(Molesim::abs(x), Molesim::abs(y));
    }

    Scalar dot(const Vector2 &o) const
    {
        return x*o.x + y*o.y;
    }

    Scalar cross(const Vector2 &o) const
    {
        return x*o.y - y*o.x;
    }

    Scalar length2() const
    {
        return dot(*this);
    }

    Scalar length() const
    {
        return sqrt(length2());
    }

    Vector2 normalized() const
    {
        auto l = length();
        if(l <= 0)
            return Vector2(0);
        return Vector2(x / l, y / l);
    }
    
    Vector2 reciprocal() const
    {
        return Vector2(1/x, 1/y);
    }

    Vector2 floor() const
    {
        return Vector2(::floor(x), ::floor(y));
    }

    Vector2 floorRounded() const
    {
        return (*this + Vector2(0.5f)).floor();
    }

    bool operator==(const Vector2 &o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const Vector2 &o) const
    {
        return !(*this == o);
    }

    Vector2 operator+() const
    {
        return *this;
    }

    Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }
    
    Vector2 operator+=(const Vector2 &o)
    {
        return *this = (*this + o);
    }

    Vector2 operator*=(const Vector2 &o)
    {
        return *this = (*this * o);
    }

    Vector2 operator+(const Vector2 &o) const
    {
        return Vector2(x + o.x, y + o.y);
    }

    Vector2 operator-(const Vector2 &o) const
    {
        return Vector2(x - o.x, y - o.y);
    }

    Vector2 operator*(const Vector2 &o) const
    {
        return Vector2(x * o.x, y * o.y);
    }

    Vector2 operator/(const Vector2 &o) const
    {
        return Vector2(x / o.x, y / o.y);
    }

    friend std::ostream &operator<<(std::ostream &out, const Vector2 &vector)
    {
        out << "Vector2(" << vector.x << ", " << vector.y << ")";
        return out;
    }

    Scalar x, y;
};

inline bool closeTo(const Vector2 &a, const Vector2 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y);
}

inline Vector2 min(Vector2 a, Vector2 b)
{
    return Vector2(min(a.x, b.x), min(a.y, b.y));
}

inline Vector2 max(Vector2 a, Vector2 b)
{
    return Vector2(max(a.x, b.x), max(a.y, b.y));
}

}

#endif // MOLESIM_VECTOR2_HPP
