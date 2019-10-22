#pragma once

class MapFrame;
class ScatterWnd;
class DataGrid;
class VDataTable;


/*------------------------------------------------------------
 * ActiveWnd - static class that manages the active window
 *
 * Possible Active Window include:
 *   1) MapFrames (main MapPanel or MapFrames embedded in a ResultsMapWnd)
 *   2) DataGrid (contained in the DataPanel)
 *   3) ScatterWnds(contained in the ResultWnds and RtViews)
 *   4) VDataTables( contained in ResultsWnd )
 *
 *  Any time any of these windows become activated, they should
 *  call ActiveWnd::SetActiveWnd();
 *------------------------------------------------------------*/

enum AWTYPE { AWT_UNKNOWN=-1, AWT_MAPFRAME, AWT_DATAGRID, AWT_SCATTERWND, AWT_VDATATABLE, AWT_VISUALIZER };

class ActiveWnd
   {
   public:
      ActiveWnd(void);
      ~ActiveWnd(void);

      static bool SetActiveWnd( CWnd *pWnd );

      static AWTYPE GetType();

      static void OnEditCopy();
      static void OnEditCopyLegend();
      static void OnFilePrint();
      static void OnFileExport();

      static void OnUpdateFilePrint( CCmdUI *pCmdUI );
      static void OnUpdateFileExport( CCmdUI *pCmdUI );
      static void OnUpdateEditCopy(CCmdUI *pCmdUI);

      static bool IsMapFrame() { return ( ( GetType() == AWT_MAPFRAME ) ? true : false ); }

      static MapFrame   *GetActiveMapFrame();
      static ScatterWnd *GetActiveScatterWnd();
      static DataGrid   *GetActiveDataGrid();
      static VDataTable *GetActiveVDataTable();

   protected:
      static CWnd *m_pWnd;

   public:
   };
