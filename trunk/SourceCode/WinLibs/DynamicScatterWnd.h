#pragma once
#include "scatterwnd.h"


enum DSP_FLAG { DSP_USELASTONLY, DSP_USECOMPOSITETIME, DSP_USECOMPOSITESPACE }; 


class WINLIBSAPI DynamicScatterWnd : public ScatterWnd
   {
   public:
      DynamicScatterWnd(void);
      
      ~DynamicScatterWnd(void);

      int AddDataObj( FDataObj *pDataObj ) { return (int) m_dataObjArray.Add( pDataObj ); }
      bool CreateDataObjectFromComposites( DSP_FLAG, bool makeDataLocal );

   private:
      FDataObj *m_pCompositeDataObj;
      CArray< FDataObj*, FDataObj* > m_dataObjArray;
   };
