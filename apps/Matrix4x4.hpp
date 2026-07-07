#ifndef MOLESIM_MATRIX4X4_HPP
#define MOLESIM_MATRIX4X4_HPP

#include "Scalar.hpp"
#include "Matrix3x3.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

namespace Molesim
{
/**
 * 4x4 Matrix
 */
class Matrix4x4
{
public:
    
    Matrix4x4(Scalar s = 1)
    {
        m11 = s; m12 = 0; m13 = 0; m14 = 0;
        m21 = 0; m22 = s; m23 = 0; m24 = 0;
        m31 = 0; m32 = 0; m33 = s; m34 = 0;
        m41 = 0; m42 = 0; m43 = 0; m44 = s;
    }

    Matrix4x4(
        Scalar cm11, Scalar cm12, Scalar cm13, Scalar cm14,
        Scalar cm21, Scalar cm22, Scalar cm23, Scalar cm24,
        Scalar cm31, Scalar cm32, Scalar cm33, Scalar cm34,
        Scalar cm41, Scalar cm42, Scalar cm43, Scalar cm44
    )
    {
        m11 = cm11; m12 = cm12; m13 = cm13; m14 = cm14;
        m21 = cm21; m22 = cm22; m23 = cm23; m24 = cm24;
        m31 = cm31; m32 = cm32; m33 = cm33; m34 = cm34;
        m41 = cm41; m42 = cm42; m43 = cm43; m44 = cm44;
    }

    static Matrix4x4 Identity()
    {
        return Matrix4x4(1);
    }

    static Matrix4x4 WithMatrix3x3AndTranslation(const Matrix3x3 &m, const Vector3 &t)
    {
        return Matrix4x4(
            m.m11, m.m12, m.m13, t.x,
            m.m21, m.m22, m.m23, t.y,
            m.m31, m.m32, m.m33, t.z,
            0,     0,     0,     1
        );
    }

    static Matrix4x4 WithMatrix3x3(const Matrix3x3 &m)
    {
        return Matrix4x4(
            m.m11, m.m12, m.m13, 0,
            m.m21, m.m22, m.m23, 0,
            m.m31, m.m32, m.m33, 0,
            0,     0,     0,     1
        );
    }

    static Matrix4x4 WithTranslation(const Vector3 &t)
    {
        return Matrix4x4(
            1, 0, 0, t.x,
            0, 1, 0, t.y,
            0, 0, 1, t.z,
            0, 0, 0, 1
        );
    }

    static Matrix4x4 ProjectionInvertYMatrix()
    {
        return Matrix4x4(
            1,  0, 0, 0,
            0, -1, 0, 0,
            0,  0, 1, 0,
            0,  0, 0, 1
        );
    }

    static Matrix4x4 ReverseDepthFrustum(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar near, Scalar far)
    {
        return Matrix4x4(
            2*near /(right - left), 0, (right + left) / (right - left), 0,
            0, 2*near /(top - bottom), (top + bottom) / (top - bottom), 0,
            0, 0, near / (far - near), near*far / (far - near),
            0, 0, -1, 0
        );
    }

    static Matrix4x4 DepthOrtho(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar near, Scalar far)
    {
        return Matrix4x4(
    		2 /(right - left), 0, 0, -((right + left) / (right - left)),
    		0, 2 /(top - bottom), 0, -((top + bottom) / (top - bottom)),
    		0, 0, -1 / (far - near), -near / (far - near),
    		0, 0, 0, 1
        );
    }

    static Matrix4x4 ReverseDepthOrtho(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar near, Scalar far)
    {
        return Matrix4x4(
    		2 /(right - left), 0, 0, -((right + left) / (right - left)),
    		0, 2 /(top - bottom), 0, -((top + bottom) / (top - bottom)),
    		0, 0, 1 / (far - near), far / (far - near),
    		0, 0, 0, 1
        );
    }

    static Matrix4x4 ReverseDepthPerspective(Scalar fovy, Scalar aspect, Scalar near, Scalar far)
    {
        Scalar fovyRad = Scalar((fovy *M_PI / 180.0) * 0.5);
        Scalar top = Scalar(near * tan(fovyRad));
        Scalar right = Scalar(top*aspect);

        return ReverseDepthFrustum(
            -right, right,
            -top, top,
             near, far
        );
    }

    Vector4 firstColumn() const
    {
        return Vector4(m11, m21, m31, m41);
    }

    Vector4 secondColumn() const
    {
        return Vector4(m12, m22, m32, m42);
    }

    Vector4 thirdColumn() const
    {
        return Vector4(m13, m23, m33, m43);
    }

    Vector4 fourthColumn() const
    {
        return Vector4(m14, m24, m34, m44);
    }

    Vector4 firstRow() const
    {
        return Vector4(m11, m12, m13, m14);
    }

    Vector4 secondRow() const
    {
        return Vector4(m21, m22, m23, m24);
    }

    Vector4 thirdRow() const
    {
        return Vector4(m31, m32, m33, m34);
    }

    Vector4 fourthRow() const
    {
        return Vector4(m41, m42, m43, m44);
    }

    Matrix3x3 topLeftMatrix3x3() const
    {
        return Matrix3x3(
            m11, m12, m13,
            m21, m22, m23,
            m31, m32, m33
        );
    }

    Matrix3x3 minorMatrixAt(int row, int column) const
    {

        switch(column)
        {
        case 0: return Matrix3x3::WithColumns(
                secondColumn().minorAt(row),
                thirdColumn().minorAt(row),
                fourthColumn().minorAt(row)
            );
        case 1: return Matrix3x3::WithColumns(
                firstColumn().minorAt(row),
                thirdColumn().minorAt(row),
                fourthColumn().minorAt(row)
            );
        case 2: return Matrix3x3::WithColumns(
                firstColumn().minorAt(row),
                secondColumn().minorAt(row),
                fourthColumn().minorAt(row)
            );
        case 3: return Matrix3x3::WithColumns(
                firstColumn().minorAt(row),
                secondColumn().minorAt(row),
                thirdColumn().minorAt(row)
        );
        default: abort();
        }
    }

    Scalar minorAt(int row, int column) const
    {
        return minorMatrixAt(row, column).determinant();
    }

    Scalar determinant() const
    {
        return
          minorAt(0, 0)*firstColumn() .x
        - minorAt(0, 1)*secondColumn().x
        + minorAt(0, 2)*thirdColumn() .x
        - minorAt(0, 3)*fourthColumn().x;
    }

    Scalar cofactorAt(int row, int column) const
    {
        return minorAt(column, row) * ((row + column) & 1 ? -1 : 1);
    }

    Matrix4x4 adjugate()const
    {
        return Matrix4x4(
            cofactorAt(0, 0),
            cofactorAt(0, 1),
            cofactorAt(0, 2),
            cofactorAt(0, 3),

            cofactorAt(1, 0),
            cofactorAt(1, 1),
            cofactorAt(1, 2),
            cofactorAt(1, 3),

            cofactorAt(2, 0),
            cofactorAt(2, 1),
            cofactorAt(2, 2),
            cofactorAt(2, 3),

            cofactorAt(3, 0),
            cofactorAt(3, 1),
            cofactorAt(3, 2),
            cofactorAt(3, 3)
        );
    }

    Matrix4x4 inverse() const
    {
        auto det = determinant();
        auto adj = adjugate();
        return Matrix4x4(
            adj.m11 / det, adj.m12 / det, adj.m13 / det, adj.m14 / det,
            adj.m21 / det, adj.m22 / det, adj.m23 / det, adj.m24 / det,
            adj.m31 / det, adj.m32 / det, adj.m33 / det, adj.m34 / det,
            adj.m41 / det, adj.m42 / det, adj.m43 / det, adj.m44 / det
        );
    }

    friend Matrix4x4 operator*(const Matrix4x4 &a, const Matrix4x4 &b)
    {
        return Matrix4x4(
            a.m11*b.m11 + a.m12*b.m21 + a.m13*b.m31 + a.m14*b.m41,
            a.m11*b.m12 + a.m12*b.m22 + a.m13*b.m32 + a.m14*b.m42,
            a.m11*b.m13 + a.m12*b.m23 + a.m13*b.m33 + a.m14*b.m43,
            a.m11*b.m14 + a.m12*b.m24 + a.m13*b.m34 + a.m14*b.m44,

            a.m21*b.m11 + a.m22*b.m21 + a.m23*b.m31 + a.m24*b.m41,
            a.m21*b.m12 + a.m22*b.m22 + a.m23*b.m32 + a.m24*b.m42,
            a.m21*b.m13 + a.m22*b.m23 + a.m23*b.m33 + a.m24*b.m43,
            a.m21*b.m14 + a.m22*b.m24 + a.m23*b.m34 + a.m24*b.m44,

            a.m31*b.m11 + a.m32*b.m21 + a.m33*b.m31 + a.m34*b.m41,
            a.m31*b.m12 + a.m32*b.m22 + a.m33*b.m32 + a.m34*b.m42,
            a.m31*b.m13 + a.m32*b.m23 + a.m33*b.m33 + a.m34*b.m43,
            a.m31*b.m14 + a.m32*b.m24 + a.m33*b.m34 + a.m34*b.m44,

            a.m41*b.m11 + a.m42*b.m21 + a.m43*b.m31 + a.m44*b.m41,
            a.m41*b.m12 + a.m42*b.m22 + a.m43*b.m32 + a.m44*b.m42,
            a.m41*b.m13 + a.m42*b.m23 + a.m43*b.m33 + a.m44*b.m43,
            a.m41*b.m14 + a.m42*b.m24 + a.m43*b.m34 + a.m44*b.m44
        );
    }


    Vector3 transformPosition(const Vector3 &p) const
    {
        return ((*this) * Vector4(p, 1)).xyz();
    }

    friend Vector4 operator*(const Matrix4x4 &m, const Vector4 &v)
    {
        return m.firstColumn()*v.x + m.secondColumn()*v.y + m.thirdColumn()*v.z + m.fourthColumn()*v.w;
    }

    friend Vector4 operator*(const Vector4 &v, const Matrix4x4 &m)
    {
        return Vector4(
            m.firstColumn().dot(v),
            m.secondColumn().dot(v),
            m.thirdColumn().dot(v),
            m.fourthColumn().dot(v)
        );
    }

    friend std::ostream &operator<<(std::ostream &out, const Matrix4x4 &m)
    {
        out << "Matrix4x4(\n"
            << m.m11 << ", " << m.m12 << ", " << m.m13 << ", " << m.m14 << "\n"
            << m.m21 << ", " << m.m22 << ", " << m.m23 << ", " << m.m24 << "\n"
            << m.m31 << ", " << m.m32 << ", " << m.m33 << ", " << m.m34 << "\n"
            << m.m41 << ", " << m.m42 << ", " << m.m43 << ", " << m.m44 << "\n"
        ")";
        return out;
    }

    Scalar m11, m21, m31, m41;
    Scalar m12, m22, m32, m42;
    Scalar m13, m23, m33, m43;
    Scalar m14, m24, m34, m44;
};


inline bool closeTo(const Matrix4x4 &a, const Matrix4x4 &b)
{
    return
        closeTo(a.m11, b.m11) && closeTo(a.m12, b.m12) && closeTo(a.m13, b.m13) && closeTo(a.m14, b.m14) &&
        closeTo(a.m21, b.m21) && closeTo(a.m22, b.m22) && closeTo(a.m23, b.m23) && closeTo(a.m24, b.m24) &&
        closeTo(a.m31, b.m31) && closeTo(a.m32, b.m32) && closeTo(a.m33, b.m33) && closeTo(a.m34, b.m34) &&
        closeTo(a.m41, b.m41) && closeTo(a.m42, b.m42) && closeTo(a.m43, b.m43) && closeTo(a.m44, b.m44);
}

}

#endif // MOLESIM_MATRIX4X4_HPP
