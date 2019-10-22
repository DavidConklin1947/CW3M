#if !defined(AFX_SCALE_H_INCLUDED_)
#define AFX_SCALE_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


void WINLIBSAPI bestscale(double umin, double umax, double *low, double *high, double *ticks);
void WINLIBSAPI goodscales(double xmin, double xmax, double *low, double *high, double *ticks);

#endif