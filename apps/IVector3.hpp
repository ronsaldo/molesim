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

    int x, y, z;
};

} // End of namespace Molesim

#endif //MOLESIM_IVECTOR3_HPP