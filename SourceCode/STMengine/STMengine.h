#pragma once
#include <afx.h>
#include "stdafx.h"
#include <iostream>
#include <EnvExtension.h>
#include <randgen\Randunif.hpp>
#include <randgen\RandExp.h>
#include <QueryEngine.h>
#include <EnvEngine\EnvConstants.h>
#include <vector>
#include <Vdataobj.h>
#include <map>
#include <PtrArray.h>
#include <afxtempl.h>
#include <initializer_list>


using namespace std;

#define gm gM->

#define STM_CLASS_MIN 1000
#define STM_CLASS_MAX 999999

/* Columns in the conditional transitions table
* WET_FRACatLeast,WET_FRAClessThan,WETLONGESTatLeast,WETLONGESTlessThan,WETAVGDPTHatLeast,WETAVGDPTHlessThan,VEGCLASSfrom,VEGCLASSto,ABBREVfrom,ABBREVto,FUTSI,PVT,PVTto,REGEN,p,pxPropor,REGION,DISTURB,VDDTProbType,MINAGE,MAXAGE,TSD,RELATIVEAGE,KEEPRELAGE,PROPORTION,TSDMAX,RELTSD,RNDAGE
IDU attributes containing values to be compared to thresholds are: 
WET_FRAC - The fraction of the year in which WETNESS is >= 0 (i.e. soil is saturated or inundated)
WETLONGEST - Longest sequence of consecutive days in the year in which WETNESS >= 0
WETAVGDPTH - Average value of WETNESS over the days on which WETNESS >= 0
*/

// Values for the VEGTRANSTYPE attribute
#define TRANS_NONE 0
#define TRANS_DETERMINISTIC 1
#define TRANS_PROBABILISTIC 2
#define TRANS_DISTURBANCE 3

struct MC1_OUTPUT
   {
   int id;
   CString name;
   CString siteIndexFile;
   CString pvtFile;
   };

struct USEVEGTRANSTYPE
   {   
   int useVegTransFlag;
   };

struct VEGTRANSFILE
   {   
   CString probability_filename;
   CString deterministic_filename;
   };
    
struct OUTPUT
   {
   CString name;
   CString query;

   Query  *pQuery;

   float value;

   OUTPUT( void ) : pQuery( NULL ), value( 0 ) { }
   };

struct INITIALIZER
   {
   int initAgeClass;
   int initTSD;
   };

struct PVTstruct
	{
	int dynamic_update;
	};


class CondTrans
{
	~CondTrans() {};

	int m_sourceState;
	int m_destState;

	// WET_FRACatLeast, WET_FRAClessThan, WETLONGESTatLeast, WETLONGESTlessThan, WETAVGDPTHatLeast, WETAVGDPTHlessThan
	double WET_FRACatLeast, WET_FRAClessThan; // days with WETNESS >= 0 as a fraction of days in year
	int WETLONGESTatLeast, WETLONGESTlessThan; // length of longest consecutive run of days this year in which WETNESS >= 0
	double WETAVGDPTHatLeast, WETAVGDPTlessThan; // average value of WETNESS on days on which WETNESS >= 0
};


class STMengine : public EnvAutoProcess
{
public:
   STMengine(void);
   ~STMengine( void );

   BOOL Init   ( EnvContext *pEnvContext, LPCTSTR initStr );
	BOOL InitRun( EnvContext *pEnvContext, bool useInitSeed);
   BOOL Run    ( EnvContext *pEnvContext );
	BOOL EndRun ( EnvContext *pEnvContext );
   bool LoadXml( LPCTSTR filename );
     
protected:
	int LoadDeterministicTransCSV( CString firefilename, EnvContext *pEnvContext);
	bool LoadProbCSV(CString probfilename, EnvContext* pEnvContext);

	int DeterministicTransition(int idu, int currSTMndx, int currAge); // Returns index of new state in the DetTransTable.
	int ConditionalTransition(int iduPolyNdx, int currSTMndx, int currAge);  // Returns index of new state in the DetTransTable.

   bool ConditionsAreMet(int condTransNdx, int iduPolyNdx);
   int ChooseProbTrans(double rand_num, float probability_sum, vector<pair<int, float> >* m_permute_prob_vec, std::vector< std::pair<int, float> >* m_original_final_probs, float& orig_probability);
	inline double Att(int iduPolyNdx, int col);
	inline float AttFloat(int iduPolyNdx, int col);
	inline int AttInt(int iduPolyNdx, int col);
	inline void SetAtt(int IDUpolyNdx, int col, double attValue);
	inline void SetAttFloat(int IDUpolyNdx, int col, float attValue);
	inline void SetAttInt(int IDUpolyNdx, int col, int attValue);

	EnvContext* m_pEnvContext;

	CArray<CondTrans *, CondTrans *> m_condTransSourceStates;

   int m_colDetCURR_STATE;
   int m_colDetNEW_STATE;
	int m_colDetSTARTAGE;
   int m_colDetENDAGE;
   int m_colDetLAI;

   int m_colCondCURR_STATE;
   int m_colCondNEW_STATE;
   int m_colProbMINAGE;
   int m_colProbMAXAGE;

   protected:
     // mc1_output stuff
     //PtrArray< MC1_OUTPUT > m_mc1_outputArray;
     PtrArray< OUTPUT > m_outputArray;	 
	  VEGTRANSFILE m_vegtransfile;
     INITIALIZER m_initializer;
	  PVTstruct m_dynamic_update;      
	  RandUniform   m_rn;
     bool m_staticsInitialized;

	  VDataObj m_condTransTable; // conditional transition table
	  VDataObj m_detTransTable; // deterministic transition table

		int m_colManage;
		int m_colRegion;
		int m_colCalcStandAge;
      int m_colVEGTRNTYPE;
      int m_mc1_output;
      int m_cursi;
		int m_futsi;
      int m_regen;              
      int m_variant;
      int m_mc1row;
      int m_mc1col;
      int m_mc1pvt;
		int m_mc1Cell;
		int m_manage;
	   float m_selected_prob;
      float m_carbon_conversion_factor;

      bool m_validFlag;
      bool m_useProbMultiplier;
		bool m_flagDeterministicFile;
      bool m_pvtProbMultiplier;
      bool m_vegClassProbMultipier;

      bool m_unseenCCtrans[CC_DISTURB_UB - CC_DISTURB_LB];
     
      CString m_msg;
          
	  vector<vector<vector<int> > > m_vpvt;  //3d vector to hold MC1 pvt data [year,row,column]

     vector<vector<vector<float> > > m_vsi;  //3d vector to hold MC1 si data [year,row,column]

	  CArray< CString, CString > m_badVegTransClasses;

     CArray< CString, CString > m_badDeterminVegTransClasses;

   };

