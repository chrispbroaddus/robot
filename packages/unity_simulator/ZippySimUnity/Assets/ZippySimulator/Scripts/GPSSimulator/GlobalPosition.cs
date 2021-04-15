using System;
using UnityEngine;

namespace Zippy {
    public class GlobalPosition {
        double _latitudeInDegrees = 0.0;
        double _longitudeInDegrees = 0.0;

        public GlobalPosition(double latitudeDegrees, double longitudeDegrees, double altitudeMeters) {
            LatitudeInDegrees = latitudeDegrees;
            LongitudeInDegrees = longitudeDegrees;
            AltitudeInMeters = altitudeMeters;
            Timestamp = Time.realtimeSinceStartup;
        }

        public GlobalPosition(string latLonAltString) {
            if (string.IsNullOrEmpty(latLonAltString)) {
                throw new ArgumentNullException("Must have a valid latLonAlt string: \"lat, lon, alt\"");
            }

            var latLonAltSplit = latLonAltString.Split(',');

            if (latLonAltSplit.Length != 3) {
                throw new ArgumentException("Must have a valid latLonAlt string: \"lat, lon, alt\"");
            }

            double lat, lon, alt;

            if (double.TryParse (latLonAltSplit [0], out lat)) {
                LatitudeInDegrees = lat;
            }
            else {
                throw new ArgumentException("Latitude must be a valid double");
            }

            if (double.TryParse (latLonAltSplit [1], out lon)) {
                LongitudeInDegrees = lon;
            }
            else {
                throw new ArgumentException("Longitude must be a valid double");
            }

            if (double.TryParse (latLonAltSplit [2], out alt)) {
                AltitudeInMeters = alt;
            }
            else {
                throw new ArgumentException("Altitude must be a valid double");
            }

            Timestamp = Time.realtimeSinceStartup;
        }

        bool InRange(double value, double min, double max) {
            return value >= min && value <= max;
        }

        public double LatitudeInDegrees {
            get {
                return _latitudeInDegrees;
            }

            private set {
                if (InRange (value, -90, 90)) {
                    _latitudeInDegrees = value;
                }
                else {
                    throw new System.ArgumentOutOfRangeException ("Latitude must be in range of -90:90. Value was: " + value);
                }
            }
        }

        public double LongitudeInDegrees {
            get {
                return _longitudeInDegrees;
            }

            private set {
                if (InRange (value, -180, 180)) {
                    _longitudeInDegrees = value;
                }
                else {
                    throw new System.ArgumentOutOfRangeException ("Longitude must be in range of -180:180. Value was: " + value);
                }
            }
        }

        public double AltitudeInMeters {
            get;
            private set;
        }

        public float Timestamp {
            get;
            private set;
        }
        public double LatitudeInRadians {
            get {
                return LatitudeInDegrees * GlobalPositioning.DEG_TO_RAD;
            }
        }

        public double LongitudeInRadians {
            get {
                return LongitudeInDegrees * GlobalPositioning.DEG_TO_RAD;
            }
        }

        public override System.String ToString () {
            return LatitudeInDegrees.ToString("0.000000") + "°, " + LongitudeInDegrees.ToString("0.000000") + "°, " + AltitudeInMeters.ToString("0.000000") + "m";
        }

        public GPSReading ConvertToGPSReading() {
            var reading = new GPSReading ();
            reading.latitude = LatitudeInDegrees;
            reading.longitude = LongitudeInDegrees;
            reading.altitude = AltitudeInMeters;
            reading.timestamp = Timestamp;
            return reading;
        }
    }
}
