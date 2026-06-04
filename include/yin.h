#ifndef YIN_H
#define YIN_H
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>   // malloc, free, exit
#include <limits.h>   // INT64_MAX
#include "swhms.h"


int64_t ACF(const int16_t* buffer, uint16_t window, uint16_t time, uint16_t lag);
int64_t DF(const int16_t* buffer, uint16_t window, uint16_t time, uint16_t lag);
int64_t CMNDF(const int16_t* buffer, uint16_t window, uint16_t time, uint16_t lag);
double parabolic_minimum(int64_t y_prev, int64_t y0, int64_t y_next);

// Fills cmndf_q31[tau] for tau = 0..UPPER_TAU (Q31, 1.0 = 0x7fffffff)
// Only cmndf_q31[LOWER_TAU..UPPER_TAU] is typically used.
static inline int64_t DF_direct(const int16_t* restrict buffer,
                                uint16_t window, uint16_t time, uint16_t tau);
void CMNDF_frame_q31(const int16_t* buffer, uint16_t window, uint16_t time,
                     int64_t* cmndf_q31);
uint16_t detect_pitch2(const int16_t* buffer,
                      uint16_t window, uint16_t time,
                      double thresh);
static inline int16_t parabolic_delta_q15(int64_t y_prev, int64_t y0, int64_t y_next);

#define CMNDF_BUFFER_SIZE (UPPER_TAU - LOWER_TAU)

#endif