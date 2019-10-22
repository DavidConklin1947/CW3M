#pragma once

#include "EnvEngine/EnvContext.h"
#include "DeltaArray.h"
#include "EnvEngine/Policy.h"
#include "EnvEngine/Scenario.h"
#include "EnvEngine/EnvConstants.h"

//#include <VideoRecorder.h>


#ifndef NO_MFC
#ifdef ENV_IMPORT
#define ENV_EXPORT __declspec( dllimport )
#else
#define ENV_EXPORT  __declspec( dllexport )
#endif
#else
#define ENV_EXPORT
#endif



#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

// class Map/MapLayer methods
//ENV_EXPORT bool GetIDUPath( CString &path )'
ENV_EXPORT int  GetLayerIndex( LPCTSTR name );
ENV_EXPORT bool GetLayerName ( int index, LPTSTR name, int maxLength );

ENV_EXPORT INT_PTR AddDelta();
ENV_EXPORT DELTA&  GetDelta( DeltaArray*, INT_PTR index ); 
ENV_EXPORT int ApplyDeltaArray( void );

ENV_EXPORT bool GetDataFloat ( int layer, int row, int col, float *value );
ENV_EXPORT bool GetDataInt   ( int layer, int row, int col, int *value );
ENV_EXPORT bool GetDataBool  ( int layer, int row, int col, bool *value );
ENV_EXPORT bool GetDataString( int layer, int row, int col, LPTSTR value, int maxLength );
ENV_EXPORT bool GetDataDouble( int layer, int row, int col, double *value );

ENV_EXPORT bool SetDataFloat ( int layer, int row, int col, float value );
ENV_EXPORT bool SetDataInt   ( int layer, int row, int col, int value );
ENV_EXPORT bool SetDataBool  ( int layer, int row, int col, bool value );
ENV_EXPORT bool SetDataString( int layer, int row, int col, LPCTSTR value );
ENV_EXPORT bool SetDataDouble( int layer, int row, int col, double value );

// class EnvModel methods
ENV_EXPORT int GetModelCount();
ENV_EXPORT ENV_EVAL_MODEL* GetModelInfo( int i );
ENV_EXPORT ENV_EVAL_MODEL* FindModelInfo( LPCTSTR name );

ENV_EXPORT int GetAutoProcessCount();
ENV_EXPORT ENV_AUTO_PROCESS* GetAutoProcessInfo( int i );

ENV_EXPORT int ChangeIDUActor( EnvContext*, int idu, int actorGroup, bool randomize );

// PolicyManager methods
ENV_EXPORT int     GetPolicyCount();
ENV_EXPORT int     GetUsedPolicyCount();
ENV_EXPORT Policy *GetPolicy( int i );
ENV_EXPORT Policy *GetPolicyFromID( int id );


// ScenarioManager methods
ENV_EXPORT int       EnvGetScenarioCount();
ENV_EXPORT Scenario *EnvGetScenario( int i );
ENV_EXPORT Scenario *EnvGetScenarioFromName( LPCTSTR name, int *index );

// map window interactions
ENV_EXPORT int EnvAddVideoRecorder( int type, LPCTSTR name, LPCTSTR path, int frameRate, int method, int extra );  // method: see VRMETHOD enum in videorecorder.h
ENV_EXPORT int EnvStartVideoCapture( int vrID );
ENV_EXPORT int EnvCaptureVideo( int vrID );
ENV_EXPORT int EnvEndVideoCapture( int vrID );

ENV_EXPORT void EnvSetLLMapText( LPCTSTR text );
ENV_EXPORT void EnvRedrawMap( void );
ENV_EXPORT int EnvRunQueryBuilderDlg( LPTSTR buffer, int bufferSize );


// Standard Path Information
ENV_EXPORT int EnvStandardOutputFilename( LPTSTR filename, LPCTSTR pathAndFilename, int maxLength );
ENV_EXPORT int EnvCleanFileName( LPTSTR filename );

#ifdef __cplusplus
}
#endif


