/*
  This file is part of the Calibu Project.
  https://github.com/arpg/Calibu

  Copyright (C) 2015
  Steven Lovegrove,
  Nima Keivan
  Christoffer Heckman,
  Gabe Sibley,
  University of Colorado at Boulder,
  George Washington University.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#pragma once
#include <calibu/cam/camera_crtp.h>
#include <calibu/cam/camera_crtp_impl.h>
#include <calibu/cam/camera_utils.h>
#include <iostream>

namespace calibu {

/// Model "fov," colloquially known as "fisheye" model.
constexpr double kFovCamDistEps = 1e-5;
template <typename Scalar = double> class FovCamera : public CameraImpl<Scalar, 5, FovCamera<Scalar>> {
    typedef CameraImpl<Scalar, 5, FovCamera<Scalar>> Base;

public:
    using Base::Base;

    static constexpr int NumParams = 5;

    template <typename T> static void Scale(const double s, T* params) { CameraUtils::Scale(s, params); }

    // NOTE: A camera calibration matrix only makes sense for a LinearCamera.
    // Such a matrix only exists if derived for linear projection models. Ideally
    // any time we call for K it would be on a linear camera, but we provide this
    // functionality for obtaining approximate solutions.
    // FURTHER NOTE: We ASSUME the first four entries of params_ to be fu, fv, sx
    // and sy. If your camera model doesn't respect this ordering, then evaluating
    // K for it will result in an incorrect (even approximate) matrix.
    template <typename T> static void K(const T* params, T* Kmat) { CameraUtils::K(params, Kmat); }

    // For these derivatives, refer to the camera_derivatives.m matlab file.
    template <typename T> static T Factor(const T rad, const T* params) {
        const T param = params[4];
        if (param * param > (T)kFovCamDistEps) {
            const T mul2_tanw_by2 = (T)2.0 * tan(param / (T)2.0);
            if (rad * rad < (T)kFovCamDistEps) {
                // limit r->0
                return mul2_tanw_by2 / param;
            }
            return atan(rad * mul2_tanw_by2) / (rad * param);
        }
        // limit w->0
        return (T)1;
    }

    template <typename T> static T dFactor_dparam(const T rad, const T* params, T* fac) {
        const T param = params[4];
        if (param * param > kFovCamDistEps) {
            const T tanw_by2 = tan(param / (T)2.0);
            const T mul2_tanw_by2 = (T)2.0 * tanw_by2;
            if (rad * rad < kFovCamDistEps) {
                // limit r->0
                *fac = mul2_tanw_by2 / param;
                return ((T)2 * ((tanw_by2 * tanw_by2) / (T)2 + (T)0.5)) / param - mul2_tanw_by2 / (param * param);
            }
            const T tanw_by2_sq = tanw_by2 * tanw_by2;
            const T atan_mul2_tanw_by2 = atan(rad * mul2_tanw_by2);
            const T rad_mul_param = (rad * param);
            *fac = atan_mul2_tanw_by2 / rad_mul_param;
            return ((T)2 * (tanw_by2_sq / (T)2 + (T)0.5)) / (param * ((T)4 * tanw_by2_sq * rad * rad + (T)1))
                - atan_mul2_tanw_by2 / (rad_mul_param * param);
        }
        // limit w->0
        *fac = (T)1;
        return (T)0;
    }

    template <typename T> static T dFactor_drad(const T rad, const T* params, T* fac) {
        const T param = params[4];
        if (param * param < kFovCamDistEps) {
            *fac = (T)1;
            return (T)0;
        } else {
            const T tan_wby2 = tan(param / (T)2.0);
            const T mul2_tanw_by2 = (T)2.0 * tan_wby2;

            if (rad * rad < kFovCamDistEps) {
                *fac = mul2_tanw_by2 / param;
                return (T)0;
            } else {
                const T atan_mul2_tanw_by2 = atan(rad * mul2_tanw_by2);
                const T rad_mul_param = (rad * param);

                *fac = atan_mul2_tanw_by2 / rad_mul_param;
                return ((T)2 * tan_wby2) / (rad * param * ((T)4 * rad * rad * tan_wby2 * tan_wby2 + (T)1))
                    - atan_mul2_tanw_by2 / (rad * rad_mul_param);
            }
        }
    }

    template <typename T> static T Factor_inv(const T rad, const T* params) {
        const T param = params[4];
        if (param * param > kFovCamDistEps) {
            const T w_by2 = param / (T)2.0;
            const T mul_2tanw_by2 = tan(w_by2) * (T)2.0;

            if (rad * rad < kFovCamDistEps) {
                // limit r->0
                return param / mul_2tanw_by2;
            }
            return tan(rad * param) / (rad * mul_2tanw_by2);
        }
        // limit w->0
        return (T)1.0;
    }

    template <typename T> static T dFactor_inv_dparam(const T rad, const T* params) {
        const T param = params[4];
        if (param * param > kFovCamDistEps) {
            const T tan_wby2 = tan(param / (T)2.0);
            if (rad * rad < kFovCamDistEps) {
                return (T)1.0 / ((T)2 * tan_wby2) - (param * (tan_wby2 * tan_wby2 / (T)2.0 + (T)0.5)) / ((T)2.0 * tan_wby2 * tan_wby2);
            }
            const T tan_rad_mul_w = tan(rad * param);
            const T tanw_by2_sq = tan_wby2 * tan_wby2;
            return (tan_rad_mul_w * tan_rad_mul_w + (T)1.0) / ((T)2.0 * tan_wby2)
                - (tan_rad_mul_w * (tanw_by2_sq / (T)2.0 + (T)0.5)) / ((T)2.0 * rad * tanw_by2_sq);
        }
        // limit w->0
        return (T)0.0;
    }

    template <typename T> static T dFactor_inv_drad(const T rad, const T* params, T* fac) {
        const T param = params[4];
        if (param * param > kFovCamDistEps) {
            const T w_by2 = param / (T)2.0;
            const T tan_w_by2 = tan(w_by2);
            const T mul_2tanw_by2 = tan_w_by2 * (T)2.0;
            if (rad * rad < kFovCamDistEps) {
                *fac = param / tan_w_by2;
                return (T)0;
            }
            const T one_by_tan_w_by_2 = (T)1.0 / tan_w_by2;
            const T tan_rad_mul_w = tan(rad * param);
            *fac = tan_rad_mul_w / (rad * mul_2tanw_by2);
            return (one_by_tan_w_by_2 * param * (tan_rad_mul_w * tan_rad_mul_w + (T)1)) / ((T)2 * rad)
                - (one_by_tan_w_by_2 * tan_rad_mul_w) / (2 * rad * rad);
        }
        // limit w->0
        *fac = (T)1;
        return (T)0.0;
    }

    template <typename T> static void Unproject(const T* pix, const T* params, T* ray) {
        // First multiply by inverse K and calculate distortion parameter.
        T pix_kinv[2];
        CameraUtils::MultInvK(params, pix, pix_kinv);
        const T fac_inv = Factor_inv(CameraUtils::PixNorm(pix_kinv), params);
        pix_kinv[0] *= fac_inv;
        pix_kinv[1] *= fac_inv;
        // Homogenize the point.
        CameraUtils::Homogenize<T>(pix_kinv, ray);
    }

    template <typename T> static void dUnproject_dparams(const T* pix, const T* params, T* j) {
        T pix_kinv[2];
        CameraUtils::MultInvK(params, pix, pix_kinv);
        CameraUtils::dMultInvK_dparams(params, pix, j);

        // The complexity of this jacobian is due to the fact that the distortion
        // factor is calculated _after_ the multiplication by K^-1, therefore
        // the parameters of K affect both pix_kinv and fac_inv. Therefore we
        // use the product rule to push the derivatives w.r.t. K through both
        // functions.

        T rad = CameraUtils::PixNorm(pix_kinv);
        T fac_inv;
        const T dfac_inv_drad_byrad = dFactor_inv_drad(rad, params, &fac_inv) / rad;
        const T dfac_inv_dp[2] = { pix_kinv[0] * dfac_inv_drad_byrad, pix_kinv[1] * dfac_inv_drad_byrad };
        // Calculate the jacobian of the factor w.r.t. the K parameters.
        T dfac_dparams[4];
        dfac_dparams[0] = dfac_inv_dp[0] * j[0];
        dfac_dparams[1] = dfac_inv_dp[1] * j[4];
        dfac_dparams[2] = dfac_inv_dp[0] * j[6];
        dfac_dparams[3] = dfac_inv_dp[1] * j[10];

        // This is the derivatives w.r.t. K parameters for K^-1 * fac_inv
        T dfac_inv = dFactor_inv_dparam(rad, params);
        j[0] *= fac_inv;
        j[4] *= fac_inv;
        j[6] *= fac_inv;
        j[10] *= fac_inv;

        // Total derivative w.r.t. the K parameters for fac_inv added to the
        // previous values (product rule)
        j[0] += dfac_dparams[0] * pix_kinv[0];
        j[1] += dfac_dparams[0] * pix_kinv[1];

        j[3] += dfac_dparams[1] * pix_kinv[0];
        j[4] += dfac_dparams[1] * pix_kinv[1];

        j[6] += dfac_dparams[2] * pix_kinv[0];
        j[7] += dfac_dparams[2] * pix_kinv[1];

        j[9] += dfac_dparams[3] * pix_kinv[0];
        j[10] += dfac_dparams[3] * pix_kinv[1];

        // Derivatives w.r.t. the w parameter do not need to go through the chain
        // rule as they do not affect the multiplication by K^-1.
        j[12] = pix_kinv[0] * dfac_inv;
        j[13] = pix_kinv[1] * dfac_inv;
        j[14] = 0;
    }

    template <typename T> static void Project(const T* ray, const T* params, T* pix) {
        // De-homogenize and multiply by K.
        CameraUtils::Dehomogenize(ray, pix);
        // Calculate distortion parameter.
        const T fac = Factor(CameraUtils::PixNorm(pix), params);
        pix[0] *= fac;
        pix[1] *= fac;
        CameraUtils::MultK<T>(params, pix, pix);
    }

    template <typename T> static void dProject_dparams(const T* ray, const T* params, T* j) {
        T pix[2];
        CameraUtils::Dehomogenize(ray, pix);
        // This derivative is simplified compared to the unproject derivative,
        // as we first calculate the distortion parameter, then multiply by K.
        // The derivatives are then a simple application of the chain rule.
        T fac;
        T d_fac = dFactor_dparam(CameraUtils::PixNorm(pix), params, &fac);
        CameraUtils::dMultK_dparams(params, pix, j);
        j[0] *= fac;
        j[3] *= fac;
        // Derivatives w.r.t. w:
        j[8] = params[0] * pix[0] * d_fac;
        j[9] = params[1] * pix[1] * d_fac;
    }

    template <typename T> static void dProject_dray(const T* ray, const T* params, T* j) {
        // De-homogenize and multiply by K.
        T pix[2];
        CameraUtils::Dehomogenize(ray, pix);
        // Calculte the dehomogenization derivative.
        T j_dehomog[6];
        CameraUtils::dDehomogenize_dray(ray, j_dehomog);

        const T rad = CameraUtils::PixNorm(pix);
        T fac;
        const T dfac_drad_byrad = dFactor_drad(rad, params, &fac) / rad;
        const T dfac_dp[2] = { pix[0] * dfac_drad_byrad, pix[1] * dfac_drad_byrad };

        // Calculate the k matrix and distortion derivative.
        const T params0_pix0 = params[0] * pix[0];
        const T params1_pix1 = params[1] * pix[1];
        const T k00 = dfac_dp[0] * params0_pix0 + fac * params[0];
        const T k01 = dfac_dp[1] * params0_pix0;
        const T k10 = dfac_dp[0] * params1_pix1;
        const T k11 = dfac_dp[1] * params1_pix1 + fac * params[1];

        // Do the multiplication dkmult * ddehomogenized_dray
        j[0] = j_dehomog[0] * k00;
        j[1] = j_dehomog[0] * k10;
        j[2] = j_dehomog[3] * k01;
        j[3] = j_dehomog[3] * k11;
        j[4] = j_dehomog[4] * k00 + j_dehomog[5] * k01;
        j[5] = j_dehomog[4] * k10 + j_dehomog[5] * k11;
    }
};

/** A two-coefficient polynomial distortion model. */
template <typename Scalar = double> class Poly2Camera : public CameraImpl<Scalar, 6, Poly2Camera<Scalar>> {
    typedef CameraImpl<Scalar, 6, Poly2Camera<Scalar>> Base;

public:
    using Base::Base;

    static constexpr int NumParams = 6;

    template <typename T> static void Scale(const double s, T* params) { CameraUtils::Scale(s, params); }

    // NOTE: A camera calibration matrix only makes sense for a LinearCamera.
    // Such a matrix only exists if derived for linear projection models. Ideally
    // any time we call for K it would be on a linear camera, but we provide this
    // functionality for obtaining approximate solutions.
    // FURTHER NOTE: We ASSUME the first four entries of params_ to be fu, fv, sx
    // and sy. If your camera model doesn't respect this ordering, then evaluating
    // K for it will result in an incorrect (even approximate) matrix.
    template <typename T> static void K(const T* params, T* Kmat) { CameraUtils::K(params, Kmat); }

    template <typename T> static T Factor(const T rad, const T* params) {
        T r2 = rad * rad;
        T r4 = r2 * r2;
        return (static_cast<T>(1.0) + params[4] * r2 + params[5] * r4);
    }

    template <typename T> static T dFactor_drad(const T r, const T* params, T* fac) {
        *fac = Factor(r, params);
        return 2.0 * params[4] * r + 4.0 * params[5] * r * r * r;
    }

    template <typename T> static T Factor_inv(const T r, const T* params) {
        T k1 = params[4];
        T k2 = params[5];

        // Use Newton's method to solve (fixed number of iterations)
        // (for explanation, see notes in beginning of camera_models_crtp.h)
        T ru = r;
        for (int i = 0; i < 5; i++) {
            // Common sub-expressions of d, d2
            T ru2 = ru * ru;
            T ru4 = ru2 * ru2;
            T pol = k1 * ru2 + k2 * ru4 + 1;
            T pol2 = 2 * ru2 * (k1 + 2 * k2 * ru2);
            T pol3 = pol + pol2;

            // 1st derivative
            T d = (ru * (pol)-r) * 2 * pol3;
            // 2nd derivative
            T d2 = (4 * ru * (ru * pol - r) * (3 * k1 + 10 * k2 * ru2) + 2 * pol3 * pol3);
            // Delta update
            T delta = d / d2;
            ru -= delta;
        }

        // Return the undistortion factor
        return ru / r;
    }

    template <typename T> static void Unproject(const T* pix, const T* params, T* ray) {
        // First multiply by inverse K and calculate distortion parameter.
        T pix_kinv[2];
        CameraUtils::MultInvK(params, pix, pix_kinv);

        // Homogenize the point.
        CameraUtils::Homogenize<T>(pix_kinv, ray);
        const T fac_inv = Factor_inv(CameraUtils::PixNorm(pix_kinv), params);
        pix_kinv[0] *= fac_inv;
        pix_kinv[1] *= fac_inv;
        CameraUtils::Homogenize<T>(pix_kinv, ray);
    }

    template <typename T> static void Project(const T* ray, const T* params, T* pix) {
        // De-homogenize and multiply by K.
        CameraUtils::Dehomogenize(ray, pix);

        // Calculate distortion parameter.
        const T fac = Factor(CameraUtils::PixNorm(pix), params);
        pix[0] *= fac;
        pix[1] *= fac;
        CameraUtils::MultK<T>(params, pix, pix);
    }

    template <typename T> static void dProject_dray(const T* ray, const T* params, T* j) {
        // De-homogenize and multiply by K.
        T pix[2];
        CameraUtils::Dehomogenize(ray, pix);

        // Calculate the dehomogenization derivative.
        T j_dehomog[6];
        CameraUtils::dDehomogenize_dray(ray, j_dehomog);

        T rad = CameraUtils::PixNorm(pix);
        T fac;
        T dfac_drad_byrad = dFactor_drad(rad, params, &fac) / rad;
        T dfac_dp[2] = { pix[0] * dfac_drad_byrad, pix[1] * dfac_drad_byrad };

        // Calculate the k matrix and distortion derivative.
        T params0_pix0 = params[0] * pix[0];
        T params1_pix1 = params[1] * pix[1];
        T k00 = dfac_dp[0] * params0_pix0 + fac * params[0];
        T k01 = dfac_dp[1] * params0_pix0;
        T k10 = dfac_dp[0] * params1_pix1;
        T k11 = dfac_dp[1] * params1_pix1 + fac * params[1];

        // Do the multiplication dkmult * ddehomogenized_dray
        j[0] = j_dehomog[0] * k00;
        j[1] = j_dehomog[0] * k10;
        j[2] = j_dehomog[3] * k01;
        j[3] = j_dehomog[3] * k11;
        j[4] = j_dehomog[4] * k00 + j_dehomog[5] * k01;
        j[5] = j_dehomog[4] * k10 + j_dehomog[5] * k11;
    }

    template <typename T> static void dProject_dparams(const T*, const T*, T*) {
        std::cerr << "dProjedt_dparams not defined for the poly2 model. "
                     " Throwing exception."
                  << std::endl;
        throw 0;
    }

    template <typename T> static void dUnproject_dparams(const T*, const T*, T*) {
        std::cerr << "dUnproject_dparams not defined for the poly2 model. "
                     " Throwing exception."
                  << std::endl;
        throw 0;
    }
};

/** A three-coefficient polynomial distortion model. */
template <typename Scalar = double> class Poly3Camera : public CameraImpl<Scalar, 7, Poly3Camera<Scalar>> {
    typedef CameraImpl<Scalar, 7, Poly3Camera<Scalar>> Base;

public:
    using Base::Base;

    static constexpr int NumParams = 7;

    template <typename T> static void Scale(const double s, T* params) { CameraUtils::Scale(s, params); }

    // NOTE: A camera calibration matrix only makes sense for a LinearCamera.
    // Such a matrix only exists if derived for linear projection models. Ideally
    // any time we call for K it would be on a linear camera, but we provide this
    // functionality for obtaining approximate solutions.
    // FURTHER NOTE: We ASSUME the first four entries of params_ to be fu, fv, sx
    // and sy. If your camera model doesn't respect this ordering, then evaluating
    // K for it will result in an incorrect (even approximate) matrix.
    template <typename T> static void K(const T* params, T* Kmat) { CameraUtils::K(params, Kmat); }

    template <typename T> static T Factor(const T rad, const T* params) {
        T r2 = rad * rad;
        T r4 = r2 * r2;
        return (static_cast<T>(1.0) + params[4] * r2 + params[5] * r4 + params[6] * r4 * r2);
    }

    template <typename T> static T dFactor_drad(const T r, const T* params, T* fac) {
        *fac = Factor(r, params);
        T r2 = r * r;
        T r3 = r2 * r;
        return (2.0 * params[4] * r + 4.0 * params[5] * r3 + 6.0 * params[6] * r3 * r2);
    }

    template <typename T> static T Factor_inv(const T r, const T* params) {
        T k1 = params[4];
        T k2 = params[5];
        T k3 = params[6];

        // Use Newton's method to solve (fixed number of iterations)
        // (for explanation, see notes in beginning of camera_models_crtp.h)
        T ru = r;
        for (int i = 0; i < 5; i++) {
            // Common sub-expressions of d, d2
            T ru2 = ru * ru;
            T ru4 = ru2 * ru2;
            T ru6 = ru4 * ru2;
            T pol = k1 * ru2 + k2 * ru4 + k3 * ru6 + 1;
            T pol2 = 2 * ru2 * (k1 + 2 * k2 * ru2 + 3 * k3 * ru4);
            T pol3 = pol + pol2;

            // 1st derivative
            T d = (ru * (pol)-r) * 2 * pol3;
            // 2nd derivative
            T d2 = (4 * ru * (ru * pol - r) * (3 * k1 + 10 * k2 * ru2 + 21 * k3 * ru4) + 2 * pol3 * pol3);
            // Delta update
            T delta = d / d2;
            ru -= delta;
        }

        // Return the undistortion factor
        return ru / r;
    }

    template <typename T> static void Unproject(const T* pix, const T* params, T* ray) {
        // First multiply by inverse K and calculate distortion parameter.
        T pix_kinv[2];
        CameraUtils::MultInvK(params, pix, pix_kinv);

        // Homogenize the point.
        CameraUtils::Homogenize<T>(pix_kinv, ray);
        const T fac_inv = Factor_inv(CameraUtils::PixNorm(pix_kinv), params);
        pix_kinv[0] *= fac_inv;
        pix_kinv[1] *= fac_inv;
        CameraUtils::Homogenize<T>(pix_kinv, ray);
    }

    template <typename T> static void Project(const T* ray, const T* params, T* pix) {
        // De-homogenize and multiply by K.
        CameraUtils::Dehomogenize(ray, pix);

        // Calculate distortion parameter.
        const T fac = Factor(CameraUtils::PixNorm(pix), params);
        pix[0] *= fac;
        pix[1] *= fac;
        CameraUtils::MultK<T>(params, pix, pix);
    }

    template <typename T> static void dProject_dray(const T* ray, const T* params, T* j) {
        // De-homogenize and multiply by K.
        T pix[2];
        CameraUtils::Dehomogenize(ray, pix);

        // Calculate the dehomogenization derivative.
        T j_dehomog[6];
        CameraUtils::dDehomogenize_dray(ray, j_dehomog);

        T rad = CameraUtils::PixNorm(pix);
        T fac;
        T dfac_drad_byrad = dFactor_drad(rad, params, &fac) / rad;
        T dfac_dp[2] = { pix[0] * dfac_drad_byrad, pix[1] * dfac_drad_byrad };

        // Calculate the k matrix and distortion derivative.
        T params0_pix0 = params[0] * pix[0];
        T params1_pix1 = params[1] * pix[1];
        T k00 = dfac_dp[0] * params0_pix0 + fac * params[0];
        T k01 = dfac_dp[1] * params0_pix0;
        T k10 = dfac_dp[0] * params1_pix1;
        T k11 = dfac_dp[1] * params1_pix1 + fac * params[1];

        // Do the multiplication dkmult * ddehomogenized_dray
        j[0] = j_dehomog[0] * k00;
        j[1] = j_dehomog[0] * k10;
        j[2] = j_dehomog[3] * k01;
        j[3] = j_dehomog[3] * k11;
        j[4] = j_dehomog[4] * k00 + j_dehomog[5] * k01;
        j[5] = j_dehomog[4] * k10 + j_dehomog[5] * k11;
    }

    template <typename T> static void dProject_dparams(const T*, const T*, T*) {
        std::cerr << "dProjedt_dparams not defined for the poly3 model. "
                     " Throwing exception."
                  << std::endl;
        throw 0;
    }

    template <typename T> static void dUnproject_dparams(const T*, const T*, T*) {
        std::cerr << "dUnproject_dparams not defined for the poly3 model. "
                     " Throwing exception."
                  << std::endl;
        throw 0;
    }
};
}
