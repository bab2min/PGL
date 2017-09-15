#include "stdafx.h"
#include "PGLVM.h"
#include "PGLLibrary.h"
#include "PGLTree.h"
#include "Unicoding.h"

int PGLBasicLibrary::_print(PGLVMThread* th)
{
	for(int i = 0; i < th->GetParameterCnt(); ++i)
	{
		auto p = th->GetParameter(i);
		if(p->GetType() == PGLCompoundData::String)
		{
			wprintf(L"%s", utf8_to_utf16(((PGLStringData*)p.get())->data.c_str()).c_str());
		}
		else if(p->GetType() == PGLCompoundData::Integer)
		{
			printf("%d", ((PGLIntData*)p.get())->data);
		}
		else if(p->GetType() == PGLCompoundData::Real)
		{
			printf("%f", ((PGLRealData*)p.get())->data);
		}
		else if(p->GetType() == PGLCompoundData::None)
		{
			printf("null");
		}
		else if(p->GetType() == PGLCompoundData::Boolean)
		{
			printf(static_pointer_cast<PGLBooleanData>(p)->data ? "true" : "false");
		}
		else
		{
			printf("unknown");
		}
	}

	th->PushNull();
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::_copy(PGLVMThread* th)
{
	auto p = th->GetParameter(0);
	th->Push(p->Copy());
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::_len(PGLVMThread* th)
{
	auto p = th->GetParameter(0);
	switch(p->GetType())
	{
	case PGLCompoundData::Array:
		th->Push(ConvertToPGLType(static_pointer_cast<PGLArrayData>(p)->data.size()));
		break;
	case PGLCompoundData::Dictionary:
		th->Push(ConvertToPGLType(static_pointer_cast<PGLDictionaryData>(p)->data.size()));
		break;
	case PGLCompoundData::String:
		th->Push(ConvertToPGLType(static_pointer_cast<PGLStringData>(p)->data.size()));
		break;
	default:
		return PGL_RUNERR;
	}
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::_type(PGLVMThread* th)
{
	auto p = th->GetParameter(0);
	th->Push(ConvertToPGLType((int)p->GetType()));
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::_thread(PGLVMThread* th)
{
	auto newth = th->GetVM()->NewThread();
	newth->Push(th->GetParameter(0));
	newth->Entry(0);
	th->Push(ConvertToPGLType(shared_ptr<PGLVMThread>(newth)));
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::_int(PGLVMThread* th)
{
	auto p = th->GetParameter(0);
	switch(p->GetType())
	{
	case PGLCompoundData::Integer:
		th->Push(p);
		break;
	case PGLCompoundData::Real:
		th->Push(ConvertToPGLType((int)static_pointer_cast<PGLRealData>(p)->data));
		break;
	case PGLCompoundData::String:
		th->Push(ConvertToPGLType(atoi(static_pointer_cast<PGLStringData>(p)->data.c_str())));
		break;
	case PGLCompoundData::Time:
		th->Push(ConvertToPGLType((int)static_pointer_cast<PGLTimeData>(p)->data));
		break;
	default:
		return PGL_RUNERR;
	}
	th->SetReturnCnt(1);
	return 0;
}

int PGLBasicLibrary::resume(PGLVMThread* th)
{
	auto thrd = static_pointer_cast<PGLThreadData>(th->GetParameter(0))->data;
	if(thrd->GetState() != PGL_ENTRY && thrd->GetState() != PGL_YIELD)
	{
		th->Push(ConvertToPGLType(false));
		th->SetReturnCnt(1);
		return 0;
	}

	if(thrd->Resume() == PGL_YIELD)
	{
		th->Receive(&*thrd, thrd->GetReturnCnt());
		th->SetReturnCnt(thrd->GetReturnCnt());
	}
	return 0;
}

int PGLBasicLibrary::status(PGLVMThread* th)
{
	auto thrd = static_pointer_cast<PGLThreadData>(th->GetParameter(0))->data;
	string state;
	switch(thrd->GetState())
	{
	case PGL_OK:
		state = "OK";
		break;
	case PGL_YIELD:
		state = "Yield";
		break;
	case PGL_RUNERR:
		state = "Runtime Error";
		break;
	case PGL_ENTRY:
		state = "Prepare";
		break;
	default:
		state = "Unknown";
		break;
	}
	th->Push(ConvertToPGLType(state));
	th->SetReturnCnt(1);
	return 0;
}

void PGLBasicLibrary::Include(ResolveData* rd)
{
	rd->RegisterGlobal("print", ConvertToPGLType<void*>(_print));
	rd->RegisterGlobal("copy", ConvertToPGLType<void*>(_copy));
	rd->RegisterGlobal("len", ConvertToPGLType<void*>(_len));
	rd->RegisterGlobal("type", ConvertToPGLType<void*>(_type));
	rd->RegisterGlobal("thread", ConvertToPGLType<void*>(_thread));
	rd->RegisterGlobal("int", ConvertToPGLType<void*>(_int));
	rd->RegisterGlobal("TYPE_INTEGER", ConvertToPGLType((int)PGLCompoundData::Integer));
	rd->RegisterGlobal("TYPE_REAL", ConvertToPGLType((int)PGLCompoundData::Real));
	rd->RegisterGlobal("TYPE_BOOLEAN", ConvertToPGLType((int)PGLCompoundData::Boolean));
	rd->RegisterGlobal("TYPE_STRING", ConvertToPGLType((int)PGLCompoundData::String));
	rd->RegisterGlobal("TYPE_NONE", ConvertToPGLType((int)PGLCompoundData::None));
	rd->RegisterGlobal("TYPE_ARRAY", ConvertToPGLType((int)PGLCompoundData::Array));
	rd->RegisterGlobal("TYPE_DICTIONARY", ConvertToPGLType((int)PGLCompoundData::Dictionary));
	rd->RegisterGlobal("TYPE_FUNCTION", ConvertToPGLType((int)PGLCompoundData::Function));
	rd->RegisterGlobal("TYPE_CFUNCTION", ConvertToPGLType((int)PGLCompoundData::CFunction));
	rd->RegisterGlobal("TYPE_CLOSURE", ConvertToPGLType((int)PGLCompoundData::Closure));
	rd->RegisterGlobal("TYPE_TIME", ConvertToPGLType((int)PGLCompoundData::Time));
	rd->RegisterGlobal("TYPE_THREAD", ConvertToPGLType((int)PGLCompoundData::Thread));
}
