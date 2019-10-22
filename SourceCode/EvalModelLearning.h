#pragma once
#define NOMINMAX
#include "HistogramCDF.h"
#ifndef __INI_TEST__
#include "EnvEngine\EnvModel.h"
#endif
#include <string>
#include <memory>
#include <vector>
#include <map>
using std::map;
using std::vector;
using std::string;
using std::auto_ptr;
#undef NOMINMAX
//=======================================================================================
/*   USE CASES:
   1. ***Read Envision.ini and set ENV_EVAL_MODEL gain and offset.
      EvalModelLearningIniFile::ReadIni()
      EvalModelLearningIniFile::SetModelInfo()
   2. ***Calculate Gain and Offset
      EvalModelLearning::AdUbiOmniHoc()
   3. ***Update Envision.ini
      EvalModelLearningIniFile::SetModelInfoLine()
*/
// Every model that is used in either or both altruism and self-interest is a metagoal.
// So metagoal and scarcity semantically are not identical. 
// LOTS OF CODE TO MAKE SURE DEALING WITH SAME SET OF MODELS.

// ISSUE: Envision DEVIANT-STYLE CODING:  
//    capitalize member function
//    hungarian notation
//    indentation
//    openspace abundance
//    | dangerously omit const in arg to copy ctor and operator=
//    | to screw up conscientious code 
//======================================================================================= 
// For unit testing define. (INTELLISENSE GETS SCREWED UP UNLESS YOU ALSO COMMENT OUT WHEN NOT UNIT-TESTING) 
////
//#ifdef  __INI_TEST__  //========//
//class ENV_EVAL_MODEL {            //
//public:                       //
//   CString         name;       //
//   float           gain;         //
//   float           offset;          //
//   auto_ptr< Non_param_cdf<> > apNpCdf; //
//   };                                    //
//class EnvModel {                          //
//public:                                         //
//   static vector <ENV_EVAL_MODEL> mi;                   //
//   static ENV_EVAL_MODEL *FindModelInfo( LPCSTR name );    //  
//   static int FindModelIndex(LPCSTR name);                //
//   };                                                        //
//class Policy {                                                //
//public:                                                        //
//   Policy() : m_name("nul"), m_id(-1) {}                       //
//   Policy(LPCTSTR name, int id) : m_name(name), m_id(id) {}   //
//   int m_id;                                                //
//   CString m_name;                                        //
//   };                                                   //
//#endif                                                //
//
//======================================================================================= 
// Accumulator of learned information.  From Sandbox, operator() is called repeatedly
// and processes data about model behavior that yields statistical knowledge.
class EvalModelLearningStats 
{
public:
   enum {
      MIN=0, 
      MAX, 
      N_STATS
      };
   // Each vector cell corresponds to each Model::m_scarcityToModelMap cell. 
   CArray< CArray<double, double>, CArray<double, double> & > m_stats;      // 
   CArray< ENV_EVAL_MODEL*, ENV_EVAL_MODEL* >                         m_modelInfos; //
   CArray< HistogramCDF, HistogramCDF & >                     m_densities;

   int m_evalFreq;
   int m_yearsToRun;
   int m_percentCoverage;
   bool DoEvalYearBase0(int year) const;
   CString Trace() const ;

   EvalModelLearningStats(INT_PTR N); // number of models to keep stats for

   // operator () calls other methods, e.g. MaxMin()
   void operator()( size_t model_index, double value );  
  
   // MaxMin arg, idx, is the first index in m_stats corresponding to ENV_EVAL_MODEL.
   // MaxMin accumulates the max & min stat over a series of calls
   void MaxMin( size_t idx, double value );

   // Uses accumulated Stats and writes to m_modelInfos
   void SetGainOffset( );

   // Completes HistogramCDF stuff and updates ENV_EVAL_MODEL
   void SetCDF( );
};
//======================================================================================= 
// READ/WRITE OF Envision.ini, esp wrt ENV_EVAL_MODEL and gain, offset
class EvalModelLearningIniFile
   {
   public:
   const string KEY_models  ;                 // [models] section of the ini file we want to process.
   const string KEY_cdfs    ;                 // [cdfs] ditto
   const string iniF  ;                       // Envision.ini
   enum {GAIN_TOK = 8, OFFSET_TOK, END_TOK};  // the field positions of interest in the record.
   enum {INIT=0, REPARSE=128,
         BEGIN_SECTION_MODELS=1, BEGIN_SECTION_CDFS=2}; // state for parsing the Envision.in.

   EvalModelLearningIniFile() : KEY_models("[models]"), KEY_cdfs("[cdfs]"), iniF("Envsion.ini") {}

	vector<string> llines;                  // simply all the lines of the Envision.ini file.
   map<string, int> modelNameToFileLlines; // <ENV_EVAL_MODEL::name, idx for llines> for the [models] section
   map<string, int> modelNameToFileLlinesCDF; // ditto but for the [cdfs] section

   // Rewrites lline to incorporate the new values of gain and offset (passed in).
   void SetModelInfoLline(const char * modelName, float gain, float offset) ;
   //
   void SetModelInfoLlineCDF( const char * modelName, const Non_param_cdf<> & cdf);

   // IF ifn != NULL, then use that path instead of standard.  
   // Reads the Envision.ini file; stores it in llines; assigns modelNameToFileLlines;
   void ReadIni(const char * ifn = NULL);
   // Simply writes llines out to Envision .ini
   void WriteIni(const char * ifn=NULL) const;

   // initializes global ENV_EVAL_MODEL structs.
   void SetModelInfo() const;

   // Tokenizes a line from the Envision.ini that is a ENV_EVAL_MODEL record.
   // If bPutGainOffset is true, then args, gain and offset, are put into the toks;
   // and if false then gain and offset return what was in the line.
   void TokenizeModelLine(const string & line, vector<string> & toks, 
      float & gain, float & offset, bool bPutGainOffset=false) const;

   ////void CdfFromInputLine( Non_param_cdf<> & cdf, const string & line );
   ////void OutputLineFromCdf( string & line, const Non_param_cdf<> & cdf, const char * modelName );

   };
//======================================================================================= 
// Use case for learning.  Interacts with Scenario
class EvalModelLearning
{
public:
   auto_ptr<EvalModelLearningStats> emStats;
   auto_ptr<EvalModelLearningIniFile> emIniFile;

   EvalModelLearning(void);
   ~EvalModelLearning(void);

   // Use Case: For all scenarios over all policies and all time learn this
   void adUbiOmniHoc(CWnd * pWndParent);
};
