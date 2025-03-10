#pragma once
/****************************************************************************/
/** @file       RDAlgo.h                                                    */
/** @title      Risk Detection Algorithm class                              */
/** @brief      Risk Detection Algorithm - RAII                             */
/** @date       09/20/2024                                                  */
/** @author     Fausto Pignatta                                             */
/**                                                                         */
/** @see        www.spokesafety.com                                         */
/****************************************************************************/

#include "RisksDataTypes.h"

namespace SpokeRisk {

	class RDAlgo {
	public:
		virtual ~RDAlgo(){};
		virtual EgoRisk calculateRisk(void) = 0;
	};

} // namesapce spokeRisk
