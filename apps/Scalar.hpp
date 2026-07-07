#ifndef MOLESIM_SCALAR_HPP
#define MOLESIM_SCALAR_HPP

#define _USE_MATH_DEFINES 
#include <math.h>

namespace Molesim
{

typedef float Scalar;

const Scalar CloseToEpsilon = Scalar(1.0e-6);
const Scalar ScalarPositiveInfinity = INFINITY;
const Scalar ScalarNegativeInfinity = -INFINITY;


inline Scalar abs(Scalar x)
{
    return x <= 0 ? -x : x;
}

template<typename T>
T mix(const T&a, const T&b, Scalar alpha)
{
    return a*(1 - alpha) + b*alpha;
}

template<typename T>
T min(const T &a, const T &b)
{
    return a <= b ? a : b;
}

template<typename T>
T max(const T &a, const T &b)
{
    return a >= b ? a : b;
}

template<typename T>
T clamp(const T &x, const T &min, const T &max)
{
    return Molesim::max(Molesim::min(x, max), min);
}

inline bool closeTo(Scalar a, Scalar b)
{
    float delta = a - b;
    return -CloseToEpsilon <= delta && delta <= CloseToEpsilon;
}

inline Scalar sign(Scalar v)
{
    if (v < 0)
        return -1;
    else if (v > 0)
        return 1;
    return 0;
}

inline Scalar sqrt(Scalar angle)
{
    return Scalar(::sqrt(angle));
}

inline Scalar cos(Scalar angle)
{
    return Scalar(::cos(angle));
}

inline Scalar sin(Scalar angle)
{
    return Scalar(::sin(angle));
}

inline Scalar min(Scalar a, Scalar b)
{
    return a < b ? a : b;
}

inline Scalar max(Scalar a, Scalar b)
{
    return a < b ? b : a;
}

inline Scalar floorFractionPart(Scalar x)
{
    return Scalar(x - floor(x));
}

}

#endif // MOLESIM_SCALAR_HPP
