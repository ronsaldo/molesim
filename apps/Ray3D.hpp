#ifndef MOLESIM_RAY3D_HPP
#define MOLESIM_RAY3D_HPP

#include "Vector3.hpp"
#include <vector>

namespace Molesim
{
/**
 * I am a 3D ray
 */
class Ray3D
{
public:
    Ray3D()
        : tmin(0), tmax(1)
    {
    }

    Ray3D(const Vector3 &corigin, const Vector3 &cdirection, Scalar ctmin, Scalar ctmax)
        : origin(corigin), direction(cdirection), inverseDirection(cdirection.reciprocal()), tmin(ctmin), tmax(ctmax)
    {
    }

    static Ray3D FromTo(const Vector3 &startingPoint, const Vector3 &destinationPoint)
    {
        auto vector = destinationPoint - startingPoint;
        auto vectorLength = vector.length();
        auto direction = vector;
        if(vectorLength > 0)
            direction = direction / vectorLength;
        return Ray3D(startingPoint, direction, 0, vectorLength);
    }

    Vector3 pointAtDistance(Scalar distance) const
    {
        return origin + (direction*distance);
    }

    Vector3 getStartPoint() const
    {
        return origin + direction*tmin;
    }

    Vector3 getEndPoint() const
    {
        return origin + direction*tmax;
    }

    Vector3 origin, direction;
    Vector3 inverseDirection;
    Scalar tmin, tmax;
};

struct RayCastingResult
{
    Ray3D ray;
    std::vector<Scalar> intersectionPoints;

    bool isEmpty() const
    {
        return intersectionPoints.empty();
    }
};

} // End of namespace Molesim

#endif //WODEN_MATH_RAY3D_HPP