#include "stdafx.h"
#include "PGLVM.h"
#include "PGLLibrary.h"
#include "PGLTree.h"

int PGLOSLibrary::_now(PGLVMThread* th)
{
	th->Push(ConvertToPGLType(time(nullptr)));
	th->SetReturnCnt(1);
	return 0;
}

int PGLOSLibrary::_locale(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::String)
	{
		std::locale::global(
			std::locale(static_pointer_cast<PGLStringData>(p)->data));
	}
	else
	{
	}
	th->PushNull();
	th->SetReturnCnt(1);
	return 0;
}

void PGLOSLibrary::Include(ResolveData* rd)
{
	auto mdl = make_shared<PGLDictionaryData>();
#define LIBINSERT(func) mdl->data.insert(make_pair(ConvertToPGLType(string("@"#func)), ConvertToPGLType<void*>(_##func)))
	
	LIBINSERT(now);
	LIBINSERT(locale);
	
	rd->RegisterGlobal("os", mdl);
}