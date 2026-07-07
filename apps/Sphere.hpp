#ifndef MOLEVIS_SPHERE_HPP
#define MOLEVIS_SPHERE_HPP

#include "Vector3.hpp"

struct Sphere
{
    Vector3 center;
    float radius;

    Sphere() = default;
    Sphere(const Vector3 &initCenter, float initRadius)
        : center(initCenter), radius(initRadius) {}

    float sdf(const Vector3 &p)
    {
        return (p - center).length() - radius;
    }
};


#endif // MOLEVIS_SPHERE_HPP
