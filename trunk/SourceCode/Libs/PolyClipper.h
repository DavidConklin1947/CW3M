/*
===========================================================================

Project:   Generic Polygon Clipper

           A new algorithm for calculating the difference, intersection,
           exclusive-or or union of arbitrary polygon sets.

File:      gpc.h
Author:    Alan Murta (email: gpc@cs.man.ac.uk)
Version:   2.32
Date:      17th December 2004

Copyright: (C) 1997-2004, Advanced Interfaces Group,
           University of Manchester.

           This software is free for non-commercial use. It may be copied,
           modified, and redistributed provided that this copyright notice
           is preserved on all copies. The intellectual property rights of
           the algorithms used reside with the University of Manchester
           Advanced Interfaces Group.

           You may not use this software, in whole or in part, in support
           of any commercial product without the express consent of the
           author.

           There is no warranty or other guarantee of fitness of this
           software for any purpose. It is provided solely "as is".

===========================================================================
*/

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Frank has made extensive cosmetic changes

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#pragma once
#include "libs.h"

#include "Maplayer.h"

/*
===========================================================================
                               Constants
===========================================================================
*/

/* Increase GPC_EPSILON to encourage merging of near coincident edges    */

#define GPC_EPSILON (DBL_EPSILON*10)

#define GPC_VERSION "2.32"

typedef enum                         /* Set operation type                */
   {
   GPC_DIFF,                         /* Difference                        */
   GPC_INT,                          /* Intersection                      */
   GPC_XOR,                          /* Exclusive or                      */
   GPC_UNION                         /* Union                             */
   } GPC_OP;

typedef Vertex gpc_vertex;

typedef struct                       /* Vertex list structure             */
   {
   int                 num_vertices; /* Number of vertices in list        */
   Vertex             *vertex;       /* Vertex array pointer              */
   } gpc_vertex_list;

typedef struct                       /* Polygon set structure             */
   {
   int                 num_contours; /* Number of contours in polygon     */
   gpc_vertex_list    *contour;      /* Contour array pointer             */
   } gpc_polygon;

/*
===========================================================================
                               Class
===========================================================================
*/

class LIBSAPI PolyClipper
   {
   private:
      PolyClipper(){}  // no need to declare an instance.  Just call the static member function.
      ~PolyClipper(){}

   public:
      static void PolygonClip( GPC_OP set_operation, Poly &subjectPolygon, Poly &clipPolygon, Poly &resultPolygon );
      static void PolygonUnion( /*const*/ CArray< Poly*, Poly* > &polyArray, const CArray< int, int > &queryResults, Poly &resultPolygon );

   private:
      static void gpc_polygon_clip( GPC_OP set_operation, gpc_polygon *subj, gpc_polygon *clip, Poly &resultPolygon );
   };
