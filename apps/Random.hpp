#ifndef MOLESIM_RANDOM_HPP
#define MOLESIM_RANDOM_HPP

#include <random>
#include "Vector3.hpp"
#include "Vector4.hpp"

namespace Molesim
{
struct Random
{
    Random(int seed = 45)
    {
        rand.seed(seed);
    }

    float randFloat(float min, float max)
    {
        return float(std::uniform_real_distribution<> (min, max)(rand));
    }

    double randDouble(float min, float max)
    {
        return std::uniform_real_distribution<> (min, max)(rand);
    }

    Vector3 randVector3(const Vector3 &min, const Vector3 &max)
    {
        return Vector3{randFloat(min.x, max.x), randFloat(min.y, max.y), randFloat(min.z, max.z)};
    }

    Vector4 randVector4(const Vector4 &min, const Vector4 &max)
    {
        return Vector4{randFloat(min.x, max.x), randFloat(min.y, max.y), randFloat(min.z, max.z), randFloat(min.w, max.w)};
    }

    uint32_t randUInt(uint32_t max)
    {
        return rand() % max;
    }

    std::mt19937 rand;
};

} // End of namespace Molesim

#endif //MOLESIM_RANDOM_HPP
