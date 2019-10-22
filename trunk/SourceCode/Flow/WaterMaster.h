#pragma once

#include "globalMethods.h"
#include "FlowContext.h"
#include <EnvExtension.h>
#include <PtrArray.h>
#include <Vdataobj.h>
#include <vector>
#include <map>
#include <EnvEngine\EnvConstants.h>
#include <EnvInterface.h>
#include "reachtree.h"
#include "GeoSpatialDataObj.h"

#include "FlowInterface.h"

using namespace std;

enum WR_SPECIALCODE {
   WRSC_NONE = 0, WRSC_NEWINSTREAM = 1, WRSC_MUNIBACKUP = 2, WRSC_NOT_NEWINSTREAM_ONLY = 4,
   WRSC_OBSOLETE = 8, // no longer used in simulations 
   // next one should be 16, then 32, and so on
};

enum WR_PODSTATUS {
	WRPS_CANCELED = 1, WRPS_EXPIRED = 2, WRPS_MISFILED = 3, WRPS_NONCANCELED = 4,
	WRPS_REJECTED = 5, WRPS_SUSPENDED = 6, WRPS_SUPERCEDED = 7, WRPS_UNKNOWN = 8,
	WRPS_UNUSED = 9, WRPS_WITHDRAWN = 10
};

typedef struct {
	int idu_id;     // IDU_ID in POU input data file
} IDUIndexKeyClass;


typedef struct {
	int pouID;     // WR ID or SnapID in POU input data file

} PouKeyClass;

typedef struct {
	int       priorYr;
	int       priorDoy;
	int       beginDoy;
	int       endDoy;
	int       podStatus;
	int       appCode;
	WR_USE    useCode;
	WR_PERMIT permitCode;
	int       supp;
	float     podRate;
	int       podID;
	int       pouID;
	double    xCoord;
	double    yCoord;
} PodSortStruct;

struct podSortFunc {
	bool operator() (const PodSortStruct& lhs, const PodSortStruct& rhs) const
	{
		if (lhs.priorYr == rhs.priorYr)
		{
			if (lhs.priorDoy == rhs.priorDoy)
			{
				if (lhs.beginDoy == rhs.beginDoy)
				{
					if (lhs.endDoy == rhs.endDoy)
					{
						if (lhs.podStatus == rhs.podStatus)
						{
							if (lhs.appCode == rhs.appCode)
							{
								if (lhs.useCode == rhs.useCode)
								{
									if (lhs.permitCode == rhs.permitCode)
									{
										if (lhs.supp == rhs.supp)
										{
											if (lhs.podRate == rhs.podRate)
											{
												if (lhs.podID == rhs.podID)
												{
													if (lhs.pouID == rhs.pouID)
													{
														if (lhs.xCoord == rhs.xCoord)
														{
															return lhs.yCoord < rhs.yCoord;
														}
														else
														{
															return lhs.xCoord < rhs.xCoord;
														}
													}
													else
													{
														return lhs.pouID < rhs.pouID;
													}
												}
												else
												{
													return lhs.podID < rhs.podID;
												}
											}
											else
											{
												return lhs.podRate < rhs.podRate;
											}
										}
										else
										{
											return lhs.supp < rhs.supp;
										}
									}
									else
									{
										return lhs.permitCode < rhs.permitCode;
									}
								}
								else
								{
									return lhs.useCode < rhs.useCode;
								}
							}
							else
							{
								return lhs.appCode < rhs.appCode;
							}
						}
						else
						{
							return lhs.podStatus < rhs.podStatus;
						}
					}
					else
					{
						return lhs.endDoy < rhs.endDoy;
					}
				}
				else
				{
					return lhs.beginDoy < rhs.beginDoy;
				}
			}
			else
			{
				return lhs.priorDoy < rhs.priorDoy;
			}
		}
		else
		{
			return lhs.priorYr < rhs.priorYr;
		}
	}
};

// comparison structure for looking up POU index in POU input file/map, when given pouID
struct IDUIndexClassComp {
	bool operator() (const IDUIndexKeyClass& lhs, const IDUIndexKeyClass& rhs) const
	{
		return lhs.idu_id < rhs.idu_id;
	}
};

// comparison structure for looking up POU index in POU input file/map, when given pouID
struct PouKeyClassComp {
	bool operator() (const PouKeyClass& lhs, const PouKeyClass& rhs) const
	{
		return lhs.pouID < rhs.pouID;
	}
};

class WaterRight
{
public:
	WaterRight(void) : m_idu(-1), m_xCoord(-1.), m_yCoord(-1.), m_podID(-1), m_pouID(-1)
		, m_appCode(-1), m_permitCode(WRP_UNKNOWN), m_podRate_cfs(0.), m_useCode(WRU_UNKNOWN)
		, m_supp(-1), m_priorDoy(-1), m_priorYr(-1), m_beginDoy(-1)
		, m_endDoy(-1), m_pouRate(0.), m_podStatus(WRPS_UNKNOWN)
		, m_pctFlow(0), m_pReach(NULL), m_inUse(true)
		, m_reachComid(-1), m_streamIndex(-1), m_consecDaysNotUsed(0), m_inConflict(false)
		, m_lastDOYNotUsed(-1), m_nDaysPerYearSO(0), m_nDaysPerWeekSO(0), m_previousWeekConflict(false)
		, m_nPODSperWR(1), m_wrID(0), m_consecYearsNotUsed(0), m_priorYearConflict(0), m_dutyShutOffThreshold(2.5f)
		, m_suspendForYear(false), m_pouArea(0.0f), m_wrAnnualDuty(0.0f), m_surfaceHruLayer(0), m_use(WRU_UNKNOWN)
		, m_podUseRate_cfs(0.0f), m_distanceToReach(0.0f), m_maxDutyPOD(1000000.0f), m_stepShortageFlag(false), m_stepsSuspended(0)
      , m_stepRequest(0.0f), m_lastDOYShortage(0), m_instreamWRlength_m(0.f), m_specialCode(WRSC_NONE)
      , m_allocatedToday_cms(0.f), m_allocatedYesterday_cms(0.f)
	{ }

	// member variables
	int m_idu;                  // which IDU is this associated with

	WR_USE m_use;

	//POD point of diversion variables
	int          m_wrID;             // Water right id
	double       m_xCoord;           // UTM zone x coordinate (m)
	double       m_yCoord;           // UTM zone y coordinate (m)
	int          m_podID;            // Water wright ID or SnapID in POD input data file
	int          m_pouID;            // Water wright ID or SnapID in POD input data file
	int          m_appCode;          // WR Application code http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
	WR_PERMIT    m_permitCode;       // WR Permit Code http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
	float        m_podRate_cfs;      // WR point of diversion rate (cfs, i.e. cubic feet per second)
	float        m_podUseRate_cfs;   // WR point of use rate (cfs)
	WR_USE       m_useCode;          // Use Code http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
	int          m_supp;             // supplemental code 0-Primary 1-Supplemental
	int          m_priorDoy;         // WR priority date day of year
	int          m_priorYr;          // WR priority date year
	int          m_beginDoy;         // WR seasonal begin day of year
	int          m_endDoy;           // WR season end day year
	float        m_pouRate;          // WR point of use max rate (m3/sec)
	float        m_pouArea;          // WR point of use area (m2)
	WR_PODSTATUS m_podStatus;        // WR point of diversion status code
	int          m_reachComid;       // WR Reach comid, relates to COMID in reach layer
   int          m_streamIndex;      // Index of reach in stream layer
   float        m_wrAnnualDuty;     // WR annual duty, used to compare to annual duty threshold
	float        m_distanceToReach;  // This is the distance from a POD to the closest vertex in a reach (m)
	float        m_maxDutyPOD;		 // Max duty associated with POD (acre-ft / acre)
	float        m_stepRequest;      // the amount requested for current step (m3/sec)
   float        m_allocatedToday_cms; // amount allocated today (m3/sec)
   float        m_allocatedYesterday_cms; // amount allocated yesterday (m3/sec)

   // For a given instream water right, either or both of the next two variables may be zero.
   float        m_instreamWRlength_m; // length of instream water right (m)
   int          m_downstreamComid; // COMID of downstream end of instream WR

   WR_SPECIALCODE m_specialCode;
	//POU point of use variables
	float        m_pctFlow;          // portion of the flow associated with this IDU

	//Water Right conflict variables
	int          m_consecDaysNotUsed;      // number of consequtive days WR is not used
	int          m_consecYearsNotUsed;     // number of consequtive years where Consequtive days threshold was exceeded
	bool         m_inConflict;             // if WR is in conflict. Not used, can be revoked
	int          m_priorYearConflict;      // prior year that WR was in Conflict
	int          m_lastDOYNotUsed;         // the last day of year WR not used
	int          m_nDaysPerYearSO;         // number of days per year water right is shut off
	int          m_nDaysPerWeekSO;         // number of day per week water right is shut off
	bool         m_previousWeekConflict;   // previous week was in conflict
	float        m_dutyShutOffThreshold;   // each water right will have a maximum duty it is allowed (default is 2.5 acre/feet/acre)
	bool			 m_suspendForYear;		   // if true water right is suspended for the rest of the year
	bool			 m_stepShortageFlag;			// within a growing season, if a senior water right experiences shortage, then keep track when this WR juniors are suspended from daily diversions  
	int			 m_stepsSuspended;			// number of consecutive steps a water right is in conflict
	int          m_lastDOYShortage;			// day of year water right is in shortage

	//economic variables
	//bool         m_wrIrrigateFlag;   //if economics restrict irrigating

	//demand related variables
	//bool         m_demandIrrigateFlag; //if no demand no irrigating 

	Reach        *m_pReach;          // for surface water rights only

	// runtime info;
	bool         m_inUse;
	int          m_surfaceHruLayer;  // index of HRU layer where water is applied
	int          m_nPODSperWR;       // The number of Points of Diversion per Water Right	

	bool IsSurface() { return true; }  //temporary
};


