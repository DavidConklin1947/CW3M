/*
   AVIAdapter
   
   wrapper for AVIGenerator class to allow generation of movies from an arbitrary CWnd

   1. Pass to the constructor the window pointer, the desired frame rate, and
      the filename of the resulting movie
   2. Call AddFrame() for every frame, which will grab the pixels from the screen space
      that the window occupies (not necessarily what you'd see in the window itself, if
      it's in the background), and forward them to the AVIGenerator in a device-independant
      format.
   3. Call End() to finalize the movie.
*/

#pragma once
#include "AVIGenerator.h"

class AVIAdapter
{
private:
   AVIAdapter(void);

   void Init();

public:
   AVIAdapter( CWnd* w, unsigned int fr, CString &filename );

   virtual ~AVIAdapter(void);

   void AddFrame();

   void End();

private:
   CAVIGenerator* aviGen;

   CWnd* wnd;
   unsigned int frameRate;
   CString fileName;

   CBitmap bitmap;
   CDC bitmapDC;
   BITMAPINFO bi;
   
   BYTE* bitAddress;
};
