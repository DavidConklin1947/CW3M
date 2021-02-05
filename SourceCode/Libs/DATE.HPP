//-------------------------------------------------------------------
//   DATE.HPP
//
//   Include file for date conversion routines.
//-------------------------------------------------------------------

#pragma once
#include "libs.h"

//-- Constants. -----------------------------------------------------
const int JD_INVALID_MONTH = -1;
const int JD_INVALID_DAY   = -2;
const int JD_INVALID_YEAR  = -3;

//-- Structures -----------------------------------------------------




//-- Prototypes. ----------------------------------------------------
class SYSDATE;

   //-- Time methods. -----------------------------------------------
   int  LIBSAPI GetDaysInMonth0( int month0, int daysInYear );
   int  LIBSAPI GetDaysInCalendarMonth(int calendarMonth, int calendarYear);
   int  LIBSAPI GetDaysInCalendarYear(int calendarYear);
   int  LIBSAPI GetJulianDay ( int month, int calDay, int year, int maxDaysInYear );
   int LIBSAPI DaysBetweenDates(SYSDATE date1, SYSDATE date2);

   BOOL LIBSAPI GetCalDate(int julDay, int *pYear, int *pMonth, int *pCalDay, BOOL updateYear);
   BOOL LIBSAPI GetCalDate0(int jday0, int *pMonth, int *pCalDay, int daysInYear);
   bool LIBSAPI DateComesBefore(SYSDATE date1, SYSDATE date2);


#ifndef NO_MFC
   void LIBSAPI LoadMonthCombo( HWND hCombo );
#endif

   LPCTSTR LIBSAPI GetMonthStr( int month ); // months are one-based

   enum MON { JAN = 1, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC };

   class SYSDATE
   {
   public:
      MON month; // 1-based; Jan = 1
      int day; // 1-based; first day of month = 1
      int year;   // eg. 1994

      SYSDATE() { month = JAN; day = 1; year = 2010; }
      SYSDATE(int mo, int dy, int yr) { month = (MON)mo; day = dy; year = yr; }
      ~SYSDATE() {}

      void MoveToNextDay(int maxDaysInYear)
      {
         int days_in_year = maxDaysInYear == 366 ? GetDaysInCalendarYear(year) : maxDaysInYear;
         int days_in_month = GetDaysInMonth0(month - 1, days_in_year);
         day++;
         if (day > days_in_month)
         {
            day = 1;
            month = (MON)(month + 1);
            if (month > 12)
            {
               year++;
               month = JAN;
            }
         }
      } // end of MoveToNextDay()
   };
