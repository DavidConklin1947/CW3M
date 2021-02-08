/*
 *  ScienceFcns.cpp
 *  MC2
 */

#include "libs.h"
//#include "StdAfx.h"

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

#include "ScienceFcns.h"

#ifndef ASSERT
   #define ASSERT assert
#endif

ScienceFcns::ScienceFcns()
{
   int day, month;

   days_per_mo[0] = 31;       /* JAN */
   days_per_mo[1] = 28;       /* FEB */
   days_per_mo[2] = 31;       /* MAR */
   days_per_mo[3] = 30;       /* APR */
   days_per_mo[4] = 31;       /* MAY */
   days_per_mo[5] = 30;       /* JUN */
   days_per_mo[6] = 31;       /* JUL */
   days_per_mo[7] = 31;       /* AUG */
   days_per_mo[8] = 30;       /* SEP */
   days_per_mo[9] = 31;       /* OCT */
   days_per_mo[10] = 30;       /* NOV */
   days_per_mo[11] = 31;       /* DEC */
   day = -1;
   for (month = JAN; month <= DEC; month++)
   {
      month_days[month][0] = day + 1;
      day += days_per_mo[month];
      month_days[month][1] = day;
   }
} // end of ScienceFcns() constructor


/*
 * 
 * ----------------------------  efold
 * 
 */
/*
 * Do single value efold.
 * send old value in previous, current raw value in current.
 * returns current efolded value.
 */
float ScienceFcns::efold(float tau, float raw, float previous)
{
	float	weight = (float)(tau > 0.0f ? exp(-1.0f/tau) : 0.0f);
  float smoothed;
	
	smoothed = previous*weight + raw*(1.0f - weight);
  return(smoothed);
} // end of efold()








/*
********************************************************************************
**
**  name
**      satw, sati - saturation vapor pressure over water and ice
**
**  synopsis
**      double  satw(tk)
**      double  tk;
**
**      double  sati(tk)
**      double  tk;
**
**  description
**      Satw returns saturation vapor pressure over water (in Pa)
**      as a function of temperature (degrees K).
**
**      Sati returns saturation vapor pressure over ice (in Pa)
**      as a function of temperature (degrees K).
**
**  diagnostics
**      Sets Qerrno and writes message to Qerrstr.
**
**  history
**      July 1982: written by J. Dozier, Department of Geography, UCSB
**
**  bugs
*/





/*
 * 
 * ----------------------------  GrowingDegreeDays
 * Note: this algorithm actually estimates the growing degree days from the
 * middle of the month to the middle of the next month. When the sum of the
 * month lengths is odd, it may also understate the growing degree day sum by half a
 * days worth.
 */
float ScienceFcns::GrowingDegreeDays(float tmp[], float degree)
{
	int	mo, day;
	float	days, diff, delta, grow;
	float grow_days = 0.0;

	for (mo = JAN; mo <= DEC; mo++) 
  {
		days = ((float) ((days_per_mo[NEXT_MO(mo)] / 2.0) + (days_per_mo[mo] / 2.0)));
		diff = tmp[NEXT_MO(mo)] - tmp[mo];
		delta = diff / days;
		for (day = 0; day < (int) days; day++) 
    {
			grow = tmp[mo] + (day * delta);
			if (grow > degree) grow_days += grow - degree;
		}
	}
	
	return(grow_days);
} // end of GrowingDegreeDays()


// Estimate warmest mean monthly soil temperature from mean monthly air temperatures.
float ScienceFcns::estimate_max_soil_tmp(float meantmp[MONTHS]) 
{
  int month, dayy, julday, i, j, k;
  float n_days, tmp_delt, max_soil_tmp, tmp_sum;
  float tmp[DAYS_PER_YEAR];
  float soil_tmp[DAYS_PER_YEAR];
  float soil_mean[MONTHS];
  bool flagx;

  /* ESTIMATE DAILY MEAN TEMP BY INTERPOLATION */
  julday = 15; 
  flagx = FALSE;
  for (month = JAN; month <= DEC; month++)
  {
    n_days = (days_per_mo[NEXT_MO(month)] + days_per_mo[month]) / 2.0f; 
    n_days = (float)ceil(n_days); 
    tmp_delt = (meantmp[NEXT_MO(month)] - meantmp[month]) / n_days;
     
    for (dayy = 0; dayy < (int) n_days; dayy++)
    {
      if (julday == DAYS_PER_YEAR)
      {
        julday = 0;
        flagx = TRUE;
      }
      if ((julday == 15) && flagx) break;
      
      tmp[julday]  = meantmp[month] + (dayy * tmp_delt);
      julday++;
    }
  } 
 
  /* ESTIMATE DAILY SOIL TEMP FROM RUNNING AVE OF AIR TEMP */      
  for (i = 0; i < DAYS_PER_YEAR; i++)
  {
    tmp_sum = 0.;
    for (j = 13; j >= 0; j--)
    {
      if ((i - j) < 0.0) k = 364 + (i - j);
      else k = i - j;
      if (tmp[k]>0.) tmp_sum += tmp[k];
    }     
    soil_tmp[i] = tmp_sum / 14.0f;
    ASSERT(soil_tmp[i]>=0.0);
  }  
     
  /* ESTIMATE MONTHLY SOIL TEMP */ 
  max_soil_tmp = -100.;  
  for (month = JAN; month <= DEC; month++)
  {
    /* this is how it should be done
    tmp_sum = 0.;
    for (julday = month_days[month][MONTH_BEGIN]; julday<=month_days[month][MONTH_END]; julday++) tmp_sum += soil_tmp[julday];
    soil_mean[month] = tmp_sum/days_per_mo[month];
    */
    // this is how MC1 does it
    soil_mean[month] = (soil_tmp[month_days[month][MONTH_BEGIN]] + soil_tmp[month_days[month][MONTH_END]]) / 2.0f;

    if (soil_mean[month] > max_soil_tmp) max_soil_tmp = soil_mean[month]; 
  }
  
  return(max_soil_tmp);
} // end of estimate_max_soil_tmp()


float ScienceFcns::C3prodPct(float meantmp[])
{
  float   x, prod_C3, prod_C4, sum, C3pct;
    
  x = estimate_max_soil_tmp(meantmp);
  prod_C3 = normalizedC3prod(x);
  prod_C4 = normalizedC4prod(x);
 
  /* compute C3 "dominance" as percent of C3 + C4 prod */
  /* index used to parameterize CENTURY supergrass */
  /* ranges from 0-100 C3 dominance */
  sum = prod_C3 + prod_C4;
  C3pct = sum>0.0f ? (prod_C3 / sum) * 100.f : 0.0f;

  if (C3pct > 90.) C3pct = 100.;
  if (C3pct < 10.) C3pct = 0.;

  return(C3pct);
 
} // end of C3prodPct()


/*
 * 
 * ----------------------------  FindRatio
 * 
 */
float ScienceFcns::FindRatio(float x, float slope, float y_intercept)
{
	float		ratio;

	ratio = (slope * x) + y_intercept;
	ratio = MIN(ratio, 100.0f);
	ratio = MAX(ratio, 0.0f);
	return(ratio);
} // end of FindRatio()


/*
 * 
 * ----------------------------  FindSlopeYIntercept
 * 
 */
/********
	Find the slope and y-intercept of a line passing between 2 points.
*********/
void ScienceFcns::FindSlopeYIntercept(float X1, float Y1, float X2, float Y2, float * slopeP, float * y_interceptP)
{
  float slope;
  
	slope = (Y2 - Y1)/(X2 - X1); 
  *slopeP = slope;
	*y_interceptP = Y1 - (slope * X1);
} // end of FindSlopeYIntercept()


/* compute relative production of 100% C3 canopy using CENTURY model function */
float ScienceFcns::normalizedC3prod(float x)
{ // x is mean soil temp in warmest month
  float prod_C3;
  double xx, prodd;
  
  xx = x;   
  prodd = (0.1788196117882357 + xx*(0.03606057742960269 + xx* -0.001302093024389319)) 
      / (1.0 + xx * (-0.08856743168395311 + xx*(0.004250159047418467 + xx* -6.002654039746731E-05)));
  prod_C3 = (float)prodd;
  if (prod_C3 < 0.0f) prod_C3 = 0.0f;     
  else if (prod_C3 > 1.0f) prod_C3 = 1.0f;
  
  return(prod_C3);
} // end of normalizedC3prod()


/* compute relative production of 100% C4 canopy using CENTURY model function */
float ScienceFcns::normalizedC4prod(float x)
{ // x is mean soil temp in warmest month
  float prod_C4;
  double xx, prodd;
  
  xx = x; 
  prodd = (0.006239519724182143 + xx * (0.006513678347464982 + xx * (-0.0005775866974182693
      + xx * (0.0001491401308644086 + xx * -3.094445566664409E-06)))) 
      / (1.0 + xx * (-0.0001694826018411467 + xx * (-1.244515882217747E-05 + xx * (-1.454616827810395E-05 
      + xx * 7.533664205038196E-07)))); 
  prod_C4 = (float)prodd;
  if (prod_C4 < 0.0f) prod_C4 = 0.0f;
  else if (prod_C4 > 1.0f) prod_C4 = 1.0f;
  
  return(prod_C4);
} // end of normalizedC4prod()
     

float ScienceFcns::annual_max(float monthly_vals[])
{
  int mo;
  float max_val;
  
  max_val = monthly_vals[0];
  for (mo = 1; mo<12; mo++) if (monthly_vals[mo]>max_val) max_val = monthly_vals[mo];
  
  return(max_val);
} // end of annual_max()


float ScienceFcns::annual_min(float monthly_vals[])
{
  int mo;
  float min_val;
  
  min_val = monthly_vals[0];
  for (mo = 1; mo<12; mo++) if (monthly_vals[mo]<min_val) min_val = monthly_vals[mo];
  
  return(min_val);
} // end of annual_min()


float ScienceFcns::annual_average(float monthly_vals[], bool shift_flag)
{
  int mo;
  float weighted_sum, average_val;
  int days_per_year = DAYS_PER_YEAR;
  int days_per_month[] = DAYS_PER_MONTH;
  
  weighted_sum = 0.;
  for (mo = 0; mo<12; mo++) weighted_sum += days_per_month[shift_flag ? (mo + 6)%12 : mo]*monthly_vals[mo];
  
  average_val = weighted_sum/days_per_year;
  
  return(average_val);
} // end of annual_average()
  
  
float ScienceFcns::annual_sum(float monthly_vals[], bool shift_flag)
{
  int mo;
  int days_per_year = DAYS_PER_YEAR;
  int days_per_month[] = DAYS_PER_MONTH;
  float length_of_average_mo = days_per_year/12.f;
  float weighted_sum = 0.;
  
  for (mo = 0; mo<12; mo++) weighted_sum += days_per_month[shift_flag ? (mo + 6)%12 : mo]*monthly_vals[mo];  
  return(weighted_sum/length_of_average_mo);
  
} // end of annual_sum()
  
  
bool ScienceFcns::close_enough(double a, double b, double tolerance)
{
   bool ansFlag;
   
   if (a!=0.) ansFlag = fabs((b - a)/a)<=tolerance;
   else ansFlag = fabs(b)<=tolerance;
   
   return(ansFlag);
} // end of close_enough(double a, double b, double tolerance)


bool ScienceFcns::close_enough(double a, double b, double tol_ratio, double tol_diff)
{
   bool ansFlag;
   
   if ((a!=0.) && (fabs((b - a)/a)<=tol_ratio)) ansFlag = true;
   else ansFlag = fabs(a - b)<=tol_diff;
   
   return(ansFlag);
} // end of close_enough(double a, double b, double tol_ratio, double tol_diff)


void ScienceFcns::MixIndex(float ppt[], float tmp[], float p_hi_mult, float * tmp_indexP, float * ppt_indexP, float * mix_indexP)
{
  int mo;

  float ci_hi, ci_low, ci;
  float t_hi, t_mid, t_low, low_tmp;
  float p_hi, p_low, ppt_warm, ppt_avg;
  float cix, tmpx, pptx, mix;
  float high_tmp;
  int high_mo;
  float xpptx, xtmpx;
  float sum;


  /* SET ENDS OF CLIMATIC INDEX GRADIENTS */
   
  /* continental index gradient (deg C) */
  ci_hi  = 60.;
  ci_low = 55.;

  /* low monthly temperature index gradient (deg C) */
  t_hi  =  18.0;
  t_mid =   1.5;
  t_low = -15.0;

  /* warm season precipitation index gradient (mm) */
  p_low = 70.;
  sum = 0.;
  for (mo=0; mo<12; mo++) sum += ppt[mo];
  ppt_avg = sum/12.f;
  p_hi  = MAX(90.f, p_hi_mult*ppt_avg);


  /* DERIVED VARS */

  /* find monthly low tmp index */
  low_tmp = 100.f;
  for (mo = 0; mo < 12; mo++) if (tmp[mo] < low_tmp)
      low_tmp = tmp[mo];


  /* find monthly high temp */
  high_tmp = -100.;
  high_mo = -1;
  for(mo = 0; mo < 12; mo++) if (tmp[mo] > high_tmp)
  {
    high_tmp = tmp[mo];
    high_mo = mo;
  }
  assert(0<=high_mo && high_mo<=11);

  /* calculate continental index */
  ci = high_tmp - low_tmp;

  /* calculate ave monthly ppt during warm season */
  /* (i.e., warm season is 3 warmest consecutive months) */
  if (high_mo == 11) ppt_warm = ppt[10] + ppt[11] + ppt[0];
  else if (high_mo == 0) ppt_warm = ppt[11] + ppt[0] + ppt[1];
  else ppt_warm = ppt[high_mo-1] + ppt[high_mo] + ppt[high_mo+1];
  ppt_warm = ppt_warm / 3.0f;

  /* TEMPERATURE INDEX */

  if (ppt_warm <= -99.) tmpx = -201.; /* assign an out-of-bounds value to a cell in the background .... */
  else if (low_tmp >= t_hi || low_tmp <= t_low)
  { /* outside EN-DB-EB transition zone.... */
    if (ci >= ci_hi) tmpx = -200.; // pure DN zone
    else if (ci >= ci_low)
    { // DN-EN transition zone
      cix = (ci - ci_low) / (ci_hi - ci_low); /* calculate position on continental index gradient */
      tmpx = (cix * -100.f) - 100.f; /* scale result to value between -100 and -200 */
    }
    else if (low_tmp <= t_low) tmpx = -100.f; // pure EN zone
    else tmpx = 100.f; // else pure EB zone
  }
  else
  { /* in EN-DB-EB transition zone.... */
    /* calculate position on low monthly tmp gradient */
    if (low_tmp >= t_mid) tmpx = (low_tmp - t_mid) / (t_hi - t_mid);
    else tmpx = (t_mid - low_tmp) / (t_mid - t_low);
    if (low_tmp >= t_mid) tmpx *= 100.;
    else tmpx *= -100.;
  }


  /* PRECIPITATION INDEX */
   
  if (ppt_warm <= -99.) pptx = -201.; /* assign an out-of-bounds value to a cell in the background .... */
  /* assign values when cell are beyond ends of warm ppt gradient */
  else if (ppt_warm >= p_hi) pptx = 0.;
  else if (ppt_warm <= p_low) pptx = 100.;   
  else
  { /* in warm ppt transition gradient ... */
    /* calculate position on warm ppt gradient */
    pptx = (p_hi - ppt_warm) / (p_hi - p_low);
    pptx *= 100.;
  }

  /* MIX INDEX */

  if (low_tmp >= t_hi || low_tmp <= t_low) mix = tmpx;
  else if (ppt_warm <= p_low) mix = -100.;
  else
  { /* calculate mix as weighted average of positions on two gradients */   
    xpptx = pptx / 100.f;
    xtmpx = (float)fabs(tmpx);
    xtmpx = xtmpx / 100.f;
    mix = (xpptx * xpptx) + (xtmpx * (1.f - xpptx));
    if (low_tmp >= t_mid) mix *= 100.f;
    else mix *= -100.f;
  }  

  *mix_indexP = mix;
  *tmp_indexP = tmpx;
  *ppt_indexP = pptx;
} // end of MixIndex()


float ScienceFcns::FrostIndex(float tmp[])
// Calculate frost index for the year from average temperatures for 12 months
{
  int mo, day;
  float ddf; // degree days at or below 0C
  float ddt; // degree days above 0C
  float frost_index, sqrtddf;
  float days, diff, delta, day_tmp;
  float divisor;
  
  ddt = ddf = 0.;
  for (mo = JAN; mo <= DEC; mo++) 
  {
    days = (float)floor((days_per_mo[mo] + days_per_mo[NEXT_MO(mo)])/2.f);
    diff = tmp[NEXT_MO(mo)] - tmp[mo];
    delta = diff/days;
    
    for (day = 0; day<days; day++)
    {
      day_tmp = tmp[mo] + day*delta;
      if (day_tmp>0.) ddt += day_tmp;
      else ddf += day_tmp;
    } // end of day loop
  } // end of month loop

  sqrtddf = (float)sqrt(fabs(ddf));
  divisor = (float)(sqrtddf + sqrt(ddt));
  frost_index = divisor > 0.f ? (float)(sqrtddf/divisor) : 1.0f;
  assert(frost_index>=0.f && frost_index<=1.0f);
  
  // printf("ddf = %f, ddt = %f\n", ddf, ddt);
  
  return(frost_index);
} // end of FrostIndex()
  

int ScienceFcns::ClimateZone4MAPSS(float min_tmp, float n_decid_bound, float s_decid_bound, float frost)
{
  int zone;
  
  assert(n_decid_bound<s_decid_bound && s_decid_bound<frost);
  
  if (min_tmp<n_decid_bound) zone = mapssBOREAL;
  else if (min_tmp<=s_decid_bound) zone = mapssTEMPERATE;
  else if (min_tmp<frost) zone = mapssSUBTROPICAL;
  else zone = mapssTROPICAL;
  
  return(zone);
  
} // end of ClimateZone4MAPSS()


ClimateZone ScienceFcns::ClimateZone4Biogeography(float gdd_zero, float min_tmp, float az_thres, float bz_thres, float tz_thres, float stz_thres)
{
  ClimateZone zone;
  
  assert(bz_thres<tz_thres && tz_thres<stz_thres);
  
  if (gdd_zero <= az_thres) zone = ARCTICzone;
  else if (min_tmp <= bz_thres) zone = BOREALzone;
  else if (min_tmp <= tz_thres) zone = TEMPERATEzone;    
  else if (min_tmp <= stz_thres) zone = SUBTROPICALzone;
  else zone = TROPICALzone;
  
  return(zone);
  
} // end of ClimateZone4Biogeography()


float ScienceFcns::GrowingDegreeDays(float tmp[], float degree, bool SouthernHemisphereFlag)
{
	float	days, diff, delta, grow, grow_days;
	
  grow_days = 0.0;
  
	for (int mo = 0; mo < 12; mo++) 
  { int this_mo, next_mo;
    this_mo = SouthernHemisphereFlag ? (mo + 6)%12 : mo;
    next_mo = SouthernHemisphereFlag ? (mo + 7)%12 : (mo + 1)%12;
    
		days = ((float) ((days_per_mo[next_mo] / 2.0) +(days_per_mo[this_mo] / 2.0)));
		diff = tmp[(mo + 1)%12] - tmp[mo];
		delta = diff / days;
		for (int day = 0; day < (int) days; day++) 
    {
			grow = tmp[mo] + (day * delta);
			if (grow > degree) grow_days += grow - degree;
		}
	}
	
	return(grow_days);
} // end of GrowingDegreeDays()


void ScienceFcns::PhytomorphologicalIndices(float tmp_index, float ppt_index, float * evergP, float * needlP)
{
  float needl_dry, needl_wet, everg_dry, everg_wet;
  
  // Wet case
  if (tmp_index<= -100.)
  { // from pure deciduous needleleaf to pure evergreen needleleaf
    needl_wet = 100.f; // 100% needleleaf
    everg_wet = tmp_index + 200.f; // % evergreen, shading from pure deciduous needleleaf to pure evergreen needleleaf
  }
  else if (tmp_index<=0.)
  { // tmp_index between -100 and 0: from pure evergreen needleleaf to pure deciduous broadleaf
    needl_wet = -tmp_index; // % needleleaf
    everg_wet = -tmp_index; // % evergreen
  }
  else 
  { // tmp_index between 0 and +100: from pure deciduous broadleaf to pure evergreen broadleaf
    needl_wet = 0.; // 0% needleleaf, i.e. pure broadleaf
    everg_wet = tmp_index; // % evergreen
  }

  // Dry case
  if (tmp_index<= -100.)
  { // from pure deciduous needleleaf to pure evergreen needleleaf
    needl_dry = 100.; // 100% needleleaf
    everg_dry = tmp_index + 200.f; // % evergreen, shading from pure deciduous needleleaf to pure evergreen needleleaf
  }
  else if (tmp_index<=0.)
  { // tmp_index between -100 and 0: pure evergreen needleleaf
    needl_dry = 100; // % needleleaf
    everg_dry = 100; // % evergreen
  }
  else 
  { // tmp_index between 0 and +100: from pure evergreen needleleaf to pure evergreen broadleaf
    needl_dry = 100.f - tmp_index; // % needleleaf
    everg_dry = 100.f; // 100% evergreen
  }

  *needlP = needl_dry*ppt_index/100.f + needl_wet*(100.f - ppt_index)/100.f;
  *needlP = clip(*needlP, 0.f, 100.f);
  *evergP = everg_dry*ppt_index/100.f + everg_wet*(100.f - ppt_index)/100.f;
  *evergP = clip(*evergP, 0.f, 100.f);

} // end of PhytomorphologicalIndices()


float ScienceFcns::clip(float val, float minval, float maxval)
   {
   if (minval>maxval)
      { float temp;
      temp = maxval;
      maxval = minval;
      minval = temp;
      }
   if (val<minval) return(minval);
   else if (val>maxval) return(maxval);
   else return(val);
   } // end of ScienceFcns::clip()


void ScienceFcns::allometry_DavidKing(float abovegr_live_wood, bool db_flag, float * tree_htP, float * dbhP)
// David King's tree allometry algorithm, September 2011
// abovegr_live_wood is aboveground live wood biomass in g m-2
// returns tree_ht in meters and dbh in cm
{
   float height, diameter;
   
	if (db_flag)
      { /* parameterized for deciduous broadleaved trees */		
         height = (float)(abovegr_live_wood>=4000.f ? 0.03484 * pow(abovegr_live_wood, 0.64242) 
         : 0.4523 * pow(abovegr_live_wood, (1./3.)));
         if (height > 35.) height = 35.;
         diameter = (float)(height >= 16. ? 0.30 * pow(height, 1.5) : 1.2 * height);
      }	
	else
      { /* parameterized for evergreen conifers, incuding Pacific NW Douglas-fir */
         height = (float)(abovegr_live_wood>=4000. ? 0.03771 * pow(abovegr_live_wood, 0.62027) 
         : 0.4074 * pow(abovegr_live_wood, (1./3.)));
         if (height > 70.) height = 70;
         diameter = (float)(height>=15.52 ? 0.33 * pow(height, 1.5) : 1.3 * height);	
      }	
	if (diameter<0.001) diameter = 0.001f;
   
   *tree_htP = height;
   *dbhP = diameter;
} // end of ScienceFcns::allometry_DavidKing()  


void ScienceFcns::live_wood(float dbh, bool deciduous, float lwood, float * lbranchP, float * lstemP)
{
   float btt, bbl, bsw, bsb;
   float total;
   float branch_frac;
   float lstem, lbranch;   
   float dbh_in;
   
   dbh_in = cm_to_in(dbh);
   
   
   /********************************************
    Estimate branch portion of live wood bio
    
    var key:
    
    btt = biomass total tree
    bbl = biomass branch
    bsw = biomass stem wood
    bsb = biomass stem bark
    
    *********************************************/
   
   /* if lifeform is deciduous broadleaf ... */
   
   if (deciduous)
      {
      
      /* using QUVE equations in Stanek and State 1978 */
      
      btt = (float)(1.00005 + (2.10621 * log10(dbh_in)));
      bbl = (float)(0.50580 + (2.09357 * log10(dbh_in)));
      
      btt = (float)pow(10., btt);
      bbl = (float)pow(10., bbl);
      
      /* compute branch fraction of total tree biomass */
      
      branch_frac = bbl / btt;
      
      /* distinguish branch and stem biomass using fraction */
      
      lbranch = lwood * branch_frac;
      lstem   = lwood - lbranch;
      }
   
   /* if lifeform is evergreen needleleaf ... */
   
   else
      {
      
      /* using BIOPAK PIPO equations */
      
      bbl = (float)(1.5223 + (2.7185 * log(dbh)));
      bsw = (float)(2.4171 + (2.7587 * log(dbh)));
      bsb = (float)(2.7015 + (2.2312 * log(dbh)));
      
      /* using BIOPAK PSME equations
       
       bbl = 3.2137 + (2.1382 * log(dbh));
       bsw = 3.8682 + (2.5951 * log(dbh));
       bsb = 2.5975 + (2.4300 * log(dbh));
       */
      
      bbl = (float)exp(bbl);
      bsw = (float)exp(bsw);
      bsb = (float)exp(bsb);
      
      /* compute branch fraction of total tree biomass */
      
      total = bbl + bsw + bsb;
      branch_frac = bbl / total;
      
      /* distinguish stem and branch biomass using fraction */
      
      lbranch = lwood * branch_frac;
      lstem = lwood * (1.f - branch_frac);
      }
   
   *lbranchP = lbranch;
   *lstemP = lstem;
} // end of ScienceFcns::live_wood()


int ScienceFcns::doy_of_dom0(int tgt_month, bool southernHemiFlag)
// returns day-of-year of the first day of the month (day-of-month==0)
// doy is 0 for the first day of the year; dom is 0 for the first day of the month
// when southernHemiFlag is false, the first month of the year (mo==0) is Jan
// when southernHemiFlag is true, the first month of the year (mo==0) is July
   {
   int doy = 0;
   int month_index = 0;
   while (month_index<tgt_month) 
      {
      doy += days_per_mo[southernHemiFlag ? (month_index + 6)%12 : month_index];
      month_index++;
      }
   return(doy);
   } // end of ScienceFcns::doy_of_dom0()
   
   
float ScienceFcns::crown_kill(float ht, float cl, float hk)
/* calc fraction of crown volume killed
   equation from Peterson and Ryan (1986) */
// ht = tree height, m
// cl = crown length, m
// hk = lethal scorch height, m
// returns fraction of crown volume killed
   {
   float ck; // fraction of crown volume killed

   if (hk<=0.0) ck = 0.0;
   else if (hk>ht) ck = 1.0;
   else ck = (float)(((hk - ht + cl) * (ht - hk + cl)) / pow(cl, 2.));
   
   ck = clip(ck, 0., 1.);
   
   return(ck);

   } // end of ScienceFcns::crown_kill()
   

float ScienceFcns::btu_per_sec_per_ft_to_kW_per_m(float btu_per_sec_per_ft) 
/* A BTU is 1055.06 joules.
A watt is 1 joule per second.
So one BTU per second is 1.05506 kW.
One foot is 0.3048 meters.
So one BTU per second per foot is 1.05506 kW per 0.3048 meters, i.e. 3.461483 kW per meter.
So when we convert from fire line intensity in BTU per second per foot to fire line intensity in kW per meter, 
the new number should be about 3 times the old number.
*/
   { 
   return(btu_per_sec_per_ft*3.461483f);
   } // end of btu_per_sec_per_ft_to_kW_per_m()


float ScienceFcns::DewpointTemperature(float satvpPa) 
// Returns dewpoint temperature in deg C
// Equations based on iridl.ldeo.columbia.edu/dochelp/QA/Basic/dewpoint.html
  {
  float E0 = 0.611f; // kPa
  float LoverRv = 5423.f; // deg K
  float T0 = 273.f; // deg K

  float satvp_kPa = satvpPa/1000.f;

  float tDewPtK = (float)(1./((1./T0) - (1./LoverRv)*log(satvp_kPa/E0)));

  return(degK_to_degC(tDewPtK));

  } // end of DewPointTemperature()



