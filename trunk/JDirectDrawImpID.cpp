#include "JDirectDrawImp.h"

#pragma warning(disable:4244)

#define ERROR_CHECK		if(idx < 0 || idx >= HASH_SIZE)return false; if(!table[idx]) return false

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

bool JDirectDrawImp::SavePicture(int idx,char* type,char* filename)
{
	ERROR_CHECK;
	Render(true);

	DisableGraphics(idx);
	WCHAR wsz[1001];
	MultiByteToWideChar(CP_ACP,0,filename,-1,wsz,1000);
	WCHAR wt[101];
	CLSID clsid;
	MultiByteToWideChar(CP_ACP,0,type,-1,wt,100);
	GetEncoderClsid(wt,&clsid);

	LPDIRECTDRAWSURFACE7 surf7=NULL;
	table[joblist->surf1]->GetSurface()->QueryInterface(IID_IDirectDrawSurface7,(void**)&surf7);
	if(!surf7) return false;

	Image* img=new Bitmap(surf7);
	img->Save(wsz,&clsid);
	delete img;
	surf7->Release();

	return img->GetLastStatus()==Ok;
}

bool JDirectDrawImp::GetPictureInfo(int idx,JPictureInfo* buffer)
{
	ERROR_CHECK;

	*buffer=*table[idx];
	return true;
}

bool JDirectDrawImp::SetPictureInfo(int idx,JPictureInfo* buffer)
{
	ERROR_CHECK;

	JobItem* p=new JobItem;
	p->type=JobItem::SetPictureInfo;
	p->surf1=idx;
	p->picinfo=new JPictureInfo(*buffer);

	return AddJobList(p);
}

bool JDirectDrawImp::RedrawSurface(int idx)
{
	ERROR_CHECK;

	JobItem* p=new JobItem;
	p->type=JobItem::RedrawSurface;
	p->surf1=idx;
	
	return AddJobList(p);
}

bool JDirectDrawImp::DeleteSurface(int idx)
{
	ERROR_CHECK;

	JobItem* p=new JobItem;
	p->type=JobItem::DeleteSurface;
	p->surf1=idx;
	
	return AddJobList(p);
}

bool JDirectDrawImp::ApplyColorMatrix(int iddest,int idsrc,int px,int py,LPRECT pRect,const ColorMatrix2& matrix)
{
	if(!table[idsrc] || !table[iddest]) return false;

	JobItem* p=new JobItem;
	p->type=JobItem::ApplyColorMatrix;
	p->surf1=iddest;
	p->surf2=idsrc;
	p->point1.x=px;
	p->point1.y=py;
	if(pRect)
	{
		p->srcrect=new RECT;
		*p->srcrect=*pRect;
	} else p->srcrect=NULL;
	p->matrix=new ColorMatrix2(matrix);

	return AddJobList(p);
}

bool JDirectDrawImp::DrawPicture(int iddest,int idsrc,int px,int py,LPRECT pRect)
{
	if(!table[idsrc] || !table[iddest]) return false;

	JobItem* p=new JobItem;
	p->type=JobItem::DrawPicture;
	p->surf1=iddest;
	p->surf2=idsrc;
	p->point1.x=px;
	p->point1.y=py;
	if(pRect)
	{
		p->srcrect=new RECT;
		*p->srcrect=*pRect;
	} else p->srcrect=NULL;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawPictureEx(int iddest,int idsrc,int px,int py,LPRECT pRect,DWORD dwFlags)
{
	if(!table[idsrc] || !table[iddest]) return false;

	JobItem* p=new JobItem;
	p->type=JobItem::DrawStretchedPicture;
	p->surf1=iddest;
	p->surf2=idsrc;
	p->point1.x=px;
	p->point1.y=py;
	p->srcrect=new RECT;
	if(pRect)
	{
		*p->srcrect=*pRect;
	} else {
		p->srcrect->left=0;
		p->srcrect->top=0;
		p->srcrect->right=table[idsrc]->GetWidth();
		p->srcrect->bottom=table[idsrc]->GetHeight();
		pRect=p->srcrect;
	}
	p->destrect=new RECT;
	p->destrect->left=px;
	p->destrect->top=py;
	p->destrect->right=px+pRect->right-pRect->left;
	p->destrect->bottom=py+pRect->bottom-pRect->top;
	if(dwFlags&DPX_HFLIP)
	{
		int t=p->srcrect->right-1;
		p->srcrect->right=p->srcrect->left-1;
		p->srcrect->left=t;
	}
	if(dwFlags&DPX_VFLIP)
	{
		int t=p->srcrect->bottom;
		p->srcrect->bottom=p->srcrect->top;
		p->srcrect->top=t;
	}
	
	return AddJobList(p);
}

bool JDirectDrawImp::DrawStretchedPicture(int iddest,int idsrc,LPRECT destRect,LPRECT srcRect)
{
	if(!table[idsrc] || !table[iddest]) return false;
	if(!destRect || !srcRect) return false;

	JobItem* p=new JobItem;
	p->type=JobItem::DrawStretchedPicture;
	p->surf1=iddest;
	p->surf2=idsrc;
	p->destrect=new RECT;
	*p->destrect=*destRect;
	p->srcrect=new RECT;
	*p->srcrect=*srcRect;
	p->dwFlags=0;
	
	return AddJobList(p);
}

bool JDirectDrawImp::Blur(int iddest,int idsrc,int px,int py,LPRECT pRect,BlurSize blursize)
{
	if(!table[idsrc] || !table[iddest]) return false;

	JobItem* p=new JobItem;
	p->type=JobItem::Blur;
	p->surf1=iddest;
	p->surf2=idsrc;
	p->point1.x=px;
	p->point1.y=py;
	if(pRect)
	{
		p->srcrect=new RECT;
		*p->srcrect=*pRect;
	} else p->srcrect=NULL;
	p->blursize=blursize;
	
	return AddJobList(p);
}

bool JDirectDrawImp::DrawText(int idx,char* szText,JFont font,LPRECT pRect,JColor color)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawText;
	p->surf1=idx;
	p->text=strdup(szText);
	p->font=font;
	p->color=color;
	if(pRect)
	{
		p->srcrect=new RECT;
		*p->srcrect=*pRect;
	} else p->srcrect=NULL;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawText(int idx,char* szText,JFont font,int px,int py,JColor color)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawSingleLineText;
	p->surf1=idx;
	p->text=strdup(szText);
	p->font=font;
	p->color=color;
	p->point1.x=px;
	p->point1.y=py;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawLine(int idx,JBrush pBrush,int sx,int sy,int ex,int ey,float width)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawLine;
	p->surf1=idx;
	p->brush=pBrush;
	p->point1.x=sx;
	p->point1.y=sy;
	p->point2.x=ex;
	p->point2.y=ey;
	p->width=width;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawRect(int idx,JBrush pBrush,LPRECT pRect,float width)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawRect;
	p->surf1=idx;
	p->brush=pBrush;
	p->point1.x=pRect->left;
	p->point1.y=pRect->top;
	p->point2.x=pRect->right;
	p->point2.y=pRect->bottom;
	p->width=width;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawPolygon(int idx,JBrush pBrush,int* ppx,int* ppy,int pcount,float width)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawPolygon;
	p->surf1=idx;
	p->brush=pBrush;
	p->plist.px=new int[pcount];
	memcpy(p->plist.px,ppx,sizeof(int)*pcount);
	p->plist.py=new int[pcount];
	memcpy(p->plist.py,ppy,sizeof(int)*pcount);
	p->pcount=pcount;
	p->width=width;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawEllipse(int idx,JBrush pBrush,LPRECT pRect,float width)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawEllipse;
	p->surf1=idx;
	p->brush=pBrush;
	p->point1.x=pRect->left;
	p->point1.y=pRect->top;
	p->point2.x=pRect->right;
	p->point2.y=pRect->bottom;
	p->width=width;

	return AddJobList(p);
}

bool JDirectDrawImp::DrawPie(int idx,JBrush pBrush,LPRECT pRect,float fStartAngle,float fSweepAngle,float width)
{
	JobItem* p=new JobItem;
	p->type=JobItem::DrawPie;
	p->surf1=idx;
	p->brush=pBrush;
	p->point1.x=pRect->left;
	p->point1.y=pRect->top;
	p->point2.x=pRect->right;
	p->point2.y=pRect->bottom;
	p->startangle=fStartAngle;
	p->sweepangle=fSweepAngle;
	p->width=width;

	return AddJobList(p);
}

bool JDirectDrawImp::GetPixel(int idx,int px,int py,JColor* pColor)
{
	Render(true);
	if(!pColor) return false;

	DisableGraphics(idx);

	LPDIRECTDRAWSURFACE7 surf7=NULL;
	table[joblist->surf1]->GetSurface()->QueryInterface(IID_IDirectDrawSurface7,(void**)&surf7);
	if(!surf7) return false;

	Bitmap* bmp=new Bitmap(surf7);
	Color color;
	bmp->GetPixel(joblist->point1.x,joblist->point1.y,&color);
	delete bmp;
	surf7->Release();

	pColor->r=color.GetRed();
	pColor->g=color.GetGreen();
	pColor->b=color.GetBlue();
	return true;
}

bool JDirectDrawImp::SetPixel(int idx,int px,int py,JColor pColor)
{
	JobItem* p=new JobItem;
	p->type=JobItem::SetPixel;
	p->surf1=idx;
	p->point1.x=px;
	p->point1.y=py;
	p->color=pColor;

	return AddJobList(p);
}

int JDirectDrawImp::GetBackBufferID()
{
	return backbufferkey;
}