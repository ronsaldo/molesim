#ifndef MOLESIM_RIGID_TRANSFORM_HPP
#define MOLESIM_RIGID_TRANSFORM_HPP

#include "Quaternion.hpp"
#include "Vector3.hpp"
#include "Matrix4x4.hpp"
#include "TRSTransform3D.hpp"

namespace Molesim
{

/**
 *I am a 3D transform that is decomposed in a successive sequence of scale, rotation, and translation.
 */
struct RigidTransform
{
    static RigidTransform Identity()
    {
        return RigidTransform();
    }

    static RigidTransform WithRotation(const Quaternion &rotation)
    {
        RigidTransform result;
        result.rotation = rotation;
        return result;
    }

    static RigidTransform WithTranslation(const Vector3 &translation)
    {
        RigidTransform result;
        result.translation = translation;
        return result;
    }

    static RigidTransform WithRotationAndTranslation(const Quaternion &rotation, const Vector3 &translation)
    {
        RigidTransform result;
        result.rotation = rotation;
        result.translation = translation;
        return result;
    }

    Vector3 transformPosition(const Vector3 &position) const
    {
        return rotation.rotateVector(position) + translation;
    }

    Vector3 transformNormalVector(const Vector3 &normal) const
    {
        return rotation.rotateVector(normal);
    }

    Vector3 inverseTransformPosition(const Vector3 &position) const
    {
        return rotation.conjugated().rotateVector(position - translation);
    }

    Vector3 inverseTransformNormalVector(const Vector3 &normal) const
    {
        return rotation.conjugated().rotateVector(normal);
    }

    Matrix3x3 asMatrix3x3() const
    {
        return rotation.asMatrix();
    }

    Matrix4x4 asMatrix() const
    {
        return Matrix4x4::WithMatrix3x3AndTranslation(asMatrix3x3(), translation);
    }

    Matrix3x3 asInverseMatrix3x3() const
    {
        return rotation.conjugated().asMatrix();
    }

    Vector3 inverseTranslation() const
    {
        return rotation.inverseRotateVector(-translation);
    }

    Matrix4x4 asInverseMatrix() const
    {
        return Matrix4x4::WithMatrix3x3AndTranslation(asInverseMatrix3x3(), inverseTranslation());
    }

    RigidTransform transformTransform(const RigidTransform &o) const
    {
        RigidTransform result;
        result.rotation = rotation * o.rotation;
        result.translation = transformPosition(o.translation);
        return result;
    }

    RigidTransform inverseTransformTransform(const RigidTransform &o) const
    {
        RigidTransform result;
        result.rotation = rotation.conjugated() * o.rotation;
        result.translation = inverseTransformPosition(o.translation);
        return result;
    }

    TRSTransform3D asTRSTransform3D() const
    {
        TRSTransform3D transform;
        transform.rotation = rotation;
        transform.translation = translation;
        return transform;
    }

    bool operator==(const RigidTransform &o) const
    {
        return rotation == o.rotation && translation == o.translation;
    }

    RigidTransform interpolateTo(const RigidTransform &targetTransform, Scalar lambda) const
    {
        RigidTransform result;
        result.rotation = mix(rotation, targetTransform.rotation, lambda).normalized();
        result.translation = mix(translation, targetTransform.translation, lambda);
        return result;
    }

    Quaternion rotation = Quaternion::Identity();
    Vector3 translation = Vector3(0, 0, 0);
};

} // namespace Math

#endif // MOLESIM_RIGID_TRANSFORM_HPP