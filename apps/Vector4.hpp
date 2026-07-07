#ifndef MOLESIM_VECTOR4_HPP
#define MOLESIM_VECTOR4_HPP

#include "Scalar.hpp"
#include "Vector3.hpp"
#include <stdlib.h>

namespace Molesim
{
/**
 * 4D vector.
 */
class Vector4
{
public:
    Vector4(Scalar s = 0) : x(s), y(s), z(s), w(s)
    {
    }

    Vector4(Scalar cx, Scalar cy, Scalar cz, Scalar cw) : x(cx), y(cy), z(cz), w(cw)
    {
    }

    Vector4(const Vector3 &v, Scalar cw) : x(v.x), y(v.y), z(v.z), w(cw)
    {
    }

    Vector4 abs() const
    {
        return Vector4(Molesim::abs(x), Molesim::abs(y), Molesim::abs(z), Molesim::abs(w));
    }

    Scalar dot(const Vector4 &o) const
    {
        return x*o.x + y*o.y + z*o.z + w*o.w;
    }

    Scalar length2() const
    {
        return dot(*this);
    }

    Scalar length() const
    {
        return sqrt(length2());
    }

    Vector4 normalized() const
    {
        auto l = length();
        if(l <= 0)
            return Vector4(0);
        return Vector4(x / l, y / l, z / l, w/l);
    }

    Vector3 minorAt(int index) const
    {
        switch(index)
        {
        case 0: return Vector3(y, z, w);
        case 1: return Vector3(x, z, w);
        case 2: return Vector3(x, y, w);
        case 3: return Vector3(x, y, z);
        default: abort();
        }
    }


    Vector4 operator+() const
    {
        return *this;
    }

    Vector4 operator-() const
    {
        return Vector4(-x, -y, -z, -w);
    }

    Vector4 operator+(const Vector4 &o) const
    {
        return Vector4(x + o.x, y + o.y, z + o.z, w + o.w);
    }

    Vector4 operator-(const Vector4 &o) const
    {
        return Vector4(x - o.x, y - o.y, z - o.z, w - o.w);
    }

    Vector4 operator*(const Vector4 &o) const
    {
        return Vector4(x * o.x, y * o.y, z * o.z, w * o.w);
    }

    Vector4 operator/(const Vector4 &o) const
    {
        return Vector4(x / o.x, y / o.y, z / o.z, w / o.w);
    }

    Vector3 xyz()
    {
        return Vector3(x, y, z);
    }

    Scalar x, y, z, w;
};

}

#endif // MOLESIM_VECTOR4_HPP
