// PGLight.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PGLParser.h"
#include "PGLVM.h"
#include "PGLLibrary.h"
#include "PGLAPI.h"

int _tmain(int argc, _TCHAR* argv[])
{
	locale::global(locale(""));
	string code;
	FILE* f;
	fopen_s(&f, "test.pgl", "rt");
	char buf[1024];
	while(fgets(buf, 1024, f))
	{
		code += buf;
	}
	fclose(f);

	/*ErrorCnt err;
	auto output = Parse(code.c_str(), err);
	ResolveData rd;
	PGLBasicLibrary::Include(&rd);
	PGLMathLibrary::Include(&rd);
	PGLOSLibrary::Include(&rd);
	PGLTimeLibrary::Include(&rd);

	rd.scope.push_back(nullptr);
	rd.err = &err;

	GenerateData gd;
	gd.rd = &rd;
	gd.cur.push_back(nullptr);

	if(err.GetErrCnt())
	{
		puts(err.GetErrMsg().c_str());
		return -1;
	}

	output.CollectID(&rd);
	output.ResolveIdentifier(&rd);

	if(err.GetErrCnt())
	{
		puts(err.GetErrMsg().c_str());
		return -1;
	}

	output.GenerateCode(&gd);

	if(err.GetErrCnt())
	{
		puts(err.GetErrMsg().c_str());
		return -1;
	}

	PGLVM vm(PGLLink(&gd));
	puts(vm.GetDisassembly().c_str());
	auto thread = vm.NewThread();
	if(thread->Resume() == PGL_RUNERR)
	{
		puts(thread->GetErrMsg().c_str());
	}*/

	PGLM m = PGL_Init();
	PGL_LoadStdLibrary(m);
	if(PGL_Load(m, code.c_str(), code.size()))
	{
		PGL_GetErrorMsg(m, buf, 1024);
		puts(buf);
	}

	if(PGL_Run(m))
	{
		PGL_GetErrorMsg(m, buf, 1024);
		puts(buf);
	}

	PGL_GetDisassembly(m, "test.printer", buf, 1024);
	puts(buf);

	PGLThread th = PGL_NewThread(m);
	if(PGLT_Resume(th) == PGL_RUNERR)
	{
		PGLT_GetErrorMsg(th, buf, 1024);
		puts(buf);
	}
	return 0;
}

