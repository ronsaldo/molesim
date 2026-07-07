#ifndef MOLESIM_MATRIX3x3_HPP
#define MOLESIM_MATRIX3x3_HPP

#include "Vector2.hpp"
#include "Vector3.hpp"

namespace Molesim
{
/**
 * 3x3 Matrix
 */
class Matrix3x3
{
public:
    
    Matrix3x3(Scalar s = 1)
        : m11(s), m12(0), m13(0),
          m21(0), m22(s), m23(0),
          m31(0), m32(0), m33(s)
    {
    }

    Matrix3x3(
        Scalar cm11, Scalar cm12, Scalar cm13,
        Scalar cm21, Scalar cm22, Scalar cm23,
        Scalar cm31, Scalar cm32, Scalar cm33)
        : m11(cm11), m12(cm12), m13(cm13),
          m21(cm21), m22(cm22), m23(cm23),
          m31(cm31), m32(cm32), m33(cm33)
    {
    }

    Matrix3x3(Vector3 scale)
        : m11(scale.x), m12(0), m13(0),
          m21(0), m22(scale.y), m23(0),
          m31(0), m32(0), m33(scale.z)
    {
    }

    static Matrix3x3 WithColumns(const Vector3 &first, const Vector3 &second, const Vector3 &third)
    {
        return Matrix3x3(
            first.x, second.x, third.x,
            first.y, second.y, third.y,
            first.z, second.z, third.z
        );
    }

    static Matrix3x3 WithRows(const Vector3 &first, const Vector3 &second, const Vector3 &third)
    {
        return Matrix3x3(
            first.x, first.y, first.z,
            second.x, second.y, second.z,
            third.x, third.y, third.z
        );
    }

    static Matrix3x3 XRotation(Scalar angle)
    {
        auto c = cos(angle);
        auto s = sin(angle);
        return Matrix3x3(
            1, 0, 0,
            0, c, s,
            0, -s, c
        ).transpose();
    }

    static Matrix3x3 YRotation(Scalar angle)
    {
        auto c = cos(angle);
        auto s = sin(angle);
        return Matrix3x3(
            c, 0, -s,
            0, 1, 0,
            s, 0, c
        ).transpose();
    }

    static Matrix3x3 ZRotation(Scalar angle)
    {
        auto c = cos(angle);
        auto s = sin(angle);
        return Matrix3x3(
            c, -s, 0,
            s, c, 0,
            0, 0, 1
        );
    }

    static Matrix3x3 Identity()
    {
        return Matrix3x3(
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        );
    }

    static Matrix3x3 Ones()
    {
        return Matrix3x3(
            1, 1, 1,
            1, 1, 1,
            1, 1, 1
        );
    }

    static Matrix3x3 Zeros()
    {
        return Matrix3x3(
            0, 0, 0,
            0, 0, 0,
            0, 0, 0
        );
    }

    static Matrix3x3 ColumnMajorIndices()
    {
        return Matrix3x3(
            0, 3, 6,
            1, 4, 7,
            2, 5, 8
        );
    }

    static Matrix3x3 RowMajorIndices()
    {
        return Matrix3x3(
            0, 1, 2,
            3, 4, 5,
            6, 7, 8
        );
    }

    static Matrix3x3 TextureScaleAndOffset(const Vector2 &scale, const Vector2 &offset)
    {
        return Matrix3x3(
            scale.x, 0,       offset.x,
            0,       scale.y, offset.y,
            0,       0,       1
        );
    }

    static Matrix3x3 SkewSymmetric(const Vector3 &v)
    {
        return Matrix3x3(
            0, -v.z, v.y,
            v.z, 0, -v.x,
            -v.y, v.x, 0
        );
    }

    Scalar determinant() const
    {
	    // | m11 m12 m13 | m11 m12
	    // | m21 m22 m23 | m21 m22
	    // | m31 m32 m33 | m31 m32
        return
            (m11 * m22 * m33) + (m12 * m23 * m31) + (m13 * m21 *m32)
		  - (m31 * m22 * m13) - (m32 * m23 * m11) - (m33 * m21 * m12);
    }

    Matrix3x3 inverse() const
    {
        Scalar det = determinant();
        Vector3 detVector = Vector3(det);

        auto firstColumn = this->firstColumn();
        auto secondColumn = this->secondColumn();
        auto thirdColumn = this->thirdColumn();

        return Matrix3x3::WithRows(
            secondColumn.cross(thirdColumn) / detVector,
            thirdColumn.cross(firstColumn) / detVector,
            firstColumn.cross(secondColumn) / detVector
        );
    }

    Matrix3x3 transpose() const
    {
        return Matrix3x3(
            m11, m21, m31,
            m12, m22, m32,
            m13, m23, m33
        );
    }

    bool operator==(const Matrix3x3 &o) const
    {
        return
            m11 == o.m11 && m12 == o.m12 && m13 == o.m13 &&
            m21 == o.m21 && m22 == o.m22 && m23 == o.m23 &&
            m31 == o.m31 && m32 == o.m32 && m33 == o.m33;
    }

    bool operator!=(const Matrix3x3 &o) const
    {
        return !(*this == o);
    }

    Matrix3x3 operator-() const
    {
        return Matrix3x3(
            -m11, -m12, -m13,
            -m21, -m22, -m23,
            -m31, -m32, -m33
        );
    }

    Matrix3x3 operator+(const Matrix3x3 &o) const
    {
        return Matrix3x3(
            m11 + o.m11, m12 + o.m12, m13 + o.m13,
            m21 + o.m21, m22 + o.m22, m23 + o.m23,
            m31 + o.m31, m32 + o.m32, m33 + o.m33
        );
    }

    Matrix3x3 operator-(const Matrix3x3 &o) const
    {
        return Matrix3x3(
            m11 - o.m11, m12 - o.m12, m13 - o.m13,
            m21 - o.m21, m22 - o.m22, m23 - o.m23,
            m31 - o.m31, m32 - o.m32, m33 - o.m33
        );
    }

    Vector3 firstColumn() const
    {
        return Vector3(m11, m21, m31);
    }

    Vector3 secondColumn() const
    {
        return Vector3(m12, m22, m32);
    }

    Vector3 thirdColumn() const
    {
        return Vector3(m13, m23, m33);
    }

    Vector3 firstRow() const
    {
        return Vector3(m11, m12, m13);
    }

    Vector3 secondRow() const
    {
        return Vector3(m21, m22, m23);
    }

    Vector3 thirdRow() const
    {
        return Vector3(m31, m32, m33);
    }

    friend Matrix3x3 operator*(const Matrix3x3 &a, const Matrix3x3 &b)
    {
        return Matrix3x3(
            a.m11*b.m11 + a.m12*b.m21 + a.m13*b.m31,
            a.m11*b.m12 + a.m12*b.m22 + a.m13*b.m32,
            a.m11*b.m13 + a.m12*b.m23 + a.m13*b.m33,

            a.m21*b.m11 + a.m22*b.m21 + a.m23*b.m31,
            a.m21*b.m12 + a.m22*b.m22 + a.m23*b.m32,
            a.m21*b.m13 + a.m22*b.m23 + a.m23*b.m33,

            a.m31*b.m11 + a.m32*b.m21 + a.m33*b.m31,
            a.m31*b.m12 + a.m32*b.m22 + a.m33*b.m32,
            a.m31*b.m13 + a.m32*b.m23 + a.m33*b.m33
        );
    }

    friend Vector2 operator*(const Matrix3x3 &m, const Vector2 &v)
    {
        return Vector2(
            m.m11*v.x + m.m12*v.y + m.m13,
            m.m21*v.x + m.m22*v.y + m.m23
        );
    }

    friend Vector3 operator*(const Matrix3x3 &m, const Vector3 &v)
    {
        return m.firstColumn()*v.x + m.secondColumn()*v.y + m.thirdColumn()*v.z;
    }

    friend Vector3 operator*(const Vector3 &v, const Matrix3x3 &m)
    {
        return Vector3(
            m.firstColumn().dot(v),
            m.secondColumn().dot(v),
            m.thirdColumn().dot(v)
        );
    }

    friend std::ostream &operator<<(std::ostream &out, const Matrix3x3 &m)
    {
        out << "Matrix3x3(\n"
            << m.m11 << ", " << m.m12 << "," << m.m13 << "\n"
            << m.m21 << ", " << m.m22 << "," << m.m23 << "\n"
            << m.m31 << ", " << m.m32 << "," << m.m33 << "\n"
        ")";
        return out;
    }

    static const Matrix3x3 CubeMapFaceRotations[6];

    Scalar m11, m12, m13;
    Scalar m21, m22, m23;
    Scalar m31, m32, m33;
};

inline bool closeTo(const Matrix3x3 &a, const Matrix3x3 &b)
{
    return
        closeTo(a.m11, b.m11) && closeTo(a.m12, b.m12) && closeTo(a.m13, b.m13) &&
        closeTo(a.m21, b.m21) && closeTo(a.m22, b.m22) && closeTo(a.m23, b.m23) &&
        closeTo(a.m31, b.m31) && closeTo(a.m32, b.m32) && closeTo(a.m33, b.m33);
}

}

#endif // MOLESIM_MATRIX3x3_HPP
