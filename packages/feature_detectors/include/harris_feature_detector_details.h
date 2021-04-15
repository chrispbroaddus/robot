
#include <cstddef>
#include <cstdint>

namespace feature_detectors {
namespace details {

    /// Apply 5-tap [1 4 6 4 1] binomial kernel. The input here is short.
    constexpr int32_t horizontalBinomial(const int16_t xm2, const int16_t xm1, const int16_t x, const int16_t xp1, const int16_t xp2) {
        return xm2 + (xm1 << 2) + ((x << 1) + (x << 2)) + (xp1 << 2) + xp2;
    }

    /// Apply 5-tap [1 4 6 4 1] binomial kernel. The input here is int32.
    constexpr int32_t verticalBinomial(const int32_t xm2, const int32_t xm1, const int32_t x, const int32_t xp1, const int32_t xp2) {
        return xm2 + (xm1 << 2) + ((x << 1) + (x << 2)) + (xp1 << 2) + xp2;
    }

    /// Apply binomial kernel to an array of points in horizontal direction.
    inline void horizontalBinomial(int32_t* dst, const int16_t* src, int n) {
        for (int i = 0; i < n; i++, src++) {
            dst[i] = horizontalBinomial(src[-2], src[-1], src[0], src[1], src[2]);
        }
    }

    /// Apply binomial kernel to an array of points in vertical direction.
    inline void verticalBinomial(
        int32_t* dst, const int32_t* pm2, const int32_t* pm1, const int32_t* p, const int32_t* pp1, const int32_t* pp2, int n) {
        for (int i = 0; i < n; i++) {
            dst[i] = verticalBinomial(pm2[i], pm1[i], p[i], pp1[i], pp2[i]);
        }
    }

    /// Compute [-1 0 1] derivative in vertical and horizontal direction and compute the 3 unique components of the
    /// second moment matrix.
    inline void computeDerivatives(int16_t* Ixx, int16_t* Iyy, int16_t* Ixy, const unsigned char* pm1, const unsigned char* p,
        const unsigned char* pp1, const size_t n) {
        for (size_t i = 0; i < n; i++, pm1++, p++, pp1++) {
            // [-1,0,1], divide by 2 so that it fits into a int16_t
            const int16_t Ix = (-p[-1] + p[1]) / 2;
            const int16_t Iy = (-pm1[0] + pp1[0]) / 2;

            Ixx[i] = Ix * Ix;
            Iyy[i] = Iy * Iy;
            Ixy[i] = Ix * Iy;
        }
    }

    /// Compute the harris stengths given the weighted box sum of the derivative components.
    inline void harrisStengths(float* S, const int32_t* Gxx, const int32_t* Gyy, const int32_t* Gxy, const int n, const float k) {
        for (int i = 0; i < n; i++) {
            const float gxx = (float)Gxx[i];
            const float gyy = (float)Gyy[i];
            const float gxy = (float)Gxy[i];

            S[i] = (gxx * gyy - (gxy * gxy)) - (k * (gxx + gyy) * (gxx + gyy));
        }
    }

    /// Swap the 5 row pointers in a sliding window.
    template <typename T> inline void swapPointers(T** pm2, T** pm1, T** p, T** pp1, T** pp2) {
        T* tmp = *pm2;
        *pm2 = *pm1;
        *pm1 = *p;
        *p = *pp1;
        *pp1 = *pp2;
        *pp2 = tmp;
    }
}
}