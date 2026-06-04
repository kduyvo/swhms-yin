import sys

from scipy import signal
import scipy.io
import numpy as np
from scipy.io import wavfile

def f(x):
    f_0 = 1000
    # envelope = lambda x: np.exp(-x)
    return (
        np.sin(x*np.pi * 2 * f_0)
        # * envelope(x)
    )

def CMNDF(f, W, t, lag):
    if lag == 0:
        return 1
    return DF(f, W, t,  lag)\
        / np.sum([DF(f, W, t, j + 1) for j in range(lag)]) * lag

def parabolic_minimum(y_prev, y0, y_next):
    # returns offset in [-0.5, 0.5] typically
    denom = (y_prev - 2*y0 + y_next)
    if denom == 0:
        return 0.0
    return 0.5 * (y_prev - y_next) / denom

def ACF(f, W, t, lag):
    return np.sum(f[t : t + W] * f[lag + t : lag + W + t])

def DF(f, W, t, lag):
    return ACF(f, W, t, 0)\
        + ACF(f, W, t + lag, 0)\
        - (2 * ACF(f, W, t, lag))

# def detect_pitch(f, W, t, sample_rate, bounds, thresh = 0.1):
#     CMNDF_vals = [CMNDF(f, W, t, i) for i in range(*bounds)]
#     sample = None
#     for i, val in enumerate(CMNDF_vals):
#         if val < thresh:
#             sample = i + bounds[0]
#             break
#     if sample is None:
#         sample = np.argmin(CMNDF_vals) + bounds[0]
#     return sample_rate/sample
def detect_pitch(f, W, t, sample_rate, bounds, thresh=0.1):
    CMNDF_vals = [CMNDF(f, W, t, i) for i in range(*bounds)]

    # pick lag: first below threshold, else global min
    sample = None
    for i, val in enumerate(CMNDF_vals):
        if val < thresh:
            sample = i + bounds[0]
            break
    if sample is None:
        sample = int(np.argmin(CMNDF_vals) + bounds[0])

    # --- sub-sample refinement (parabolic interpolation) ---
    k = sample - bounds[0]  # index into CMNDF_vals
    if 1 <= k < len(CMNDF_vals) - 1:
        delta = parabolic_minimum(CMNDF_vals[k-1], CMNDF_vals[k], CMNDF_vals[k+1])
        sample_refined = sample + delta
    else:
        sample_refined = float(sample)

    return sample_rate / sample_refined

def detect_pitch_fast(f, W, t, fs, bounds, thresh=0.1):
    f = np.asarray(f, dtype=np.float64)

    low, high = bounds

    # Clamp so slices are valid: need lag + t + W <= len(f)
    max_lag_allowed = len(f) - (t + W)
    high = min(high, max_lag_allowed)
    low = max(low, 1)
    if high <= low:
        return None

    # Compute DF for lags 1..high-1 (so normalization sum_{j=1..tau} is correct)
    all_lags = np.arange(1, high, dtype=int)
    DF_all = np.array([DF(f, W, t, lag) for lag in all_lags], dtype=np.float64)

    csum_all = np.cumsum(DF_all)  # csum_all[k] = sum_{j=1..(k+1)} DF(j)

    # Now slice to the requested [low, high)
    lags = np.arange(low, high, dtype=int)
    DF_vals = DF_all[low-1 : high-1]
    den = csum_all[low-1 : high-1]

    CMNDF_vals = (DF_vals / (den + 1e-12)) * lags

    # pick first below threshold else min
    if np.any(CMNDF_vals < thresh):
        idx = int(np.argmax(CMNDF_vals < thresh))  # first True
    else:
        idx = int(np.argmin(CMNDF_vals))

    lag0 = float(lags[idx])

    # parabolic refine
    if 0 < idx < len(CMNDF_vals) - 1:
        delta = parabolic_minimum(CMNDF_vals[idx-1], CMNDF_vals[idx], CMNDF_vals[idx+1])
        lag0 += delta

    return fs / lag0
def main():
    sample_rate = 11025
    start = 0
    end = 0.05
    num_samples = int(sample_rate * (end - start) + 1)
    window_size = 30

    min_f, max_f = 50, 1600
    bounds = [int(sample_rate / max_f), int(sample_rate / min_f)]

    x = np.linspace(start, end, num_samples)
    print(detect_pitch(f(x), window_size, 1 , sample_rate, bounds))

def main2():
    sample_rate, data = wavfile.read("test400Hz.wav")
    data = data.astype(np.float64)
    start = 0
    end = 0.1
    # num_samples = int(sample_rate * (end - start) + 1)
    window_size = 30

    min_f, max_f = 50, 1550
    bounds = [int(sample_rate / max_f), int(sample_rate / min_f)]

    # x = np.linspace(start, end, num_samples)
    print(detect_pitch(data, window_size, 1, sample_rate, bounds))
#
def main3():
    sample_rate = 11025
    start = 0
    # num_samples = int(sample_rate * (end - start) + 1)
    min_f, max_f = 50, 1100
    min_tau, max_tau = int(sample_rate / max_f), int(sample_rate / min_f)
    num_samples = 276 

    window_size = 138
    end = (num_samples-1)/sample_rate

    # min_f, max_f = 50, 1100
    bounds = [min_tau, max_tau]

    x = np.linspace(start, end, num_samples)
    print(detect_pitch_fast(f(x), window_size, 1 , sample_rate, bounds))
# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    main3()

