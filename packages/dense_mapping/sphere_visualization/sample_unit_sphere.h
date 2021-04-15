#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

template <typename T> bool solveQuadratic(T a, T b, T c, T& r1, T& r2) {
    const auto discriminant = b * b - 4 * a * c;

    if (discriminant >= 0) {
        // See http://www.it.uom.gr/teaching/linearalgebra/NumericalRecipiesInC/c5-6.pdf for why all the craziness
        const auto q = -(b + std::copysign(std::sqrt(discriminant), b)) / 2;
        r2 = c / q;
        r1 = (a != 0) ? q / a : r2;

        return true;
    } else {
        return false;
    }
}

template <typename T>
bool rayIntersectionWithSphere(std::array<T, 3> rayOrigin, std::array<T, 3> rayDirection, std::array<T, 3> sphereCenter, T sphereRadius,
    std::array<T, 3>& nearestIntersection) {
    // How does this work?
    //
    // Let R be a ray which passes through point O (= rayOrigin) and moves in direction D (rayDirection).
    // Let S be a sphere centered at C (= sphereCenter) with radius r(=sphereRadius).
    //
    // Suppose there exists a t such that O + tD is on S, that is:
    // <O + tD - C, O + tD - C> = r*r
    //
    // Let:
    // alpha = O - C
    //
    // Then:
    // <alpha + tD, alpha + tD> = r * r
    // <alpha, alpha> + 2t<D,alpha> + t^2<D,D> - r*r = 0
    //
    // Define:
    // a = <D, D>
    // b = 2<D, alpha>
    // c = <alpha, alpha> - r*r
    //
    // then a*t^2+b*t+c = 0, which we can solve using the quadratic equation, picking the smallest positive root as
    // our answer since this is the closest to the origin of the ray.

    const std::array<T, 3> alpha{ { rayOrigin[0] - sphereCenter[0], rayOrigin[1] - sphereCenter[1], rayOrigin[2] - sphereCenter[2] } };
    const auto a = dense_mapping::Geometry::innerProduct(rayDirection, rayDirection);
    const auto b = 2 * dense_mapping::Geometry::innerProduct(alpha, rayDirection);
    const auto c = dense_mapping::Geometry::innerProduct(alpha, alpha) - sphereRadius * sphereRadius;
    T t1, t2;
    bool result = false;

    if (std::abs(a) > std::numeric_limits<T>::epsilon() && solveQuadratic(a, b, c, t1, t2)) {
        // if t1, t2 have the same sign, return the smallest of them; otherwise pick the largest which is the only
        // non-negative solution
        const T smallest = (t1 * t2) > 0 ? std::min(t1, t2) : std::max(t1, t2);

        if (smallest >= 0) {
            nearestIntersection[0] = rayOrigin[0] + smallest * rayDirection[0];
            nearestIntersection[1] = rayOrigin[1] + smallest * rayDirection[1];
            nearestIntersection[2] = rayOrigin[2] + smallest * rayDirection[2];
            result = true;
        }
    }

    return result;
}

template <typename T>
std::pair<std::array<T, 3>, std::vector<std::array<T, 3> > > samplePointsOnUnitSphere(
    T distanceToSphereCenter, size_t alphaSamples, size_t thetaSamples) {
    const std::array<T, 3> cameraOrigin{ { 0, 0, -distanceToSphereCenter } };
    constexpr std::array<T, 3> sphereOrigin{ { 0, 0, 0 } };
    constexpr T sphereRadius{ 1 };
    const T alpha = T(M_PI) / 2 - std::atan(std::sqrt(distanceToSphereCenter * distanceToSphereCenter - 1)); // cot^{-1}(sqrt(R^2-1)

    std::vector<std::array<T, 3> > result;
    for (size_t a = 0; a <= alphaSamples; ++a) {
        const T effAlpha = -alpha + 2 * alpha * a / alphaSamples;
        const T sinAlpha = std::sin(effAlpha);
        const T cosAlpha = std::cos(effAlpha);

        for (size_t t = 0; t <= thetaSamples; ++t) {
            const T effTheta = -T(M_PI) + 2 * T(M_PI) * t / thetaSamples;
            const T sinTheta = std::sin(effTheta);
            const T cosTheta = std::cos(effTheta);

            const std::array<T, 3> direction{ { sinAlpha * cosTheta, sinAlpha * sinTheta, cosAlpha } };
            std::array<T, 3> intersection{ { 0, 0, 0 } };

            if (rayIntersectionWithSphere(cameraOrigin, direction, sphereOrigin, sphereRadius, intersection)) {
                result.push_back(intersection);
            }
        }
    }

    return std::make_pair(cameraOrigin, result);
}