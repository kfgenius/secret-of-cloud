#ifndef __JDIRECTDRAW_H__
#define __JDIRECTDRAW_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>

#include "ColorMatrix2.h"
#include "DDEffects3.h"

#ifndef PURE
#define PURE =0
#endif

#ifndef UINT_DEFINED
#define UINT_DEFINED
typedef unsigned int uint;
#endif

#define INVALID_COLOR (JColor(255,255,255,255))

struct JColor
{
	JColor() : color(0) {}
	JColor(BYTE _r,BYTE _g,BYTE _b) : r(_r), g(_g), b(_b), a(0) {}
	JColor(BYTE _r,BYTE _g,BYTE _b,BYTE _a) : r(_r), g(_g), b(_b), a(_a) {}
	operator DWORD() { return color; }
	union
	{
		struct
		{
			BYTE b,g,r,a;
		};
		DWORD color;
	};
};

enum PicLayerType
{
	PicOverlayNormal=0x0000,
	PicOverlayLight=0x0001,
	PicOverlayDark=0x0002,
	PicOverlayAddition=0x0004,
	PicOverlayForceDWORD=0xffffffff
};

enum PicInfoFlag
{
	PicInfo_None=0,
	PicInfo_StreamSource=0x0001,
	PicInfo_FileSource=0x0002,
	PicInfo_LayerType=0x0004,
	PicInfo_Opacity=0x0008,
	PicInfo_MaskName=0x0010,
	PicInfo_ColorKey=0x0020,
	PicInfo_RedrawFunc=0x0040,
	PicInfo_ClipRegion=0x0080,
	PicInfo_ForceDWORD=0xffffffff
};

enum DrawPicExFlags
{
	DPX_HFLIP=0x0001,
	DPX_VFLIP=0x0002
};

class JPictureInfo;
typedef bool(__cdecl *RedrawFunc)(LPDIRECTDRAWSURFACE pSurface,JPictureInfo* pPicInfo,void* pContext);

class JPictureInfo
{
protected:
	DWORD dwWidth,dwHeight;
	LPDIRECTDRAWSURFACE surface;

	DWORD dwFlags;
	union
	{
		char* pSource;
		IStream* sSource;
	};
	PicLayerType eLayerType;
	float fOpacity; // 0<=Opacity<=1.0f
	char* szMaskName;
	JColor cColorKey;
	RedrawFunc fRedrawFunc;
	RECT rClipRegion;

public:
	JPictureInfo();
	JPictureInfo(JPictureInfo&);
	~JPictureInfo();

	DWORD GetWidth() { return dwWidth; }
	DWORD GetHeight() { return dwHeight; }
	LPDIRECTDRAWSURFACE GetSurface() { return surface; }
	PicInfoFlag GetSourceType() { return (PicInfoFlag)(dwFlags&(PicInfo_StreamSource|PicInfo_FileSource)); }
	char* GetFileSource() { return pSource; }
	IStream* GetStreamSource() { return sSource; }
	PicLayerType GetLayerType() { if(!(dwFlags&PicInfo_LayerType)) SetLayerType(); return eLayerType; }
	float GetOpacity() { if(!(dwFlags&PicInfo_Opacity)) SetOpacity(); return fOpacity; }
	char* GetMaskName() { if(!(dwFlags&PicInfo_MaskName)) SetMaskName(); return szMaskName; }
	JColor GetColorKey() { if(!(dwFlags&PicInfo_ColorKey)) SetColorKey(); return cColorKey; }
	RedrawFunc GetRedrawFunc() { if(!(dwFlags&PicInfo_RedrawFunc)) SetRedrawFunc(); return fRedrawFunc; }
	LPRECT GetClipRegion() { if(dwFlags&PicInfo_ClipRegion) return &rClipRegion; else return NULL; }

	void SetWidth(DWORD width) { dwWidth=width; }
	void SetHeight(DWORD height) { dwHeight=height; }
	void SetSurface(LPDIRECTDRAWSURFACE surface);
	void SetLayerType(PicLayerType type=PicOverlayNormal);
	void SetOpacity(float opacity=1.0f);
	void SetMaskName(char* maskname=NULL);
	void SetColorKey(JColor colorkey=INVALID_COLOR);
	void SetRedrawFunc(RedrawFunc func=NULL);
	void SetClipRegion(LPRECT rect=NULL);
	
	void* pContext;
};

typedef struct tagBrush {} *JBrush;
typedef struct tagFont {} *JFont;

struct JDirectDraw
{
	static uint GetDeviceCount();
	static uint GetDeviceDesc(uint idx,char* buffer);

	virtual ~JDirectDraw() {}

	virtual bool Initialize(uint devid,HWND hwnd,uint width,uint height,uint bpp,bool sysmem=false) PURE;
	virtual bool Cleanup() PURE;

	virtual int GetID(char* name) PURE;

	virtual bool CreateSurface(char* name,JPictureInfo* picinfo,bool system=false,int* pid=NULL) PURE;
	virtual bool LoadPicture(char* name,char* filename,JPictureInfo* picinfo=NULL,bool sysmem=false,int* pid=NULL) PURE;
	virtual bool LoadPicture(char* name,IStream* stream,JPictureInfo* picinfo=NULL,bool sysmem=false,int* pid=NULL) PURE;
	virtual bool AddSurface(char* name,JPictureInfo* picinfo,int* pid=NULL) PURE;
	virtual bool SavePicture(char* name,char* type,char* filename) PURE;
	virtual bool SavePicture(int hID,char* type,char* filename) PURE;

	// Name based
	virtual bool GetPictureInfo(char* name,JPictureInfo* buffer) PURE;
	virtual bool SetPictureInfo(char* name,JPictureInfo* buffer) PURE;

	virtual bool RedrawSurface(char* name) PURE;
	virtual bool DeleteSurface(char* name) PURE;

	virtual bool ApplyColorMatrix(char* szDest,char* szSrc,int px,int py,LPRECT srcRect,const ColorMatrix2& matrix) PURE;
	virtual bool DrawPicture(char* szDest,char* szSrc,int px,int py,LPRECT srcRect) PURE;
	virtual bool DrawPictureEx(char* szDest,char* szSrc,int px,int py,LPRECT srcRect,DWORD dwFlags) PURE;
	virtual bool DrawStretchedPicture(char* szDest,char* szSrc,LPRECT destRect,LPRECT srcRect) PURE;
	virtual bool Blur(char* szDest,char* szSrc,int px,int py,LPRECT srcRect,BlurSize blursize) PURE;
	virtual bool DrawText(char* szDest,char* szText,JFont font,LPRECT pRect,JColor pColor) PURE;
	virtual bool DrawText(char* szDest,char* szText,JFont font,int px,int py,JColor pColor) PURE;
	virtual bool MeasureText(char* text,JFont font,LPRECT pRect) PURE;
	virtual bool DrawLine(char* szDest,JBrush pBrush,int sx,int sy,int ex,int ey,float fWidth=0.0f) PURE;
	virtual bool DrawRect(char* szDest,JBrush pBrush,LPRECT pRect,float fWidth=0.0f) PURE;
	virtual bool DrawPolygon(char* szDest,JBrush pBrush,int* ppx,int* ppy,int pcount,float fWidth=0.0f) PURE;
	virtual bool DrawEllipse(char* szDest,JBrush pBrush,LPRECT pRect,float fWidth=0.0f) PURE;
	virtual bool DrawPie(char* szDest,JBrush pBrush,LPRECT pRect,float fStartAngle,float fSweepAngle,float fWidth=0.0f) PURE;
	virtual bool GetPixel(char* szSrc,int px,int py,JColor* pColor) PURE;
	virtual bool SetPixel(char* szDest,int px,int py,JColor pColor) PURE;

	// ID based

	virtual bool GetPictureInfo(int ID,JPictureInfo* buffer) PURE;
	virtual bool SetPictureInfo(int ID,JPictureInfo* buffer) PURE;

	virtual bool RedrawSurface(int ID) PURE;
	virtual bool DeleteSurface(int ID) PURE;

	virtual bool ApplyColorMatrix(int hDest,int hSrc,int px,int py,LPRECT srcRect,const ColorMatrix2& matrix) PURE;
	virtual bool DrawPicture(int hDest,int hSrc,int px,int py,LPRECT srcRect) PURE;
	virtual bool DrawPictureEx(int hDest,int hSrc,int px,int py,LPRECT srcRect,DWORD dwFlags) PURE;
	virtual bool DrawStretchedPicture(int hDest,int hSrc,LPRECT destRect,LPRECT srcRect) PURE;
	virtual bool Blur(int hDest,int hSrc,int px,int py,LPRECT srcRect,BlurSize blursize) PURE;
	virtual bool DrawText(int hDest,char* szText,JFont font,LPRECT pRect,JColor pColor) PURE;
	virtual bool DrawText(int hDest,char* szText,JFont font,int px,int py,JColor pColor) PURE;
	virtual bool DrawLine(int hDest,JBrush pBrush,int sx,int sy,int ex,int ey,float fWidth=0.0f) PURE;
	virtual bool DrawRect(int hDest,JBrush pBrush,LPRECT pRect,float fWidth=0.0f) PURE;
	virtual bool DrawPolygon(int hDest,JBrush pBrush,int* ppx,int* ppy,int pcount,float fWidth=0.0f) PURE;
	virtual bool DrawEllipse(int hDest,JBrush pBrush,LPRECT pRect,float fWidth=0.0f) PURE;
	virtual bool DrawPie(int hDest,JBrush pBrush,LPRECT pRect,float fStartAngle,float fSweepAngle,float fWidth=0.0f) PURE;
	virtual bool GetPixel(int hSrc,int px,int py,JColor* pColor) PURE;
	virtual bool SetPixel(int hDest,int px,int py,JColor pColor) PURE;


	virtual JBrush CreateBrush(JColor jColor,float fOpacity=1.0f) PURE;
	virtual JFont CreateFont(char* szFace,int iHeight,bool bBold=false,bool bItalic=false,bool bUnderline=false,bool bStrikeout=false,bool bAntiAlias=false) PURE;
	virtual bool DeleteBrush(JBrush brush) PURE;
	virtual bool DeleteFont(JFont font) PURE;

	virtual char* GetBackBuffer() PURE;
	virtual int GetBackBufferID() PURE;

	virtual bool RestoreAllSurfaces() PURE;
	virtual bool SetVerticalSync(bool bVSync=true) PURE;
	virtual bool SetFrameRate(int iFPS,bool bBlocking=true) PURE;
	virtual bool Render() PURE;

	virtual HRESULT GetLastError() PURE;
};

JDirectDraw* CreateDirectDraw();

#endif //__JDIRECTDRAW_H__