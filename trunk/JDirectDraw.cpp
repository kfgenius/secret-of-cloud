#include "JDirectDraw.h"

#include <string.h>
#include <crtdbg.h>

JPictureInfo::JPictureInfo()
: dwFlags(PicInfo_None)
, eLayerType(PicOverlayNormal)
, fOpacity(1.0f)
, szMaskName(NULL)
, pSource(NULL)
, surface(NULL)
, cColorKey(INVALID_COLOR)
{
}

JPictureInfo::JPictureInfo(JPictureInfo& src) 
: dwWidth(src.dwWidth)
, dwHeight(src.dwHeight)
, surface(src.surface)
, dwFlags(src.dwFlags)
, eLayerType(src.eLayerType)
, fOpacity(src.fOpacity)
, cColorKey(src.cColorKey)
, fRedrawFunc(src.fRedrawFunc)
, rClipRegion(src.rClipRegion)
, pContext(src.pContext)
{
	if(dwFlags&PicInfo_StreamSource)
	{
		sSource=src.sSource;
		sSource->AddRef();
	} else if(dwFlags&PicInfo_FileSource) {
		pSource=strdup(src.pSource);
	} else sSource=NULL;
	if((dwFlags&PicInfo_MaskName) && src.szMaskName)
		szMaskName=strdup(src.szMaskName);
	else
		szMaskName=NULL;
	if(surface) surface->AddRef();
}

JPictureInfo::~JPictureInfo()
{
	if(dwFlags&PicInfo_StreamSource)
	{
		sSource->Release();
	} else if(dwFlags&PicInfo_FileSource) {
		free(pSource);
	}
	free(szMaskName);
}

void JPictureInfo::SetSurface(LPDIRECTDRAWSURFACE surface)
{
	if(this->surface) this->surface->Release();
	DDSURFACEDESC ddsd;
	ddsd.dwSize=sizeof(ddsd);
	surface->GetSurfaceDesc(&ddsd);
	dwWidth=ddsd.dwWidth;
	dwHeight=ddsd.dwHeight;
	this->surface=surface;
	surface->AddRef();
}

void JPictureInfo::SetLayerType(PicLayerType type)
{
	dwFlags|=PicInfo_LayerType;
	eLayerType=type;
}

void JPictureInfo::SetOpacity(float opacity)
{
	dwFlags|=PicInfo_Opacity;
	fOpacity=opacity;
}

void JPictureInfo::SetMaskName(char* maskname)
{
	dwFlags|=PicInfo_MaskName;
	if(szMaskName) free(szMaskName);
	szMaskName=strdup(maskname);
}

void JPictureInfo::SetColorKey(JColor colorkey)
{
	dwFlags|=PicInfo_ColorKey;
	cColorKey=colorkey;
}

void JPictureInfo::SetRedrawFunc(RedrawFunc func)
{
	dwFlags|=PicInfo_RedrawFunc;
	fRedrawFunc=func;
}

void JPictureInfo::SetClipRegion(LPRECT rect)
{
	if(rect==NULL) dwFlags&=~PicInfo_ClipRegion; else
	{
		dwFlags|=PicInfo_ClipRegion;
		rClipRegion=*rect;
	}
}

GUID dd_devguids[10];
char dd_devdesc[10][100];
uint dd_devcount;

BOOL WINAPI DDEnumCallBackEx(GUID FAR* lpGUID,LPSTR szDeviceDesc,LPSTR,LPVOID,HMONITOR)
{
	if(lpGUID==NULL)
		dd_devguids[dd_devcount].Data1=0;
	else
		dd_devguids[dd_devcount]=*lpGUID;
	strcpy(dd_devdesc[dd_devcount],szDeviceDesc);
	dd_devcount++;
	return DDENUMRET_OK;
}

void DDEnumDevices()
{
	dd_devcount=0;
	DirectDrawEnumerateEx(DDEnumCallBackEx,NULL,DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_DETACHEDSECONDARYDEVICES);
}

uint JDirectDraw::GetDeviceCount()
{
	return dd_devcount;
}

uint JDirectDraw::GetDeviceDesc(uint idx,char* buffer)
{
	strcpy(buffer,dd_devdesc[idx]);
	return strlen(buffer);
}

#include "JDirectDrawImp.h"

JDirectDraw* CreateDirectDraw() { return new JDirectDrawImp; }
