#if !defined _CWNDMNGR_H
#define _CWNDMNGR_H  1

#include <afxtempl.h>

#undef  AFX_DATA
#define AFX_DATA  WINLIBSAPI

 
				

class  WINLIBSAPI  CWndManager : public CArray< CWnd*, CWnd* >
   {
   protected:
      CWnd *pActive;

      bool deleteOnDelete;

   public:
      //-- constructor --//
      CWndManager( void );
      ~CWndManager();

      void Add   ( CWnd* );
      void Remove( CWnd* );

		void  SetActive( CWnd *pWnd ) { pActive = pWnd; }
      CWnd *GetActive( void ) 		{ return pActive;  }
   };

template class  WINLIBSAPI  CArray< CWnd*, CWnd* >;


 
#undef  AFX_DATA
#define AFX_DATA

#endif





