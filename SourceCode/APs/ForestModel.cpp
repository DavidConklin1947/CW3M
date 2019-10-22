// ForestModel.cpp 
//

#include "stdafx.h"
#include "APs.h"

#include <Maplayer.h>
#include <Map.h>
#include <math.h>
#include <UNITCONV.H>
#include <path.h>
#include <EnvEngine\EnvModel.h>
#include <EnvInterface.h>
#include <direct.h>
#include <random>
#include <PathManager.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool WW2100AP::InitRunDGVMvegtypeData( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   m_projectionWKT = pContext->pMapLayer->m_projection;

   CString pathAndFileName = ReplaceSubstring(m_DGVMvegtypefile.DGVMvegtype_filename, "CLIMATE_SCENARIO_NAME", m_climateScenarioName);
   pathAndFileName = PathManager::MakeAbsolutePath(pathAndFileName, PM_PROJECT_DIR);
   CString msg;
   msg.Format( "Forest Model: Opening DGVM vegtype file %s", (LPCTSTR) pathAndFileName );
   Report::LogMsg( msg, RT_INFO );

   CString varName(_T("VTYPE"));
   m_pDataObj_DGVMvegtype = new GeoSpatialDataObj();
   m_pDataObj_DGVMvegtype->InitLibraries();
   m_pDataObj_DGVMvegtype->Open( (LPCTSTR) pathAndFileName, (LPCTSTR) varName );
   m_pDataObj_DGVMvegtype->ReadSpatialData();

   GetDGVMvegtypeData( pContext, 0 );

   return true;
   }  // end of InitRunDGVMvegtypeData()


int WW2100AP::CrosswalkVtypeToKuchler(int vtype)
   {
   int kuchler = 0;
   switch (vtype)
      {
      case 7: // MC2 maritime needleleaf forest -> Kuchler 2 Cedar/Hemlock/Douglas-fir
         kuchler = 2; break;
      case 36: // MC2 moist temperate needleleaf forest -> Kuchler 3 Silver fir/Douglas-fir
         kuchler = 3; break;
      case 6: // MC2 subalpine forest -> Kuchler 4 Fir/Hemlock
         kuchler = 4; break;
      case 10: // MC2 temperate cool mixed forest ->Kuchler 25 Alder/Ash
         kuchler = 25; break;
      case 11: // MC2 temperate warm mixed forest -> Kuchler 28 "Mosaic of #2 and #26"
         kuchler = 28; break;
      default: // other MC2 -> unclassified 0
         kuchler = 0; break;
      }
   return(kuchler);
   } // end of CrosswalkVtypeToKuchler()


bool WW2100AP::GetDGVMvegtypeData( EnvContext *pContext, int yearNdx )
// Gets the DGVM vegtype data for the specified year.
// Also, populate the Kuchler veg class attribute, from the DGVM vegtype.
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   // loop through the IDUs
   CString msg;
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      int prev_vtype = -1; pLayer->GetData(idu, m_colDGVMvegtyp, prev_vtype);
      int vtype = prev_vtype;

      int gridCellIndex; int &r_gridCellIndex = gridCellIndex;
      pLayer->GetData(idu, m_colGrdCellNdx, gridCellIndex);
      vtype = (int)m_pDataObj_DGVMvegtype->Get(r_gridCellIndex, yearNdx);

      // If the new DGVMvegtyp is meaningful and different
      // from the current DGVMvegtyp, write out both DGVMvegtyp and KUCHLER.
      if (vtype > 0 && prev_vtype != vtype)
         {
         AddDelta(pContext, idu, m_colDGVMvegtyp, vtype);
         int kuchler = CrosswalkVtypeToKuchler(vtype);
         AddDelta(pContext, idu, m_colKUCHLER, kuchler);
         } // end of if (vtype > 0 && prev_vtype != vtype)
      } // end IDU loop

   return TRUE;
   } // end of GetDGVMvegTypeData(pContext, yearNdx)


bool WW2100AP::RunDGVMvegtypeData( EnvContext *pContext )
// Gets the DGVM vegtype data for the next timestep, except do nothing if this is the final timestep.
   {
   testMessage(pContext, _T("RunDGVMvegtypeData"));
   if (pContext->currentYear==pContext->endYear) return TRUE;

   bool rtnFlag = FALSE;

   rtnFlag = GetDGVMvegtypeData(pContext, pContext->yearOfRun);

   return(rtnFlag);
   } // end of RunDGVMvegtypeData(pContext)


float WW2100AP::LeafAreaIndex(CString vegclassAbbrev)
   {
      int iRec = find(&m_LAIandC_table, m_records_LAIandC, m_colLAIandC_Abbrev, vegclassAbbrev);
      float LAI = 0.f;
      if (iRec>=0) LAI = m_LAIandC_table.GetAsFloat(m_colLAIandC_LAI, iRec);
      else
         {
         CString msg;
         msg.Format("LeafAreaIndex: can't find %s in LAIandC file", (LPCTSTR) vegclassAbbrev);
         Report::LogMsg(msg);
         }
      return(LAI);
   }


VegState::VegState(CString PVTtext, CString CTSStext): m_valid_forest_state(false)
   {
   const PVTencoding PVTs[] = {
      {_T("OWC_fal"), _T("OWC"), _T("Subalpine parkland"), fmh},
      {_T("OCR_fcw"), _T("OCR"), _T("White fir - cool"), unk},
      {_T("OWC_fcw"), _T("OWC"), _T("White fir - cool"), unk},
      {_T("OCR_fdm"), _T("OCR"), _T("Douglas-fir - moist"), fdd},
      {_T("OWC_fdm"), _T("OWC"), _T("Douglas-fir - moist"), fdd},
      {_T("OCR_fdw"), _T("OCR"), _T("Douglas-fir/white oak"), fdw},
      {_T("OWC_fdw"), _T("OWC"), _T("Douglas-fir/white oak"), fdw},
      {_T("OCR_fdx"), _T("OCR"), _T("Douglas-fir - xeric"), unk},
      {_T("OWC_fdx"), _T("OWC"), _T("Douglas-fir - xeric"), unk},
      {_T("OCR_fhc"), _T("OCR"), _T("Western hemlock - coastal"), fwi},
      {_T("OCR_fiw"), _T("OCR"), _T("White fir - intermediate"), fdd},
      {_T("OWC_fiw"), _T("OWC"), _T("White fir - intermediate"), fdd},
      {_T("OWC_fmc"), _T("OWC"), _T("Mountain hemlock - cold/dry (coastal, west Cascades"), fmh},
      {_T("OWC_fmh"), _T("OWC"), _T("Mountain hemlock - cold/dry"), fmh},
      {_T("OWC_fmi"), _T("OWC"), _T("Mountain hemlock - intermediate"), fmh},
      {_T("OCR_fsi"), _T("OCR"), _T("Pacific silver fir - intermediate"), fsi},
      {_T("OWC_fsi"), _T("OWC"), _T("Pacific silver fir - intermediate"), fsi},
      {_T("OCR_fss"), _T("OCR"), _T("Sitka spruce"), fwi},
      {_T("OCR_fsw"), _T("OCR"), _T("Pacific silver fir - warm"), fsi},
      {_T("OWC_fsw"), _T("OWC"), _T("Pacific silver fir - warm"), fsi},
      {_T("OCR_fvg"), _T("OCR"), _T("Grand fir - valley"), fvg},
      {_T("OWC_fvg"), _T("OWC"), _T("Grand fir - valley"), fvg},
      {_T("OCR_fwc"), _T("OCR"), _T("Western hemlock - cold"), fwi},
      {_T("OWC_fwc"), _T("OWC"), _T("Western hemlock - cold"), fwi},
      {_T("OCR_fwi"), _T("OCR"), _T("Western hemlock - intermediate"), fwi},
      {_T("OWC_fwi"), _T("OWC"), _T("Western hemlock - intermediate"), fwi},
      {_T("OCR_fwm"), _T("OCR"), _T("Western hemlock - moist"), fwi},
      {_T("OWC_fwm"), _T("OWC"), _T("Western hemlock - moist"), fwi},
      {_T("OCR_fwn"), _T("OCR"), _T("Western hemlock - moist (coastal)"), fwi},
      {_T("OCR_fww"), _T("OCR"), _T("Western hemlock - wet"), fwi},
      {_T("OWC_fww"), _T("OWC"), _T("Western hemlock - wet"), fwi},
      {_T("OCR_fwx"), _T("OCR"), _T("Western hemlock - hyperdry"), fwi},
      {_T("OWC_fwx"), _T("OWC"), _T("Western hemlock - hyperdry"), fwi},
      {_T("na"), _T("NA"), _T("not available"), unk}};
   const CTSSencoding CoverTypes[] = {
      {_T("B"), _T("Bare ground"), 199},
      {_T("GS"), _T("Grass/shrub"), 200},
      {_T("PK"), _T("Subalpine parkland"), 205},
      {_T("Al"), _T("Red alder"), 250},
      {_T("AW"), _T("Trembling aspen/willow"), 255},
      {_T("BM"), _T("Big leaf maple"), 270},
      {_T("Oa"), _T("Oregon white oak"), 300},
      {_T("OP"), _T("Oregon white oak/Ponderosa pine"), 305},
      {_T("TO"), _T("Tanoak"), 310},
      {_T("Ow"), _T("Oak woodland"), 315},
      {_T("OD"), _T("Oregon white oak/Douglas-fir"), 325},
      {_T("DFal"), _T("Douglas-fir/Red alder"), 350},
      {_T("DO"), _T("Douglas-fir/White oak"), 375},
      {_T("DF"), _T("Douglas-fir"), 400},
      {_T("DFmx"), _T("Douglas-fir mix"), 405},
      {_T("DFWF"), _T("Douglas-fir/White fir"), 410},
      {_T("DFWH"), _T("Douglas-fir/Western hemlock"), 415},
      {_T("DG"), _T("Douglas-fir/Grand fir"), 420},
      {_T("GFES"), _T("Grand fir/Engelmann spruce"), 425},
      {_T("LPWL"), _T("Lodgepole pine/Western larch"), 430},
      {_T("SFDF"), _T("Pacific silver fir/Douglas-fir"), 435},
      {_T("WLLP"), _T("Western larch/Lodgepole pine"), 440},
      {_T("WH"), _T("Western hemlock"), 445},
      {_T("GF"), _T("Grand fir"), 450},
      {_T("WF"), _T("White fir"), 455},
      {_T("RF"), _T("Shasta red fir"), 505},
      {_T("RFWF"), _T("Red fir/White fir"), 510},
      {_T("Co"), _T("Conifer"), 520},
      {_T("ESAF"), _T("Engelmann spruce/Subalpine fir"), 600},
      {_T("MH"), _T("Mountain hemlock"), 605},
      {_T("WB"), _T("Whitebark pine"), 610},
      {_T("LP"), _T("Lodgepole pine"), 705},
      {_T("LPWI"), _T("Lodgepole pine/Wildland-Urban interface"), 710},
      {_T("MXPI"), _T("Mixed pine"), 750},
      {_T("PP"), _T("Ponderosa pine"), 755},
      {_T("PPLP"), _T("Ponderosa pine/Lodgepole pine"), 760},
      {_T("WP"), _T("Western white pine"), 765},
      {_T("JP"), _T("Jeffrey pine"), 770},
      {_T("SK"), _T("Sitka spruce"), 800}};
   const CTSSencoding StructuralStages[] = {
      {_T("GF"), _T("Grass forb"), 200},
      {_T("GFp"), _T("Grass forb post-disturbance"), 210},
      {_T("Me"), _T("Meadow"), 220},
      {_T("HBc"), _T("Closed HerbLand"), 230},
      {_T("S"), _T("Shrubs"), 300},
      {_T("SHo"), _T("OpenShrub"), 310},
      {_T("Sho"), _T("OpenShrub"), 310},
      {_T("SHc"), _T("ClosedShrub"), 330},
      {_T("SHc2"), _T("ClosedShrub_multi"), 335},
      {_T("SHp"), _T("Shrubs post-disturbance"), 340},
      {_T("Yo"), _T("SSap_open"), 410},
      {_T("Ym"), _T("SSap_med"), 420},
      {_T("Yop"), _T("SSap_PostDist"), 440},
      {_T("Po1"), _T("Pole_open_single"), 511},
      {_T("Po2"), _T("Pole_open_multi"), 512},
      {_T("Pm1"), _T("Pole_med_single"), 521},
      {_T("Pm2"), _T("Pole_med_multi"), 522},
      {_T("Pc1"), _T("Pole_close_single"), 531},
      {_T("Pc2"), _T("Pole_close_multi"), 532},
      {_T("P1p"), _T("Pole_single_PostDist"), 541},
      {_T("P2p"), _T("Pole_multi_PostDist"), 542},
      {_T("So1"), _T("Smtree_open_single"), 611},
      {_T("So2"), _T("Smtree_open_multi"), 612},
      {_T("Sm1"), _T("Smtree_med_single"), 621},
      {_T("Sm2"), _T("Smtree_med_multi"), 622},
      {_T("Sc1"), _T("Smtree_close_single"), 631},
      {_T("Sc2"), _T("Smtree_close_multi"), 632},
      {_T("S1p"), _T("Smtree_single_PostDist"), 641},
      {_T("S2p"), _T("Smtree_multi_PostDist"), 642},
      {_T("Mo1"), _T("Mdtree_open_single"), 711},
      {_T("Mo2"), _T("Mdtree_open_multi"), 712},
      {_T("Mm1"), _T("Mdtree_med_single"), 721},
      {_T("Mm2"), _T("Mdtree_med_multi"), 722},
      {_T("Mc1"), _T("Mdtree_close_single"), 731},
      {_T("Mc2"), _T("Mdtree_close_multi"), 732},
      {_T("M1p"), _T("Mdtree_single_PostDist"), 741},
      {_T("Lo1"), _T("Lgtree_open_single"), 811},
      {_T("Lo2"), _T("Lgtree_open_multi"), 812},
      {_T("Lm1"), _T("Lgtree_med_single"), 821},
      {_T("Lm2"), _T("Lgtree_med_multi"), 822},
      {_T("Lc1"), _T("Lgtree_close_single"), 831},
      {_T("Lc2"), _T("Lgtree_close_multi"), 832},
      {_T("L1p"), _T("Lgtree_single_PostDist"), 841},
      {_T("Go1"), _T("Gttree_open_single"), 911},
      {_T("Go2"), _T("Gttree_open_multi"), 912},
      {_T("Gm1"), _T("Gttree_med_single"), 921},
      {_T("Gm2"), _T("Gttree_med_multi"), 922},
      {_T("Gc1"), _T("Gttree_close_single"), 931},
      {_T("Gc2"), _T("Gttree_close_multi"), 932},
      {_T("G1p"), _T("Gttree_single_PostDist"), 941}};

   m_foundPVTflag = m_foundCTflag = m_foundSSflag = m_valid_forest_state = false;
   PVTtext.TrimRight();
   CTSStext.TrimRight();
   if (PVTtext.IsEmpty() && CTSStext.IsEmpty()) return;

   int numPVTs = sizeof(PVTs)/sizeof(PVTencoding);
   int pvtNdx;
   if (!PVTtext.IsEmpty()) for (int i = 0; !m_foundPVTflag && i<numPVTs; i++)
      {
      pvtNdx = i;
      m_foundPVTflag = PVTtext.Compare(PVTs[i].abbrev)==0;
      }
   m_stratum_id = m_foundPVTflag ? PVTs[pvtNdx].stratum : -99;

   int CTSSlen = CTSStext.GetLength();
   int colon_position = CTSStext.Find(':', 0);
   if (colon_position<1 || colon_position>(CTSSlen-2)) return;

   int numCoverTypes = sizeof(CoverTypes)/sizeof(CTSSencoding);
   m_cover_type = CTSStext.Left(colon_position);
   int ctNdx;
   if (!m_cover_type.IsEmpty()) for (int i = 0; !m_foundCTflag && i<numCoverTypes; i++)
      {
      ctNdx = i;
      m_foundCTflag = m_cover_type.Compare(CoverTypes[i].abbrev)==0;
      }
   m_cover_type_id = m_foundCTflag ? CoverTypes[ctNdx].id : -99;

   int numStructuralStages = sizeof(StructuralStages)/sizeof(CTSSencoding);
   m_structural_stage = CTSStext.Right(CTSSlen - (colon_position + 1));
   int ssNdx;
   if (!m_structural_stage.IsEmpty()) for (int i = 0; !m_foundSSflag && i<numStructuralStages; i++)
      {
      ssNdx = i;
      m_foundSSflag = m_structural_stage.Compare(StructuralStages[i].abbrev)==0;
      }
   m_structural_stage_id = m_foundSSflag ? StructuralStages[ssNdx].id : -99;

   m_valid_forest_state = m_foundPVTflag && m_foundCTflag && m_foundSSflag;

   m_ctss_id = m_valid_forest_state ? 
         m_cover_type_id*10000 + m_structural_stage_id*10 : -99;
   
   return;

   } // end of VegState::VegState(PVTtext, CTSStext)


