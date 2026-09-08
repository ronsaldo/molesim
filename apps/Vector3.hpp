#ifndef MOLESIM_VECTOR3_HPP
#define MOLESIM_VECTOR3_HPP

#include "Scalar.hpp"
#include <stdint.h>
#include <ostream>

// Benchmark is slower.
#if defined(__x86_64__) || defined(__i386__)
#include <xmmintrin.h>
//#define MOLEVIS_USE_SSE
#endif

namespace Molesim
{
/**
 * 3D vector.
 */
class Vector3
{
public:
    Vector3(Scalar s = 0) : x(s), y(s), z(s) {}

    Vector3(Scalar cx, Scalar cy, Scalar cz) : x(cx), y(cy), z(cz) {}

#ifdef MOLEVIS_USE_SSE
    Vector3(__m128 cvector) : sseVector(cvector) {}
#endif

    static Vector3 PositiveInfinity()
    {
        return Vector3(ScalarPositiveInfinity);
    }

    static Vector3 NegativeInfinity()
    {
        return Vector3(ScalarNegativeInfinity);
    }

    static Vector3 Zeros()
    {
        return Vector3(0, 0, 0);
    }

    static Vector3 Ones()
    {
        return Vector3(1, 1, 1);
    }

    Scalar sum() const
    {
        return x + y + z;
    }

    Vector3 abs() const
    {
#ifdef MOLEVIS_USE_SSE
        return Vector3(_mm_max_ps(sseVector, -(*this).sseVector));
#else
        return Vector3(Molesim::abs(x), Molesim::abs(y), Molesim::abs(z));
#endif
    }

    Scalar dot(const Vector3 &o) const
    {
        return ((*this)*o).sum();
    }

    Vector3 cross(const Vector3 &o) const
    {
        return Vector3(
            y*o.z - z*o.y,
            z*o.x - x*o.z,
            x*o.y - y*o.x 
        );
    }

    Scalar length2() const
    {
        return dot(*this);
    }

    Scalar length() const
    {
        return sqrt(length2());
    }

    Vector3 normalized() const
    {
        auto l = length();
        if(l <= 0)
            return Vector3(0);
        return Vector3(x / l, y / l, z / l);
    }

    Vector3 reciprocal() const
    {
        return Vector3(1/x, 1/y, 1/z);
    }

    bool operator==(const Vector3 &o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const Vector3 &o) const
    {
        return !(*this == o);
    }

    Vector3 operator+() const
    {
        return *this;
    }

#ifdef MOLEVIS_USE_SSE
    Vector3 operator-() const
    {
        return Vector3(_mm_sub_ps(_mm_set1_ps(0.0f), sseVector));
    }

    Vector3 operator+(const Vector3 &o) const
    {
        return Vector3(_mm_add_ps(sseVector, o.sseVector));
    }

    Vector3 operator-(const Vector3 &o) const
    {
        return Vector3(_mm_sub_ps(sseVector, o.sseVector));
    }

    Vector3 operator*(const Vector3 &o) const
    {
        return Vector3(_mm_mul_ps(sseVector, o.sseVector));
    }

    Vector3 operator/(const Vector3 &o) const
    {
        return Vector3(_mm_div_ps(sseVector, o.sseVector));
    }

#else 
    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    }

    Vector3 operator+(const Vector3 &o) const
    {
        return Vector3(x + o.x, y + o.y, z + o.z);
    }

    Vector3 operator-(const Vector3 &o) const
    {
        return Vector3(x - o.x, y - o.y, z - o.z);
    }

    Vector3 operator*(const Vector3 &o) const
    {
        return Vector3(x * o.x, y * o.y, z * o.z);
    }

    Vector3 operator/(const Vector3 &o) const
    {
        return Vector3(x / o.x, y / o.y, z / o.z);
    }
#endif

    Vector3 operator+=(const Vector3 &o)
    {
        return *this = (*this + o);
    }

    Vector3 operator-=(const Vector3 &o)
    {
        return *this = (*this - o);
    }

    Vector3 operator*=(const Vector3 &o)
    {
        return *this = (*this * o);
    }

    Vector3 operator/=(const Vector3 &o)
    {
        return *this = (*this / o);
    }

    friend std::ostream &operator<<(std::ostream &out, const Vector3 &vector)
    {
        out << "Vector3(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
        return out;
    }

    uint32_t computeNormalAxis();

    static Vector3 TangentForAxis(uint32_t axis)
    {
        return NormalAxis[axis + 2];
    }

    static Vector3 BitangentForAxis(uint32_t axis)
    {
        return NormalAxis[axis + 4];
    }

    static const Vector3 NormalAxis[12];

    union
    {
        struct
        {
            Scalar x, y, z, _padding;
        };
        
        Scalar elements[4];

#ifdef MOLEVIS_USE_SSE
        __m128 sseVector;
#endif 
    };
    
};

inline bool closeTo(const Vector3 &a, const Vector3 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y) && closeTo(a.z, b.z);
}

#ifdef MOLEVIS_USE_SSE
inline Vector3 min(Vector3 a, Vector3 b)
{
    return Vector3(_mm_min_ps(a.sseVector, b.sseVector));
}

inline Vector3 max(Vector3 a, Vector3 b)
{
    return Vector3(_mm_max_ps(a.sseVector, b.sseVector));
}

#else
inline Vector3 min(Vector3 a, Vector3 b)
{
    return Vector3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

inline Vector3 max(Vector3 a, Vector3 b)
{
    return Vector3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
#endif

class CompactVector3
{
public:
    CompactVector3() {}
    CompactVector3(const Vector3 &v)
        : x(v.x), y(v.y), z(v.z)
    {}
    CompactVector3(Scalar cx, Scalar cy, Scalar cz)
        : x(cx), y(cy), z(cz)
    {}

    Vector3 asVector3() const
    {
        return Vector3(x, y, z);
    }
    
    Scalar x, y, z;
};

}    

#endif // MOLESIM_VECTOR3_HPP
