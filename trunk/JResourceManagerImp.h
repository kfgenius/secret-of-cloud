#include "JResourceManager.h"

class IStreamMinimumImp : public IStream
{
protected:
	HANDLE hfile;
	ULONG refcount;
	char* filename;
	DWORD emulatedbegin,emulatedsize;
	char* emfilename;
public:
	IStreamMinimumImp(char* filename);
	~IStreamMinimumImp();

	bool Loaded() { return hfile!=INVALID_HANDLE_VALUE && hfile!=NULL; }
	bool EmulateSize(DWORD size,char* filename=NULL);

	// Stubs
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm) { if(ppstm) *ppstm=NULL; return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*,ULARGE_INTEGER,ULARGE_INTEGER*,ULARGE_INTEGER*) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE Revert() { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE Write(const void*,ULONG,ULONG*) { return S_OK; }

	// Implemented
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,LPVOID* ppv);
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg,DWORD grfStatFlag);
	virtual HRESULT STDMETHODCALLTYPE Read(void *pv,ULONG cb,ULONG *pcbRead);
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER* plibNewPosition);
};

#define HASH_NUMBER 37
#define HASH_SIZE 262139

class JResourceManagerImp : public JResourceManager
{
protected:
	JDirectDraw* dd;
	unsigned int sizepool[HASH_SIZE];
	void* datapool[HASH_SIZE];
public:
	JResourceManagerImp(JDirectDraw* dd);
	virtual ~JResourceManagerImp();

	virtual int ReadData(char* name,void* dt,unsigned int len);

	virtual bool LoadResource(char* filename);
	virtual bool UnloadResource(char* filename);
};
