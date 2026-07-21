#ifndef MOLESIM_AABOX_HPP
#define MOLESIM_AABOX_HPP

#include "Ray3D.hpp"

namespace Molesim
{
/**
 * I am an axis aligned bounding box.
 */
class AABox
{
public:
    AABox()
        : minCorner(Vector3::PositiveInfinity()), maxCorner(Vector3::NegativeInfinity())
    {
    }

    AABox(const Vector3 &cminCorner, const Vector3 &cmaxCorner)
        : minCorner(cminCorner), maxCorner(cmaxCorner)
    {
    }

    static AABox Empty()
    {
        return AABox();
    }

    static AABox ForSphere(const Vector3 &center, Scalar radius)
    {
        return AABox(center - radius, center + radius);
    }

    static AABox WithHalfExtent(const Vector3 &halfExtent)
    {
        return AABox(-halfExtent, halfExtent);
    }

    template<typename C>
    static AABox Encompassing(const C &collection)
    {
        auto result = Empty();
        for(auto &point : collection)
            result.insertPoint(point);
        return result;
    }

    template<typename FT>
    void cornersDo(FT &&function)
    {
        function(Vector3(minCorner.x, minCorner.y, minCorner.z));
        function(Vector3(maxCorner.x, minCorner.y, minCorner.z));
        function(Vector3(minCorner.x, maxCorner.y, minCorner.z));
        function(Vector3(maxCorner.x, maxCorner.y, minCorner.z));

        function(Vector3(minCorner.x, minCorner.y, maxCorner.z));
        function(Vector3(maxCorner.x, minCorner.y, maxCorner.z));
        function(Vector3(minCorner.x, maxCorner.y, maxCorner.z));
        function(Vector3(maxCorner.x, maxCorner.y, maxCorner.z));
    }

    template<typename T>
    AABox transformedWith(T&& transform)
    {
        auto result = AABox::Empty();
        cornersDo([&](const Vector3 &corner){
            result.insertPoint(transform.transformPosition(corner));
        });

        return result;
    }

    template<typename T>
    AABox inverseTransformedWith(T&& transform)
    {
        auto result = AABox::Empty();
        cornersDo([&](const Vector3 &corner){
            result.insertPoint(transform.inverseTransformPosition(corner));
        });

        return result;
    }

    AABox expandedBy(Scalar expansion) const
    {
        return AABox(minCorner - expansion, maxCorner + expansion);
    }

    bool isEmpty() const
    {
        return minCorner.x > maxCorner.x;
    }

    void insertBox(const AABox &box)
    {
        minCorner = min(minCorner, box.minCorner);
        maxCorner = max(maxCorner, box.maxCorner);
    }

    void insertPoint(const Vector3 &point)
    {
        minCorner = min(minCorner, point);
        maxCorner = max(maxCorner, point);
    }

    Vector3 halfExtent() const
    {
        return (maxCorner - minCorner)*Vector3(0.5);
    }

    Vector3 center() const
    {
        return minCorner + halfExtent();
    }

    Vector3 extent() const
    {
        return maxCorner - minCorner;
    }

    Vector3 computeNormalForPoint(const Vector3 &point) const
    {
        auto delta = point - center();
        auto deltaAbsolute = delta.abs() / halfExtent();
        if(deltaAbsolute.x >= deltaAbsolute.y)
        {
            if(deltaAbsolute.x >= deltaAbsolute.z)
                return Vector3(sign(delta.x), 0, 0);
            else
                return Vector3(0, 0, sign(delta.z));
        }
        else
        {
            if(deltaAbsolute.y >= deltaAbsolute.z)
                return Vector3(0, sign(delta.y), 0);
            else
                return Vector3(0, 0, sign(delta.z));
        }
    }

    bool isBoxOutside(const AABox &o) const
    {
        return
            o.maxCorner.x < minCorner.x || maxCorner.x < o.minCorner.x ||
            o.maxCorner.y < minCorner.y || maxCorner.y < o.minCorner.y ||
            o.maxCorner.z < minCorner.z || maxCorner.z < o.minCorner.z;
    }

    bool hasIntersectionWithBox(const AABox &o) const
    {
        return !isBoxOutside(o);
    }

    bool containsPoint(const Vector3 &point) const
    {
        return
            minCorner.x <= point.x && point.x <= maxCorner.x &&
            minCorner.y <= point.y && point.y <= maxCorner.y &&
            minCorner.z <= point.z && point.z <= maxCorner.z;
    }

    RayCastingResult intersectionsWithRay(const Ray3D &ray) const
    {
        // Slab testing algorithm from: A Ray-Box Intersection Algorithm andEfficient Dynamic Voxel Rendering. By Majercik et al
        auto t0 = (minCorner - ray.origin)*ray.inverseDirection;
        auto t1 = (maxCorner - ray.origin)*ray.inverseDirection;

        auto tmin = min(t0, t1);
        auto tmax = max(t0, t1);

        auto maxTMin = max(max(max(tmin.x, tmin.y), tmin.z), ray.tmin);
        auto minTMax = min(min(min(tmax.x, tmax.y), tmax.z), ray.tmax);

        RayCastingResult result;
        result.ray = ray;
        auto hasIntersection = maxTMin <= minTMax;
        if(hasIntersection)
        {
            result.intersectionPoints.reserve(2);
            result.intersectionPoints.push_back(min(maxTMin, minTMax));
            result.intersectionPoints.push_back(max(maxTMin, minTMax));
        }

        return result;
    }

    bool hasIntersectionWithRay(const Ray3D &ray) const
    {
        // Slab testing algorithm from: A Ray-Box Intersection Algorithm andEfficient Dynamic Voxel Rendering. By Majercik et al
        auto t0 = (minCorner - ray.origin)*ray.inverseDirection;
        auto t1 = (maxCorner - ray.origin)*ray.inverseDirection;

        auto tmin = min(t0, t1);
        auto tmax = max(t0, t1);

        auto maxTMin = max(max(max(tmin.x, tmin.y), tmin.z), ray.tmin);
        auto minTMax = min(min(min(tmax.x, tmax.y), tmax.z), ray.tmax);

        auto hasIntersection = maxTMin <= minTMax;
        return hasIntersection;
    }

    Vector3 positiveVertex(const Vector3 &D) const
    {
        auto vertex = minCorner;
        if(D.x >= 0) vertex.x = maxCorner.x;
        if(D.y >= 0) vertex.y = maxCorner.y;
        if(D.z >= 0) vertex.z = maxCorner.z;
        return vertex;
    }

    Vector3 support(const Vector3 &D) const
    {
        return positiveVertex(D);
    }

    Vector3 minCorner;
    Vector3 maxCorner;
};

}

#endif //MOLESIM_AABOX_HPP
