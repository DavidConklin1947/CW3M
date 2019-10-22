#pragma once

#include <Vdataobj.h>

class DeltaArray;
class MapLayer;

class ColumnTrace
   {
   public:
      ColumnTrace( MapLayer *, int columnIndex, int run = -1, const DeltaArray *pDeltaArray = NULL );
      ~ColumnTrace();

   public:
      VData Get( int row );
      int   GetCurrentYear(){ return m_currentYear; }
      int   GetRowCount(){ return m_rows; }
      bool  SetCurrentYear( int year );

      bool Get( int row, float &value )         { return m_column.Get( 0, row, value); }
      bool Get( int row, double &value)         { return m_column.Get( 0, row, value); }
      bool Get( int row, COleVariant &value )   { return m_column.Get( 0, row, value); }
      bool Get( int row, VData &value )         { return m_column.Get( 0, row, value); }
      bool Get( int row, int &value )           { return m_column.Get( 0, row, value); }
      bool Get( int row, bool &value )          { return m_column.Get( 0, row, value); }

   private:
      int m_run;
      int m_columnIndex;
      int m_rows;
      int m_currentYear;  // interpret as beginning of year
      int m_maxYear;

      const DeltaArray *m_pDeltaArray;
      VDataObj m_column;
   };
