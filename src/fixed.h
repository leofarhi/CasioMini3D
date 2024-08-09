#ifndef FIXED_H
#define FIXED_H
#include <math.h>
#include <stdlib.h>

/* The fixed point type (to make the code easier to read). */
typedef int32_t fixed_t;

/* The precision of the fixed point numbers. */
#define PRECISION 16

/* Convert an integer to a fixed point number. */
#define INT_TO_FIXED(num) ((num)<<PRECISION)
/* Convert a float to a fixed point number. */
#define FLOAT_TO_FIXED(num) (fixed_t)((num)*(1<<PRECISION))
/* Convert a fixed point number to an integer. */
#define TO_INT(num) ((num)>>PRECISION)
/* Convert a fixed point number to a float. */
#define TO_FLOAT(num) ((float)(num)/(1<<PRECISION))
/* Multiply two fixed point numbers together. */
#define MUL(a, b) ((a*b)>>PRECISION)

#define DIV(a, b) ((a<<PRECISION)/b)

static inline fixed_t fmul(fixed_t left, fixed_t right)
{
    /* Generally optimized by the compiler to use dmuls.l and xtrct */
    int64_t p = (int64_t)left * (int64_t)right;
    return (int32_t)(p >> PRECISION);
}

static inline fixed_t fdiv(fixed_t left, fixed_t right)
{
    /* Pretty slow */
    int64_t d = (int64_t)left << PRECISION;
    return d / right;
}

#define fix(x) ((int)((x) * 65536))

static inline fixed_t fixdouble(double constant)
{
    return (fixed_t)(constant * 65536);
}

static inline fixed_t fixfloat(float constant)
{
    return (fixed_t)(constant * 65536);
}

static inline fixed_t fdec(fixed_t f)
{
    return f & 0xffff;
}

static inline int ffloor(fixed_t f)
{
    return f >> PRECISION;
}

static inline int fceil(fixed_t f)
{
    return (f + 0xffff) >> PRECISION;
}

static inline int fround(fixed_t f)
{
    return (f + 0x8000) >> PRECISION;
}

static inline float f2float(fixed_t f)
{
    return (float)f / 65536;
}

static inline double f2double(fixed_t f)
{
    return (double)f / 65536;
}

static inline double f2int(fixed_t f)
{
    return (int)f / 65536;
}

static inline fixed_t feasein(fixed_t x)
{
    return fmul(x, x);
}

static inline fixed_t fease(fixed_t x)
{
    if(x <= fix(0.5)) {
        return 2 * fmul(x, x);
    }
    else {
        x = fix(1) - x;
        return fix(1) - 2 * fmul(x, x);
    }
}

static inline void swap(fixed_t *x, fixed_t *y)
{
    fixed_t temp = *x;
    *x = *y;
    *y = temp;
}

#endif
