/// @file
/// @copyright
/// (C) Commsignia Ltd. - All Rights Reserved.
/// Unauthorised copying of this file, via any medium is strictly prohibited.
/// Proprietary and confidential.
/// @author Balazs Toth \<balazs.toth@commsignia.com\>
/// @date 2019

#include "Geodesy.h"

using Geodesy::Position;
using Geodesy::Point;


namespace {

constexpr double EARTH_MERIDIAN = 6371008.0; //meters

} // namespace


namespace Geodesy {


Position delta(const Position& p1, const Position& p2) 
{
    return {(p1.latitude - p2.latitude), (p1.longitude - p2.longitude)};
}


struct LatLonSclCoeff {
    double latScl;
    double lonScl;
};


LatLonSclCoeff scaleLatLon(const Position& pos) 
{
    return {EARTH_MERIDIAN, (EARTH_MERIDIAN * std::cos(pos.latitude))};
}


double angularDistance(const Position& p1, const Position& p2) 
{
    auto deltaP = delta(p2, p1);
    double sLatSq = std::sin(0.5 * deltaP.latitude);
    sLatSq *= sLatSq;
    double sLonSq = std::sin(0.5 * deltaP.longitude);
    sLonSq *= sLonSq;
    double a = sLatSq + std::cos(p1.latitude) * std::cos(p2.latitude) * sLonSq;
    double deltaAng = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    return deltaAng;
}


static constexpr double COORD_EPS = degToRad(1e-8);


bool closeToZero(double val, double eps = COORD_EPS) {

    return std::fabs(val) < eps;
}


Position transform(const Position& pos, const Position& offset, double angle) {

    double cAng = std::cos(angle);
    double sAng = std::sin(angle);
    double lat = (cAng * pos.latitude) - (sAng * pos.longitude) + offset.latitude;
    double lon = (sAng * pos.latitude) + (cAng * pos.longitude) + offset.longitude;

    return {lat, lon};
}

}  // namespace Geodesy


double Geodesy::distance(const Position& p1, const Position& p2) 
{

    return EARTH_MERIDIAN * angularDistance(p1, p2);
}


double Geodesy::initialBearing(const Position& p1, const Position& p2) {

    double deltaLon = p2.longitude - p1.longitude;
    double sDeltaLon = std::sin(deltaLon);
    double cDeltaLon = std::cos(deltaLon);
    double cLat2 = std::cos(p2.latitude);
// X = COS(lat1)*SIN(lat2)-SIN(lat1)*COS(lat2)*COS(lon2-lon1),
// Y = SIN(lon2-lon1)*COS(lat2))
    double bearing = std::atan2(sDeltaLon * cLat2,
                                (std::cos(p1.latitude) * std::sin(p2.latitude))
                                    - (std::sin(p1.latitude) * cLat2 * cDeltaLon));

    return bearing;
}


Position Geodesy::offset(const Position& ref, double bearing, double distance) {

    double delta = distance / EARTH_MERIDIAN;
    double cDelta = std::cos(delta);
    double sDelta = std::sin(delta);
    double cLat1 = std::cos(ref.latitude);
    double sLat1 = std::sin(ref.latitude);

    double lat = std::asin((sLat1 * cDelta) + (cLat1 * sDelta * std::cos(bearing)));
    double lon = ref.longitude
                 + std::atan2((std::sin(bearing) * sDelta * cLat1),
                              (cDelta - (sLat1 * std::sin(lat))));

    return {lat, lon};
}


Position Geodesy::offset(const Position& ref, double direction, const Point& relPos) {

    double relDist = std::sqrt((relPos.x * relPos.x) + (relPos.y * relPos.y));
    double bearing = std::atan2(-relPos.y, relPos.x) + direction;
    return offset(ref, bearing, relDist);
}


Point Geodesy::relativePosition(const Position& ref,double direction,const Position& pos) {

    LatLonSclCoeff scl = scaleLatLon(ref);
    Position deltaP = delta(pos, ref);
    Position transScl = {(deltaP.latitude * scl.latScl), (deltaP.longitude * scl.lonScl)};
    transScl = transform(transScl, {0.0, 0.0}, (-1.0 * direction));
    Position trans = {((transScl.latitude / scl.latScl) + ref.latitude),
                      ((transScl.longitude / scl.lonScl) + ref.longitude)};

    double x = distance(ref, {trans.latitude, ref.longitude});
    if (transScl.latitude < 0.0) {
        x = -x;
    }

    double y = distance(ref, {ref.latitude, trans.longitude});
    if (0.0 < transScl.longitude) {
        y = -y;
    }

    return {x, y};
}

double Geodesy::relDirection(double heading1, double heading2) 
{
    // Normalize the headings to be in the range [0, 360]
    heading1 = fmod(heading1, 360.0);
    heading2 = fmod(heading2, 360.0);

    // Calculate the absolute difference between the headings
    double diff = std::abs(heading1 - heading2);

    // Normalize the difference to be in the range [0, 180)
    if (diff >= 180.0) 
    {
        diff = 360.0 - diff;
    }
    return diff;
}

