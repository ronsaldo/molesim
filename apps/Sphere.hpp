#ifndef MOLESIM_SPHERE_HPP
#define MOLESIM_SPHERE_HPP

#include "Ray3D.hpp"

namespace Molesim
{
/**
 * I am a sphere
 */
class Sphere
{
public:
    Sphere()
        : center(Vector3(0)), radius(0)
    {
    }

    Sphere(const Vector3 &ccenter, Scalar cradius)
        : center(ccenter), radius(cradius)
    {
    }

    static Sphere WithRadius(Scalar radius)
    {
        return Sphere(Vector3(0), radius);
    }

    Vector3 computeNormalForPoint(const Vector3 &point) const
    {
        return (point - center).normalized();
    }

    Scalar computeDistanceWithSphere(const Sphere &o) const
    {
        return max((center - o.center).length() - (radius + o.radius), 0);
    }

    RayCastingResult intersectionsWithRay(const Ray3D &ray) const
    {
        // Ray sphere intersection formula from: https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
        auto a = ray.direction.dot(ray.direction);
        auto b = 2*(ray.direction.dot(ray.origin - center));
        auto c = (ray.origin - center).length2() - (radius*radius);

        auto delta = b*b - 4*a*c;

        RayCastingResult result;
        result.ray = ray;
        auto hasIntersection = delta >= 0;
        if(hasIntersection)
        {
            auto deltaSqrt = sqrt(delta);
            auto t1 = (-b - deltaSqrt) / (2*a);
            auto t2 = (-b + deltaSqrt) / (2*a);

            auto isT1Valid = (ray.tmin <= t1 && t1 <= ray.tmax);
            auto isT2Valid = (ray.tmin <= t2 && t2 <= ray.tmax);

            if(isT1Valid && isT2Valid)
            {
                result.intersectionPoints.reserve(2);
                result.intersectionPoints.push_back(min(t1, t2));
                result.intersectionPoints.push_back(max(t1, t2));
            }
            else if(isT1Valid)
            {
                result.intersectionPoints.reserve(1);
                result.intersectionPoints.push_back(t1);
            }
            else if(isT2Valid)
            {
                result.intersectionPoints.reserve(1);
                result.intersectionPoints.push_back(t2);
            }

        }

        return result;
    }

    Vector3 support(const Vector3 &D) const
    {
        return center + D.normalized()*radius;
    }

    Vector3 center;
    Scalar radius;
};

}

#endif //MOLESIM_SPHERE_HPP
