#ifndef __DDEFFECTS3_H__
#define __DDEFFECTS3_H__

#include <ddraw.h>

#include "ColorMatrix2.h"

BOOL AlphaBlt(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,float opacity=1.0f,DWORD colorkey=CLR_INVALID);
BOOL MaskAlphaBlt(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,LPDIRECTDRAWSURFACE alphachannel,float opacity=1.0f,DWORD colorkey=CLR_INVALID);
BOOL LightBlt(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,float opacity=1.0f);
BOOL DarkBlt(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,float opacity=1.0f);
BOOL AddBlt(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,float opacity=1.0f,DWORD colorkey=CLR_INVALID);
BOOL Invert(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect);
BOOL ApplyColorMatrix(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE src,int x,int y,LPRECT rect,const ColorMatrix2& matrix);

enum BlurSize
{
	Blur2x2,
	Blur3x3,
	Blur5x5
};

BOOL Blur(LPDIRECTDRAWSURFACE dest,LPDIRECTDRAWSURFACE source,int px,int py,LPRECT srcrect,BlurSize blursize);

#define RGB2(r,g,b) (((int)(unsigned char)r)<<16)+(((int)(unsigned char)g)<<8)+(unsigned char)b

#endif //__DDEFFECTS3_H__