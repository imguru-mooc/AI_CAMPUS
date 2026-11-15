"""QP Day 1 — NumPy 양자화 함수 모음 (강사 참고본)"""
import numpy as np

def calc_scale_zp(xmin, xmax, qmin=0, qmax=255):
    xmin, xmax = min(float(xmin), 0.0), max(float(xmax), 0.0)
    if xmax == xmin:
        return 1.0, qmin
    scale = (xmax - xmin) / (qmax - qmin)
    zp = int(np.clip(qmin - round(xmin / scale), qmin, qmax))
    return scale, zp

def quantize(x, scale, zp, qmin=0, qmax=255):
    return np.clip(np.round(np.asarray(x) / scale) + zp, qmin, qmax).astype(np.int32)

def dequantize(q, scale, zp):
    return (np.asarray(q, np.float64) - zp) * scale

def sym_scale(x, bits=8):
    return float(np.abs(x).max()) / (2 ** (bits - 1) - 1)
