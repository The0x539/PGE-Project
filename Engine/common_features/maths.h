#ifndef MATHS_H
#define MATHS_H
#include <type_traits>
#include <cmath>
#include <cfloat>

namespace Maths
{
    long    roundTo(long src, long grid);
    double  roundTo(double src, double grid);

    inline int iRound(double d)
    {
        return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
    }
    inline unsigned int uRound(double d)
    {
        return d >= 0.0 ?
               static_cast<unsigned int>(d + 0.5) :
               static_cast<unsigned int>(std::abs(d) + 0.5);
    }
    inline long lRound(double d)
    {
        return d >= 0.0 ? long(d + 0.5) : long(d - double(long(d - 1)) + 0.5) + long(d - 1);
    }

    inline int iRound(float d)
    {
        return d >= 0.0f ? int(d + 0.5f) : int(d - float(int(d - 1)) + 0.5f) + int(d - 1);
    }

    inline long lRound(float d)
    {
        return d >= 0.0f ? long(d + 0.5f) : long(d - float(long(d - 1)) + 0.5f) + long(d - 1);
    }

    inline bool equals(long double a, long double b, long double epsilon = LDBL_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }
    inline bool equals(double a, double b, double epsilon = DBL_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }
    inline bool equals(float a, float b, float epsilon = FLT_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }

    template <typename T>
    T max(T n1, T n2)
    {
        static_assert(std::is_arithmetic<T>::value, "The value for \"max\" must be arithemtic");
        return (n1 > n2) ? n1 : n2;
    }

    template <typename T>
    T min(T n1, T n2)
    {
        static_assert(std::is_arithmetic<T>::value, "The value for \"min\" must be arithemtic");
        return (n1 < n2) ? n1 : n2;
    }
    //    template <typename T>
    //    static int sgn(T val) {
    //        static_assert(std::is_arithmetic<T>::value, "The value for \"val\" must be arithemtic");
    //        return int(T(0) < val) - int(val < T(0));
    //    }
    template <typename T>
    T sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
};

#endif // MATHS_H
