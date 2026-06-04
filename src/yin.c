
#include "yin.h"
// #include <stdio.h>
// #include <time.h>

int64_t ACF(const int16_t *buffer, uint16_t window, uint16_t time,
            uint16_t lag) {
   int64_t sum = 0;
   for (int i = 0; i < window; i++) {
      sum += (int32_t)buffer[time + i] * (int32_t)buffer[time + lag + i];
   }
   return sum;
}

int64_t DF(const int16_t *buffer, uint16_t window, uint16_t time,
           uint16_t lag) {
   return ACF(buffer, window, time, 0) + ACF(buffer, window, time + lag, 0) -
          (2 * ACF(buffer, window, time, lag));
}

int64_t CMNDF(const int16_t *buffer, uint16_t window, uint16_t time,
              uint16_t lag) {
   if (lag == 0)
      return 0x7fffffffLL; // 1.0 in Q31

   static const int16_t *last_buf = NULL;
   static uint16_t last_W = 0, last_t = 0, last_lag = 0;
   static int64_t den_sum = 0;

   // reset / update denominator
   if (buffer != last_buf || window != last_W || time != last_t ||
       lag != (uint16_t)(last_lag + 1)) {
      den_sum = 0;
      for (uint16_t j = 1; j <= lag; j++) {
         den_sum += DF(buffer, window, time, j);
      }
   } else {
      den_sum += DF(buffer, window, time, lag);
   }

   last_buf = buffer;
   last_W = window;
   last_t = time;
   last_lag = lag;

   if (den_sum <= 0)
      return 0x7fffffffLL;

   int64_t df_lag = DF(buffer, window, time, lag);

   // num = DF(lag) * lag
   __int128 num = (__int128)df_lag * (__int128)lag;

   // Q31 scaling safely in 128-bit
   __int128 scaled = num * (__int128)0x7fffffffLL;
   __int128 q = scaled / (__int128)den_sum;

   // clamp to [0, 1.0] in Q31 (optional but usually sensible)
   if (q < 0)
      q = 0;
   if (q > 0x7fffffffLL)
      q = 0x7fffffffLL;

   return (int64_t)q;
}

double parabolic_minimum(int64_t y_prev, int64_t y0, int64_t y_next) {
   int64_t denom = (y_prev - 2 * y0 + y_next);
   if (denom == 0)
      return 0.0;
   return 0.5 * ((double)y_prev - (double)y_next) / denom;
}

static inline int16_t parabolic_delta_q15(int64_t y_prev, int64_t y0,
                                          int64_t y_next) {
   int64_t denom = y_prev - 2 * y0 + y_next;
   if (denom == 0)
      return 0;

   int64_t num = y_prev - y_next; // numerator (without 0.5)
   // delta = 0.5 * num / denom
   // Q15 scaling: delta_q15 = delta * 32768 = (num * 16384) / denom
   int64_t dq15 = (num * 16384LL) / denom;

   // clamp to [-0.5, +0.5] in Q15 just to be safe
   if (dq15 > 16384)
      dq15 = 16384;
   if (dq15 < -16384)
      dq15 = -16384;

   return (int16_t)dq15;
}

uint16_t detect_pitch(const int16_t *buffer, uint16_t window, uint16_t time,
                      // uint16_t fs,
                      // uint16_t left_bound, uint16_t right_bound,
                      double thresh) {
   if (UPPER_TAU <= LOWER_TAU)
      return 0;

   int64_t CMNDF_vals[CMNDF_BUFFER_SIZE];

   int64_t minval = INT64_MAX;
   uint16_t minval_index = 0;

   for (uint16_t i = 0; i < CMNDF_BUFFER_SIZE; i++) {
      CMNDF_vals[i] = CMNDF(buffer, window, time, (uint16_t)(i + LOWER_TAU));
      if (CMNDF_vals[i] < minval) {
         minval = CMNDF_vals[i];
         minval_index = i;
      }
   }

   int64_t thresh_q31 = (int64_t)(thresh * 2147483647.0); // 0x7fffffff
   int32_t sample = -1;

   for (uint16_t i = 0; i < CMNDF_BUFFER_SIZE; i++) {
      if (CMNDF_vals[i] < thresh_q31) {
         sample = (int32_t)(i + LOWER_TAU);
         break;
      }
   }
   if (sample < 0)
      sample = (int32_t)(minval_index + LOWER_TAU);

   // parabolic refinement
   uint16_t k = (uint16_t)(sample - LOWER_TAU);
   double sample_refined = (double)sample;

   if (k > 0 && k + 1 < CMNDF_BUFFER_SIZE) {
      double delta =
         parabolic_minimum(CMNDF_vals[k - 1], CMNDF_vals[k], CMNDF_vals[k + 1]);
      sample_refined += delta;
   }

   if (sample_refined <= 0.0)
      return 0;
   return ((uint16_t)SAMPLING_RATE) / sample_refined;
}

static inline int64_t DF_direct(const int16_t *restrict buffer, uint16_t window,
                                uint16_t time, uint16_t tau) {
   const int16_t *restrict x = buffer + time;
   const int16_t *restrict y = buffer + time + tau;

   int64_t sum = 0;
   for (uint16_t i = 0; i < window; i++) {
      int32_t d = (int32_t)x[i] - (int32_t)y[i];
      sum += (int64_t)d * (int64_t)d;
   }
   return sum;
}

void CMNDF_frame_q31(const int16_t *buffer, uint16_t window, uint16_t time,
                     int64_t *cmndf_q31) {
   // Convention: CMNDF(0) = 1.0 (or large). You used max Q31.
   cmndf_q31[0] = 0x7fffffffLL;

   __int128 running =
      0; // cumulative sum of d(tau), use 128 to avoid overflow risk

   for (uint16_t tau = 1; tau <= UPPER_TAU; tau++) {
      int64_t d_tau = DF_direct(buffer, window, time, tau);
      running += (__int128)d_tau;

      if (running <= 0) {
         cmndf_q31[tau] = 0x7fffffffLL;
         continue;
      }

      // cmndf = d(tau) * tau / (sum_{j=1..tau} d(j))
      __int128 num = (__int128)d_tau * (__int128)tau;
      __int128 scaled = num * (__int128)0x7fffffffLL;
      __int128 q = scaled / running;

      if (q < 0)
         q = 0;
      if (q > 0x7fffffffLL)
         q = 0x7fffffffLL;
      cmndf_q31[tau] = (int64_t)q;
   }
}

uint16_t detect_pitch2(const int16_t *buffer, uint16_t window, uint16_t time,
                       double thresh) {
   static int64_t cmndf_q31[UPPER_TAU + 1];

   CMNDF_frame_q31(buffer, window, time, cmndf_q31);

   int64_t thresh_q31 = (int64_t)(thresh * 2147483647.0);

   // Find first LOCAL minimum below threshold in [LOWER_TAU..UPPER_TAU]
   int32_t sample = -1;
   int64_t minval = INT64_MAX;
   uint16_t min_tau = LOWER_TAU;

   for (uint16_t tau = LOWER_TAU; tau <= UPPER_TAU; tau++) {
      int64_t v = cmndf_q31[tau];
      if (v < minval) {
         minval = v;
         min_tau = tau;
      }

      if (tau > LOWER_TAU && tau < UPPER_TAU) {
         if (v < thresh_q31 && v <= cmndf_q31[tau - 1] &&
             v <= cmndf_q31[tau + 1]) {
            sample = (int32_t)tau;
            break;
         }
      }
   }
   if (sample < 0)
      sample = (int32_t)min_tau;

   // Parabolic refinement in Q15
   int32_t lag_q15 = sample * 32768; // tau in Q15

   if (sample > 1 && sample + 1 <= (int32_t)UPPER_TAU) {
      int16_t delta_q15 = parabolic_delta_q15(
         cmndf_q31[sample - 1], cmndf_q31[sample], cmndf_q31[sample + 1]);
      lag_q15 += (int32_t)delta_q15;
   }

   if (lag_q15 <= 0)
      return 0;

   // Hz rounded: f = fs*32768 / lag_q15
   uint32_t hz = ((uint32_t)SAMPLING_RATE * 32768u + (uint32_t)(lag_q15 / 2)) /
                 (uint32_t)lag_q15;
   if (hz > 65535u)
      hz = 65535u;
   return (uint16_t)hz;
}

int main() {
   // clock_t start, end;
   int16_t buffer[BUFFER_SIZE];
   while (read(STDIN_FILENO, buffer, sizeof(buffer)) ==
          (ssize_t)sizeof(buffer)) {
      // start = clock();
      uint16_t pitch = detect_pitch2(buffer, WINDOW_SIZE, 0, 0.09);
      // printf("%d\n", pitch);
      // end = clock();
      // double latency_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
      write(STDOUT_FILENO, &pitch, sizeof(uint16_t));
      // fprintf(stderr, "latency: %.3f\n", latency_ms);
   }
   return 0;
}
