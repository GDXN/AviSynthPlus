// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#ifndef __Text_overlay_H__
#define __Text_overlay_H__

#include "internal.h"
#include "convert.h"


/********************************************************************
********************************************************************/



class Antialiaser 
/**
  * Helper class to anti-alias text
 **/
{  
public:
  Antialiaser(int width, int height, const char fontname[], int size);
  virtual ~Antialiaser();
  HDC GetDC();
  
  void Apply(const VideoInfo& vi, BYTE* buf, int pitch, int textcolor, int halocolor);
  void ApplyYUY2(BYTE* buf, int pitch, int textcolor, int halocolor);
  void ApplyRGB24(BYTE* buf, int pitch, int textcolor, int halocolor);
  void ApplyRGB32(BYTE* buf, int pitch, int textcolor, int halocolor);  

private:
  const int w, h;
  HDC hdcAntialias;
  HBITMAP hbmAntialias;
  void* lpAntialiasBits;
  HFONT hfontDefault;
  HBITMAP hbmDefault;
  char* alpha_bits;
  bool dirty;

  void GetAlphaRect();
};



class ShowFrameNumber : public GenericVideoFilter 
/**
  * Class to display frame number on a video clip
 **/
{  
public:
  ShowFrameNumber(PClip _child, bool _scroll);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  Antialiaser antialiaser;
  bool scroll;
};



class ShowSMPTE : public GenericVideoFilter 
/**
  * Class to display SMPTE codes on a video clip
 **/
{
public:
  ShowSMPTE(PClip _child, double _rate, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);


private:
  Antialiaser antialiaser;
  char rate;
  bool dropframe;
};



class Subtitle : public GenericVideoFilter 
/**
  * Subtitle creation class
 **/
{
public:
  Subtitle( PClip _child, const char _text[], int _x, int _y, int _firstframe, int _lastframe, 
            const char _fontname[], int _size, int _textcolor, int _halocolor );
  virtual ~Subtitle(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);  

private:
  void InitAntialiaser(void);
  
  const int x, y, firstframe, lastframe, size;
  /*const*/ int textcolor, halocolor;
  const char* const fontname;
  const char* const text;
  Antialiaser* antialiaser;  
};




/**** Helper functions ****/

void ApplyMessage( PVideoFrame* frame, const VideoInfo& vi, const char* message, int size, 
                   int textcolor, int halocolor, int bgcolor, IScriptEnvironment* env );

bool GetTextBoundingBox( const char* text, const char* fontname, int size, bool bold, 
                         bool italic, int align, int* width, int* height );




/**** Inline helper functions ****/

inline static HFONT LoadFont(const char name[], int size, bool bold, bool italic) 
{
  return CreateFont( size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
                     italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE | DEFAULT_PITCH, name );
}

inline char* MyStrdup(const char* s) {
  return lstrcpy(new char[(lstrlen(s)+1)], s);
}



#endif  // __Text_overlay_H__