#pragma once

#include "libs.h"

#include "tinyxml.h"

#include "Typedefs.h"
#include "Maplayer.h"


struct XML_ATTR {
   LPCTSTR name;
   TYPE    type;
   void   *value;
   bool    isRequired;
   int     checkCol;
   };


//bool LIBSAPI TiXmlGetAttributes( TiXmlElement *pXml, XML_ATTR attrs[], LPCTSTR element, LPCTSTR filename, MapLayer *pLayer=NULL );  // pLayer only needed if CheckCol != 0 (DEPRECATED, use version defined below
bool LIBSAPI TiXmlGetAttributes( TiXmlElement *pXml, XML_ATTR attrs[], LPCTSTR filename, MapLayer *pLayer=NULL ); 


bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, int    &value,  int     defaultValue, bool isRequired );
bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, long   &value,  long    defaultValue, bool isRequired );
bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, float  &value,  float   defaultValue, bool isRequired );
bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, bool   &value,  bool    defaultValue, bool isRequired );
bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, LPTSTR  &value, LPCTSTR defaultValue, bool isRequired );
bool LIBSAPI TiXmlGetAttr( TiXmlElement *pElement, LPCTSTR attrName, CString &value, LPCTSTR defaultValue, bool isRequired );
void LIBSAPI TiXmlError( LPCTSTR name, LPCTSTR nodeName );

bool LIBSAPI GetXmlStr( LPCTSTR input, CString &output );
