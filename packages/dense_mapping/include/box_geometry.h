#pragma once

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

namespace dense_mapping {
/// Common 3D geometry primitives that support Octree operations
namespace Geometry {

    template <typename T> constexpr T innerProduct(const std::array<T, 3>& a, const std::array<T, 3>& b) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    template <typename T> constexpr T normL2(const std::array<T, 3>& vec) { return std::sqrt(innerProduct(vec, vec)); }

    template <typename T> constexpr T distance(const std::array<T, 3>& a, const std::array<T, 3>& b) {
        return normL2(std::array<T, 3>{ { a[0] - b[0], a[1] - b[1], a[2] - b[2] } });
    }

    template <typename T> constexpr std::array<T, 3> unitVector(const std::array<T, 3>& x) {
        const auto xNormInv = T(1) / normL2(x);
        return { { x[0] * xNormInv, x[1] * xNormInv, x[2] * xNormInv } };
    }

    template <typename T> constexpr std::array<T, 3> directionVector(const std::array<T, 3>& from, const std::array<T, 3>& to) {
        return { { (to[0] - from[0]), (to[1] - from[1]), (to[2] - from[2]) } };
    }

    template <typename T> constexpr std::array<T, 3> normalizedDirectionVector(const std::array<T, 3>& from, const std::array<T, 3>& to) {
        const auto inverseDistance = T(1) / distance(from, to);
        return { { (to[0] - from[0]) * inverseDistance, (to[1] - from[1]) * inverseDistance, (to[2] - from[2]) * inverseDistance } };
    }

    template <typename T> constexpr std::array<T, 3> reciprocalDirectionVector(const std::array<T, 3>& from, const std::array<T, 3>& to) {
        return { { 1 / (to[0] - from[0]), 1 / (to[1] - from[1]), 1 / (to[2] - from[2]) } };
    }

    template <typename T> constexpr std::array<T, 3> crossProduct(const std::array<T, 3>& a, const std::array<T, 3>& b) {
        return std::array<T, 3>{ { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] } };
    }

    /// Given a vector v=[vx,vy,vz] compute its reciprocal vector v'=[1/vx, 1/vy, 1/vz]
    /// \tparam T
    /// \param v
    /// \return
    template <typename T> constexpr std::array<T, 3> reciprocalDirectionVector(const std::array<T, 3>& v) {
        return std::array<T, 3>{ { T(1) / v[0], T(1) / v[1], T(1) / v[2] } };
    }

    /// Trivially convertible to bool if all you want is to check was there an intersection or not.
    template <typename T> struct SegmentAxisAlignedBoundingBoxIntersection {
        constexpr SegmentAxisAlignedBoundingBoxIntersection(T minT, T maxT)
            : tMin(minT)
            , tMax(maxT) {}

        constexpr operator bool() const {
            // -epsilon <= tMax
            const bool t1 = (-epsilon < tMax);

            // -epsilon <= tMin <= 1 + epsilon
            const bool t2 = (tMin < 1 + epsilon);

            // At least one of the end points of the segment [tMin, tMax] is in the range (-eps, 1+eps)
            // and we exit after we enter
            return (t1 && t2) && (tMax > tMin - epsilon);
        }

        constexpr bool isValid() const { return (!(std::isnan(tMin) || std::isnan(tMax))); }

        const T tMin;
        const T tMax;

        static constexpr T epsilon = std::numeric_limits<T>::epsilon();
    };

    template <typename T> inline std::ostream& operator<<(std::ostream& out, const SegmentAxisAlignedBoundingBoxIntersection<T>& x) {
        std::ios origState(nullptr);
        origState.copyfmt(out);

        out << std::setiosflags(std::ios::showpoint | std::ios::scientific) << std::setprecision(15) << "M: [" << x.tMin << ", " << x.tMax
            << "]";

        out.copyfmt(origState);
        return out;
    }

    /// Trivially convertible to bool if all you want is to check was there an intersection or not.
    template <typename T> struct RayAxisAlignedBoundingBoxIntersection {
        constexpr RayAxisAlignedBoundingBoxIntersection(T minT, T maxT)
            : tMin(minT)
            , tMax(maxT) {}

        constexpr operator bool() const { return tMax >= tMin; }

        constexpr bool isValid() const { return (!(std::isnan(tMin) || std::isnan(tMax))); }

        const T tMin;
        const T tMax;

        static constexpr T epsilon = std::numeric_limits<T>::epsilon();
    };

    template <typename T> inline std::ostream& operator<<(std::ostream& out, const RayAxisAlignedBoundingBoxIntersection<T>& x) {
        std::ios origState(nullptr);
        origState.copyfmt(out);

        out << std::setiosflags(std::ios::showpoint | std::ios::scientific) << std::setprecision(15) << "M: [" << x.tMin << ", " << x.tMax
            << "]";

        out.copyfmt(origState);
        return out;
    }

    namespace details {
        template <typename T, template <typename> class RESULT_TEMPLATE>
        constexpr RESULT_TEMPLATE<T> lineIntersectsAxisAlignedBoundingBox(T boxHalfExtent, const std::array<T, 3>& boxCenter,
            const std::array<T, 3>& source, const std::array<T, 3>& reciprocalDirection) {
            const auto boxMinX = boxCenter[0] - boxHalfExtent;
            const auto boxMinY = boxCenter[1] - boxHalfExtent;
            const auto boxMinZ = boxCenter[2] - boxHalfExtent;
            const auto boxMaxX = boxCenter[0] + boxHalfExtent;
            const auto boxMaxY = boxCenter[1] + boxHalfExtent;
            const auto boxMaxZ = boxCenter[2] + boxHalfExtent;

            // Compute the unscaled distance to the closest / farthest plane. Here we take advantage of our knowledge of the
            // sign of the reciprocal direction to avoid additional min/max calls (see the Utah paper for details as to why)
            const auto dMinX = (reciprocalDirection[0] >= 0 ? (boxMinX - source[0]) : (boxMaxX - source[0]));
            const auto dMaxX = (reciprocalDirection[0] >= 0 ? (boxMaxX - source[0]) : (boxMinX - source[0]));
            const auto dMinY = (reciprocalDirection[1] >= 0 ? (boxMinY - source[1]) : (boxMaxY - source[1]));
            const auto dMaxY = (reciprocalDirection[1] >= 0 ? (boxMaxY - source[1]) : (boxMinY - source[1]));
            const auto dMinZ = (reciprocalDirection[2] >= 0 ? (boxMinZ - source[2]) : (boxMaxZ - source[2]));
            const auto dMaxZ = (reciprocalDirection[2] >= 0 ? (boxMaxZ - source[2]) : (boxMinZ - source[2]));

            // Here we try to scale the distance. This is why we use the reciprocal direction -- the graphics community
            // tribal knowledge suggests that this is faster but I haven't had a chance to benchmark. The theory here is that
            // we pay once for the inversion and then get to use faster multiplies which is a win when you're testing the same
            // segment against many rays.
            //
            // This attempts to be clever: in the case that the source or target point is not exactly on the face of the box,
            // this is exactly as you would expect if you've read about any of the slab testing approaches for AABB. On the
            // other hand, if we are exactly on one of those planes and we do not move parallel to that plane, the reciprocal
            // direction will be +/-inf, which will potentially cause NaNs in the computed tMin/tMax. To solve this,
            // we declare that if we are on a minimum face, we have a distance of exactly -infinity, otherwise if we are on a
            // maximum face we are at a distance of exactly infinity. This seems to appease the unit test gods and is kinda sensible.
            const auto tMinX = dMinX != 0 ? dMinX * reciprocalDirection[0] : -std::numeric_limits<T>::infinity();
            const auto tMinY = dMinY != 0 ? dMinY * reciprocalDirection[1] : -std::numeric_limits<T>::infinity();
            const auto tMinZ = dMinZ != 0 ? dMinZ * reciprocalDirection[2] : -std::numeric_limits<T>::infinity();
            const auto tMaxX = dMaxX != 0 ? dMaxX * reciprocalDirection[0] : std::numeric_limits<T>::infinity();
            const auto tMaxY = dMaxY != 0 ? dMaxY * reciprocalDirection[1] : std::numeric_limits<T>::infinity();
            const auto tMaxZ = dMaxZ != 0 ? dMaxZ * reciprocalDirection[2] : std::numeric_limits<T>::infinity();

            const auto tMin = std::max(tMinX, std::max(tMinY, tMinZ));
            const auto tMax = std::min(tMaxX, std::min(tMaxY, tMaxZ));
            return RESULT_TEMPLATE<T>(tMin, tMax);
        }
    }

    /// Determine whether the line segment defined by (source, source + direction) (@emph{note the scaling!}) has any
    /// intersection with the box with given center and half-extent (half height/width/depth).
    ///
    /// This correctly handles a number of special cases:
    /// -# Degenerate segments (those where source = destination and therefore direction = 0) -- these produce results
    ///    which are identical to simply performing a point/box intersection test.
    /// -# Segments which start or end on a corner, edge, or face of the box (up to machine precision)
    /// .
    ///
    /// \tparam T Floating point type
    /// \param source one end point of the segment
    /// \param direction Exactly (destination - source)
    /// \param reciprocalDirection {1/direction[0], ..., 1/direction[2]} -- +/-Inf is expected
    /// \param boxCenter
    /// \param boxHalfExtent
    /// \return An object which is implicitly convertible to bool. Right now, this object also contains additional
    /// book-keeping information to assist in debugging and is also a hedge to enable additional use cases (e.g.
    /// generalizing to general ray intersection or line intersection tests, which all rely on the same calculations
    /// but which use slightly different logic to interpret results).
    template <typename T>
    constexpr SegmentAxisAlignedBoundingBoxIntersection<T> segmentIntersectsAxisAlignedBoundingBox(
        T boxHalfExtent, const std::array<T, 3>& boxCenter, const std::array<T, 3>& source, const std::array<T, 3>& reciprocalDirection) {
        return details::lineIntersectsAxisAlignedBoundingBox<T, SegmentAxisAlignedBoundingBoxIntersection>(
            boxHalfExtent, boxCenter, source, reciprocalDirection);
    }

    template <typename T>
    constexpr SegmentAxisAlignedBoundingBoxIntersection<T> segmentIntersectsAxisAlignedBoundingBox(
        const std::array<T, 3>& destination, const std::array<T, 3>& source, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
        const auto rayDirection = directionVector(source, destination);
        const auto reciprocalDirection = reciprocalDirectionVector(rayDirection);
        return segmentIntersectsAxisAlignedBoundingBox(boxHalfExtent, boxCenter, source, reciprocalDirection);
    }

    template <typename T>
    constexpr RayAxisAlignedBoundingBoxIntersection<T> rayIntersectsAxisAlignedBoundingBox(
        T boxHalfExtent, const std::array<T, 3>& boxCenter, const std::array<T, 3>& source, const std::array<T, 3>& reciprocalDirection) {
        return details::lineIntersectsAxisAlignedBoundingBox<T, RayAxisAlignedBoundingBoxIntersection>(
            boxHalfExtent, boxCenter, source, reciprocalDirection);
    }

    template <typename T>
    constexpr RayAxisAlignedBoundingBoxIntersection<T> rayIntersectsAxisAlignedBoundingBox(
        const std::array<T, 3>& destination, const std::array<T, 3>& source, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
        const auto rayDirection = directionVector(source, destination);
        const auto reciprocalDirection = reciprocalDirectionVector(rayDirection);
        return rayIntersectsAxisAlignedBoundingBox(boxHalfExtent, boxCenter, source, reciprocalDirection);
    }

    /// Determine whether a point intersects an axis-aligned bounding box.
    ///
    /// \tparam T
    /// \param point
    /// \param boxCenter
    /// \param boxHalfExtent
    /// \return
    template <typename T>
    constexpr bool pointIntersectsAxisAlignedBoundingBox(
        const std::array<T, 3>& point, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
        const bool inX = (point[0] >= (boxCenter[0] - boxHalfExtent)) && (point[0] <= (boxCenter[0] + boxHalfExtent));
        const bool inY = (point[1] >= (boxCenter[1] - boxHalfExtent)) && (point[1] <= (boxCenter[1] + boxHalfExtent));
        const bool inZ = (point[2] >= (boxCenter[2] - boxHalfExtent)) && (point[2] <= (boxCenter[2] + boxHalfExtent));
        return inX && inY && inZ;
    }

    /// Determine whether two axis-aligned bounding boxes intersect
    ///
    /// \tparam T
    /// \param point
    /// \param boxCenter
    /// \param boxHalfExtent
    /// \return
    template <typename T>
    constexpr bool axisAlignedBoxIntersectsAxisAlignedBox(
        const std::array<T, 3>& centerA, T halfExtentA, const std::array<T, 3>& centerB, T halfExtentB) {
        return (std::abs(centerA[0] - centerB[0]) <= halfExtentA + halfExtentB)
            && (std::abs(centerA[1] - centerB[1]) <= halfExtentA + halfExtentB)
            && (std::abs(centerA[2] - centerB[2]) <= halfExtentA + halfExtentB);
    }

    /// Given a rotation vector, return the corresponding rotation matrix
    template <typename T> constexpr std::array<T, 3 * 3> rotationMatrixFromRotationVector(const std::array<T, 3>& rotation) {
        const T theta = normL2(rotation);

        // Only do rotation if we have a useful amount to rotate by
        constexpr T eps = std::numeric_limits<T>::min();

        if (theta > eps) {
            // Rodrigues formula (https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula)
            const T sinT = std::sin(theta);
            const T cosT = std::cos(theta);

            // clang-format off
                const T K[3][3] = {
                        {0,                    -rotation[2] / theta,  rotation[1] / theta},
                        { rotation[2] / theta,                    0, -rotation[0] / theta},
                        {-rotation[1] / theta,  rotation[0] / theta,                    0}};

                const T K2[3][3] = {
                        {K[0][0] * K[0][0] + K[0][1] * K[1][0] + K[0][2] * K[2][0], K[0][0] * K[0][1] + K[0][1] * K[1][1] + K[0][2] * K[2][1], K[0][0] * K[0][2] + K[0][1] * K[1][2] + K[0][2] * K[2][2]},
                        {K[1][0] * K[0][0] + K[1][1] * K[1][0] + K[1][2] * K[2][0], K[1][0] * K[0][1] + K[1][1] * K[1][1] + K[1][2] * K[2][1], K[1][0] * K[0][2] + K[1][1] * K[1][2] + K[1][2] * K[2][2]},
                        {K[2][0] * K[0][0] + K[2][1] * K[1][0] + K[2][2] * K[2][0], K[2][0] * K[0][1] + K[2][1] * K[1][1] + K[2][2] * K[2][1], K[2][0] * K[0][2] + K[2][1] * K[1][2] + K[2][2] * K[2][2]},
                };

                const T R[3][3] = {
                        {1 + sinT * K[0][0] + (1 - cosT) * K2[0][0],     sinT * K[0][1] + (1 - cosT) * K2[0][1],     sinT * K[0][2] + (1 - cosT) * K2[0][2]},
                        {    sinT * K[1][0] + (1 - cosT) * K2[1][0], 1 + sinT * K[1][1] + (1 - cosT) * K2[1][1],     sinT * K[1][2] + (1 - cosT) * K2[1][2]},
                        {    sinT * K[2][0] + (1 - cosT) * K2[2][0],     sinT * K[2][1] + (1 - cosT) * K2[2][1], 1 + sinT * K[2][2] + (1 - cosT) * K2[2][2]},
                };
            // clang-format on

            return std::array<T, 3 * 3>{ { R[0][0], R[0][1], R[0][2], R[1][0], R[1][1], R[1][2], R[2][0], R[2][1], R[2][2] } };
        } else {
            return std::array<T, 3 * 3>{ { 1, 0, 0, 0, 1, 0, 0, 0, 1 } };
        }
    }

    /// Everything we need to know about a frustum
    template <typename T> struct Frustum {
        constexpr Frustum(const std::array<T, 3>& origin, const std::array<T, 3>& top, const std::array<T, 3>& bottom,
            const std::array<T, 3>& left, const std::array<T, 3>& right)
            : m_origin(origin)
            , m_topPlaneNormal(top)
            , m_bottomPlaneNormal(bottom)
            , m_leftPlaneNormal(left)
            , m_rightPlaneNormal(right) {}

        const std::array<T, 3> m_origin;
        const std::array<T, 3> m_topPlaneNormal;
        const std::array<T, 3> m_bottomPlaneNormal;
        const std::array<T, 3> m_leftPlaneNormal;
        const std::array<T, 3> m_rightPlaneNormal;
    };

    /// Generate a new frustum which is defined by three quantities:
    ///
    /// 1. The horizontal and vertical half angles (angle between the primary frustum axis and the left/right and top/bottom planes
    /// respectively)
    /// 2. The overall translation and rotation
    ///
    /// Notes:
    /// 1. Planes are oriented such that the interior of the frustum should be in the positive half-space of all the
    ///    bounding planes
    /// 2. Frustums follow the same conventions as cameras for coordinate frames: the frustum native +z axis is in the
    ///    center of the intersection of the frustum bounding planes, analagous to the optical axis for a camera.
    ///
    /// \tparam T floating point type (float or double)
    /// \param frustumApex The "center" of the frustum (i.e. the peak/point)
    /// \param frustumRotation SO(3) / Rodrigues rotation vector. The magnitude of the vector encodes the amount of rotation
    ///        while the unit norm vector defines the rotation axis.
    /// \param frustumHorizontalHalfAngleRadians
    /// \param frustumVerticalHalfAngleRadians
    ///
    /// \return frustum instance
    template <typename T>
    constexpr Frustum<T> frustum(const std::array<T, 3>& frustumApex, const std::array<T, 3>& frustumRotation,
        T frustumHorizontalHalfAngleRadians, T frustumVerticalHalfAngleRadians) {
        const T cosH = std::cos(frustumHorizontalHalfAngleRadians);
        const T sinH = std::sin(frustumHorizontalHalfAngleRadians);

        // Right face half plane should have normal [-1, 0, 0]^T rotated about Y by alpha_h. This gives us:
        // R_y(alpha_h) [-1, 0, 0]^T or:
        //
        // [ cos(alpha_h), 0, sin(alpha_h)] [-1]
        // [            0, 1,            0] [ 0]
        // [-sin(alpha_h), 0, cos(alpha_h)] [ 0]
        //
        // Or:
        // [-cos(alpha_h), 0, sin(alpha_h)]
        const std::array<T, 3> rightPlaneNormal{ { -cosH, 0, sinH } };

        // Left face half plane should have normal [-1, 0, 0]^T rotated about Y by -alpha_h. This gives us:
        // R_y(-alpha_h) [-1, 0, 0]^T or:
        //
        // [ cos(-alpha_h), 0, sin(-alpha_h)] [-1]
        // [             0, 1,             0] [ 0]
        // [-sin(-alpha_h), 0, cos(-alpha_h)] [ 0]
        //
        // Or:
        // [ cos(-alpha_h), 0, -sin(-alpha_h)]^T
        //
        // which, after applying evenness/oddness to simplify, yields:
        // [cos(alpha_h), 0, sin(alpha_h)]^T
        const std::array<T, 3> leftPlaneNormal{ { cosH, 0, sinH } };

        // Top plane should have normal [0, -1, 0]^T rotated about X by alpha_v. This gives us:
        // R_x(alpha_v) [0, -1, 0]^T or:
        //
        // [1             0            0 ] [ 0 ]
        // [0  cos(alpha_v) sin(alpha_v) ] [-1 ]
        // [0 -sin(alpha_v) cos(alpha_v) ] [ 0 ]
        //
        // Or:
        //
        // [0, -cos(alpha_v), sin(alpha_v)]^T
        const T cosV = std::cos(frustumVerticalHalfAngleRadians);
        const T sinV = std::sin(frustumVerticalHalfAngleRadians);
        const std::array<T, 3> topPlaneNormal{ { 0, -cosV, sinV } };

        // Bottom plane should have normal [0, 1, 0]^T rotated about X by -alpha_v. This gives us:
        // R_x(-alpha_v) [0, 1, 0]^T or:
        //
        // [1              0             0 ] [ 0 ]
        // [0  cos(-alpha_v) sin(-alpha_v) ] [ 1 ]
        // [0 -sin(-alpha_v) cos(-alpha_v) ] [ 0 ]
        //
        // Or:
        // [0, cos(-alpha_v), -sin(-alpha_v)]^T
        //
        // Leveraging oddness/evenness:
        // [0, cos(alpha_v), sin(alpha_v)]^T
        const std::array<T, 3> bottomPlaneNormal{ { 0, cosV, sinV } };

        const auto rotMatrix = rotationMatrixFromRotationVector(frustumRotation);

        // clang-format off
            const std::array<T, 3> topNormal{{rotMatrix[0 * 3 + 0] * topPlaneNormal[0] + rotMatrix[0 * 3 + 1] * topPlaneNormal[1] + rotMatrix[0 * 3 + 2] * topPlaneNormal[2],
                                              rotMatrix[1 * 3 + 0] * topPlaneNormal[0] + rotMatrix[1 * 3 + 1] * topPlaneNormal[1] + rotMatrix[1 * 3 + 2] * topPlaneNormal[2],
                                              rotMatrix[2 * 3 + 0] * topPlaneNormal[0] + rotMatrix[2 * 3 + 1] * topPlaneNormal[1] + rotMatrix[2 * 3 + 2] * topPlaneNormal[2],
                                             }};

            const std::array<T, 3> bottomNormal{{rotMatrix[0 * 3 + 0] * bottomPlaneNormal[0] + rotMatrix[0 * 3 + 1] * bottomPlaneNormal[1] + rotMatrix[0 * 3 + 2] * bottomPlaneNormal[2],
                                                 rotMatrix[1 * 3 + 0] * bottomPlaneNormal[0] + rotMatrix[1 * 3 + 1] * bottomPlaneNormal[1] + rotMatrix[1 * 3 + 2] * bottomPlaneNormal[2],
                                                 rotMatrix[2 * 3 + 0] * bottomPlaneNormal[0] + rotMatrix[2 * 3 + 1] * bottomPlaneNormal[1] + rotMatrix[2 * 3 + 2] * bottomPlaneNormal[2],
                                                }};

            const std::array<T, 3> leftNormal{{rotMatrix[0 * 3 + 0] * leftPlaneNormal[0] + rotMatrix[0 * 3 + 1] * leftPlaneNormal[1] + rotMatrix[0 * 3 + 2] * leftPlaneNormal[2],
                                               rotMatrix[1 * 3 + 0] * leftPlaneNormal[0] + rotMatrix[1 * 3 + 1] * leftPlaneNormal[1] + rotMatrix[1 * 3 + 2] * leftPlaneNormal[2],
                                               rotMatrix[2 * 3 + 0] * leftPlaneNormal[0] + rotMatrix[2 * 3 + 1] * leftPlaneNormal[1] + rotMatrix[2 * 3 + 2] * leftPlaneNormal[2],
                                              }};

            const std::array<T, 3> rightNormal{{rotMatrix[0 * 3 + 0] * rightPlaneNormal[0] + rotMatrix[0 * 3 + 1] * rightPlaneNormal[1] + rotMatrix[0 * 3 + 2] * rightPlaneNormal[2],
                                                rotMatrix[1 * 3 + 0] * rightPlaneNormal[0] + rotMatrix[1 * 3 + 1] * rightPlaneNormal[1] + rotMatrix[1 * 3 + 2] * rightPlaneNormal[2],
                                                rotMatrix[2 * 3 + 0] * rightPlaneNormal[0] + rotMatrix[2 * 3 + 1] * rightPlaneNormal[1] + rotMatrix[2 * 3 + 2] * rightPlaneNormal[2],
                                               }};
        // clang-format on

        return Frustum<T>(frustumApex, topNormal, bottomNormal, leftNormal, rightNormal);
    }

    /// Represents an oriented bounding box. In this case, we store:
    /// 1. The rotated coordinate frame as described by m_xDirection, m_yDirection, m_zDirection
    /// 2. The extents of the box. This is the half-width/height/depth
    /// 3. The center of the box.
    template <typename T> struct OrientedBoundingBox {
        ///
        /// \param center
        /// \param xDirection
        /// \param yDirection
        /// \param xExtent
        /// \param yExtent
        /// \param zExtent
        constexpr OrientedBoundingBox(const std::array<T, 3>& center, const std::array<T, 3>& xDirection,
            const std::array<T, 3>& yDirection, T xExtent, T yExtent, T zExtent)
            : m_center(center)
            , m_xDirection(unitVector(xDirection))
            , m_yDirection(unitVector(yDirection))
            , m_zDirection(crossProduct(xDirection, yDirection))
            , m_xExtent(xExtent)
            , m_yExtent(yExtent)
            , m_zExtent(zExtent) {

            constexpr auto epsilon = std::numeric_limits<T>::epsilon();

            if (!std::isfinite(m_center[0]) || !std::isfinite(m_center[1]) || !std::isfinite(m_center[2])) {
                throw std::runtime_error("Center contains non-finite elements");
            }

            // Sanity check x direction
            if (!std::isfinite(xDirection[0]) || !std::isfinite(xDirection[1]) || !std::isfinite(xDirection[2])) {
                throw std::runtime_error("X direction vector contains non-finite elements");
            }

            if (normL2(xDirection) < epsilon) {
                throw std::runtime_error("xDirection should have larger norm");
            }

            // X and Y should be perpendicular
            if (std::abs(innerProduct(m_xDirection, m_yDirection)) >= epsilon) {
                throw std::runtime_error("Projection of x direction onto y direction is too large -- these should be perpendicular");
            }

            // Sanity check y direction
            if (!std::isfinite(yDirection[0]) || !std::isfinite(yDirection[1]) || !std::isfinite(yDirection[2])) {
                throw std::runtime_error("X direction vector contains non-finite elements");
            }

            if (normL2(yDirection) < epsilon) {
                throw std::runtime_error("yDirection should have larger norm");
            }

            // Sanity check x extent
            if (!std::isfinite(xExtent)) {
                throw std::runtime_error("X extent is not finite");
            }

            if (xExtent <= epsilon) {
                throw std::runtime_error("xExtent must be greater than machine epsilon");
            }

            // Sanity check y extent
            if (!std::isfinite(yExtent)) {
                throw std::runtime_error("Y extent is not finite");
            }

            if (yExtent <= epsilon) {
                throw std::runtime_error("yExtent must be greater than machine epsilon");
            }

            // Sanity check z extent
            if (!std::isfinite(zExtent)) {
                throw std::runtime_error("Z extent is not finite");
            }

            if (zExtent <= epsilon) {
                throw std::runtime_error("yExtent must be greater than machine epsilon");
            }
        }

        const std::array<T, 3> m_center;
        const std::array<T, 3> m_xDirection;
        const std::array<T, 3> m_yDirection;
        const std::array<T, 3> m_zDirection;
        const T m_xExtent;
        const T m_yExtent;
        const T m_zExtent;
    };

    ///
    /// \tparam T
    /// \param center
    /// \param rotation
    /// \param xExtent
    /// \param yExtent
    /// \param zExtent
    /// \return
    template <typename T>
    constexpr OrientedBoundingBox<T> orientedBoundingBox(
        const std::array<T, 3>& center, const std::array<T, 3>& rotation, T xExtent, T yExtent, T zExtent) {
        const auto rotMatrix = rotationMatrixFromRotationVector(rotation);

        const auto alpha = rotMatrix[0 * 3 + 0] * rotMatrix[0 * 3 + 1] + rotMatrix[1 * 3 + 0] * rotMatrix[1 * 3 + 1]
            + rotMatrix[2 * 3 + 0] * rotMatrix[2 * 3 + 1];

        return OrientedBoundingBox<T>(center, std::array<T, 3>{ { rotMatrix[0 * 3 + 0], rotMatrix[1 * 3 + 0], rotMatrix[2 * 3 + 0] } },
            // Gram-Schmidt magic happens here to ensure that these are perpendicular
            std::array<T, 3>{ { rotMatrix[0 * 3 + 1] - alpha * rotMatrix[0 * 3 + 0], rotMatrix[1 * 3 + 1] - alpha * rotMatrix[1 * 3 + 0],
                rotMatrix[2 * 3 + 1] - alpha * rotMatrix[2 * 3 + 0] } },
            xExtent, yExtent, zExtent);
    }

    namespace details {
        template <typename T>
        constexpr int allCornersInSameHalfSpace(
            const std::array<T, 3>& normal, const std::array<T, 3>& origin, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
            int negativeCount = 0;
            int positiveCount = 0;

            for (int c = 0; c < 8; ++c) {
                const std::array<T, 3> pointWrtOrigin{ { boxCenter[0] - origin[0] + ((c & 1) ? 1 : -1) * boxHalfExtent,
                    boxCenter[1] - origin[1] + ((c & 2) ? 1 : -1) * boxHalfExtent,
                    boxCenter[2] - origin[2] + ((c & 4) ? 1 : -1) * boxHalfExtent } };

                const auto d = innerProduct(normal, pointWrtOrigin);

                if (d < 0) {
                    ++negativeCount;
                } else if (d > 0) {
                    ++positiveCount;
                }
            }

            if (positiveCount && negativeCount) {
                return 0;
            } else if (negativeCount) {
                return -1;
            } else {
                return 1;
            }
        }

        template <typename T>
        constexpr bool allBoxCornersInNegativeHalfSpace(
            const OrientedBoundingBox<T>& obb, const std::array<T, 3>& boxCenter, T boxHalfExtent) {

            // clang-format off
                const std::pair <std::array<T, 3>, std::array<T, 3> > boundingPlanePointNormalPairs[] = {
                        {{{obb.m_center[0] + obb.m_zExtent * obb.m_zDirection[0], obb.m_center[1] + obb.m_zExtent * obb.m_zDirection[1], obb.m_center[2] + obb.m_zExtent * obb.m_zDirection[2]}}, {{-obb.m_zDirection[0], -obb.m_zDirection[1], -obb.m_zDirection[2]}}},
                        {{{obb.m_center[0] - obb.m_zExtent * obb.m_zDirection[0], obb.m_center[1] - obb.m_zExtent * obb.m_zDirection[1], obb.m_center[2] - obb.m_zExtent * obb.m_zDirection[2]}}, {{obb.m_zDirection[0],  obb.m_zDirection[1],  obb.m_zDirection[2]}}},
                        {{{obb.m_center[0] + obb.m_yExtent * obb.m_yDirection[0], obb.m_center[1] + obb.m_yExtent * obb.m_yDirection[1], obb.m_center[2] + obb.m_yExtent * obb.m_yDirection[2]}}, {{-obb.m_yDirection[0], -obb.m_yDirection[1], -obb.m_yDirection[2]}}},
                        {{{obb.m_center[0] - obb.m_yExtent * obb.m_yDirection[0], obb.m_center[1] - obb.m_yExtent * obb.m_yDirection[1], obb.m_center[2] - obb.m_yExtent * obb.m_yDirection[2]}}, {{obb.m_yDirection[0],  obb.m_yDirection[1],  obb.m_yDirection[2]}}},
                        {{{obb.m_center[0] + obb.m_xExtent * obb.m_xDirection[0], obb.m_center[1] + obb.m_xExtent * obb.m_xDirection[1], obb.m_center[2] + obb.m_xExtent * obb.m_xDirection[2]}}, {{-obb.m_xDirection[0], -obb.m_xDirection[1], -obb.m_xDirection[2]}}},
                        {{{obb.m_center[0] - obb.m_xExtent * obb.m_xDirection[0], obb.m_center[1] - obb.m_xExtent * obb.m_xDirection[1], obb.m_center[2] - obb.m_xExtent * obb.m_xDirection[2]}}, {{obb.m_xDirection[0],  obb.m_xDirection[1],  obb.m_xDirection[2]}}},
                };

                return (   (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[0].second, boundingPlanePointNormalPairs[0].first, boxCenter, boxHalfExtent) < 0)
                        || (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[1].second, boundingPlanePointNormalPairs[1].first, boxCenter, boxHalfExtent) < 0)
                        || (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[2].second, boundingPlanePointNormalPairs[2].first, boxCenter, boxHalfExtent) < 0)
                        || (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[3].second, boundingPlanePointNormalPairs[3].first, boxCenter, boxHalfExtent) < 0)
                        || (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[4].second, boundingPlanePointNormalPairs[4].first, boxCenter, boxHalfExtent) < 0)
                        || (allCornersInSameHalfSpace(boundingPlanePointNormalPairs[5].second, boundingPlanePointNormalPairs[5].first, boxCenter, boxHalfExtent) < 0));
            // clang-format on
        }

        template <typename T>
        constexpr int allCornersInSameHalfSpace(
            const std::array<T, 3>& normal, const std::array<T, 3>& origin, const OrientedBoundingBox<T>& obb) {
            int positiveCount = 0;
            int negativeCount = 0;

            for (int corner = 0; corner < 8; ++corner) {
                const int xSign = corner & 1 ? 1 : -1;
                const int ySign = corner & 2 ? 1 : -1;
                const int zSign = corner & 4 ? 1 : -1;

                // clang-format off
                    const std::array<T, 3> point{{obb.m_center[0] + xSign * obb.m_xExtent * obb.m_xDirection[0] + ySign * obb.m_yExtent * obb.m_yDirection[0] + zSign * obb.m_zExtent * obb.m_zDirection[0] - origin[0],
                                                  obb.m_center[1] + xSign * obb.m_xExtent * obb.m_xDirection[1] + ySign * obb.m_yExtent * obb.m_yDirection[1] + zSign * obb.m_zExtent * obb.m_zDirection[1] - origin[1],
                                                  obb.m_center[2] + xSign * obb.m_xExtent * obb.m_xDirection[2] + ySign * obb.m_yExtent * obb.m_yDirection[2] + zSign * obb.m_zExtent * obb.m_zDirection[2] - origin[2]}};
                // clang-format on

                const auto d = innerProduct(point, normal);

                if (d < 0) {
                    ++negativeCount;
                } else if (d > 0) {
                    ++positiveCount;
                }
            }

            if (positiveCount && negativeCount) {
                return 0;
            } else if (negativeCount) {
                return -1;
            } else {
                return 1;
            }
        }

        /// Given a box and a frustum, return true if all box corners project into the negative half-space of one
        /// of the frustum planes
        template <typename T>
        constexpr bool allBoxCornersInNegativeHalfSpace(const Frustum<T>& frustum, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
            return ((allCornersInSameHalfSpace(frustum.m_topPlaneNormal, frustum.m_origin, boxCenter, boxHalfExtent) < 0)
                || (allCornersInSameHalfSpace(frustum.m_bottomPlaneNormal, frustum.m_origin, boxCenter, boxHalfExtent) < 0)
                || (allCornersInSameHalfSpace(frustum.m_leftPlaneNormal, frustum.m_origin, boxCenter, boxHalfExtent) < 0)
                || (allCornersInSameHalfSpace(frustum.m_rightPlaneNormal, frustum.m_origin, boxCenter, boxHalfExtent) < 0));
        }

        template <typename T>
        constexpr int allCornersInSameHalfSpace(const std::array<T, 3>& normal, const std::array<T, 3>& origin, const Frustum<T>& frustum) {
            int positiveCount = 0;
            int negativeCount = 0;

            const std::array<T, 3> apex{ { frustum.m_origin[0] - origin[0], frustum.m_origin[1] - origin[1],
                frustum.m_origin[2] - origin[2] } };

            auto d = innerProduct(apex, normal);

            if (d < 0) {
                ++negativeCount;
            } else if (d > 0) {
                ++positiveCount;
            }

            d = innerProduct(normal, frustum.m_topPlaneNormal);
            if (d < 0) {
                ++negativeCount;
            } else {
                ++positiveCount;
            }

            d = innerProduct(normal, frustum.m_bottomPlaneNormal);
            if (d < 0) {
                ++negativeCount;
            } else {
                ++positiveCount;
            }

            d = innerProduct(normal, frustum.m_leftPlaneNormal);
            if (d < 0) {
                ++negativeCount;
            } else {
                ++positiveCount;
            }

            d = innerProduct(normal, frustum.m_rightPlaneNormal);
            if (d < 0) {
                ++negativeCount;
            } else {
                ++positiveCount;
            }

            if (positiveCount && negativeCount) {
                return 0;
            } else if (negativeCount) {
                return -1;
            } else {
                return 1;
            }
        }

        template <typename T>
        constexpr bool allFrustumCornersInNegativeHalfSpace(const Frustum<T>& frustum, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
            // clang-format off
                const std::array<T, 3> boxPlanes[6][2]{
                        {{{1,  0,  0}},  {{boxCenter[0] - boxHalfExtent, boxCenter[1],                 boxCenter[2]}}},
                        {{{-1, 0,  0}},  {{boxCenter[0] + boxHalfExtent, boxCenter[1],                 boxCenter[2]}}},
                        {{{0,  1,  0}},  {{boxCenter[0],                 boxCenter[1] - boxHalfExtent, boxCenter[2]}}},
                        {{{0,  -1, 0}},  {{boxCenter[0],                 boxCenter[1] + boxHalfExtent, boxCenter[2]}}},
                        {{{0,  0,  1}},  {{boxCenter[0],                 boxCenter[1],                 boxCenter[2] - boxHalfExtent}}},
                        {{{0,  0,  -1}}, {{boxCenter[0],                 boxCenter[1],                 boxCenter[2] + boxHalfExtent}}}};
            // clang-format on

            for (int f = 0; f < 6; ++f) {
                const auto& normal = boxPlanes[f][0];
                const auto& point = boxPlanes[f][1];

                if (allCornersInSameHalfSpace(normal, point, frustum) < 0) {
                    return true;
                }
            }

            return false;
        }

        template <typename T>
        constexpr bool allOBBCornersInNegativeHalfSpace(
            const OrientedBoundingBox<T>& obb, const std::array<T, 3>& boxCenter, T boxHalfExtent) {
            // clang-format off
                const std::array<T, 3> boxPlanes[6][2]{
                        {{{1,  0,  0}},  {{boxCenter[0] - boxHalfExtent, boxCenter[1],                 boxCenter[2]}}},
                        {{{-1, 0,  0}},  {{boxCenter[0] + boxHalfExtent, boxCenter[1],                 boxCenter[2]}}},
                        {{{0,  1,  0}},  {{boxCenter[0],                 boxCenter[1] - boxHalfExtent, boxCenter[2]}}},
                        {{{0,  -1, 0}},  {{boxCenter[0],                 boxCenter[1] + boxHalfExtent, boxCenter[2]}}},
                        {{{0,  0,  1}},  {{boxCenter[0],                 boxCenter[1],                 boxCenter[2] - boxHalfExtent}}},
                        {{{0,  0,  -1}}, {{boxCenter[0],                 boxCenter[1],                 boxCenter[2] + boxHalfExtent}}}};
            // clang-format on

            for (int f = 0; f < 6; ++f) {
                const auto& normal = boxPlanes[f][0];
                const auto& point = boxPlanes[f][1];

                if (allCornersInSameHalfSpace(normal, point, obb) < 0) {
                    return true;
                }
            }

            return false;
        }
    }

    /// Determine whether a frustum intersects an axis-aligned bounding box.
    ///
    /// \tparam T
    /// \param frustum
    /// \param boxCenter
    /// \param boxHalfExtent
    /// \return
    template <typename T>
    constexpr bool frustumIntersectsAxisAlignedBoundingBox(const Frustum<T>& frustum, const std::array<T, 3>& boxCenter, T boxHalfExtent) {

        // Apply the separating axis theorem (https://en.wikipedia.org/wiki/Hyperplane_separation_theorem). Step 1: see if all
        // the corners from the box lie in the negative half-space of any of the frustum planes. If so, things are definitely
        // disjoint.
        //
        // Because the frustum is a convex polytope, it is always in the positive half-space of one of its faces, so
        // we need only check to see if the box lies in a negative half-space
        if (details::allBoxCornersInNegativeHalfSpace(frustum, boxCenter, boxHalfExtent)) {
            return false;
        }

        // Step 2: see if the frustum lies entirely in one of the negative half-spaces of the box faces. If so, things
        // are definitely disjoint.
        //
        // Same reasoning as above: the box is, trivially, in the positive half-space of all of its faces, hence we need
        // only check to see if the frustum lies entirely in the negative half-space of one of its faces.
        if (details::allFrustumCornersInNegativeHalfSpace(frustum, boxCenter, boxHalfExtent)) {
            return false;
        }

        // Step 3: Consider all axes perpendicular to all pairs of (box edge, frustum edge). If we find an axis such that
        // all corners from the box project into the opposite half-space as the frustum, they are disjoint.
        //
        // We reconstruct the edge directions of the frustum from the normals by making the following observations:
        // 1. We know that any edge of the frustum must pass through its apex. This suggests that the edge is a line
        //    in the form sX + p where X is the direction of the edge and p is the apex of the frustum
        // 2. We know that all points on the line need to be contained in both planes. This gives us:
        //    <sX + p - p, a> = 0 (where a is the first face normal) and <sX + p - p, b> = 0, where b is the adjacent face
        //    normal.
        // 3. Simplifying the above, we get s<X,a> = s<X,b> = 0 --> X = a cross b
        const std::array<T, 3> frustumEdges[] = { crossProduct(frustum.m_topPlaneNormal, frustum.m_leftPlaneNormal),
            crossProduct(frustum.m_topPlaneNormal, frustum.m_rightPlaneNormal),
            crossProduct(frustum.m_bottomPlaneNormal, frustum.m_leftPlaneNormal),
            crossProduct(frustum.m_bottomPlaneNormal, frustum.m_rightPlaneNormal) };

        constexpr std::array<T, 3> boxEdges[] = { { { 1, 0, 0 } }, { { 0, 1, 0 } }, { { 0, 0, 1 } } };

        for (const auto& fe : frustumEdges) {
            for (const auto& be : boxEdges) {
                const auto normal = crossProduct(fe, be);

                const auto boxSide = details::allCornersInSameHalfSpace(normal, frustum.m_origin, boxCenter, boxHalfExtent);

                if (0 == boxSide) {
                    continue;
                }

                const auto frustumSide = details::allCornersInSameHalfSpace(normal, frustum.m_origin, frustum);

                if (0 == frustumSide) {
                    continue;
                }

                // Did we find a separating axis? If so, these objects do not intersect
                if (boxSide * frustumSide < 0) {
                    return false;
                }
            }
        }

        // We've exhausted all possible separating axes, so we must have some intersection even if only at a point
        return true;
    }

    template <typename T>
    constexpr bool obbIntersectsAxisAlignedBoundingBox(
        const OrientedBoundingBox<T>& obb, const std::array<T, 3>& boxCenter, T boxHalfExtent) {

        // Apply the separating axis theorem (https://en.wikipedia.org/wiki/Hyperplane_separation_theorem). Step 1: see if all
        // the corners from the box lie in the negative half-space of any of the obb planes. If so, things are definitely
        // disjoint.
        //
        // Because the obb is a convex polytope, it is always in the positive half-space of one of its faces, so
        // we need only check to see if the box lies in a negative half-space
        if (details::allBoxCornersInNegativeHalfSpace(obb, boxCenter, boxHalfExtent)) {
            return false;
        }

        // Step 2: see if the obb lies entirely in one of the negative half-spaces of the box faces. If so, things
        // are definitely disjoint.
        //
        // Same reasoning as above: the box is, trivially, in the positive half-space of all of its faces, hence we need
        // only check to see if the obb lies entirely in the negative half-space of one of its faces.
        if (details::allOBBCornersInNegativeHalfSpace(obb, boxCenter, boxHalfExtent)) {
            return false;
        }

        // Step 3: Consider all axes perpendicular to all pairs of (box edge, obb edge). If we find an axis such that
        // all corners from the box project into the opposite half-space as the frustum, they are disjoint.
        const std::array<T, 3> boxCorner{ { boxCenter[0] - boxHalfExtent, boxCenter[1] - boxHalfExtent, boxCenter[2] - boxHalfExtent } };

        constexpr std::array<T, 3> boxEdges[] = { { { 1, 0, 0 } }, { { 0, 1, 0 } }, { { 0, 0, 1 } } };

        const std::array<T, 3> obbEdges[] = { obb.m_xDirection, obb.m_yDirection, obb.m_zDirection };

        for (const auto& be : boxEdges) {
            for (const auto& oe : obbEdges) {
                const auto normal = crossProduct(be, oe);

                const auto boxSide = details::allCornersInSameHalfSpace(normal, boxCorner, boxCenter, boxHalfExtent);

                if (0 == boxSide) {
                    continue;
                }

                const auto obbSide = details::allCornersInSameHalfSpace(normal, boxCorner, obb);

                if (0 == obbSide) {
                    continue;
                }

                if (boxSide * obbSide < 0) {
                    return false;
                }
            }
        }

        return true;
    }
}
}