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

#include "resample_functions.h"


/*******************************************
   ***************************************
   **  Helper classes for resample.cpp  **
   ***************************************
 *******************************************/



/***************************
 ***** Triangle filter *****
 **************************/

double TriangleFilter::f(double x)
{  
  x = fabs(x);
  return (x<1.0) ? 1.0-x : 0.0;
}





/*********************************
 *** Mitchell-Netravali filter ***
 *********************************/

MitchellNetravaliFilter::MitchellNetravaliFilter (double b=1./3., double c=1./3.) 
{
  p0 = (   6. -  2.*b            ) / 6.;
  p2 = ( -18. + 12.*b +  6.*c    ) / 6.;
  p3 = (  12. -  9.*b -  6.*c    ) / 6.;
  q0 = (            8.*b + 24.*c ) / 6.;
  q1 = (         - 12.*b - 48.*c ) / 6.;
  q2 = (            6.*b + 30.*c ) / 6.;
  q3 = (      -     b -  6.*c    ) / 6.;
}

double MitchellNetravaliFilter::f (double x)
{
    x = fabs(x);
    return (x<1) ? (p0+x*x*(p2+x*p3)) : (x<2) ? (q0+x*(q1+x*(q2+x*q3))) : 0.0;
}





/******************************
 **** Resampling Patterns  ****
 *****************************/

int* GetResamplingPatternRGB( int original_width, double subrange_start, double subrange_width,
                              int target_width, ResamplingFunction* func )
/** 
  * This function returns a resampling "program" which is interpreted by the 
  * FilteredResize filters.  It handles edge conditions so FilteredResize    
  * doesn't have to.  
 **/
{
  double scale = double(target_width) / subrange_width;
  double filter_step = min(scale, 1.0);
  double filter_support = func->support() / filter_step;
  int fir_filter_size = int(ceil(filter_support*2));
  int* result = (int*) _aligned_malloc((1 + target_width*(1+fir_filter_size)) * 4, 4);

  int* cur = result;
  *cur++ = fir_filter_size;

  double pos_step = subrange_width / target_width;
  // the following translates such that the image center remains fixed
  double pos = subrange_start + ((subrange_width - target_width) / (target_width*2));

  for (int i=0; i<target_width; ++i) {
    int end_pos = int(pos + filter_support);
    if (end_pos > original_width-1)
      end_pos = original_width-1;
    int start_pos = end_pos - fir_filter_size + 1;
    if (start_pos < 0)
      start_pos = 0;
    *cur++ = start_pos;
    // the following code ensures that the coefficients add to exactly FPScale
    double total = 0.0;
    for (int j=0; j<fir_filter_size; ++j)
      total += func->f((start_pos+j - pos) * filter_step);
    double total2 = 0.0;
    for (int k=0; k<fir_filter_size; ++k) {
      double total3 = total2 + func->f((start_pos+k - pos) * filter_step) / total;
      *cur++ = int(total3*FPScale+0.5) - int(total2*FPScale+0.5);
      total2 = total3;
    }
    pos += pos_step;
  }
  return result;
}


int* GetResamplingPatternYUV( int original_width, double subrange_start, double subrange_width,
                              int target_width, ResamplingFunction* func, bool luma, BYTE *temp )
/** 
  * Same as with the RGB case, but with special
  * allowances for YUV-MMX code
 **/
{
  double scale = double(target_width) / subrange_width;
  double filter_step = min(scale, 1.0);
  double filter_support = func->support() / filter_step;
  int fir_filter_size = int(ceil(filter_support*2));
  int fir_fs_mmx = (fir_filter_size / 2) +1;  // number of loops in MMX code
  int* result = luma ?
    (int*) _aligned_malloc(2*4 + target_width*(1+fir_fs_mmx)*8, 8) :
    (int*) _aligned_malloc(2*4 + target_width*(1+fir_filter_size)*8, 8);

  int* cur[2] = { result +2, result +3 };
  *result = luma ? fir_fs_mmx : fir_filter_size;

  double pos_step = subrange_width / target_width;
  // the following translates such that the image center remains fixed
  double pos = subrange_start + ((subrange_width - target_width) / (target_width*2));

  for (int i=0; i<target_width; ++i) {
    int end_pos = int(pos + filter_support);
    if (end_pos > original_width-1)
      end_pos = original_width-1;
    int start_pos = end_pos - fir_filter_size + 1;
    if (start_pos < 0)
      start_pos = 0;
    int ii = luma ? i&1 : 0;
    *(cur[ii]) = luma ?
      (int)(temp + (start_pos & -2) * 2) :
      (int)(temp + start_pos * 8);
    cur[ii] += 2;
    // the following code ensures that the coefficients add to exactly FPScale
    double total = 0.0;
    for (int j=0; j<fir_filter_size; ++j)
      total += func->f((start_pos+j - pos) * filter_step);
    double total2 = 0.0;
    int oldCoeff = 0;
    for (int k=0; k<fir_filter_size; ++k)
    {
      double total3 = total2 + func->f((start_pos+k - pos) * filter_step) / total;
      int coeff = int(total3*FPScale+0.5) - int(total2*FPScale+0.5);
      total2 = total3;
      if (luma)
      {
        if ((k + start_pos) & 1)
        {
          *(cur[ii]) = (coeff << 16) + (oldCoeff & 0xFFFF);
          cur[ii] += 2;
        }
        else
          oldCoeff = coeff;
      }
      else
      {
        *(cur[0]) = coeff;  cur[0] += 1;
        *(cur[0]) = coeff;  cur[0] += 1;
      }
    }
    if (luma)
    {
      if ((start_pos + fir_filter_size) & 1)
      {
        *(cur[ii]) = 0 + (oldCoeff & 0xFFFF);
        cur[ii] += 2;
      }
      else
      if ((fir_filter_size & 1) == 0)
      {
        *(cur[ii]) = 0;  cur[ii] += 2;
      }
    }
    pos += pos_step;
  }
  return result;
}