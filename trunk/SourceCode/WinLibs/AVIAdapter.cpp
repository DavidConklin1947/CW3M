#include "winlibs.h"
#pragma hdrstop

#include <aviadapter.h>

AVIAdapter::AVIAdapter(void)
{
}

AVIAdapter::AVIAdapter( CWnd* w, unsigned int fr, CString &filename )
:  wnd(w),
   frameRate(fr),
   fileName(filename)
{
   Init();
}


AVIAdapter::~AVIAdapter(void)
{
   delete aviGen;
   delete [] bitAddress;
}

void AVIAdapter::Init()
{
   CRect rect;
   wnd->GetWindowRect(rect);

   // need to crop window dimensions so they're compatable (multiples of 4)

   BITMAPINFOHEADER bih;
   bih.biSize = sizeof(BITMAPINFOHEADER);
   bih.biWidth = (rect.Width()/4)*4;
   bih.biHeight = (rect.Height()/4)*4;
   bih.biPlanes = 1;
   bih.biBitCount = 24;
   bih.biSizeImage = ((bih.biWidth*bih.biBitCount+31)/32 * 4)*bih.biHeight;
   bih.biCompression = BI_RGB;
   bih.biClrUsed = 0;

   aviGen = new CAVIGenerator( fileName, &bih, frameRate );

   if( aviGen->InitEngine() != 0 )
   {
      AfxMessageBox("Couldn't initialize AVI generator.");
   }

   CDC* dc = wnd->GetWindowDC();

   bitmap.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
   bitmapDC.CreateCompatibleDC(dc);
   bitmapDC.SelectObject(&bitmap);
   bi.bmiHeader = *aviGen->GetBitmapHeader();

   wnd->ReleaseDC(dc);

   bitAddress = new BYTE[3*rect.Width()*rect.Height()];
}

void AVIAdapter::AddFrame()
{
   CDC* dc = wnd->GetWindowDC();

   CRect rect;
   wnd->GetWindowRect(rect);

   BOOL bltSuccess = bitmapDC.BitBlt( 0, 0, rect.Width(), rect.Height(), dc, 0, 0, SRCCOPY );

   GetDIBits( bitmapDC, HBITMAP(bitmap), 0, rect.Height(), bitAddress, &bi, DIB_RGB_COLORS );

   aviGen->AddFrame(bitAddress);

   wnd->ReleaseDC(dc);
}

void AVIAdapter::End()
{
   aviGen->ReleaseEngine();
}