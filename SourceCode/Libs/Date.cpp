//-------------------------------------------------------------------
//   Program: DATE.CPP
//
//   Date conversion methods.
//-------------------------------------------------------------------


#include "libs.h"
#pragma hdrstop

#if !defined( _DATE_HPP )
#include "DATE.HPP"
#endif


int GetDaysInMonth0(int month0, int daysInYear)
{
   ASSERT(0 <= month0 && month0 <= 11);
   int dpm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
   int days_in_month = -1;

   switch (daysInYear)
   {
      case 360: days_in_month = 30; break;
      case 365: days_in_month = dpm[month0]; break;
      case 366: days_in_month = month0 == 1 ? 29 : dpm[month0]; break;

      default:
         ASSERT(false);
      case 0:
      case 1:
         // These are for debugging convenience.
         days_in_month = 31;
         break;
   }
 
   return(days_in_month);
} // end of GetDaysInMonth0()


int GetDaysInCalendarMonth(int calendarMonth, int calendarYear)
{
   ASSERT(1 <= calendarMonth && calendarMonth <= 12);
   int days_in_year = GetDaysInCalendarYear(calendarYear);
   int days_in_month = GetDaysInMonth0(calendarMonth - 1, days_in_year);

   return(days_in_month);
} // end of GetDaysInCalendarMonth()


int GetDaysInCalendarYear(int calendarYear)
{
   if (calendarYear % 400 == 0) return(366);
   if (calendarYear % 100 == 0) return(365);
   if (calendarYear % 4 == 0) return(366);
   return(365);
} // end of GetDaysInCalendarYear()


//-- GetJulianDay() -------------------------------------------------
//
//   Determines julian date as a function of calendar month, day and
//   year. (1-based)
//-------------------------------------------------------------------
int GetJulianDay(int month, int calDay, int year, int maxDaysInYear) // Returns a 1-based Julian day (Jan 1 = 1)
{
   if (month  < 1 || month  > 12 || year < 0 || year > 10000) return(0);
   int days_in_year = maxDaysInYear > 365 ? GetDaysInCalendarYear(year) : maxDaysInYear;
   int days_in_month = GetDaysInMonth0(month - 1, days_in_year);

   if (calDay < 1 || calDay > days_in_month) return(0);

   int julDay = 0;
   if (month == 1) julDay = calDay;
   else
   {
      for (int i = 1; i < month; i++) julDay += GetDaysInMonth0(i - 1, days_in_year);
      julDay += calDay;
   }

   return(julDay);
} // end of int GetJulianDay(int month, int calDay, int year, int maxDaysInYear)


bool DateComesBefore(SYSDATE date1, SYSDATE date2)
{
   if (date1.year < date2.year) return(true);
   if (date1.year > date2.year) return(false);

   if (date1.month < date2.month) return(true);
   if (date1.month > date2.month) return(false);

   if (date1.day < date2.day) return(true);
   return(false);
} // end of DateComesBefore()


int DaysBetweenDates(SYSDATE date1, SYSDATE date2)
{
   if (date1.month == date2.month && date1.day == date2.day && date1.year == date2.year) return(0);

   SYSDATE first_date, last_date;
   int sign;
   if (DateComesBefore(date1, date2))
   {
      first_date = date1;
      last_date = date2;
      sign = 1;
   }
   else
   {
      first_date = date2;
      last_date = date1;
      sign = -1;
   }

   int first_jday = GetJulianDay(first_date.month, first_date.day, first_date.year, 366);
   int last_jday = GetJulianDay(last_date.month, last_date.day, last_date.year, 366);

   int days_between = -first_jday;
   int year = first_date.year;
   while (year < last_date.year)
   {
      days_between += GetDaysInCalendarYear(year);
      year++;
   }
   days_between += last_jday;

   return(days_between * sign);
} // end of DaysBetweenDates()


BOOL GetCalDate0(int jday0, int *pMonth, int *pCalDay, int daysInYear)
{
   if (!((daysInYear == 365 || daysInYear == 366 || daysInYear == 360) && (jday0 >= 0 && jday0 < daysInYear)))
   {
      *pMonth = 0;
      *pCalDay = 0;
      ASSERT(0);
      return(false);
   }

   //-- Initialize month and day, Jan. 31
   *pMonth = 1;
   int day = 31;

   while (day < (jday0 + 1))
   {
      *pMonth = *pMonth + 1;
      day += GetDaysInMonth0(*pMonth - 1, daysInYear);
   }

   *pCalDay = (jday0 + 1) - day + GetDaysInMonth0(*pMonth - 1, daysInYear);

   return TRUE;
}

//-----------------------------------------------------------------
//@ Method: GetCalDate()
//
//@ Description: Converts julian date to a calendar date.  Julian day is
//@   one-based, may be < 0 or > 365 and adjusts year if necessary.
//
//@ Arguments:
//@ - int julDay - julian day of date to convert.  Can be positive or negative
//@ - int* pYear - reference year for calculations, contains year the
//@        julian date occurs in on return.
//@ - int* pMonth - returned calendar month (1-based)
//@ - int* pDay   - returned calendar day (1-based)
//@ - BOOL statAdjYear - flag - TRUE to calculate the new year, FALSE
//@        to assume the specified julian date is in the current year
//-------------------------------------------------------------------

BOOL GetCalDate( int julDay, int *pYear, int *pMonth, int *pCalDay,
   BOOL statAdjYear )
   {
   //-- Error.
   if ( *pYear < 0 || *pYear > 10000 )
      {
      *pMonth  = 1;
      *pCalDay = 1;
      return FALSE;
      }

   if (GetDaysInCalendarYear(*pYear ) == julDay || statAdjYear == FALSE )
      ;

   else if ( julDay <= 0 )
      {
      while ( julDay <= 0 )
         {
         *pYear -= 1;
         julDay += GetDaysInCalendarYear(*pYear );
         }
      }

   else if ( julDay > GetDaysInCalendarYear(*pYear ) )
      {
      while ( julDay > GetDaysInCalendarYear(*pYear ) )
         {
         julDay -= GetDaysInCalendarYear(*pYear );
         *pYear += 1;
         }
      }

   //-- Initialize month and day, Jan. 31
   *pMonth = 1;
   int day = 31;

   while ( day < julDay )
      {
      *pMonth = *pMonth + 1;
      day += GetDaysInCalendarMonth( *pMonth, *pYear );
      }

   *pCalDay = julDay - day + GetDaysInCalendarMonth( *pMonth, *pYear );

   return TRUE;
   }


#ifndef NO_MFC
//-------------------------------------------------------------------
void LoadMonthCombo( HWND hCombo )
	{
   SendMessage( hCombo, CB_RESETCONTENT, 0, 0L );

   //-- load record combo box --//
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Jan" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Feb" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Mar" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Apr" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "May" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Jun" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Jul" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Aug" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Sep" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Oct" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Nov" );
   SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR) "Dec" );

   SendMessage( hCombo, CB_SETCURSEL, 0, 0L );
   }
#endif

LPCTSTR GetMonthStr( int month )   // month is one-based
   {
   switch( month )
      {
      case 1: return _T("Jan");
      case 2: return _T("Feb");
      case 3: return _T("Mar");
      case 4: return _T("Apr");
      case 5: return _T("May");
      case 6: return _T("Jun");
      case 7: return _T("Jul");
      case 8: return _T("Aug");
      case 9: return _T("Sep");
      case 10: return _T("Oct");
      case 11: return _T("Nov");
      case 12: return _T("Dec");
      }

   return _T( "" );
   }
