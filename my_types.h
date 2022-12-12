#ifndef MY_TYPES_H
#define MY_TYPES_H

#include "ac_int.h"

struct Point {
	ac_int<16, false> x;
	ac_int<16, false> y;
};

#endif // MY_TYPES_H
