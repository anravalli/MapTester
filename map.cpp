#include "map.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include "Geodesy.h"

using namespace Geodesy;

void testFunc()
{
    Geodesy::Position a = {Geodesy::degToRad(45.4355), Geodesy::degToRad(9.1772)};
    Geodesy::Position b = {Geodesy::degToRad(45.4353), Geodesy::degToRad(9.17552)};
    double bearing = Geodesy::initialBearing(a, b);
    std::cout << "bearing: " << Geodesy::radToDeg(bearing) << " (" << bearing << ")" << std::endl;
    auto relative_end = Geodesy::relativePosition(a, bearing, b);
    std::cout << "relative end - x: " << relative_end.x << ", y: " << relative_end.y << std::endl;
    relative_end = Geodesy::relativePosition(a, Geodesy::degToRad(-99.628), b);
    std::cout << "(99) relative end - x: " << relative_end.x << ", y: " << relative_end.y << std::endl;
}

Map::Map()
{
    m_rect = Rect(0,0, 0.1,0.1);
    //testFunc();
}

void Map::loadMap(const std::string &osmFile)
{
    m_streets.clear();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(osmFile.c_str());

    if (result.status != pugi::status_ok) {
        std::cerr << "Error loading OSM file: " << result.description() << std::endl;
        return;
    }

    extractBounds(doc);
    extractStreets(doc);
    if (!m_streets.empty()) {
        printStreets();
    }
}

void Map::extractStreets(pugi::xml_document &doc)
{
    /*
     * pugixml implementation
    */

    Street currentStreet;
    // XPath query to find all way elements with a "highway" tag
    for (auto way : doc.select_nodes("//way[tag[@k='highway']]")) {


        // Extract street name (if available)
        pugi::xml_node nameTag =
            way.node().find_child( [](pugi::xml_node n){
                return not std::strcmp(n.name(), "tag") && not std::strcmp(n.attribute("k").value(),"name");

            } );
        if (nameTag) {
            currentStreet.name = nameTag.attribute("v").value();
        }

        // Extract street type
        pugi::xml_node highwayTag =
            way.node().find_child( [](pugi::xml_node n){
                return not std::strcmp(n.name(), "tag") && not std::strcmp(n.attribute("k").value(),"highway");
            } );
        if (highwayTag) {
            currentStreet.type = highwayTag.attribute("v").value();
        }

        if (isStreet(currentStreet.type)) {
            // Extract coordinates from nd nodes
            for (auto nd : way.node().select_nodes("nd")) {
                std::string ref = nd.node().attribute("ref").value();
                // Find the corresponding node element to get coordinates
                for (auto node : doc.select_nodes(std::string("//node[@id='" + ref + "']").c_str())) {
                    double lat = node.node().attribute("lat").as_double();
                    double lon = node.node().attribute("lon").as_double();
                    // std::cout << "lat as string: " << node.node().attribute("lat").as_string() << std::endl;
                    // std::cout << "lat as string: " << node.node().attribute("lon").as_string() << std::endl;
                    // std::string s_lat = node.node().attribute("lat").as_string();
                    // std::string s_lon = node.node().attribute("lon").as_string();
                    // double lon = std::stod(s_lon);
                    // double lat = std::stod(s_lat);
                    currentStreet.points.push_back({lat, lon});
                    break; // Exit inner loop once node is found
                }
            }
            m_streets.push_back(currentStreet);
        }
    }

}

void Map::extractBounds(pugi::xml_document &doc)
{

    //<bounds minlat="39.5291700" minlon="-122.3498900" maxlat="39.5467100" maxlon="-122.3172300"/>
    for (auto bound : doc.select_nodes("//bounds")) {
        double min_lat = bound.node().attribute("minlat").as_double();
        double max_lat = bound.node().attribute("maxlat").as_double();
        double min_lon = bound.node().attribute("minlon").as_double();
        double max_lon = bound.node().attribute("maxlon").as_double();
        std::cout << "Map Bounds: min_lat:" << min_lat << ", min_lon: " <<min_lon << "max_lat:" <<
                    max_lat << " max_lon: " << max_lon << std::endl;
        m_rect = Rect({min_lon, min_lat}, {max_lon, max_lat});
        break;
    }
}

bool Map::isStreet(std::string type)
{
    bool is_street = false;
    if(not type.empty()){
        if((type == "residential") or
                (type == "primary") or
                (type == "secondary") or
                (type == "tertiary") or
                (type == "tertiary_link") or
                (type == "unclassified") or
                (type == "service") or
                (type == "track") or
                (type == "raceway"))
            is_street = true;
    }
    return is_street;
}

Geodesy::Position positionDegToRad (const Geodesy::Position &point)
{
    Geodesy::Position pos;
    pos.latitude = Geodesy::degToRad(point.latitude);
    pos.longitude = Geodesy::degToRad(point.longitude);
    return pos;
}

bool Map::mapToSegment(const Geodesy::Position &point, SegmentInfo &segment)
{
    bool matched = false;
    //Geodesy::Position has lat and lon swapped
    Geodesy::Position g_start = positionDegToRad(segment.start);
    Geodesy::Position g_end = positionDegToRad(segment.end);
    Geodesy::Position g_p = positionDegToRad(point);
    double segment_bearing = Geodesy::initialBearing(g_start, g_end);
    //double segment_direction = std::atan2(dy,dx);
    Geodesy::Point relative_p = Geodesy::relativePosition(g_start, segment_bearing, g_p);
    Geodesy::Point relative_end = Geodesy::relativePosition(g_start, segment_bearing, g_end);

    if(relative_p.x >= 0 and relative_p.x <= relative_end.x
            and fabs(relative_p.y) < street_to_side_line)
    {
        matched = true;
        segment.lenght = relative_end.x;
        segment.relative_position =relative_p;
        std::cout << "matched: " << matched << std::endl;
    }
    return matched;
}

bool Map::matchToMap(Geodesy::Position p, SegmentInfo &segment)
{
    // bool matched = false;
    for(int i=0; i < m_streets.size(); i++)
    {
        auto street = m_streets[i];
        //double min_distance = 0xFFFFFFFF;
        for(int j=0; j<street.points.size()-1; j++)
        {
            segment.start = street.points[j];
            segment.end = street.points[j+1];
            // matched = ;
            if(mapToSegment(p, segment)){
                segment.start_idx = j;
                segment.street_idx = i;
                segment.street_name = street.name.c_str();
                std::cout << "street #" << segment.street_idx << ": " << segment.street_name << std::endl;
                return true; //assume only one match is possible
            }
        }
    }
    return false;
}

bool Map::matchEgoToMap(const Geodesy::Position p, SegmentInfo &segment)
{
    std::cout << "Matching EGO to map" << std::endl;
    //SegmentInfo segment;
    return matchToMap(p, segment);
}

bool Map::matchOtherToMap(const Geodesy::Position p, SegmentInfo &segment)
{
    std::cout << "Matching OTHER to map" << std::endl;
    // SegmentInfo segment; // = {0};
    return matchToMap(p, segment);
}

Rect Map::rect()
{
    return m_rect;
}

double Map::width()
{
    return m_rect.width();
}

double Map::height()
{
    return m_rect.height();
}

Geodesy::Point Map::bottom_right()
{
    return m_rect.bottomRight();
}

Geodesy::Point Map::top_left()
{
    return m_rect.topLeft();
}

std::vector<Street> Map::streets() const
{
    return m_streets;
}

void Map::printStreets(bool verbose)
{

    // Print extracted data
    for (const auto& street : m_streets) {
        std::cout << "street: " << street.name << " (" << street.type << ")" << std::endl;
        if(verbose){
            for (auto p : street.points) {
                std::cout << "(" << std::fixed << std::setprecision(8) << p.longitude << ", " << p.latitude << ")";
            }
        }
        std::cout << std::endl;
    }
}
