#pragma once

#include <afxtempl.h>
#include <tixml.h>
#include <PtrArray.h>

#include <QueryEngine.h>

using namespace std;

class HRU;

// a layer distribution consists of a layer index and a fraction
struct LayerDist
{
	int   layerIndex;
	float fraction;      // decimal percent
};

// layer consists of an array of layer distributions and an optional query 
struct Layer
{
	PtrArray< LayerDist > layerDistArray;
	Query *pLayerQuery;
   // bool isRatio;

   Layer() : pLayerQuery(NULL) /*, isRatio(false) */ { }
};

/*!
This class 
*/
class LayerDistributions
{
 public:

	 LayerDistributions();     // Default Constructor
	 ~LayerDistributions();    // Default Destructor

	 PtrArray< Layer > m_layerArray;

protected:
   CString m_layerDistStr;
   CString m_layerQueryStr;
   PtrArray< LayerDist > m_layerDistArray;
   QueryEngine *m_pLayerDistQE;

public:

	 // parses the layer distribution (index,fraction) into an array
	 // a layer's fractions of all the layer_distrubtions must total one
    int ParseLayerDistributions( PCTSTR ldString );

	 // parse the layers into an array
	 int ParseLayers(TiXmlElement *pXmlElement, MapLayer *pIDULayer, LPCTSTR filename);

	 //determine which layer query matches the idu
	 int GetLayerIndex(int idu);
};