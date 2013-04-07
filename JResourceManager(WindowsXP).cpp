#include "JResourceManagerImp.h"

#include <string.h>
#include <stdio.h>

JResourceManager* CreateDXResourceManager(JDirectDraw* pdd)
{
	return new JResourceManagerImp(pdd);
}

IStreamMinimumImp::IStreamMinimumImp(char* filename) : refcount(1), emulatedsize(0), emulatedbegin(0)
{
	this->filename=strdup(filename);
	hfile=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
}

IStreamMinimumImp::~IStreamMinimumImp()
{
	CloseHandle(hfile);
	free(filename);
}

ULONG IStreamMinimumImp::AddRef()
{
	return ++refcount;
}

ULONG IStreamMinimumImp::Release()
{
	ULONG ret=--refcount;
	if(ret==0) delete this;
	return ret;
}

HRESULT IStreamMinimumImp::QueryInterface(REFIID iid,LPVOID* ppv)
{
	if(iid==IID_IStream)
	{
		*ppv=(IStream*)this;
		return S_OK;
	}
	if(iid==IID_ISequentialStream)
	{
		*ppv=(ISequentialStream*)this;
		return S_OK;
	}
	if(iid==IID_IUnknown)
	{
		*ppv=(IUnknown*)this;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT IStreamMinimumImp::Stat(STATSTG *pstatstg,DWORD grfStatFlag)
{
	memset(pstatstg,0,sizeof(STATSTG));
	if(!(grfStatFlag&STATFLAG_NONAME))
	{
		if(!emfilename)
			mbstowcs(pstatstg->pwcsName,filename,MB_CUR_MAX);
		else
			mbstowcs(pstatstg->pwcsName,emfilename,MB_CUR_MAX);
	}
	pstatstg->type=STGTY_STREAM;
	if(emulatedsize)
		pstatstg->cbSize.QuadPart=emulatedsize;
	else
		pstatstg->cbSize.QuadPart=GetFileSize(hfile,NULL);
	pstatstg->grfMode=STGM_READWRITE|STGM_SHARE_DENY_WRITE;
	pstatstg->grfLocksSupported=0;
	GetFileTime(hfile,&pstatstg->ctime,&pstatstg->atime,&pstatstg->mtime);
	return S_OK;
}

HRESULT IStreamMinimumImp::Read(void *pv,ULONG cb,ULONG *pcbRead)
{
	DWORD x;
	if(!pcbRead) pcbRead=&x;
	return ReadFile(hfile,pv,cb,pcbRead,NULL)?S_OK:S_FALSE;
}

HRESULT IStreamMinimumImp::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER* plibNewPosition)
{
	ULARGE_INTEGER dummy;
	if(!plibNewPosition) plibNewPosition=&dummy;
	if(!emulatedbegin)
		plibNewPosition->QuadPart=SetFilePointer(hfile,(long)dlibMove.QuadPart,NULL,dwOrigin);
	else {
		switch(dwOrigin)
		{
		case STREAM_SEEK_SET:
			plibNewPosition->QuadPart=SetFilePointer(hfile,emulatedbegin+(long)dlibMove.QuadPart,NULL,FILE_BEGIN);
			break;
		case STREAM_SEEK_CUR:
			plibNewPosition->QuadPart=SetFilePointer(hfile,(long)dlibMove.QuadPart,NULL,FILE_CURRENT);
			break;
		case STREAM_SEEK_END:
			plibNewPosition->QuadPart=SetFilePointer(hfile,emulatedbegin+emulatedsize-(long)dlibMove.QuadPart,NULL,FILE_END);
			break;
		}
		plibNewPosition->QuadPart-=emulatedbegin;
	}
	return S_OK;
}

bool IStreamMinimumImp::EmulateSize(DWORD size,char* filename)
{
	if(size!=0)
	{
		emulatedbegin=SetFilePointer(hfile,0,NULL,FILE_CURRENT);
		emulatedsize=size;
		emfilename=strdup(filename);
	} else { 
		emulatedbegin=0;
		emulatedsize=0;
		free(emfilename);
		emfilename=NULL;
	}
	return true;
}

JResourceManagerImp::JResourceManagerImp(JDirectDraw* dd)
{
	this->dd=dd;
	memset(datapool,0,sizeof(datapool));
}

JResourceManagerImp::~JResourceManagerImp()
{
	for(int lp=0;lp<HASH_SIZE;lp++) free(datapool[lp]);
}

static int hash(char* sz)
{
	if(sz==NULL) return 0;
	unsigned int hashvalue=0;
	for(int lp=0;sz[lp];lp++)
		hashvalue=(hashvalue*HASH_NUMBER+sz[lp])%HASH_SIZE;
	return hashvalue;
}

int JResourceManagerImp::ReadData(char* name,void* dt,unsigned int len)
{
	if(!name) return 0;
	if(!dt) return 0;
	int idx=hash(name);
	if(!datapool[idx]) return 0;
	if(len>sizepool[idx]) len=sizepool[idx];
	memcpy(dt,datapool[idx],len);
	return len;
}

/******************************************
* All offsets are origined from beginning
* File
*	4B DWORD    : File ID (Should be CHNK)
*
* Chunk
*	128B String : Chunk Name
*   4B DWORD    : Chunk Type
*	4B DWORD    : Chunk Size
******************************************/

inline LARGE_INTEGER cv64(int x) { LARGE_INTEGER large; large.QuadPart=x; return large; }

struct chunkdata
{
	char name[128];
	char filename[128];
	DWORD type;
	DWORD size;
};

struct picturedata
{
	DWORD flag;
	PicLayerType layertype;
	float opacity;
	DWORD colorkey;
	DWORD sysmem;
	char maskname[128];
};

#define SOUND_VOL 0x01
#define SOUND_BAL 0x02
#define SOUND_LOOP 0x04

struct sounddata
{
	DWORD flag;
	double vol;
	double bal;
	DWORD loop;
};

bool JResourceManagerImp::LoadResource(char* filename)
{
#	define EXIT { istream->Release(); return false; }
	IStreamMinimumImp* istream=new IStreamMinimumImp(filename);
	if(!istream->Loaded()) EXIT;
	DWORD sign;
	istream->Read(&sign,4,NULL);

	DWORD ptr=4;
	STATSTG stat;
	istream->Stat(&stat,STATFLAG_NONAME);
	while(1)
	{
		chunkdata dt;
		ULONG read;
		istream->Read(&dt,sizeof(dt),&read);
		if(read!=sizeof(dt)) break;
		switch(dt.type)
		{
		case 0: // data
			{
				int idx=hash(dt.name);
				if(datapool[idx]) break;
				datapool[idx]=malloc(dt.size);
				sizepool[idx]=dt.size;
				istream->Read(datapool[idx],dt.size,NULL);
			}
			break;
		case 1: // picture
			{
				picturedata pdt;
				istream->Read(&pdt,sizeof(pdt),NULL);
				JPictureInfo info;
				if(pdt.flag&PicInfo_LayerType) info.SetLayerType(pdt.layertype);
				if(pdt.flag&PicInfo_Opacity) info.SetOpacity(pdt.opacity);
				if(pdt.flag&PicInfo_MaskName) info.SetMaskName(pdt.maskname);
				if(pdt.flag&PicInfo_ColorKey)
				{
					JColor color;
					color.color=pdt.colorkey;
					info.SetColorKey(color);
				}
				istream->EmulateSize(dt.size-sizeof(pdt),dt.filename);
				dd->LoadPicture(dt.name,istream,&info,pdt.sysmem!=0);
				istream->EmulateSize(0);
			}
			break;
		}
		ptr+=dt.size+sizeof(chunkdata);
		istream->Seek(cv64(ptr),STREAM_SEEK_SET,NULL);
	}
	istream->Release();
	return true;
#	undef EXIT
}

bool JResourceManagerImp::UnloadResource(char* filename)
{
#	define EXIT { istream->Release(); return false; }
	IStreamMinimumImp* istream=new IStreamMinimumImp(filename);
	if(!istream->Loaded()) EXIT;
	DWORD sign;
	istream->Read(&sign,4,NULL);
	if(sign!=0x4b4e4843) EXIT;

	DWORD ptr=4;
	STATSTG stat;
	istream->Stat(&stat,STATFLAG_NONAME);
	while(1)
	{
		chunkdata dt;
		ULONG read;
		istream->Read(&dt,sizeof(dt),&read);
		if(read!=sizeof(dt)) break;

		switch(dt.type)
		{
		case 0: // data
			{
				int idx=hash(dt.name);
				free(datapool[idx]); datapool[idx]=NULL;
				sizepool[idx]=0;
			}
			break;
		case 1: // picture
			dd->DeleteSurface(dt.name);
			break;
		}
		ptr+=dt.size+sizeof(chunkdata);
		istream->Seek(cv64(ptr),STREAM_SEEK_SET,NULL);
	}
	istream->Release();
	return true;
#	undef EXIT
}


