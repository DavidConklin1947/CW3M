#pragma once

#include "EnvEngine/EnvContext.h"

#ifndef NO_MFC
BOOL CALLBACK MoveChildrenProc( HWND hwnd, LPARAM lParam );
#endif

class EnvExtension
   {
   public:
      EnvExtension() : m_inputVars(), m_outputVars(), m_id( -1 ), m_pEnvExtension( NULL ) { }
      EnvExtension( int inputCount, int outputCount );
      virtual ~EnvExtension();

      // Exposed entry points - override these if so desired
      virtual BOOL Init   ( EnvContext *pEnvContext, LPCTSTR initStr )    { m_pEnvExtension = pEnvContext->pExtensionInfo; m_initInfo = initStr; return TRUE; }
      virtual BOOL InitRun( EnvContext *pEnvContext, bool useInitialSeed ){ return TRUE; }
      virtual BOOL Run    ( EnvContext *pContext )                        { return TRUE; }
      virtual BOOL EndRun ( EnvContext *pContext )                        { return TRUE; }
      virtual BOOL Setup( EnvContext *pContext, HWND hWnd )      { return FALSE; }
      virtual int  InputVar ( int id, MODEL_VAR** modelVar );
      virtual int  OutputVar( int id, MODEL_VAR** modelVar );

      // call this if you HAVEN'T allocated variables in the constructor
      int AddVar( bool input, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR desc, int flags );
      // overrides
      int AddVar( bool input, LPCTSTR name, float  &var, LPCTSTR description, int flags ) { return AddVar( input, name, &var, TYPE_FLOAT,  MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddVar( bool input, LPCTSTR name, int    &var, LPCTSTR description, int flags ) { return AddVar( input, name, &var, TYPE_INT,    MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddVar( bool input, LPCTSTR name, double &var, LPCTSTR description, int flags ) { return AddVar( input, name, &var, TYPE_DOUBLE, MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddVar( bool input, LPCTSTR name, bool   &var, LPCTSTR description, int flags ) { return AddVar( input, name, &var, TYPE_BOOL,   MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }

      // more specialized versions
      int AddInputVar( LPCTSTR name, float  &var, LPCTSTR description, int flags=0 ) { return AddVar( true, name, &var, TYPE_FLOAT,  MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddInputVar( LPCTSTR name, int    &var, LPCTSTR description, int flags=0 ) { return AddVar( true, name, &var, TYPE_INT,    MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddInputVar( LPCTSTR name, double &var, LPCTSTR description, int flags=0 ) { return AddVar( true, name, &var, TYPE_DOUBLE, MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }
      int AddInputVar( LPCTSTR name, bool   &var, LPCTSTR description, int flags=0 ) { return AddVar( true, name, &var, TYPE_BOOL,   MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }

	   int AddOutputVar( LPCTSTR name, float  &var, LPCTSTR description, int flags=0 ) { return AddVar( false, name, &var, TYPE_FLOAT,   MD_CONSTANT, VData( var ), VData(), VData(),  description, flags ); }
      int AddOutputVar( LPCTSTR name, int    &var, LPCTSTR description, int flags=0 ) { return AddVar( false, name, &var, TYPE_INT,     MD_CONSTANT, VData( var ), VData(), VData(),  description, flags ); }
      int AddOutputVar( LPCTSTR name, double &var, LPCTSTR description, int flags=0 ) { return AddVar( false, name, &var, TYPE_DOUBLE,  MD_CONSTANT, VData( var ), VData(), VData(),  description, flags ); }
      int AddOutputVar( LPCTSTR name, bool   &var, LPCTSTR description, int flags=0 ) { return AddVar( false, name, &var, TYPE_BOOL,    MD_CONSTANT, VData( var ), VData(), VData(),  description, flags ); }
      int AddOutputVar( LPCTSTR name, DataObj*var, LPCTSTR description, int flags=0 ) { return AddVar( false, name, var,  TYPE_PDATAOBJ, MD_CONSTANT, VData( var ), VData(), VData(), description, flags ); }

      // call this if you HAVE allocated variables in the constructor
      bool DefineVar( bool input, int index, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR desc );
      // overrides
      bool DefineVar( bool input, int index, LPCTSTR name, float  &var, LPCTSTR description ) { return DefineVar( input, index, name, &var, TYPE_FLOAT,  MD_CONSTANT, VData( var ), VData(), VData(), description ); }
      bool DefineVar( bool input, int index, LPCTSTR name, int    &var, LPCTSTR description ) { return DefineVar( input, index, name, &var, TYPE_INT,    MD_CONSTANT, VData( var ), VData(), VData(), description ); }
      bool DefineVar( bool input, int index, LPCTSTR name, double &var, LPCTSTR description ) { return DefineVar( input, index, name, &var, TYPE_DOUBLE, MD_CONSTANT, VData( var ), VData(), VData(), description ); }
      bool DefineVar( bool input, int index, LPCTSTR name, bool   &var, LPCTSTR description ) { return DefineVar( input, index, name, &var, TYPE_BOOL,   MD_CONSTANT, VData( var ), VData(), VData(), description ); }

   public:
      INT_PTR AddDelta( EnvContext *pContext, int idu, int col, VData  newValue );
      // overrides
      INT_PTR AddDelta( EnvContext *pContext, int idu, int col, float  newValue ) { return AddDelta( pContext, idu, col, VData( newValue ) ); }
      INT_PTR AddDelta( EnvContext *pContext, int idu, int col, int    newValue ) { return AddDelta( pContext, idu, col, VData( newValue ) ); }
      INT_PTR AddDelta( EnvContext *pContext, int idu, int col, LPCSTR newValue ) { return AddDelta( pContext, idu, col, VData( newValue, true ) ); }
      INT_PTR AddDelta( EnvContext *pContext, int idu, int col, bool   newValue ) { return AddDelta( pContext, idu, col, VData( newValue ) ); }

   public:
      // CheckCol checks to see if a columns exists in a maplayer.  It can optionally add it.
      // For flags, see CC_xxx enum above
      static bool CheckCol( const MapLayer *pLayer, int &col, LPCTSTR label, TYPE type, int flags );
  
      bool UpdateIDU( EnvContext *pContext, int idu, int col, VData &value, bool useAddDelta );
      bool UpdateIDU( EnvContext *pContext, int idu, int col, float value,  bool useAddDelta );
      bool UpdateIDU( EnvContext *pContext, int idu, int col, int value,    bool useAddDelta );
      bool UpdateIDU( EnvContext *pContext, int idu, int col, double value, bool useAddDelta );
      bool UpdateIDU( EnvContext *pContext, int idu, int col, short value,  bool useAddDelta );
      bool UpdateIDU( EnvContext *pContext, int idu, int col, bool value,   bool useAddDelta );

   protected:
      CArray< MODEL_VAR, MODEL_VAR& > m_inputVars;
      CArray< MODEL_VAR, MODEL_VAR& > m_outputVars;

   public:
      int m_id;      // model ID.  subclasses should set this if needed

   protected:
      int m_type;    // Envision Extension type - see EnvContext.h for EET_xxx values; can be multiple types, or'd together

      ENV_EXTENSION *m_pEnvExtension;
      CString m_initInfo;
   };


class EnvEvalModel : public EnvExtension
   {
   public:
      EnvEvalModel( int inputCount, int outputCount ) : EnvExtension( inputCount, outputCount ) { }
      EnvEvalModel() : EnvExtension() { }
      virtual ~EnvEvalModel( void ) { }
 
   protected:
   };


class EnvAutoProcess : public EnvExtension
   {
   public:
      EnvAutoProcess( int inputCount, int outputCount ) : EnvExtension( inputCount, outputCount ) { }
      EnvAutoProcess() : EnvExtension() { }
      virtual ~EnvAutoProcess( void ) { }

      virtual BOOL ProcessMap( MapLayer *pLayer, int id ) { return TRUE;}
   };


// A visualizer is an object that manages the visual display of data.  The basic idea is that
// the visualizer plugin provides for the display of (potentionally multiple) windows of data.
// Envision manages the windows associated with a visualizer; the visualizer draws in/manages this window
// as needed.

class EnvVisualizer : public EnvExtension
   {
   public:
      EnvVisualizer( int extensionType ) : EnvExtension() { m_type = extensionType; }
      virtual ~EnvVisualizer( void ) { }
      
      //---------------------------------------------------------------------                    INPUT        RUNTIME      POSTRUN
      virtual BOOL Init        ( EnvContext*, LPCTSTR ) { return TRUE; }                   // this should alway be called by children if they override
      virtual BOOL InitRun     ( EnvContext*, bool )    { return TRUE; }      // for init of run         optional      optional     optional
      virtual BOOL Run         ( EnvContext* )          { return TRUE; }      // for runtime views       optional      required     optional

      virtual BOOL InitWindow  ( EnvContext*, HWND )    { return TRUE; }
      virtual BOOL UpdateWindow( EnvContext*, HWND )    { return TRUE; }
		//Window cleanup should be handled by WM_ON_CLOSE event or OnClose() override.
   };


class EnvRtVisualizer : public EnvVisualizer
   {
   public:
      EnvRtVisualizer( void ) : EnvVisualizer( VT_RUNTIME ) { }
      virtual ~EnvRtVisualizer( void ) { }
   };


class EnvInputVisualizer : public EnvVisualizer
   {
   public:
      EnvInputVisualizer( void ) : EnvVisualizer( VT_INPUT ) { }
      virtual ~EnvInputVisualizer( void ) { }
   };


class EnvPostRunVisualizer : public EnvVisualizer
   {
   public:
      EnvPostRunVisualizer( void ) : EnvVisualizer( VT_POSTRUN_GRAPH ) { }
      virtual ~EnvPostRunVisualizer( void ) { }
   };

