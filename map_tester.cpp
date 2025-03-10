//#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

// Include necessary libraries for XML parsing (e.g., pugixml)
#include "Geodesy.h"
#include "pugixml.hpp" // Or any other XML parsing library you prefer

#include "map.h"
#include "MapAssistedCollisionAheadRiskAlgo.h"

RoadUser the_ego;
RoadUser the_other;

Geodesy::Position stringToPosition(std::string str_pos)
{
    double lat = 0.0;
    double lon = 0.0;
    int split = str_pos.find(',');
    std::string str_lat = str_pos.substr(0, split);
    std::string str_lon = str_pos.substr(split+1);
    std::cout << "position: " << str_lat << ", " << str_lon << std::endl;
    lat = std::strtod(str_lat.c_str(), nullptr);
    lon = std::strtod(str_lon.c_str(), nullptr);
    std::cout << "(double) position: " << lat << ", " << lon << std::endl;
    return {lat, lon};
}

const constexpr double kph2mps(double speed) {return speed/3.6;};

int main(int argc, char *argv[]) {

    Geodesy::Position ego = {0,0};
    Geodesy::Position other = {0,0};
    std::string osmFile;

    int opt;
    char *usage_string = "Usage: %s [-h] [-m map] [-e ego_pos] [-o other_pos]\n";

    while ((opt = getopt(argc, argv, "hm:e:o:")) != -1) {
		switch (opt) {
			case 'h':
				printf(usage_string, argv[0]);
				return 0;
			case 'm':
				osmFile = optarg;
				break;
			case 'e':
				the_ego.m_pos = stringToPosition(optarg);
				break;
			case 'o':
				the_other.m_pos = stringToPosition(optarg);
				break;
			default:
				fprintf(stderr, usage_string, argv[0]);
				return 1;
		}
	}

    RDAlgo *the_algorithm = new MappAssistedCollisionAhead();

    the_ego.m_speed = kph2mps(30);
    the_other.m_speed = kph2mps(3);;
    the_ego.m_heading = Geodesy::degToRad(20);
    the_other.m_heading = Geodesy::degToRad(15);


    std::cout << "Using map file: " << osmFile << std::endl;
    Map map;
    map.loadMap(osmFile);
    if(map.matchEgoToMap(the_ego.m_pos, the_ego.m_currentSegment)){
        std::cout << "ego matched" << osmFile << std::endl;
        std::cout << "\t--> is on: " << the_ego.m_currentSegment.street_name << std::endl;
    }
    if(map.matchOtherToMap(the_other.m_pos, the_other.m_currentSegment)){
        std::cout << "other matched" << osmFile << std::endl;
        std::cout << "\t--> is on: " << the_other.m_currentSegment.street_name << std::endl;
        // if speed >= speed_threashold
        Risk risk = the_algorithm->calculateRisk();
        if (risk.risk != RiskTYpe::noRisk){
            std::cout << "Risk of incident with: tti=" << risk.tti << ", dti=" << risk.dti << std::endl;
        }

    }

    return 0;
}
