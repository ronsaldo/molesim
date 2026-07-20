#ifndef MOLESIM_IVECTOR3_HPP
#define MOLESIM_IVECTOR3_HPP

namespace Molesim
{
struct IVector3
{
    IVector3(int s = 0) : x(s), y(s), z(s)
    {
    }

    IVector3(int cx, int cy, int cz) : x(cx), y(cy), z(cz)
    {
    }

    static IVector3 Zeros()
    {
        return IVector3(0, 0, 0);
    }

    static IVector3 Ones()
    {
        return IVector3(1, 1, 1);
    }

    bool operator==(const IVector3 &o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const IVector3 &o) const
    {
        return !(*this == o);
    }

    IVector3 operator+() const
    {
        return *this;
    }

    IVector3 operator-() const
    {
        return IVector3(-x, -y, -z);
    }

    IVector3 operator+(const IVector3 &o) const
    {
        return IVector3(x + o.x, y + o.y, z + o.z);
    }

    IVector3 operator-(const IVector3 &o) const
    {
        return IVector3(x - o.x, y - o.y, z - o.z);
    }

    IVector3 operator*(const IVector3 &o) const
    {
        return IVector3(x * o.x, y * o.y, z * o.z);
    }

    IVector3 operator/(const IVector3 &o) const
    {
        return IVector3(x / o.x, y / o.y, z / o.z);
    }


    int x, y, z;
};

inline IVector3 min(IVector3 a, IVector3 b)
{
    return IVector3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

inline IVector3 max(IVector3 a, IVector3 b)
{
    return IVector3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}


} // End of namespace Molesim

#endif //MOLESIM_IVECTOR3_HPP