/// @file
/// @copyright
/// (C) Commsignia Ltd. - All Rights Reserved.
/// Unauthorised copying of this file, via any medium is strictly prohibited.
/// Proprietary and confidential.
/// @author Balazs Toth \<balazs.toth@commsignia.com\>
/// @date 2019

#ifndef __GEODESY__H__
#define __GEODESY__H__

#include <cmath>
#include <utility>
#include <vector>


namespace Geodesy {

/// @name Constants and conversions
/// @{

#ifdef M_PI
static constexpr double PI = M_PI;
#else
static constexpr double PI = 3.14159265358979323846;
#endif

inline constexpr double degToRad(double deg) {
    return PI / 180.0 * deg;
}

inline constexpr double radToDeg(double rad) {
    return 180.0 / PI * rad;
}

inline double angleDiff(double x, double y) {
    return std::fmod(((3.0 * PI) + x - y), (2.0 * PI)) - PI;
}

/// @}

/// @name Position and helpers
/// @{

/// Geosherical position.
/// Uses radian representation.
struct Position {
    double latitude;
    double longitude;
};


inline constexpr Position positionFromDeg(double lat, double lon) {
    return {degToRad(lat), degToRad(lon)};
}

template<typename T>
inline constexpr Position positionFromDeg(T pos) {
    return {degToRad(pos.latitude), degToRad(pos.longitude)};
}


inline constexpr Position positionFromRad(double lat, double lon) {
    return {lat, lon};
}

template<typename T>
inline constexpr Position positionFromRad(T pos) {
    return {pos.latitude, pos.longitude};
}


inline constexpr bool isValid(double val) {
    return !std::isnan(val);
}

inline constexpr bool isValid(Position pos) {
    return isValid(pos.latitude) && isValid(pos.longitude);
}
/// @}

/// @name Point and helpers
/// @{

/// Cartesian point.
/// Unit: meters.
struct Point {
    double x;
    double y;
};

Point relativePosition(const Position& ref, double direction, const Position& pos);


inline Point relativePosition(const Position& ref, const Position& pos) {
    return relativePosition(ref, 0.0, pos);
}
/// @}


/// @name Utilities
/// @{

double distance(const Position& p1, const Position& p2);
double initialBearing(const Position& p1, const Position& p2);
double relDirection(double heading1, double heading2); 

Position offset(const Position& ref, double bearing, double distance);
Position offset(const Position& ref, double direction, const Point& relPos);

inline Position offset(const Position& ref, const Point& relPos) {
    return offset(ref, 0.0, relPos);
}

}  // namespace Geodesy

/// @}


#endif  // __GEODESY__H__
