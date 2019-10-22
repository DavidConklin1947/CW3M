#pragma once

#include "datagrid.h"
#include "UltimateToolbox\OXLayoutManager.h"
#include "afxwin.h"
#include "FindDataDlg.h"

#include <DbTable.h>


//==================== DbTableDataSource ============================

struct DTDS_PROPS
{
   COLORREF bkgrColor;

   DTDS_PROPS( COLORREF _bkgrColor ) : bkgrColor( _bkgrColor ) { } 
   DTDS_PROPS() : bkgrColor( 0xFFFFFF ) { }
   DTDS_PROPS( DTDS_PROPS &p ) : bkgrColor( p.bkgrColor ) { }
};


struct DTDS_ROW_COL
{
public:
   long row;
   int  col;

   DTDS_ROW_COL() : row( -1 ), col( -1 ) { }

   DTDS_ROW_COL( DTDS_ROW_COL &rc ) { *this = rc; }
   DTDS_ROW_COL( long _row, int _col ) { row = _row; col = _col; }

   DTDS_ROW_COL& operator = (DTDS_ROW_COL &rc ) { row = rc.row; col=rc.col; return *this; }
};

class DbTableDataSource : public CUGDataSource
{
public:
   DbTableDataSource( DbTable *pDbTable ); 
   ~DbTableDataSource() { /*if ( m_propsMatrix ) delete m_propsMatrix;*/ }

public:
   virtual int  GetCell( int col, long row, CUGCell *cell );
   virtual int  SetCell( int col, long row, CUGCell *cell );

   virtual int  GetNumCols() { ASSERT( m_pDbTable ); return m_pDbTable->GetColCount(); }
   virtual long GetNumRows() { ASSERT( m_pDbTable ); return m_pDbTable->GetRecordCount(); }

   int FindFirst( CString *string, int *col, long *row, int flags ) { *row=0; return FindNext( string, col, row, flags ); };
   int FindNext ( CString *string, int *col, long *row, int flags );
   int FindAll  ( CString *string, int  col, CArray< DTDS_ROW_COL, DTDS_ROW_COL& > &foundRowCols, int flags );

public:
   DbTable *m_pDbTable;
   int      m_index;    // UG Grid index associated with this datasource

protected:
   COLORREF m_selectionColor;
   COLORREF m_highlightColor;

   CMap< long, long, DTDS_PROPS, DTDS_PROPS& > m_selectionSet;  // index, selectionState

public:
   void AddSelection( int col, long row, bool highlightRow ); 
   void ClearSelection( int col, long row );
   void ClearAll( void );
 
protected:
   long GetPropIndex( int col, long row );

};


//========================= DataPanel Dialog ========================h

class DataPanel : public CDialog
{
	DECLARE_DYNCREATE(DataPanel)

protected:
public:
	DataPanel();           // protected constructor used by dynamic creation
	virtual ~DataPanel();

   BOOL Create(CWnd *pParent);

protected:
	enum { IDD = IDD_DATAPANEL };

public:
   DataGrid    m_grid;

protected:
   FindDataDlg m_findDlg;
   COXLayoutManager m_layoutManager;

   bool m_isDirty;

   CArray< DbTableDataSource*, DbTableDataSource* > m_dataSourceArray;

public:
   int  AddDataTable( DbTable *pData );
   void ClearGrid();
   int  LoadDataTable();

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_dataSources;
   afx_msg void OnCbnSelchangeDatasources();
   afx_msg void OnBnClickedSave();
   afx_msg void OnBnClickedSaveas();
   afx_msg void OnBnClickedEdit();
   afx_msg void OnBnClickedSearch();
   afx_msg void OnBnClickedQuery();
   afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
   afx_msg void OnBnClickedRefresh();
   };

