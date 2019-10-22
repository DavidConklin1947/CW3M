#include "StdAfx.h"
#pragma hdrstop

#include "LayerDistributions.h"
#include "AlgLib\AlgLib.h"
#include <stdstring.h>

LayerDistributions::LayerDistributions() :
m_pLayerDistQE(NULL)
   { }



LayerDistributions::~LayerDistributions(void)
   {

   if (m_pLayerDistQE != NULL)
      delete m_pLayerDistQE;
   }

int LayerDistributions::ParseLayerDistributions( PCTSTR ldString)
   {
   m_layerDistStr = ldString;

   // if string is empty
   if (m_layerDistStr.IsEmpty())
      return 0;

   CString str = m_layerDistStr;
   CString token;
   int curPos = 0;

   float fraction = 0.0;

   token = str.Tokenize(_T(",() ;\r\n"), curPos);

   while (token != _T(""))
      {
      LayerDist *pLD = new LayerDist;
      pLD->layerIndex = atoi(token);
      token = str.Tokenize(_T(",() ;\r\n"), curPos);
      pLD->fraction = (float)atof(token);
      token = str.Tokenize(_T(",() ;\r\n"), curPos);

      fraction += pLD->fraction;

      m_layerDistArray.Add(pLD);
      }

   if (alglib::fp_neq(fraction, 1.0))
      {
      CString msg;
      msg.Format(_T("LayerDistributions: Fractions of all the layer_distributions for a layer must total 1.0"));
      Report::ErrorMsg(msg);
      return -1;
      }

   return (int)m_layerDistArray.GetSize();
   }

int LayerDistributions::ParseLayers(TiXmlElement *pXmlElement, MapLayer *pIDULayer, LPCTSTR filename)
   {
   // Handle layers of layerDistribution
   TiXmlElement *pXmlLayers = pXmlElement->FirstChildElement("layers");
   if (pXmlLayers != NULL)
      {
      int defaultIndex = 0;
      TiXmlGetAttr(pXmlLayers, "default", defaultIndex, 0, false);

      TiXmlElement *pXmlLayer = pXmlLayers->FirstChildElement("layer");
      while (pXmlLayer != NULL)
         {
         // reset layer values
         m_layerDistArray.Clear();

         LPTSTR layerDistStr = NULL;
         LPTSTR layerQryStr = NULL;
         // bool isRatio = false;

         XML_ATTR layerAttrs[] = {
            // attr								type			address					     isReq	checkCol
               { "layer_distributions", TYPE_STRING, &layerDistStr, true, 0 },
               { "query", TYPE_STRING, &layerQryStr, false, 0 },
               // { "ratio", TYPE_BOOL, &isRatio, false, 0 },
               { NULL, TYPE_NULL, NULL, false, 0 } };

         bool ok = TiXmlGetAttributes(pXmlLayer, layerAttrs, filename);
         if (!ok)
            {
            CString msg;
            msg.Format(_T("Layer Distributions: Misformed element reading <layer> attributes in input file %s - it is missing the 'layer_distributions' attribute"), filename);
            Report::ErrorMsg(msg);
            break;
            }
         else
            {
            Layer *pLayer = new Layer;

            ParseLayerDistributions( layerDistStr );
            pLayer->layerDistArray = m_layerDistArray;

            // pLayer->isRatio = isRatio;

            m_layerQueryStr = layerQryStr;

            // a query exists
            if (m_layerQueryStr.GetLength() > 0 && pLayer->pLayerQuery == NULL)
               {
               if (!m_pLayerDistQE) m_pLayerDistQE = new QueryEngine(pIDULayer);
               pLayer->pLayerQuery = m_pLayerDistQE->ParseQuery(m_layerQueryStr, 0, "Layer Query");
               }
            else
               {
               // no query
               pLayer->pLayerQuery = NULL;
               }

            m_layerArray.Add(pLayer);
            }


         pXmlLayer = pXmlLayer->NextSiblingElement("layer");
         } // end of: while ( pXmlLayer != NULL )
      }

   m_layerDistArray.Clear();

   return (int)m_layerArray.GetSize();
   }

int LayerDistributions::GetLayerIndex(int idu)
   {
   // return the layer index that matches the idu
   int layersArraySize = (int)m_layerArray.GetSize();
   for (int index = 0; index < layersArraySize; index++)
      {
      // get the layer
      Layer *pSoilLayer = m_layerArray[index];

      // query exists 
      bool pass = true;
      bool ok = true;
      if (pSoilLayer->pLayerQuery)
         {
         ok = pSoilLayer->pLayerQuery->Run(idu, pass);
         }

      // found match
      if (ok && pass)
         {
         return index;
         }
      }
   return -1;
   }