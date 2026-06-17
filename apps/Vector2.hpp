#ifndef MOLLEVIS_VECTOR2_HPP
#define MOLLEVIS_VECTOR2_HPP

struct alignas(8) Vector2
{
    float x, y;

    Vector2()
        : x(0), y(0) {}

    Vector2(float s)
        : x(s), y(s) {}

    Vector2(float cx, float cy)
        : x(cx), y(cy) {}

    float dot(const Vector2 &o) const
    {
        return x*o.x + y*o.y;
    }

    Vector2 operator+(const Vector2 &o) const
    {
        return Vector2{x + o.x, y + o.y};
    }

    Vector2 operator-(const Vector2 &o) const
    {
        return Vector2{x - o.x, y - o.y};
    }

    Vector2 operator*(const Vector2 &o) const
    {
        return Vector2{x * o.x, y * o.y};
    }

    Vector2 operator/(const Vector2 &o) const
    {
        return Vector2{x / o.x, y / o.y};
    }

    Vector2 operator+=(const Vector2 &o)
    {
        return *this = *this + o;
    }

};

#endif //MOLLEVIS_VECTOR2_HPP