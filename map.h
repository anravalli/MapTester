#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include "pugixml.hpp"
#include "Geodesy.h"

constexpr int street_to_side_line = 3; //meters

struct Street {
    std::string name;
    std::string type; // e.g., "primary", "secondary", "residential"
    // Add other relevant attributes as needed (e.g., length, points)
    // For points, you might want to use a more robust data structure
    // like a std::vector of pairs (latitude, longitude)
    std::vector<Geodesy::Position> points;
};

struct SegmentInfo {
    Geodesy::Point relative_position;
    double lenght;
    std::string street_name;
    int street_idx;
    int start_idx;
    Geodesy::Position start;
    Geodesy::Position end;
};

Geodesy::Position positionDegToRad (const Geodesy::Position &point);

class Rect
{
public:
    Rect(){};
    Rect(Geodesy::Point tl, Geodesy::Point br): m_top_left(tl), m_bottom_right(br) { }
    Rect(double tl_x, double tl_y, double width, double height)
    {
        m_top_left.x = tl_x;
        m_top_left.y = tl_y;
        m_bottom_right.x = tl_x + width;
        m_bottom_right.y = tl_y + height;
    }

    Geodesy::Point topLeft()
    {
        return m_top_left;
    }

    Geodesy::Point bottomRight()
    {
        return m_bottom_right;
    }

    double width()
    {
        return m_bottom_right.x - m_top_left.x;
    }
    double height()
    {
        return m_bottom_right.y - m_top_left.y;
    }
    Geodesy::Point center()
    {
        Geodesy::Point c = {
            m_bottom_right.x - m_top_left.x/2,
            m_bottom_right.y - m_top_left.y/2
        };
        return c;
    }

    Rect &operator = (const Rect &other)
    {
        m_top_left.x = other.m_top_left.x;
        m_top_left.y = other.m_top_left.y;
        m_bottom_right.x = other.m_bottom_right.x;
        m_bottom_right.y = other.m_bottom_right.y;

        return *this;
    }

private:
    Geodesy::Point m_top_left;
    Geodesy::Point m_bottom_right;
};

class Map
{
public:
    Map();
    void loadMap(const std::string &osmFile);
    void printStreets(bool verbose = false);

    std::vector<Street> streets() const;

    Geodesy::Point top_left() ;

    Geodesy::Point bottom_right() ;

    double width() ;
    double height() ;

    Rect rect() ;

    bool matchEgoToMap(Geodesy::Position p, SegmentInfo &segment);
    bool matchOtherToMap(Geodesy::Position p, SegmentInfo &segment);

private:
    void extractStreets(pugi::xml_document &doc);
    void extractBounds(pugi::xml_document &doc);
    bool isStreet(std::string type);
    bool matchToMap(Geodesy::Position p, SegmentInfo &segment);
    bool mapToSegment(const Geodesy::Position& point, SegmentInfo &segment);

    std::string m_map_file = "";

    std::vector<Street> m_streets;
    Rect m_rect;

};

#endif // MAP_H
