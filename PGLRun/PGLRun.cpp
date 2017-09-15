// PGLRun.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../PGLight/PGLAPI.h"
#include "../PGLight/Unicoding.h"

#ifdef _DEBUG
#pragma comment(lib, "../PGLight/Debug/PGLight.lib")
#elif defined(_DYNAMIC)
#pragma comment(lib, "../PGLight/DLLRelease/PGLight.lib")
#else
#pragma comment(lib, "../PGLight/Release/PGLight.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	locale::global(locale(""));
	bool error = false;
	vector<string> sources;
	char buf[2048];
	if(argc <= 1)
	{
		printf("Input source code: ");
		gets_s(buf, 2048);
		sources.push_back(buf);
	}
	else
	{
		for(int i = 1; i < argc; ++i)
		{
			sources.push_back(utf16_to_utf8(argv[i]));
		}
	}
	printf("Compiling...\n");

	PGLM m = PGL_Init();
	PGL_LoadStdLibrary(m);

	for(auto source : sources)
	{
		string code;
		FILE* f;
		fopen_s(&f, source.c_str(), "rt");
		if(!f)
		{
			printf("can't find source file '%s'.\n", source.c_str());
			error = true;
			continue;
		}
		char buf[2048];
		while(fgets(buf, 2048, f))
		{
			code += buf;
		}
		fclose(f);

		if(PGL_Load(m, code.c_str(), code.size()) != PGL_OK)
		{
			PGL_GetErrorMsg(m, buf, 2048);
			puts(buf);
			error = true;
		}
	}

	if(error)
	{
		PGL_Close(m);
		return -1;
	}
	printf("Linking...\n");
	if(PGL_Run(m) != PGL_OK)
	{
		PGL_GetErrorMsg(m, buf, 2048);
		puts(buf);
		return -1;
	}
	system("cls");
	PGLThread th = PGL_NewThread(m);
	if(PGLT_Resume(th) != PGL_OK)
	{
		PGL_GetErrorMsg(m, buf, 2048);
		puts(buf);
		return -1;
	}
	PGLT_Close(th);
	PGL_Close(m);
	return 0;
}

