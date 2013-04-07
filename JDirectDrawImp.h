#ifndef __JDIRECTDRAWIMP_H__
#define __JDIRECTDRAWIMP_H__

#include "JDirectDraw.h"
#include <gdiplus.h>

using namespace Gdiplus;

#define HASH_NUMBER 37
#define HASH_SIZE 262139

class JDirectDrawImp : public JDirectDraw
{
protected:
	LPDIRECTDRAW lpDD;
	LPDIRECTDRAW7 lpDD7;
	LPDIRECTDRAWSURFACE lpDDBackSurface,lpDDPriSurface,lpDDTempSurface,lpDDTempMask;
	bool bProxy;
	uint screen_width,screen_height;
	int backbufferkey;
	ULONG_PTR ptr;

	HRESULT lasterr;
	const char* lasterrmsg;

	JPictureInfo* table[HASH_SIZE];
	HDC dctable[HASH_SIZE];
	Graphics* grptable[HASH_SIZE];
	struct JobItem
	{
		enum JobType
		{
			SavePicture,
			RedrawSurface,
			DeleteSurface,
			ApplyColorMatrix,
			DrawPicture,
			DrawPictureEx,
			DrawStretchedPicture,
			Blur,
			DrawSingleLineText,
			DrawText,
			DrawLine,
			DrawRect,
			DrawPolygon,
			DrawEllipse,
			DrawPie,
			SetPixel,
			SetPictureInfo,
			DeleteFont,
			DeleteBrush
		} type;
		// Parameter 1
		int surf1;

		// Parameter 2
		union
		{
			int surf2;
			char* text;
		};

		// Parameter 3
		union
		{
			struct
			{
				short x,y;
			} point1;
			LPRECT destrect;
			char* file;
			JPictureInfo* picinfo;
			float startangle;
		};

		// Parameter 4
		union
		{
			struct
			{
				short x,y;
			} point2;
			LPRECT srcrect;
			struct
			{
				int *px,*py;
			} plist;
		};

		// Parameter 5
		union
		{
			int pcount;
			BlurSize blursize;
		};

		union
		{
			struct
			{
				float sweepangle;
				float width;
			};
			JFont font;
			DWORD dwFlags;
		};
		JColor color;
		JBrush brush;
		ColorMatrix2* matrix;

		JobItem* next;
	} *joblist,*last;

	static __int64 qfreq;
	__int64 fpsfreq,lastdraw;
	bool fpsblock,vsync;

	bool AddJobList(JobItem* p);
	LPDIRECTDRAWSURFACE CreateSurface(DWORD width,DWORD height,bool sysmem);
	bool EnableGraphics(int idx);
	bool DisableGraphics(int idx);
	bool DrawPictureAuto(int id1,int id2,int idoverride=0,char* maskoverride=NULL);
	bool LoadPicture(int idx,char* filename,JPictureInfo* picinfo,bool sysmem,bool createsurface=true);
	bool LoadPicture(int idx,IStream* stream,JPictureInfo* picinfo,bool sysmem,bool createsurface=true);
public:
	JDirectDrawImp();
	virtual ~JDirectDrawImp();

	virtual bool Initialize(uint devid,HWND hwnd,uint width,uint height,uint bpp,bool sysmem=false);
	virtual bool Cleanup();

	int GetID(char* name);
	
	// Immediate
	virtual bool CreateSurface	(char* name,JPictureInfo* picinfo,bool sysmem=false,int* pid=NULL);
	virtual bool AddSurface		(char* name,JPictureInfo* picinfo,int *pid=NULL);
	virtual bool LoadPicture	(char* name,char* filename,JPictureInfo* picinfo=NULL,bool sysmem=false,int* pid=NULL);
	virtual bool LoadPicture	(char* name,IStream* stream,JPictureInfo* picinfo=NULL,bool sysmem=false,int* pid=NULL);
	virtual bool GetPictureInfo	(char* name,JPictureInfo* buffer);
	virtual bool GetPixel		(char* szSrc,int px,int py,JColor* pColor);
	virtual bool GetPictureInfo	(int hID,JPictureInfo* buffer);
	virtual bool GetPixel		(int hSrc,int px,int py,JColor* pColor);
	virtual bool MeasureText	(char* text,JFont font,LPRECT pRect);
	virtual JBrush CreateBrush(JColor jColor,float fOpacity=1.0f);
	virtual JFont CreateFont(char* szFace,int iHeight,bool bBold=false,bool bItalic=false,bool bUnderline=false,bool bStrikeout=false,bool bAntiAlias=false);

	// Queueable(Name Based)
	virtual bool ApplyColorMatrix(char* szDest	,char* szSrc	,int px,int py					,LPRECT pRect,const ColorMatrix2& matrix);
	virtual bool SavePicture	(char* name		,char* type		,char* filename);
	virtual bool RedrawSurface	(char* name);
	virtual bool DrawPicture	(char* szDest	,char* szSrc	,int px,int py					,LPRECT pRect=NULL);
	virtual bool DrawPictureEx	(char* szDest	,char* szSrc	,int px,int py					,LPRECT pRect=NULL,DWORD dwFlags=0);
	virtual bool DrawStretchedPicture(char* szDest,char* szSrc	,LPRECT destRect				,LPRECT srcRect);
	virtual bool Blur			(char* szDest	,char* szSrc	,int px,int py					,LPRECT srcRect,BlurSize blursize);
	virtual bool DrawText		(char* szDest	,char* szText	,JFont pFont					,LPRECT pRect,JColor color);
	virtual bool DrawText		(char* szDest	,char* szText	,JFont pFont					,int px,int py,JColor color);
	virtual bool DrawLine		(char* szDest	,JBrush pBrush	,int sx,int sy	,int ex,int ey	,float width);
	//  Setting width=0.0f will fill the shape
	virtual bool DrawRect		(char* szDest	,JBrush pBrush	,LPRECT pRect					,float width);
	virtual bool DrawPolygon	(char* szDest	,JBrush pBrush	,int* ppx,int* ppy	,int pcount	,float width);
	virtual bool DrawEllipse	(char* szDest	,JBrush pBrush	,LPRECT pRect					,float width);
	virtual bool DrawPie		(char* szDest	,JBrush pBrush	,LPRECT pRect		,float fStartAngle,float fSweepAngle,float width);

	virtual bool SetPixel		(char* szDest					,int px,int py					,JColor pColor);
	virtual bool SetPictureInfo	(char* name						,JPictureInfo* buffer);
	virtual bool DeleteSurface	(char* name);

	// Queable(ID Based)
	virtual bool ApplyColorMatrix(int hDest		,int hSrc		,int px,int py					,LPRECT pRect,const ColorMatrix2& matrix);
	virtual bool SavePicture	(int hID		,char* type		,char* filename);
	virtual bool RedrawSurface	(int hID);
	virtual bool DrawPicture	(int hDest		,int hSrc		,int px,int py					,LPRECT pRect=NULL);
	virtual bool DrawPictureEx	(int hDest		,int hSrc		,int px,int py					,LPRECT pRect=NULL,DWORD dwFlags=0);
	virtual bool DrawStretchedPicture(int hDest	,int hSrc		,LPRECT destRect				,LPRECT srcRect);
	virtual bool Blur			(int hDest		,int hSrc		,int px,int py					,LPRECT srcRect,BlurSize blursize);
	virtual bool DrawText		(int hDest		,char* szText	,JFont pFont					,LPRECT pRect,JColor color);
	virtual bool DrawText		(int hDest		,char* szText	,JFont pFont					,int px,int py,JColor color);
	virtual bool DrawLine		(int hDest		,JBrush pBrush	,int sx,int sy	,int ex,int ey	,float width);
	//  Setting width=0.0f will fill the shape
	virtual bool DrawRect		(int hDest		,JBrush pBrush	,LPRECT pRect					,float width);
	virtual bool DrawPolygon	(int hDest		,JBrush pBrush	,int* ppx,int* ppy	,int pcount	,float width);
	virtual bool DrawEllipse	(int hDest		,JBrush pBrush	,LPRECT pRect					,float width);
	virtual bool DrawPie		(int hDest		,JBrush pBrush	,LPRECT pRect		,float fStartAngle,float fSweepAngle,float width);

	virtual bool SetPixel		(int hDest						,int px,int py					,JColor pColor);
	virtual bool SetPictureInfo	(int hID						,JPictureInfo* buffer);
	virtual bool DeleteSurface	(int hID);

	// Queable
	virtual bool DeleteBrush	(				JBrush brush);
	virtual bool DeleteFont		(								JFont font);

	virtual char* GetBackBuffer();
	virtual int GetBackBufferID();

	virtual bool RestoreAllSurfaces();
	virtual bool SetVerticalSync(bool bVSync=true);
	virtual bool SetFrameRate(int iFPS,bool bBlocking=true);
	virtual bool Render(bool bDraw,bool bFlip=true);
	virtual bool Render();

	virtual HRESULT GetLastError();
};

#endif //__JDIRECTDRAWIMP_H__