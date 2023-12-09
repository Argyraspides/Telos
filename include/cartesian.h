#pragma once

struct Point
{
    float x, y, z;
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Point operator+(const Point &point)
    {
        return {
            x + point.x,
            y + point.y,
            z + point.z};
    }

    Point operator-(const Point &point)
    {
        return {
            x - point.x,
            y - point.y,
            z - point.z};
    }

    Point operator=(const Point &point)
    {
        return {
            x = point.x,
            y = point.y,
            z = point.z};
    }

    Point operator/(const float &num)
    {
        return {
            x / num,
            y / num,
            z / num};
    }

    Point operator*(const float &num)
    {
        return {
            x * num,
            y * num,
            z * num};
    }
};