#pragma once
#include "libs.h"

#include "Maplayer.h"
#include "Typedefs.h"

#include <float.h>

class LIBSAPI PolyBuffer
   {
   private:

      class MVector
         {
         public:
            double m_x;
            double m_y;
            int m_subjectSide;

            MVector() : m_x( 0.0 ), m_y( 0.0 ), m_subjectSide( -1 ) {}
            MVector( double x, double y, int subjectSide = -1 ) : m_x( x ), m_y( y ), m_subjectSide( subjectSide ) {}
            MVector( const MVector& v ){ *this = v; }
            MVector& operator=( const MVector& v ){ m_x = v.m_x; m_y = v.m_y; m_subjectSide = v.m_subjectSide; return *this; }
         };

      typedef CArray< MVector, MVector& > MVectorArray;

      class MStarDiagram
         {
         public:
            MStarDiagram() : m_index( -1 ) {}
            ~MStarDiagram();

         public:
            int m_index;
            MVectorArray m_vectorArray;

         public:
            void  Add( double x, double y, int subjectSide ){ m_vectorArray.Add( MVector( x, y, subjectSide ) ); }
            const MVector* BufferAtVertex( const Poly &subjectPoly, int startIndex, int subjectIndex, double &x, double &y, double distance );
            void  Sort();
            void  ClearSubjectSides();
            const MVector* GetNext();
            const MVector* GetSubject( int subjectIndex, int &starIndex );

         private:
            static int Compare( const void *p0, const void *p1 );
         };

      class MPolyBuilder
         {
         public:
            MPolyBuilder() : m_areaSum( 0.0 ) {}
            ~MPolyBuilder(){}

         private:
            MVectorArray m_vertexArray;
            MVectorArray m_lastPart;
            //CArray< MVectorArray, MVectorArray& > m_vertexArrayArray;
            Poly m_polygon;
            Poly m_holes;
            double m_areaSum;

         public:
            int  Add( double x, double y );
            //void Clear();
            void MakePart( bool island );
            void MakePoly( Poly &poly );
            //int  GetSize(){ return m_vertexArray.GetSize(); }
            //Vertex GetAsVertex( int index ){ return Vertex( m_vertexArray[index].m_x, m_vertexArray[index].m_y ); }

         private:
            void Test( int from, int to );
            bool Intersect( const MVector &a, const MVector &b, const MVector &c, const MVector &d, MVector &point );
         };

   private:
      PolyBuffer(){}
      ~PolyBuffer(){}

   public:
      static void MakeBuffer( const Poly &subjectPoly, Poly &resultPoly, float distance );
   };
