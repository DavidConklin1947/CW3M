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

// Values for the VEGTRANSTYPE attribute
#define TRANS_NONE 0
#define TRANS_DETERMINISTIC 1
#define TRANS_PROBABILISTIC 2
#define TRANS_DISTURBANCE 3

/*---------------------------------------------------------------------------------------------------------------------------------------
 * general idea:
 *    1) Create subclasses of EnvEvalModel, EnvAutoProcess for any models you wnat to implement
 *    2) Add any exposed variables by calling
 *         bool EnvExtension::DefineVar( bool input, int index, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR desc );
 *         for any exposed variables
 *    3) Override any of the methods defined in EnvExtension classes as needed
 *    4) Update dllmain.cpp
 *    5) Update *.def file
 *
 *
 *  Required Project Settings (<All Configurations>)
 *    1) General->Character Set: Not Set
 *    2) C/C++ ->General->Additional Include Directories: $(SolutionDir);$(SolutionDir)\libs;
 *    3) C/C++ ->Preprocessor->Preprocessor Definitions: WIN32;_WINDOWS;_AFXEXT;"__EXPORT__=__declspec( dllimport )";
 *    4) Linker->General->Additional Library Directories: $(SolutionDir)\$(ConfigurationName)
 *    5) Linker->Input->Additional Dependencies: libs.lib
 *    6) Build Events->Post Build Events: copy $(TargetPath) $(SolutionDir)
 *
 *    Debug Configuration Only:  C/C++ ->Preprocessor->Preprocessor Definitions:  Add DEBUG;_DEBUG
 *    Release Configuration Only:  C/C++ ->Preprocessor->Preprocessor Definitions:  Add NDEBUG
 *---------------------------------------------------------------------------------------------------------------------------------------
 */

struct DeterministicKeyclass {
	int from;
	int region;
	int pvt;	
   int endage;
   DeterministicKeyclass():from(-1),region(-1),pvt(-1),endage(-1) { }
} ;

struct ProbKeyclass {
	int from;
	int region;
   int pvt;
   int disturb;
   int si;
   int regen;
   ProbKeyclass():from(-1),region(-1),pvt(-1),disturb(-1),si(-1),regen(-1) { }
} ;

struct ProbIndexKeyclass {
	int   from;
   int   to;
   int   region;
   int   pvt;
	int   disturb;
   float prob;
   int   si;
   int   regen;
   ProbIndexKeyclass():from(-1),to(-1),region(-1),pvt(-1),disturb(-1),prob(-1),si(-1),regen(-1) { }
} ;

struct TSDIndexKeyclass {
	int   from;
	int   pvt;
	int   region;
	int   disturb;
	int   si;
	int   regen;
	TSDIndexKeyclass() :from(-1), pvt (-1), region(-1), disturb(-1), si(-1), regen(-1) { }
};

struct DeterminIndexKeyClass {	
	int from;
   int region;
	int pvt;
   DeterminIndexKeyClass():from(-1),region(-1),pvt(-1) { }
} ;

struct ProbMultiplierPVTKeyclass {	
   int region;
   int pvt;
   int disturb;
   int timestep;
   float si;
   ProbMultiplierPVTKeyclass():region(-1),pvt(-1),disturb(-1),timestep(-1),si(0.f) { }
} ;

struct ProbMultiplierVegClassKeyclass {	
   int vegclass;
	int region;
   int pvt;
   int disturb;
   int timestep;
   float si;
   ProbMultiplierVegClassKeyclass():vegclass(-1),region(-1),pvt(-1),disturb(-1),timestep(-1) { }
} ;


// Comparitive class for looking up values in probability "map" container
struct ProbIndexClassComp {
  bool operator() (const ProbIndexKeyclass& lhs, const ProbIndexKeyclass& rhs) const
   {
	 if(lhs.from == rhs.from)
		{
		if (lhs.to == rhs.to)
         {
         if(lhs.region == rhs.region)
		   	{
		   	if(lhs.pvt == rhs.pvt)
		   		{
		   		if(lhs.disturb == rhs.disturb)
		   		    {
						 if(lhs.prob == rhs.prob)
		   				{
		   				if(lhs.si == rhs.si)
		   					{
		   					return lhs.regen < rhs.regen;	  
		   					}	 
		   				else
		   					{
		   					return lhs.si < rhs.si;
		   					}
							}
						 else
							{
							return lhs.prob < rhs.prob;
		   				}
						}
		   	   else
		   			{
		   			return lhs.disturb < rhs.disturb;
		   			}
		   		} 
		   	else
		   	   {
		   		return lhs.pvt < rhs.pvt;
		   	   }
		      }
		   else
			{
			return lhs.region <  rhs.region;
			}  
		 }     
		else
         {
         return lhs.to < rhs.to;
         }
     }
   else
     {
		 return lhs.from < rhs.from;
	  }
  }
};

// Comparitive class for looking up values in probability "map" container
struct TSDIndexClassComp {
	bool operator() (const TSDIndexKeyclass& lhs, const TSDIndexKeyclass& rhs) const
		{
		if (lhs.from == rhs.from)
			{	
			if (lhs.region == rhs.region)
				{	
				if (lhs.pvt == rhs.pvt)	
					{	
					if (lhs.disturb == rhs.disturb)
						{					
						if (lhs.si == rhs.si)
							{
							return lhs.regen < rhs.regen;
							}
						else
							{
							return lhs.si < rhs.si;
							}					
						}
					else
						{
						return lhs.disturb < rhs.disturb;
						}
					}
				else 
					{
					return lhs.pvt < rhs.pvt;
					}						
				}
			else
				{
				return lhs.region <  rhs.region;
				}			
			}
		else
			{
			return lhs.from < rhs.from;
			}
		}
};


struct DeterminIndexClassComp {
  bool operator() (const DeterminIndexKeyClass& lhs, const DeterminIndexKeyClass& rhs) const
   {
	 
	if(lhs.from == rhs.from)
		{
		if(lhs.region == rhs.region)
			{
			return lhs.pvt < rhs.pvt;	  
			}	 
		else
			{
			return lhs.region < rhs.region;
			}
		}
	else
		{
		return lhs.from < rhs.from;
		}	
	   
	 
  }
};

struct ProbMultiplierPVTClassComp {
  bool operator() (const ProbMultiplierPVTKeyclass& lhs, const ProbMultiplierPVTKeyclass& rhs) const
   {
	 
	if(lhs.region == rhs.region)
	  {
	  if(lhs.pvt == rhs.pvt)
	    {
       if(lhs.si == rhs.si)
         {
	      if(lhs.disturb == rhs.disturb)
		     {
		   	return lhs.timestep < rhs.timestep;  
		     }
		   else
           {
           return lhs.disturb < rhs.disturb;
           }
         }
       else
         {
         return lhs.si < rhs.si;
         }
	    }      
	  else
       {
       return lhs.pvt < rhs.pvt;
       }
     }
   else
     {
     return lhs.region < rhs.region;
     } 
	   	 
  }
};

struct ProbMultiplierVegClassClassComp {
  bool operator() (const ProbMultiplierVegClassKeyclass& lhs, const ProbMultiplierVegClassKeyclass& rhs) const
   {

   if(lhs.vegclass == rhs.vegclass)
      {
	   if(lhs.region == rhs.region)
	     {
	     if(lhs.pvt == rhs.pvt)
	       {
          if(lhs.si == rhs.si)
            {
	         if(lhs.disturb == rhs.disturb)
	   	     {
	   	   	return lhs.timestep < rhs.timestep;  
	   	     }
	   	   else
              {
              return lhs.disturb < rhs.disturb;
              }
            }
          else
            {
            return lhs.si < rhs.si;
            }
	       }      
	     else
          {
          return lhs.pvt < rhs.pvt;
          }
        }
      else
        {
        return lhs.region < rhs.region;
        }
      }  
	else
     {
     return lhs.vegclass < rhs.vegclass;
     }  	 
  }
};


// Comparitive class for looking up values in probability "map" container
struct probclasscomp {
  bool operator() (const ProbKeyclass& lhs, const ProbKeyclass& rhs) const
   {
	 if(lhs.disturb == rhs.disturb)
		{
		if(lhs.from == rhs.from)
          {
          if(lhs.region == rhs.region)
			   {
			   if(lhs.regen == rhs.regen)
			      {
                if(lhs.si == rhs.si)
				   {
					return lhs.pvt < rhs.pvt;
				   }
				   else
				   {
				   return lhs.si < rhs.si;
				   }  
			      }	 
		      else
			      {
			      return lhs.regen < lhs.regen;
			      }
			   } 
		   else
		      {
		      return lhs.region < rhs.region;
		      }
		   }
		   else
		   {
		   return lhs.from <  rhs.from;
		   }
	   }
	 else
	    {
	    return lhs.disturb < rhs.disturb;
	   }	
  }
};

// Comparitive class for looking up values in fire "map" container
struct deterministicclasscomp {
  bool operator() (const DeterministicKeyclass& lhs, const DeterministicKeyclass& rhs) const
   {
   if(lhs.from == rhs.from)
	 {
	 if(lhs.region == rhs.region)
	   {
	    if(lhs.pvt == rhs.pvt)
		  {
			 return lhs.endage < rhs.endage;  
		  }
		else
          {
          return lhs.pvt < rhs.pvt;
          }           
	   }      
	 else
       {
       return lhs.region < rhs.region;
       }
     }
   else
     {
     return lhs.from < rhs.from;
     } 
  }
};

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

class VegSTM : public EnvAutoProcess
   {
   public:
      VegSTM(void);
      ~VegSTM( void );

      BOOL Init   ( EnvContext *pEnvContext, LPCTSTR initStr );
	   BOOL InitRun( EnvContext *pEnvContext, bool useInitSeed);
      BOOL Run    ( EnvContext *pEnvContext );
	   BOOL EndRun ( EnvContext *pEnvContext );
      bool LoadXml( LPCTSTR filename );
     
   protected:
      bool LoadProbCSV( CString probfilename, EnvContext *pEnvContext);
      bool LoadFireTransCSV( CString firefilename, EnvContext *pEnvContext);
	   bool LoadDeterministicTransCSV( CString firefilename, EnvContext *pEnvContext);
      bool ProbabilisticTransition(EnvContext * pEnvContext, int idu, int vegclass, int pvt, int disturb, int stand_age, int tsd, int & selected_to_trans, int & selected_pvtto_trans, float & selected_probability);
      bool DeterministicTransition(EnvContext *pEnvContext, int idu, int vegclass, int pvt, int ageclass, int & selected_to_trans, int & selected_pvtto_trans);
      int ChooseProbTrans(double rand_num, float probability_sum, vector<pair<int,float> > *m_permute_prob_vec,std::vector< std::pair<int,float> > *m_original_final_probs, float & orig_probability);

   MapLayer * m_pIDUlayer;

   int m_colDetSTARTAGE;
   int m_colDetENDAGE;
   int m_colDetLAI;

   int m_colProbMINAGE;
   int m_colProbMAXAGE;
   int m_colProbTSD;
   int m_colProbRELATIVEAGE;
   int m_colProbKEEPRELAGE;
   int m_colProbPROPORTION;
   int m_colProbTSDMAX;
   int m_colProbRELTSD;
   int m_colProbPVTto;

   protected:
     // mc1_output stuff
     //static PtrArray< MC1_OUTPUT > m_mc1_outputArray;
     static PtrArray< OUTPUT > m_outputArray;	 
	  static VEGTRANSFILE m_vegtransfile;
     static INITIALIZER m_initializer;
	  static PVTstruct m_dynamic_update;      
	  static RandUniform   m_rn;
     static bool m_staticsInitialized;

      static VDataObj m_inputtable; // probabilistic transition table
		static VDataObj m_deterministic_inputtable; // deterministic transition table
      
      static int m_colVEGCLASS;
      static int m_colDisturb;
		static int m_colAGECLASS;
      static int m_colPVT;
      static int m_colLAI;
      static int m_colCarbon;
		static int m_colTSD;
		static int m_colManage;
		static int m_colRegion;
		static int m_colCalcStandAge;
      int m_colVEGTRNTYPE;
      static int m_mc1_output;
      static int m_cursi;
		static int m_futsi;
      static int m_regen;              
      static int m_variant;
      static int m_mc1row;
      static int m_mc1col;
      static int m_mc1pvt;
		static int m_mc1Cell;
		static int m_manage;
	   static float m_selected_prob;
      static float m_carbon_conversion_factor;

      static bool m_validFlag;
      static bool m_useProbMultiplier;
		static bool m_flagDeterministicFile;
      static bool m_pvtProbMultiplier;
      static bool m_vegClassProbMultipier;

      bool m_unseenCCtrans[CC_DISTURB_UB - CC_DISTURB_LB];
     
      CString m_msg;
          
	  static vector<vector<vector<int> > > m_vpvt;  //3d vector to hold MC1 pvt data [year,row,column]

     static vector<vector<vector<float> > > m_vsi;  //3d vector to hold MC1 si data [year,row,column]

     static map<ProbKeyclass,std::vector< std::pair<int,float> >,probclasscomp> probmap;
	  
	  static map<ProbKeyclass,std::vector< std::pair<int,float> >,probclasscomp> probmap2;

     static map<ProbMultiplierPVTKeyclass,std::vector<float>,ProbMultiplierPVTClassComp> m_probMultiplierPVTMap;

     static map<ProbMultiplierVegClassKeyclass,std::vector<float>,ProbMultiplierVegClassClassComp> m_probMultiplierVegClassMap;

	  static map<TSDIndexKeyclass, std::vector<int>, TSDIndexClassComp> m_TSDIndexMap;

	  static map<ProbIndexKeyclass,std::vector<int>,ProbIndexClassComp> m_probIndexMap;

	  static map<DeterminIndexKeyClass,std::vector<int>,DeterminIndexClassComp> m_determinIndexMap;
    
     static map<DeterministicKeyclass,std::vector< std::pair<int,int> >,deterministicclasscomp> m_deterministic_trans;
         
     static ProbKeyclass m_probInsertKey,m_probLookupKey;

	  static DeterministicKeyclass m_deterministicInsertKey, m_deterministicLookupKey;

	  static ProbIndexKeyclass m_probIndexInsertKey, m_probIndexLookupKey;

	  static TSDIndexKeyclass m_TSDIndexInsertKey, m_TSDIndexLookupKey;

	  static DeterminIndexKeyClass m_determinIndexInsertKey, m_determinIndexLookupKey;

     static ProbMultiplierPVTKeyclass m_probMultiplierPVTInsertKey,  m_probMultiplierPVTLookupKey;

     static ProbMultiplierVegClassKeyclass  m_probMultiplierVegClassInsertKey, m_probMultiplierVegClassLookupKey;

	  static CArray< CString, CString > m_badVegTransClasses;

     static CArray< CString, CString > m_badDeterminVegTransClasses;

   };

