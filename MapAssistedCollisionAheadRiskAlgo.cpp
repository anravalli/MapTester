
#include <cmath>
#include <iostream>

#include "Geodesy.h"
#include "map.h"
#include "MapAssistedCollisionAheadRiskAlgo.h"
//#include "Geodesy.h"

extern RoadUser the_ego;
extern RoadUser the_other;

static constexpr double detection_radius = 200; //meters
static constexpr double speed_threashold = 5; //Km/h
const constexpr double kph2mps(double speed) {return speed/3.6;};

Risk MappAssistedCollisionAhead::calculateRisk(void)
{
	std::cout << "MappAssistedCollisionAhead" << std::endl;
	Risk risk = {0};

	if (the_other.m_speed > kph2mps(speed_threashold)){
		std::cout << "MappAssistedCollisionAhead - skip iteration" << std::endl;
		return risk;
	}

	Geodesy::Point rel_pos = Geodesy::relativePosition(positionDegToRad(the_ego.m_pos),
													   the_ego.m_heading, positionDegToRad(the_other.m_pos));
	double sqred_dist = rel_pos.x*rel_pos.x + rel_pos.y*rel_pos.y;
	double sqred_radius = detection_radius*detection_radius;
	if(sqred_dist < sqred_radius)
	{
		std::cout << "--> (1/3) inside detection radius" << std::endl;

		if(the_ego.m_currentSegment.street_name == the_other.m_currentSegment.street_name // are thye both on the same street?
			and rel_pos.x > 0 //is the OTHER ahead of EGO?
		)
		{
			std::cout << "--> (2/3) same street and other ahead" << std::endl;
			bool ego_on_the_right = the_ego.m_currentSegment.relative_position.y < 0;
			bool other_on_the_right = the_other.m_currentSegment.relative_position.y < 0;
			double delta_speed = the_ego.m_speed-the_other.m_speed;
			std::cout << "--> the_ego.m_speed: " << the_ego.m_speed << std::endl;
			std::cout << "--> the_other.m_speed: " << the_other.m_speed << std::endl;
			std::cout << "--> delta_speed: " << delta_speed << std::endl;
			// bool ego_approaching_other = (delta_speed)>0;
			if(ego_on_the_right == other_on_the_right //are EGO and OTHER proceeding on the same direction?
				and  delta_speed > 0// is the EGO approaching the OTHER?
			){
				std::cout << "--> (3/3) same direction and approaching" << std::endl;
				risk.risk = RiskTYpe::assistedCollisionAhead;
				risk.dti = std::sqrt(sqred_dist); //meters
				risk.tti = risk.dti/delta_speed; // km/(km/h)
			}
		}

	}

	return risk;
}
