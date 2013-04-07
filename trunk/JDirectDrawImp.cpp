#define WIN32_LEAN_AND_MEAN

#define INITGUID

#include <windows.h>
#include <ole2.h>
#include <crtdbg.h>
#include <memory.h>
#include <gdiplus.h>
#include <dxerr9.h>

#include "JDirectDrawImp.h"

#include "DDEffects3.h"

using namespace Gdiplus;

#pragma warning(disable:4244)

static int hash(char* sz)
{
	if(sz==NULL) return 0;
	unsigned int hashvalue=0;
	for(int lp=0;sz[lp];lp++)
		hashvalue=(hashvalue*HASH_NUMBER+sz[lp])%HASH_SIZE;
	return hashvalue;
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num=0; // number of image encoders
	UINT size=0; // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo=NULL;

	GetImageEncodersSize(&num, &size);
	if(size==0)
		return -1; // Failure

	pImageCodecInfo=(ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo==NULL)
		return -1; // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j=0;j<num;++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format)==0 )
		{
			*pClsid=pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j; // Success
		} 
	}

	free(pImageCodecInfo);
	return -1; // Failure
}

extern uint dd_devcount;
extern GUID dd_devguids[10];

__int64 JDirectDrawImp::qfreq;
__int64 ref;

#define EPSILON 1.0f/256
#define F_EQ(x,y) ((x-EPSILON)<(y) && (y)<(x+EPSILON))

#define DDC(X) if((lasterr=X)!=DD_OK) { lasterrmsg=DXGetErrorString9(lasterr); return false; }

void DDEnumDevices();

JDirectDrawImp::JDirectDrawImp() : lpDD(NULL), bProxy(NULL), lasterr(DD_OK), joblist(NULL), backbufferkey(hash(GetBackBuffer())), vsync(false), lpDD7(NULL)
{
	DDEnumDevices();
	memset(table,0,sizeof(table));
	memset(grptable,0,sizeof(grptable));

	GdiplusStartupInput in;
	GdiplusStartupOutput out;
	GdiplusStartup(&ptr,&in,&out);
	
	QueryPerformanceFrequency((LARGE_INTEGER*)&qfreq);
	QueryPerformanceCounter((LARGE_INTEGER*)&ref);
}

JDirectDrawImp::~JDirectDrawImp()
{
	Cleanup();
	GdiplusShutdown(ptr);
}

bool JDirectDrawImp::Initialize(uint devid,HWND hwnd,uint width,uint height,uint bpp,bool sysmem,bool window_mode)
{
	if(devid>dd_devcount) return false;
	if(lpDD) return false;
	DDC(DirectDrawCreate(devid==0?NULL:&dd_devguids[devid],&lpDD,NULL));
	DDC(lpDD->QueryInterface(IID_IDirectDraw7,(void**)&lpDD7));

	//윈도우 모드일 때 아닐때
	this->window_mode = window_mode;
	if(window_mode)
	{
		this->hwnd = hwnd;
		DDC(lpDD->SetCooperativeLevel(hwnd,DDSCL_NORMAL));
		SetRect(&render_rect, 0, 0, width, height);
	}
	else
	{
		DDC(lpDD->SetCooperativeLevel(hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN));
		DDC(lpDD->SetDisplayMode(width,height,bpp));
	}

	if(sysmem)
	{
		DDSURFACEDESC ddsd;
		ddsd.dwSize=sizeof(ddsd);
		ddsd.dwFlags=DDSD_CAPS;
		ddsd.dwBackBufferCount=1;
		ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
		DDC(lpDD->CreateSurface(&ddsd,&lpDDPriSurface,NULL));

		if(window_mode)
		{
			LPDIRECTDRAWCLIPPER pcClipper;
			lpDD->CreateClipper( 0, &pcClipper, NULL );
			pcClipper->SetHWnd( 0, hwnd );
			lpDDPriSurface->SetClipper( pcClipper );
			pcClipper->Release();
		}

		ddsd.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
		ddsd.dwWidth=width;
		ddsd.dwHeight=height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		DDC(lpDD->CreateSurface(&ddsd,&lpDDBackSurface,NULL));
		ddsd.dwWidth=width*2;
		ddsd.dwHeight=height*2;
		DDC(lpDD->CreateSurface(&ddsd,&lpDDTempSurface,NULL));
		DDC(lpDD->CreateSurface(&ddsd,&lpDDTempMask,NULL));
	} else {
		DDSURFACEDESC ddsd;
		ddsd.dwSize=sizeof(ddsd);
		ddsd.dwFlags=DDSD_BACKBUFFERCOUNT|DDSD_CAPS;
		ddsd.dwBackBufferCount=1;
		ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		DDC(lpDD->CreateSurface(&ddsd,&lpDDPriSurface,NULL));
		
		ddsd.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
		DDC(lpDDPriSurface->GetAttachedSurface(&ddsd.ddsCaps,&lpDDBackSurface));

		ddsd.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
		ddsd.dwWidth=width*2;
		ddsd.dwHeight=height*2;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		DDC(lpDD->CreateSurface(&ddsd,&lpDDTempSurface,NULL));
		DDC(lpDD->CreateSurface(&ddsd,&lpDDTempMask,NULL));
	}
	JPictureInfo info;
	info.SetSurface(lpDDBackSurface);
	info.SetWidth(width);
	info.SetHeight(height);
	screen_width=width;
	screen_height=height;
	bProxy=sysmem;
	AddSurface("Back Buffer",&info);

	info.SetSurface(lpDDTempSurface);
#define TEMPNAME "________Temporary Surface"
	AddSurface(TEMPNAME,&info);

	info.SetSurface(lpDDTempMask);
#define TEMPMASK "________Temporary Mask"
	AddSurface(TEMPMASK,&info);
	return true;
}

bool JDirectDrawImp::Cleanup()
{
	Render(false);

	bool clean = true;

	if(lpDDBackSurface)
	{
		lpDDBackSurface->Release();
		lpDDBackSurface = NULL;
	}
	else clean=false;

	if(lpDDPriSurface)
	{
		lpDDPriSurface->Release();
		lpDDPriSurface = NULL;
	}
	else clean=false;

	for(int lp=0;lp<HASH_SIZE;lp++)
	if(table[lp])
	{
		delete table[lp];
		table[lp] = NULL;
	}

	if(lpDD7)
	{
		lpDD7->Release();
		lpDD7=NULL;
	}
	else clean=false;

	if(lpDD)
	{
		lpDD->Release();
		lpDD = NULL;
	}
	else clean=false;

	return clean;
}

int JDirectDrawImp::GetID(char* name)
{
	return hash(name);
}

bool JDirectDrawImp::AddJobList(JobItem* p)
{
	if(joblist==NULL)
	{
		joblist=p;
		last=joblist;
		p->next=NULL;
	} else {
		last->next=p;
		last=p;
		p->next=NULL;
	}
	if(fpsfreq==0) return Render(true,false);
	return true;
}

LPDIRECTDRAWSURFACE JDirectDrawImp::CreateSurface(DWORD width,DWORD height,bool sysmem)
{
	DDSURFACEDESC ddsd;
	ddsd.dwSize=sizeof(ddsd);
	ddsd.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	ddsd.dwWidth=width;
	ddsd.dwHeight=height;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|(sysmem?DDSCAPS_SYSTEMMEMORY:0);

	LPDIRECTDRAWSURFACE surf;
	lasterr=lpDD->CreateSurface(&ddsd,&surf,NULL);

	return surf;
}

bool JDirectDrawImp::CreateSurface(char* name,JPictureInfo* picinfo,bool sysmem,int* pid)
{
	Render(true,false);
	int idx=hash(name);
	if(table[idx]) return false;

	LPDIRECTDRAWSURFACE surf=CreateSurface(picinfo->GetWidth(),picinfo->GetHeight(),sysmem);
	if(surf==NULL) return false;

	table[idx]=new JPictureInfo(*picinfo);
	table[idx]->SetSurface(surf);

	if(pid) *pid=idx;

	return true;
}

bool JDirectDrawImp::LoadPicture(int idx,char* filename,JPictureInfo* picinfo,bool sysmem,bool createsurface)
{
	static WCHAR wfilename[1001];
	MultiByteToWideChar(CP_ACP,0,filename,-1,wfilename,1000);
	Image* img=Image::FromFile(wfilename);
	if(img==NULL) return false;
	if(img->GetLastStatus()!=Ok) { delete img; return false; }
	
	if(createsurface)
	{
		LPDIRECTDRAWSURFACE surf=CreateSurface(img->GetWidth(),img->GetHeight(),sysmem);
		if(surf==NULL) return false;

		if(picinfo) table[idx]=new JPictureInfo(*picinfo); else table[idx]=new JPictureInfo;
		table[idx]->SetWidth(img->GetWidth());
		table[idx]->SetHeight(img->GetHeight());
		table[idx]->SetSurface(surf);
	}

	EnableGraphics(idx);
	grptable[idx]->DrawImage(img,Rect(0,0,img->GetWidth(),img->GetHeight()),0,0,img->GetWidth(),img->GetHeight(),UnitPixel);
	DisableGraphics(idx);
	delete img;

	return true;
}

bool JDirectDrawImp::LoadPicture(int idx,IStream* stream,JPictureInfo* picinfo,bool sysmem,bool createsurface)
{
	Image* img=Bitmap::FromStream(stream);
	if(img==NULL) return false;
	if(img->GetLastStatus()!=Ok) { delete img; return false; }
	
	if(createsurface)
	{
		LPDIRECTDRAWSURFACE surf=CreateSurface(img->GetWidth(),img->GetHeight(),sysmem);
		if(surf==NULL) return false;

		if(picinfo) table[idx]=new JPictureInfo(*picinfo); else table[idx]=new JPictureInfo;
		table[idx]->SetWidth(img->GetWidth());
		table[idx]->SetHeight(img->GetHeight());
		table[idx]->SetSurface(surf);
	}

	EnableGraphics(idx);
	grptable[idx]->DrawImage(img,Rect(0,0,img->GetWidth(),img->GetHeight()),0,0,img->GetWidth(),img->GetHeight(),UnitPixel);
	DisableGraphics(idx);
	delete img;

	return true;
}

bool JDirectDrawImp::LoadPicture(char* name,char* filename,JPictureInfo* picinfo,bool sysmem,int* pid)
{
	Render(true,false);
	int idx=hash(name);
	if(table[idx]) return false;

	bool ret=LoadPicture(idx,filename,picinfo,sysmem);
	if(ret && pid) *pid=idx;
	return ret;
}

bool JDirectDrawImp::LoadPicture(char* name,IStream* stream,JPictureInfo* picinfo,bool sysmem,int* pid)
{
	Render(true,false);
	int idx=hash(name);
	if(table[idx]) return false;

	bool ret=LoadPicture(idx,stream,picinfo,sysmem);
	if(ret && pid) *pid=idx;
	return ret;
}

bool JDirectDrawImp::AddSurface(char* name,JPictureInfo* picinfo,int* pid)
{
	int idx=hash(name);
	if(table[idx]) return false;
	if(picinfo->GetSurface()==NULL) return false;

	table[idx]=new JPictureInfo(*picinfo);
	if(pid) *pid=idx;
	return true;
}

bool JDirectDrawImp::SavePicture(char* name,char* type,char* filename)
{
	int idx=hash(name);
	return SavePicture(idx,type,filename);
}

bool JDirectDrawImp::GetPictureInfo(char* name,JPictureInfo* buffer)
{
	int idx=hash(name);
	return GetPictureInfo(idx,buffer);
}

bool JDirectDrawImp::SetPictureInfo(char* name,JPictureInfo* buffer)
{
	int idx=hash(name);
	return SetPictureInfo(idx,buffer);
}

bool JDirectDrawImp::RedrawSurface(char* name)
{
	int idx=hash(name);
	return RedrawSurface(idx);
}

bool JDirectDrawImp::DeleteSurface(char* name)
{
	int idx=hash(name);
	return DeleteSurface(idx);
}

bool JDirectDrawImp::ApplyColorMatrix(char* szDest,char* szSrc,int px,int py,LPRECT pRect,const ColorMatrix2& matrix)
{
	int idsrc=hash(szSrc);
	int iddest=hash(szDest);
	return ApplyColorMatrix(iddest,idsrc,px,py,pRect,matrix);
}

bool JDirectDrawImp::DrawPicture(char* szDest,char* szSrc,int px,int py,LPRECT pRect)
{
	int idsrc=hash(szSrc);
	int iddest=hash(szDest);
	return DrawPicture(iddest,idsrc,px,py,pRect);
}

bool JDirectDrawImp::DrawPictureEx(char* szDest,char* szSrc,int px,int py,LPRECT pRect,DWORD dwFlags)
{
	int idsrc=hash(szSrc);
	int iddest=hash(szDest);
	return DrawPictureEx(iddest,idsrc,px,py,pRect,dwFlags);
}

bool JDirectDrawImp::DrawStretchedPicture(char* szDest,char* szSrc,LPRECT destRect,LPRECT srcRect)
{
	int idsrc=hash(szSrc);
	int iddest=hash(szDest);
	return DrawStretchedPicture(iddest,idsrc,destRect,srcRect);
}

bool JDirectDrawImp::Blur(char* szDest,char* szSrc,int px,int py,LPRECT pRect,BlurSize blursize)
{
	int idsrc=hash(szSrc);
	int iddest=hash(szDest);
	return Blur(iddest,idsrc,px,py,pRect,blursize);
}

bool JDirectDrawImp::DrawText(char* szDest,char* szText,JFont font,LPRECT pRect,JColor color)
{
	int idx=hash(szDest);
	return DrawText(idx,szText,font,pRect,color);
}

bool JDirectDrawImp::DrawText(char* szDest,char* szText,JFont font,int px,int py,JColor color)
{
	int idx=hash(szDest);
	return DrawText(idx,szText,font,px,py,color);
}

bool JDirectDrawImp::MeasureText(char* text,JFont font,LPRECT pRect)
{
	if(!table[backbufferkey]) return false;
	if(!pRect) return false;

	EnableGraphics(backbufferkey);

	SIZE size;
	SelectObject(dctable[backbufferkey],(HFONT*)font);
	GetTextExtentPoint32(dctable[backbufferkey],text,strlen(text),&size);
	pRect->left=0;
	pRect->top=0;
	pRect->right=(int)size.cx;
	pRect->bottom=(int)size.cy;

	return true;
}

bool JDirectDrawImp::DrawLine(char* szDest,JBrush pBrush,int sx,int sy,int ex,int ey,float width)
{
	int idx=hash(szDest);
	return DrawLine(idx,pBrush,sx,sy,ex,ey,width);
}

bool JDirectDrawImp::DrawRect(char* szDest,JBrush pBrush,LPRECT pRect,float width)
{
	int idx=hash(szDest);
	return DrawRect(idx,pBrush,pRect,width);
}

bool JDirectDrawImp::DrawPolygon(char* szDest,JBrush pBrush,int* ppx,int* ppy,int pcount,float width)
{
	int idx=hash(szDest);
	return DrawPolygon(idx,pBrush,ppx,ppy,pcount,width);
}

bool JDirectDrawImp::DrawEllipse(char* szDest,JBrush pBrush,LPRECT pRect,float width)
{
	int idx=hash(szDest);
	return DrawEllipse(idx,pBrush,pRect,width);
}

bool JDirectDrawImp::DrawPie(char* szDest,JBrush pBrush,LPRECT pRect,float fStartAngle,float fSweepAngle,float width)
{
	int idx=hash(szDest);
	return DrawPie(idx,pBrush,pRect,fStartAngle,fSweepAngle,width);
}

bool JDirectDrawImp::GetPixel(char* szSrc,int px,int py,JColor* pColor)
{
	int idx=hash(szSrc);
	return GetPixel(idx,px,py,pColor);
}

bool JDirectDrawImp::SetPixel(char* szDest,int px,int py,JColor pColor)
{
	int idx=hash(szDest);
	return SetPixel(idx,px,py,pColor);
}

JBrush JDirectDrawImp::CreateBrush(JColor jColor,float fOpacity)
{
	if(fOpacity>1.0f) fOpacity=1.0f;
	return (JBrush)new SolidBrush(Color((BYTE)(fOpacity*255),jColor.r,jColor.g,jColor.b));
}

JFont JDirectDrawImp::CreateFont(char* szFace,int iHeight,bool bBold,bool bItalic,bool bUnderline,bool bStrikeout,bool bAntiAlias)
{
	return (JFont)::CreateFont(
		iHeight,0,0,0, // Size
		bBold?FW_BOLD:0,bItalic,bUnderline,bStrikeout, // Style
		DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,bAntiAlias?ANTIALIASED_QUALITY:DEFAULT_QUALITY,DEFAULT_PITCH, // Misc
		szFace // Face
		);
/*	WCHAR wface[100];
	mbstowcs(wface,szFace,100);
	DWORD style=0;
	if(bBold) style|=FontStyleBold;
	if(bItalic) style|=FontStyleItalic;
	if(bUnderline) style|=FontStyleUnderline;
	if(bStrikeout) style|=FontStyleStrikeout;
	return new Font(wface,iHeight,style,UnitPixel);*/
}

bool JDirectDrawImp::DeleteBrush(JBrush brush)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DeleteBrush;
	p->brush=brush;

	return AddJobList(p);
}

bool JDirectDrawImp::DeleteFont(JFont font)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DeleteFont;
	p->font=font;

	return AddJobList(p);
}

char* JDirectDrawImp::GetBackBuffer()
{
	return "Back Buffer";
}

bool JDirectDrawImp::RestoreAllSurfaces()
{
	for(int lp=0;lp<HASH_SIZE;lp++) if(table[lp] && table[lp]->GetSurface()->IsLost())
	{
		table[lp]->GetSurface()->Restore();
		if(table[lp]->GetSourceType()==PicInfo_FileSource)
		{
			LoadPicture(lp,table[lp]->GetFileSource(),NULL,false,false);
		} else {
			LoadPicture(lp,table[lp]->GetStreamSource(),NULL,false,false);
		}
	}
	return true;
}

bool JDirectDrawImp::EnableGraphics(int idx)
{
	if(grptable[idx]) return true;
	if(!table[idx]) return false;
	DDC(table[idx]->GetSurface()->GetDC(&dctable[idx]));
	SetBkMode(dctable[idx],TRANSPARENT);
	grptable[idx]=new Graphics(dctable[idx]);
	return true;
}

bool JDirectDrawImp::DisableGraphics(int idx)
{
	if(!table[idx]) return false;
	delete grptable[idx];
	grptable[idx]=NULL;
	table[idx]->GetSurface()->ReleaseDC(dctable[idx]);
	return true;
}

bool JDirectDrawImp::DrawPictureAuto(int id1,int id2,int idoverride,char* maskoverride)
{
	DisableGraphics(id1);
	DisableGraphics(id2);
	JPictureInfo* info=table[id2];
	JPictureInfo* over=(idoverride!=0)?table[idoverride]:info;
	char* maskname=(maskoverride!=0)?maskoverride:(over->GetMaskName());
	switch(over->GetLayerType())
	{
	case PicOverlayNormal:
		if(!maskname)
		{
			AlphaBlt(
				table[id1]->GetSurface(),
				table[id2]->GetSurface(),
				joblist->point1.x,
				joblist->point1.y,
				joblist->srcrect,
				over->GetOpacity(),
				over->GetColorKey());
		} else {
			int maskidx=hash(maskname);
			if(table[maskidx]) MaskAlphaBlt(
				table[id1]->GetSurface(),
				table[id2]->GetSurface(),
				joblist->point1.x,
				joblist->point1.y,
				joblist->srcrect,
				table[maskidx]->GetSurface(),
				over->GetOpacity(),
				over->GetColorKey());
		}
		break;
	case PicOverlayLight:
		LightBlt(
			table[id1]->GetSurface(),
			table[id2]->GetSurface(),
			joblist->point1.x,
			joblist->point1.y,
			joblist->srcrect,
			over->GetOpacity());
		break;
	case PicOverlayDark:
		DarkBlt(
			table[id1]->GetSurface(),
			table[id2]->GetSurface(),
			joblist->point1.x,
			joblist->point1.y,
			joblist->srcrect,
			over->GetOpacity());
		break;
	case PicOverlayAddition:
		AddBlt(
			table[id1]->GetSurface(),
			table[id2]->GetSurface(),
			joblist->point1.x,
			joblist->point1.y,
			joblist->srcrect,
			over->GetOpacity(),
			over->GetColorKey());
		break;
	}
	return true;
}

static DWORD Convert16Color(DWORD color32,int type16)
{
	if(color32==0) return 0;
	switch(type16)
	{
	case 555:
		return (color32>>19<<10)|((color32&0x00FF00)>>11<<5)|((color32&0x0000FF)>>3);
	case 565:
		return (color32>>19<<11)|((color32&0x00FF00)>>10<<5)|((color32&0x0000FF)>>3);
	}
	return 0;
}

DWORD ConvertColor(DWORD color,LPDIRECTDRAWSURFACE surf)
{
	DDPIXELFORMAT ddpf;
	ddpf.dwSize=sizeof(ddpf);
	surf->GetPixelFormat(&ddpf);
	switch(ddpf.dwRGBBitCount)
	{
	case 24:case 32:
		return color;
	case 16:
		switch(ddpf.dwGBitMask)
		{
		case 0x03E0:
			return Convert16Color(color,555);
		case 0x07E0:
			return Convert16Color(color,565);
		}
	}
	return 0xFFFFFFFF;
}

bool JDirectDrawImp::Render(bool bDraw,bool bFlip)
{
	bool bDrew=joblist!=NULL;
	while(joblist)
	{
		JobItem* next=joblist->next;
		switch(joblist->type)
		{
		case JobItem::RedrawSurface:
			break;
		case JobItem::DeleteSurface:
			if(table[joblist->surf1])
			{
				DisableGraphics(joblist->surf1);
				table[joblist->surf1]->GetSurface()->Release();
				table[joblist->surf1]->GetSurface()->Release();	//한번 릴리즈하면 바로 메모리 공간을 내놓지 않아서 두번 요청
				delete table[joblist->surf1];
				table[joblist->surf1]=NULL;
			}
			break;
		case JobItem::ApplyColorMatrix:
			if(bDraw && table[joblist->surf1] && table[joblist->surf2])
			{
				DisableGraphics(joblist->surf1);
				DisableGraphics(joblist->surf2);
				
				::ApplyColorMatrix(
					table[joblist->surf1]->GetSurface(),
					table[joblist->surf2]->GetSurface(),
					joblist->point1.x,
					joblist->point1.y,
					joblist->srcrect,
					*joblist->matrix);
			}
			delete joblist->matrix;
			break;
		case JobItem::DrawPicture:
			if(bDraw && table[joblist->surf1] && table[joblist->surf2])
			{
				DrawPictureAuto(joblist->surf1,joblist->surf2);
			}
			if(joblist->srcrect) delete joblist->srcrect;
			break;
		case JobItem::DrawStretchedPicture:
			if(bDraw && table[joblist->surf1] && table[joblist->surf2])
			{
				EnableGraphics(joblist->surf2);
				JPictureInfo* info=table[joblist->surf2];
				if(info->GetLayerType()==PicOverlayNormal && info->GetColorKey()==INVALID_COLOR && F_EQ(info->GetOpacity(),1.0f))
				{
					EnableGraphics(joblist->surf1);
					StretchBlt(dctable[joblist->surf1],
						joblist->destrect->left,
						joblist->destrect->top,
						joblist->destrect->right-joblist->destrect->left,
						joblist->destrect->bottom-joblist->destrect->top,
						dctable[joblist->surf2],
						joblist->srcrect->left,
						joblist->srcrect->top,
						joblist->srcrect->right-joblist->srcrect->left,
						joblist->srcrect->bottom-joblist->srcrect->top,
						SRCCOPY);
					delete joblist->srcrect;
					delete joblist->destrect;
				} else {
					int tempidx=hash(TEMPNAME);
					EnableGraphics(tempidx);
					StretchBlt(dctable[tempidx],
						0,
						0,
						joblist->destrect->right-joblist->destrect->left,
						joblist->destrect->bottom-joblist->destrect->top,
						dctable[joblist->surf2],
						joblist->srcrect->left,
						joblist->srcrect->top,
						joblist->srcrect->right-joblist->srcrect->left,
						joblist->srcrect->bottom-joblist->srcrect->top,
						SRCCOPY);
					if(table[joblist->surf2]->GetMaskName())
					{
						int tempmaskidx=hash(TEMPMASK);
						int maskidx=hash(table[joblist->surf2]->GetMaskName());
						EnableGraphics(tempmaskidx);
						EnableGraphics(maskidx);
						StretchBlt(dctable[tempmaskidx],
							0,
							0,
							joblist->destrect->right-joblist->destrect->left,
							joblist->destrect->bottom-joblist->destrect->top,
							dctable[maskidx],
							joblist->srcrect->left,
							joblist->srcrect->top,
							joblist->srcrect->right-joblist->srcrect->left,
							joblist->srcrect->bottom-joblist->srcrect->top,
							SRCCOPY);
						delete joblist->srcrect;
						joblist->srcrect=joblist->destrect;
						joblist->point1.x=joblist->srcrect->left;
						joblist->point1.y=joblist->srcrect->top;
						joblist->srcrect->right-=joblist->srcrect->left;
						joblist->srcrect->bottom-=joblist->srcrect->top;
						joblist->srcrect->left=0;
						joblist->srcrect->top=0;
						DrawPictureAuto(joblist->surf1,tempidx,joblist->surf2,TEMPMASK);
					} else {
						delete joblist->srcrect;
						joblist->srcrect=joblist->destrect;
						joblist->point1.x=joblist->srcrect->left;
						joblist->point1.y=joblist->srcrect->top;
						joblist->srcrect->right-=joblist->srcrect->left;
						joblist->srcrect->bottom-=joblist->srcrect->top;
						joblist->srcrect->left=0;
						joblist->srcrect->top=0;
						DrawPictureAuto(joblist->surf1,tempidx,joblist->surf2);
					}
					delete joblist->srcrect;
				}
			} else {
				delete joblist->srcrect;
				delete joblist->destrect;
			}
			break;
		case JobItem::Blur:
			if(bDraw && table[joblist->surf1] && table[joblist->surf2])
			{
				::Blur(
					table[joblist->surf1]->GetSurface(),
					table[joblist->surf2]->GetSurface(),
					joblist->point1.x,
					joblist->point1.y,
					joblist->srcrect,
					joblist->blursize
					);
			}
			if(joblist->srcrect) delete joblist->srcrect;
			break;
		case JobItem::DrawText:
			if(bDraw && table[joblist->surf1])
			{
				// GDI method
				EnableGraphics(joblist->surf1);
				SelectObject(dctable[joblist->surf1],(HFONT)joblist->font);
				SetTextColor(dctable[joblist->surf1],RGB(joblist->color.r,joblist->color.g,joblist->color.b));
				::DrawText(dctable[joblist->surf1],joblist->text,-1,joblist->srcrect,DT_LEFT|DT_TOP|DT_NOPREFIX);
			}
			free(joblist->text);
			delete joblist->srcrect;
			break;
		case JobItem::DrawSingleLineText:
			if(bDraw && table[joblist->surf1])
			{
				// GDI method
				EnableGraphics(joblist->surf1);
				SelectObject(dctable[joblist->surf1],(HFONT)joblist->font);
				SetTextColor(dctable[joblist->surf1],RGB(joblist->color.r,joblist->color.g,joblist->color.b));

				::TextOut(dctable[joblist->surf1],joblist->point1.x,joblist->point1.y,joblist->text,strlen(joblist->text));
			}
			free(joblist->text);
			break;
		case JobItem::DrawLine:
			if(bDraw && table[joblist->surf1])
			{
				EnableGraphics(joblist->surf1);
				Pen* pen=new Pen((Brush*)joblist->brush,joblist->width);
				grptable[joblist->surf1]->DrawLine(pen,joblist->point1.x,joblist->point1.y,joblist->point2.x,joblist->point2.y);
				delete pen;
			}
			break;
		case JobItem::DrawRect:
			if(bDraw && table[joblist->surf1])
			{
				EnableGraphics(joblist->surf1);
				if(joblist->width!=0.0f)
				{
					Pen* pen=new Pen((Brush*)joblist->brush,joblist->width);
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->DrawRectangle(pen,r);
					delete pen;
				} else {
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->FillRectangle((Brush*)joblist->brush,r);
				}
			}
			break;
		case JobItem::DrawPolygon:
			if(bDraw && table[joblist->surf1])
			{
				EnableGraphics(joblist->surf1);
				if(joblist->width!=0.0f)
				{
					Pen* pen=new Pen((Brush*)joblist->brush,joblist->width);
					Point* points=new Point[joblist->pcount];
					for(int lp=0;lp<joblist->pcount;lp++)
					{
						points[lp].X=joblist->plist.px[lp];
						points[lp].Y=joblist->plist.py[lp];
					}
					grptable[joblist->surf1]->DrawPolygon(pen,points,joblist->pcount);
					delete points;
					delete pen;
				} else {
					Point* points=new Point[joblist->pcount];
					for(int lp=0;lp<joblist->pcount;lp++)
					{
						points[lp].X=joblist->plist.px[lp];
						points[lp].Y=joblist->plist.py[lp];
					}
					grptable[joblist->surf1]->FillPolygon((Brush*)joblist->brush,points,joblist->pcount);
					delete points;
				}
			}
			delete joblist->plist.px;
			delete joblist->plist.py;
			break;
		case JobItem::DrawEllipse:
			if(bDraw && table[joblist->surf1])
			{
				EnableGraphics(joblist->surf1);
				if(joblist->width!=0.0f)
				{
					Pen* pen=new Pen((Brush*)joblist->brush,joblist->width);
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->DrawEllipse(pen,r);
					delete pen;
				} else {
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->FillEllipse((Brush*)joblist->brush,r);
				}
			}
			break;
		case JobItem::DrawPie:
			if(bDraw && table[joblist->surf1])
			{
				EnableGraphics(joblist->surf1);
				if(joblist->width!=0.0f)
				{
					Pen* pen=new Pen((Brush*)joblist->brush,joblist->width);
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->DrawPie(pen,r,joblist->startangle,joblist->sweepangle);
					delete pen;
				} else {
					Rect r(
						joblist->point1.x,
						joblist->point1.y,
						joblist->point2.x-joblist->point1.x,
						joblist->point2.y-joblist->point1.y);
					grptable[joblist->surf1]->FillPie((Brush*)joblist->brush,r,joblist->startangle,joblist->sweepangle);
				}
			}
			break;
		case JobItem::SetPixel:
			if(bDraw && table[joblist->surf1])
			{
				DisableGraphics(joblist->surf1);

				LPDIRECTDRAWSURFACE7 surf7=NULL;
				table[joblist->surf1]->GetSurface()->QueryInterface(IID_IDirectDrawSurface7,(void**)&surf7);
				if(surf7)
				{
					Bitmap* bmp=new Bitmap(surf7);
					bmp->SetPixel(joblist->point1.x,joblist->point1.y,Color(joblist->color.r,joblist->color.g,joblist->color.b));
					delete bmp;
					surf7->Release();
				}
			}
			break;
		case JobItem::SetPictureInfo:
			if(table[joblist->surf1])
			{
				LPDIRECTDRAWSURFACE surf=table[joblist->surf1]->GetSurface();
				*table[joblist->surf1]=*joblist->picinfo;
				table[joblist->surf1]->SetSurface(surf);
			}
			delete joblist->picinfo;
			break;
		case JobItem::DeleteBrush:
			delete (Brush*)joblist->brush;
			break;
		case JobItem::DeleteFont:
			DeleteObject((HFONT)joblist->font);
			break;
		}
		delete joblist;
		joblist=next;
	}
	if(bDraw && bDrew && bFlip || !fpsblock && bFlip)
	{
		DisableGraphics(backbufferkey);

		//윈도우 모드 출력
		if(window_mode)
		{
			if(vsync) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,NULL);
			lasterr=lpDDPriSurface->Blt( &render_rect, lpDDBackSurface, NULL, DDBLT_WAIT, NULL );
			//서페이스 리스톨
			if (lasterr == DDERR_SURFACELOST)
				lasterr = lpDDPriSurface->Restore();
		}
		//전체화면 출력
		else
		{
			if(bProxy)
			{
				RECT r={0,0,screen_width,screen_height};
				if(vsync) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,NULL);
				lasterr=lpDDPriSurface->BltFast(0,0,lpDDBackSurface,&r,DDBLTFAST_WAIT|DDBLTFAST_NOCOLORKEY);
			} else {
				lpDDPriSurface->Flip(NULL,DDFLIP_WAIT);
			}

			while(1)
			{
				if (lasterr == DD_OK)
				{
					break ;
				}

				if (lasterr == DDERR_SURFACELOST)
				{
					lasterr = lpDDPriSurface->Restore();

					if (lasterr != DD_OK)
					{
						break ;
					}
				}
			}
		}
	}
	return true;
}

bool JDirectDrawImp::SetVerticalSync(bool bVSync)
{
	vsync=bVSync;
	return true;
}

bool JDirectDrawImp::SetFrameRate(int iFPS,bool bBlocking)
{
	if(iFPS==0 || iFPS>100)
	{
		fpsfreq=0;
		fpsblock=false;
	}
	else
	{
		fpsfreq=qfreq/iFPS;
		fpsblock=bBlocking;
		QueryPerformanceCounter((LARGE_INTEGER*)&lastdraw);
	}
	return true;
}

bool JDirectDrawImp::Render()
{
	if(fpsfreq==0) return Render(true);
	__int64 ctime;
	if(fpsblock)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&ctime);
		__int64 nextdraw=(ctime+fpsfreq-1)/fpsfreq*fpsfreq;
		while(nextdraw-ctime>0)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&ctime);
			Sleep(0);
		}
		lastdraw=ctime;
		bool ret=Render(true);
		return ret;
	} else {
		QueryPerformanceCounter((LARGE_INTEGER*)&ctime);
		if(ctime-lastdraw>=fpsfreq)
		{
			lastdraw=ctime;
			Render(true);
			return true;
		} else {
			Render(false,false);
			return false;
		}
	}
}

HRESULT JDirectDrawImp::GetLastError()
{
	return lasterr;
}



/* DrawStretchedPicture Backup

		case JobItem::DrawStretchedPicture:
			if(bDraw && table[joblist->surf1] && table[joblist->surf2])
			{
				DDBLTFX bltfx={0};
				bltfx.dwSize=sizeof(bltfx);
				if(joblist->dwFlags&DPX_HFLIP) bltfx.dwDDFX|=DDBLTFX_MIRRORLEFTRIGHT;
				if(joblist->dwFlags&DPX_VFLIP) bltfx.dwDDFX|=DDBLTFX_MIRRORUPDOWN;
				DisableGraphics(joblist->surf2);
				JPictureInfo* info=table[joblist->surf2];
				if(info->GetLayerType()==PicOverlayNormal && F_EQ(info->GetOpacity(),1.0f))
				{
					DisableGraphics(joblist->surf1);
					bltfx.ddckSrcColorkey.dwColorSpaceHighValue=
						bltfx.ddckSrcColorkey.dwColorSpaceLowValue=
						ConvertColor(info->GetColorKey(),table[joblist->surf1]->GetSurface());

					table[joblist->surf1]->GetSurface()->Blt(
						joblist->destrect,
						table[joblist->surf2]->GetSurface(),
						joblist->srcrect,
						DDBLT_WAIT|
						(table[joblist->surf2]->GetColorKey()!=INVALID_COLOR?DDBLT_KEYSRCOVERRIDE:0)|
						(joblist->dwFlags?DDBLT_DDFX:0),
						&bltfx);
					delete joblist->srcrect;
					delete joblist->destrect;
				} else {
					int tempidx=hash(TEMPNAME);
					DisableGraphics(tempidx);
					RECT destrect;
					destrect.left=destrect.top=0;
					destrect.right=joblist->destrect->right-joblist->destrect->left;
					destrect.bottom=joblist->destrect->bottom-joblist->destrect->top;
					table[tempidx]->GetSurface()->Blt(
						&destrect,
						table[joblist->surf2]->GetSurface(),
						joblist->srcrect,
						DDBLT_WAIT|
						(joblist->dwFlags?DDBLT_DDFX:0),
						joblist->dwFlags?&bltfx:NULL);
					if(table[joblist->surf2]->GetMaskName())
					{
						int tempmaskidx=hash(TEMPMASK);
						int maskidx=hash(table[joblist->surf2]->GetMaskName());
						DisableGraphics(tempmaskidx);
						DisableGraphics(maskidx);
						table[tempmaskidx]->GetSurface()->Blt(
							&destrect,
							table[maskidx]->GetSurface(),
							joblist->srcrect,
							DDBLT_WAIT|
							(joblist->dwFlags?DDBLT_DDFX:0),
							joblist->dwFlags?&bltfx:NULL);
						delete joblist->srcrect;
						joblist->srcrect=joblist->destrect;
						joblist->point1.x=joblist->srcrect->left;
						joblist->point1.y=joblist->srcrect->top;
						joblist->srcrect->right-=joblist->srcrect->left;
						joblist->srcrect->bottom-=joblist->srcrect->top;
						joblist->srcrect->left=0;
						joblist->srcrect->top=0;
						DrawPictureAuto(joblist->surf1,tempidx,joblist->surf2,TEMPMASK);
					} else {
						delete joblist->srcrect;
						joblist->srcrect=joblist->destrect;
						joblist->point1.x=joblist->srcrect->left;
						joblist->point1.y=joblist->srcrect->top;
						joblist->srcrect->right-=joblist->srcrect->left;
						joblist->srcrect->bottom-=joblist->srcrect->top;
						joblist->srcrect->left=0;
						joblist->srcrect->top=0;
						DrawPictureAuto(joblist->surf1,tempidx,joblist->surf2);
					}
					delete joblist->srcrect;
				}
			} else {
				delete joblist->srcrect;
				delete joblist->destrect;
			}
			break;

*/

/*창 모드 추가*/
void JDirectDrawImp::OnMove(int x, int y)
{
	if(!window_mode)return;

	JPictureInfo info;
	GetPictureInfo("Back Buffer", &info);

	//서페이스 위치 재설정
	LONG ws=WS_OVERLAPPEDWINDOW|WS_VISIBLE;
	ws &= ~WS_THICKFRAME;
	ws &= ~WS_MAXIMIZEBOX;

	RECT crt;
	SetRect(&crt, 0, 0, info.GetWidth(), info.GetHeight());
	AdjustWindowRect(&crt, ws, FALSE);

	RECT m_rcClient;
	GetWindowRect(hwnd, &m_rcClient);
	int window_x=m_rcClient.left - crt.left;
	int window_y=m_rcClient.top - crt.top;
	SetRect(&render_rect, window_x, window_y, window_x+info.GetWidth(), window_y+info.GetHeight());
}