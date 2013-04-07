#include "JDirectDraw.h"

#ifndef PURE
#define PURE =0
#endif

#define AUTO_FRAME -1

struct JAniInfo
{
	int numframes,frameinterval;
	int pivotx,pivoty;
};

struct JAnimation
{
	bool Initialize(JDirectDraw* jdd) PURE;

	bool CreateAnimation(char* name,JAniInfo* info) PURE;
	bool LoadFrame(char* name,int frameid=AUTO_FRAME) PURE;
	bool UnloadAnimation(char* name) PURE;

	bool GetAniInfo(char* name,JAniInfo* buffer) PURE;
	bool SetAniInfo(char* name,JAniInfo* info) PURE;
};