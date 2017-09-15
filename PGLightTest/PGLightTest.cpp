// PGLightTest.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "../PGLight/PGLAPI.h"

#ifdef _DEBUG
#pragma comment(lib, "../PGLight/Debug/PGLight.lib")
#elif defined(_DYNAMIC)
#pragma comment(lib, "../PGLight/DLLRelease/PGLight.lib")
#else
#pragma comment(lib, "../PGLight/Release/PGLight.lib")
#endif

PGLData gdata;

int AddTrigger(PGLThread th)
{
	int n = PGLT_GetParamCnt(th);
	if(n == 0) return PGL_OK;
	gdata = PGLT_GetParam(th, 0);
	PGLT_PushNull(th);
	PGLT_PushCFunction(th, AddTrigger);
	PGLT_SetYieldCF(th, AddTrigger);
	return PGL_YIELD;
}

int _tmain(int argc, _TCHAR* argv[])
{
	locale::global(locale(""));
	string code;
	FILE* f;
	fopen_s(&f, "test.pgl", "rt");
	char buf[8192];
	while(fgets(buf, 2048, f))
	{
		code += buf;
	}
	fclose(f);

	PGLM m = PGL_Init();
	PGL_LoadStdLibrary(m);
	PGLData at = PGLD_CreateCFunction(AddTrigger);
	PGL_RegisterGlobal(m, "AddTrigger", at);
	PGLD_Release(at);
	PGLData dict = PGLD_CreateDictionary();
	PGL_RegisterGlobal(m, "_GLOB_", dict);
	PGLD_Release(dict);
	if(PGL_Load(m, code.c_str(), code.size()))
	{
		PGL_GetErrorMsg(m, buf, 2048);
		puts(buf);
	}

	if(PGL_Run(m))
	{
		PGL_GetErrorMsg(m, buf, 2048);
		puts(buf);
	}

	PGL_GetDisassembly(m, nullptr, buf, 8192);
	puts(buf);

	PGLThread th = PGL_NewThread(m);
	PGLT_Resume(th);
	if(PGLT_Resume(th) == PGL_RUNERR)
	{
		PGLT_GetErrorMsg(th, buf, 1024);
		puts(buf);
	}

	/*PGLData glob = PGL_GetGlobalValue(m, "_GLOB_");
	PGLData dint = PGLD_CreateInteger(100);
	PGLD_SetDictionaryValueByStr(glob, "this", dint);
	PGLD_Release(dint);

	while(1)
	{
		PGLThread nth = PGL_NewThread(m);
		PGLData sfunc = PGLD_CreateString("@func");
		PGLData func = PGLD_GetDictionaryValue(gdata, sfunc);
		PGLD_Release(sfunc);
		PGLT_Push(nth, func);
		PGLT_Push(nth, gdata);
		PGLT_Entry(nth, 1);
		if(PGLT_Resume(nth) == PGL_RUNERR)
		{
			PGLT_GetErrorMsg(nth, buf, 1024);
			puts(buf);
		}
		PGLD_Release(func);
		PGLT_Close(nth);
	}*/

	PGL_Close(m);
	return 0;
}