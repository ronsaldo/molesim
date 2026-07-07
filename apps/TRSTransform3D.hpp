#ifndef MOLESIM_TRS_TRANSFORM_3D_HPP
#define MOLESIM_TRS_TRANSFORM_3D_HPP

#include "Quaternion.hpp"
#include "Vector3.hpp"
#include "Matrix4x4.hpp"

namespace Molesim
{

/**
 *I am a 3D transform that is decomposed in a successive sequence of scale, rotation, and translation.
 */
struct TRSTransform3D
{

    static TRSTransform3D Identity()
    {
        return TRSTransform3D();
    }

    Vector3 transformPosition(const Vector3 &position)
    {
        return rotation.rotateVector(position*scale) + translation;
    }

    Matrix3x3 asMatrix3x3() const
    {
        return rotation.asMatrix()*Matrix3x3(scale);
    }

    Matrix4x4 asMatrix() const
    {
        return Matrix4x4::WithMatrix3x3AndTranslation(asMatrix3x3(), translation);
    }

    Matrix3x3 asInverseMatrix3x3() const
    {
        return Matrix3x3(scale.reciprocal())*rotation.conjugated().asMatrix();
    }

    Vector3 inverseTranslation() const
    {
        return rotation.inverseRotateVector(-translation) / scale;
    }

    Matrix4x4 asInverseMatrix() const
    {
        return Matrix4x4::WithMatrix3x3AndTranslation(asInverseMatrix3x3(), inverseTranslation());
    }

    Vector3 scale = Vector3(1, 1, 1);
    Quaternion rotation = Quaternion::Identity();
    Vector3 translation = Vector3(0, 0, 0);
};

} // namespace Woden

#endif // MOLESIM_TRS_TRANSFORM_3D_HPP