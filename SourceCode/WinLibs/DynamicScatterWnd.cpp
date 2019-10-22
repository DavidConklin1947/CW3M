#include "winlibs.h"
#include "DynamicScatterWnd.h"
#include <FDATAOBJ.H>

DynamicScatterWnd::DynamicScatterWnd(void)
: ScatterWnd()
, m_pCompositeDataObj( NULL)
{ }

DynamicScatterWnd::~DynamicScatterWnd(void)
   {
   for ( int i=0; i < m_dataObjArray.GetSize(); i++ )
      {
      //if ( m_dataObjArray[ i ] != NULL )      // assume plugins manage these
      //   delete m_dataObjArray[ i ];
      }

   if ( m_pCompositeDataObj != NULL )
      delete m_pCompositeDataObj;
   }


bool DynamicScatterWnd::CreateDataObjectFromComposites( DSP_FLAG  flag, bool makeDataLocal )
   {
   switch( flag )
      {
      case DSP_USELASTONLY:
         {
         //for ( int i=0; i < m_dataObjArray.GetSize()-1; i++ )
         //   ASSERT( m_dataObjArray[ i ] == NULL );
         if ( m_pCompositeDataObj != NULL )
            delete m_pCompositeDataObj;

         // NOTE - ASSUME source data is FDataObj - could be IDataObj
         //DataObj *pDO = m_dataObjArray [ m_dataObjArray.GetSize()-1 ]
         FDataObj *pLastDataObj = (FDataObj*) m_dataObjArray [ m_dataObjArray.GetSize()-1 ];
         ASSERT( pLastDataObj != NULL );
         m_pCompositeDataObj = new FDataObj( *pLastDataObj );   // HACK!!!!!

         this->SetDataObj( m_pCompositeDataObj, makeDataLocal );   // GraphWnd Method
         }
      return true;

      case DSP_USECOMPOSITETIME:      
      case DSP_USECOMPOSITESPACE:
         ASSERT(  0 );
      }

   return false;
   }


