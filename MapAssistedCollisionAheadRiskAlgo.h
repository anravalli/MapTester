#include "map.h"
#include "Geodesy.h"

enum class RiskTYpe {
	noRisk = 0,
	assistedCollisionAhead = 4
};

struct Risk
{
    float tti;
	float dti;
    RiskTYpe risk;
};

class RDAlgo
{
public:
	virtual ~RDAlgo(){};
	virtual Risk calculateRisk(void) = 0;
};

class RoadUser {
public:
	double m_speed = 0.0; // m/s
	double m_heading = 0.0; //deg
	Geodesy::Position m_pos = {0.0, 0.0};
	SegmentInfo m_currentSegment;
};

class MappAssistedCollisionAhead: public RDAlgo
{
public:
	virtual Risk calculateRisk(void) final override;
};
