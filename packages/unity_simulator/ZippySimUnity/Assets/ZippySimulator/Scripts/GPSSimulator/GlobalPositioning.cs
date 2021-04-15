using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    //http://www.movable-type.co.uk/scripts/latlong.html
    public static class GlobalPositioning {
        public static readonly double EARTH_RADIUS_METRES = 6378137.0; //WGS84 equatorial radius
        public static readonly double PI_BY_2 = Math.PI * 0.5;
        public static readonly double PI_BY_4 = Math.PI * 0.25;
        public static readonly double RAD_TO_DEG = 180.0 / Math.PI;
        public static readonly double DEG_TO_RAD = Math.PI / 180.0;

        static Vector2 _north = new Vector2 (0, 1);

        /// <summary>
        /// Distance between two locations using Haversine formula.
        /// </summary>
        /// <returns>The distance between two locations.</returns>
        /// <param name="position1">Position1.</param>
        /// <param name="position2">Position2.</param>
        public static double DistanceBetweenTwoLocations(GlobalPosition position1, GlobalPosition position2) {
            double lat1 = position1.LatitudeInRadians;
            double lat2 = position2.LatitudeInRadians;
            double long1 = position1.LongitudeInRadians;
            double long2 = position2.LongitudeInRadians;
            double deltaLat = lat2 - lat1;
            double deltaLong = long2 - long1;
            double halfDeltaLat = deltaLat * 0.5;
            double halfDeltaLong = deltaLong * 0.5;
            double sinHalfDeltaLat = Math.Sin (halfDeltaLat);
            double sinHalfDeltaLong = Math.Sin (halfDeltaLong);
            double a = sinHalfDeltaLat * sinHalfDeltaLat +
                       Math.Cos(lat1) * Math.Cos(lat2) * sinHalfDeltaLong * sinHalfDeltaLong;
            double c = 2.0 * Math.Atan2(Math.Sqrt(a), Math.Sqrt(1.0 - a));
            double d = EARTH_RADIUS_METRES * c;
            return d;
        }

        /// <summary>
        /// Bearing in degrees between two locations.
        /// </summary>
        /// <returns>The bearing between two locations.</returns>
        /// <param name="position1">Position1.</param>
        /// <param name="position2">Position2.</param>
        public static double BearingBetweenTwoLocations(GlobalPosition position1, GlobalPosition position2) {
            double lat1 = position1.LatitudeInRadians;
            double lat2 = position2.LatitudeInRadians;
            double long1 = position1.LongitudeInRadians;
            double long2 = position2.LongitudeInRadians;
            double deltaLong = long2 - long1;
            double x = Math.Cos(lat1) * Math.Sin(lat2) -
                       Math.Sin(lat1) * Math.Cos(lat2) * Math.Cos(deltaLong);
            double y = Math.Sin(deltaLong) * Math.Cos(lat2);
            double bearing = Math.Atan2(y, x) * RAD_TO_DEG;
            bearing = (bearing + 360.0f) % 360;
            return bearing;
        }

        /// <summary>
        /// Calculate the bearing of a position given a ref position
        /// </summary>
        /// <param name="refPosition">Reference position.</param>
        /// <param name="position">Position.</param>
        static float Bearing(Vector2 refPosition, Vector2 position) {
            Vector2 dir = position - refPosition;
            var angle = Vector2.Angle (_north, dir);
            var cross = Vector3.Cross (_north, dir);

            if (cross.z > 0) {
                angle *= -1.0f;
            }

            return angle;
        }

        /// <summary>
        /// Distance from the refPostion to the specified position
        /// </summary>
        /// <param name="refPosition">Reference position.</param>
        /// <param name="position">Position.</param>
        static float Distance(Vector2 refPosition, Vector2 position) {
            return Vector2.Distance (refPosition, position);
        }

        /// <summary>
        /// Convert a 3D postion in to a planar 2D position
        /// X_3d => X_2d. Z_3D => Y_2d, Y_3d is ignored
        /// </summary>
        /// <returns>Planar position.</returns>
        /// <param name="position">Position.</param>
        static Vector2 Convert3DtoPlanar(Vector3 position) {
            return new Vector2 (position.x, position.z);
        }

        /// <summary>
        /// Calculate the global position given a reference starting location and a current Unity position
        /// Uses Rhumb Line calculation
        /// </summary>
        /// <returns>The new global position</returns>
        /// <param name="refGlobalPosition">Reference global position.</param>
        /// <param name="refPosition">Reference position.</param>
        /// <param name="position">Position.</param>
        public static GlobalPosition GlobalPositionFromUnityPosition(GlobalPosition refGlobalPosition, Vector3 refPosition, Vector3 position) {
            Vector2 from = Convert3DtoPlanar(refPosition);
            Vector2 to = Convert3DtoPlanar(position);
            double bearing = Bearing (from, to) * DEG_TO_RAD;
            double distance = Distance (from, to);
            return GlobalPositionFromUnityPosition (refGlobalPosition, refPosition, bearing, distance, position.y);
        }

        /// <summary>
        /// Calculate the global position given a reference starting location and a bearing and distance.
        /// Uses Rhumb Line calculation
        /// </summary>
        /// <returns>The new Global position.</returns>
        /// <param name="refGlobalPosition">Reference global position.</param>
        /// <param name="refPosition">Reference position.</param>
        /// <param name="bearingRad">Bearing in radians.</param>
        /// <param name="distance">Distance in meters.</param>
        /// /// <param name="altitude">Height in meters.</param>
        public static GlobalPosition GlobalPositionFromUnityPosition(GlobalPosition refGlobalPosition, Vector3 refPosition, double bearingRad, double distance, double altitude) {
            var angularDistance = distance / EARTH_RADIUS_METRES; // angular distance in radians
            var lat = refGlobalPosition.LatitudeInRadians;
            var lon = refGlobalPosition.LongitudeInRadians;
            var deltaLat = angularDistance * Math.Cos(bearingRad);
            var lat2 = lat + deltaLat;

            // check for going past the pole, normalise latitude if so
            if (lat2 > PI_BY_2) {
                lat2 = Math.PI - lat2;
            }
            else if (lat2 < -PI_BY_2) {
                lat2 = -Math.PI - lat2;
            }

            var deltaPsi = Math.Log(Math.Tan(lat2 * 0.5 + PI_BY_4) / Math.Tan(lat * 0.5 + PI_BY_4));
            var q = Math.Abs(deltaPsi) > 10e-12 ? deltaLat / deltaPsi : Math.Cos(lat); // E-W course becomes ill-conditioned with 0/0
            var deltaLon = angularDistance * Math.Sin(bearingRad) / q;
            var lon2 = lon + deltaLon;
            var newLatDeg = lat2 * 180.0 / Math.PI;
            var newLonDeg = ((lon2 * 180.0 / Math.PI) + 540) % 360 - 180; // normalise to −180..+180°
            altitude -= refPosition.y;
            var globalPosition = new GlobalPosition (newLatDeg, newLonDeg, altitude);
            return globalPosition;
        }
    }
}
