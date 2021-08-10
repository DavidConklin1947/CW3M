#include <FDataObj.h>
#include <MapLayer.h>

class FlowContext;
class Reach;


class HBV
{
public:
   HBV( void ) : m_pClimateData( NULL )
   , m_col_cfmax (-1)      
   , m_col_tt (-1)  
   , m_col_sfcf (-1) 
   , m_col_cwh  (-1)
   , m_col_cfr (-1)  
   , m_col_lp (-1)  
   , m_col_fc (-1)  
   , m_col_beta (-1)  
   , m_col_kperc(-1) 
   , m_col_wp(-1) 
   , m_col_k0 (-1)  
   , m_col_k1 (-1)  
   , m_col_uzl (-1)  
   , m_col_k2 (-1) 
   , m_col_ksoilDrainage(-1)
	, m_colDailyUrbanDemand(-1)
	, m_colH2OResidnt(-1)
	, m_colH2OIndComm(-1)
	, m_colIDUArea(-1)
	, m_colUGB( -1 )
   , m_colSNOWEVAP_D(-1)
   , m_colSNOWEVAP_Y(-1)
   , m_colSFCF(-1)
   , m_col_rain(-1)
   , m_col_snow(-1)
   , m_col_sfcfCorrection(-1)
   , m_precipTotal(0.0f)
   , m_colAREA(-1)
   , m_colSNOWPACK(-1)
   , m_colSNOWCANOPY(-1)
   , m_colSM2SOIL(-1)
   , m_colSM2ATM(-1)
   , m_colSM2ATM_YR(-1)
   , m_colHruSNOW_BOX(-1)
   , m_colHruSM2SOIL(-1)
   , m_colHruSM2ATM(-1)

   , m_colHruQ0(-1)
   , m_colHruQ2(-1)
   , m_colHruADDEDVOLM3(-1)
   { }
   ~HBV( void ) { if ( m_pClimateData ) delete m_pClimateData; }

   MapLayer * m_pIDUlayer;
   MapLayer * m_pReachLayer;
   MapLayer * m_pHRUlayer;

   int m_col_cfmax ;
   int m_col_tt ;  
   int m_col_sfcf ; 
   int m_col_cwh  ;
   int m_col_cfr ;  
   int m_col_lp ;  
   int m_col_fc ;  
   int m_col_beta ;  
   int m_col_kperc; 
   int m_col_wp; 
   int m_col_k0 ;  
   int m_col_k1 ;  
   int m_col_uzl ; 
   int m_col_ksoilDrainage;
   int m_col_k2 ; 
   int m_col_rain;
   int m_col_snow;
   int m_col_sfcfCorrection;
   float m_precipTotal;
   int m_col_SSURGO_WP;
   int m_col_thickness;

   //-------------------------------------------------------------------
   //------ hydrologic fluxes ------------------------------------------
   //-------------------------------------------------------------------

   //float PrecipFluxHandlerNoSnow( FlowContext *pFlowContext );
   //float N_Deposition_FluxHandler( FlowContext *pFlowContext );
   //float N_ET_FluxHandler( FlowContext *pFlowContext  );
   float Musle( FlowContext *pFlowContext );
   FDataObj *m_pClimateData;

   //HBV Vertical
   float Snow(float waterDepth, float precip, float temp, float CFMAX, float CFR, float TT );
   float Melt(float waterDepth, float precip, float temp, float CFMAX, float CFR, float TT );
   float GroundWaterRecharge(float precip,float waterDepth, float FC,  float Beta); 
   float GroundWaterRechargeFraction(float waterDepth, float FC,  float Beta); 
   float Percolation(float waterDepth, float kPerc);

   //HBV Horizontal
   float Q0( float waterDepth, float k0, float k1, float UZL );
   float Q2( float waterDepth , float k2 );

   float InitHBV_Global(FlowContext *pFlowContext, LPCTSTR); 

   float HBVdailyProcess(FlowContext *pFlowContext);
//x   float HBVdailyProcess(FlowContext * pFlowContext);

   float ET( float waterDepth, float fc, float lp, float wp, float temp, int month, int doy, float &etRc);
   float CalcET(  float temp, int month, int doy ) ;//hargreaves equation

   //-------------------------------------------------------------------  
   //------- urban water demand ----------------------------------------
   //-------------------------------------------------------------------  
public:
	BOOL CalcDailyUrbanWater(FlowContext *pFlowContext);
	BOOL CalcDailyUrbanWaterInit(FlowContext *pFlowContext);
	BOOL CalcDailyUrbanWaterRun(FlowContext *pFlowContext);

protected:
	int   m_colDailyUrbanDemand;	// Calculated Daily Urband Water Demand m3/day
	int   m_colH2OResidnt;			// Annual Residential Demand ccf/day/acre
	int 	m_colH2OIndComm;			// Annual Residential Demand ccf/day/acre
	int   m_colIDUArea;				// IDU area from IDU layer
	int   m_colUGB;					// IDU UGB from IDU layer
   int   m_colSNOWEVAP_D;        
   int   m_colSNOWEVAP_Y;      
   int   m_colSNOWTHRU_D;
   int   m_colSNOWTHRU_Y;
   int   m_colSFCF;              // IDU snowfall correction factor
   int   m_colRAINTHRU_D;
   int   m_colRAINTHRU_Y;

   int m_colAREA;
   int m_colSNOWPACK;
   int m_colSNOWCANOPY;
   int m_colSM2SOIL;
   int m_colSM2ATM;
   int m_colSM2ATM_YR;

   int m_colHruSNOW_BOX;
   int m_colHruSM2SOIL;
   int m_colHruSM2ATM;

   int m_colHruQ0;
   int m_colHruQ2;
   int m_colHruADDEDVOLM3;

};