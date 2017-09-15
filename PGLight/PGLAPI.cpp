#include "stdafx.h"
#include "PGLAPI.h"
#include "PGLParser.h"
#include "PGLLibrary.h"
#include "PGLVM.h"

#include <Windows.h>

struct PGLC
{
	PGLVM* vm;
	ResolveData rd;
	GenerateData gd;
	ErrorCnt err;
	vector<PGL_whole> ws;
	map<string, HMODULE> dll;
};

PGLM PGL_Init()
{
	
	PGLC* c = new PGLC;
	c->vm = nullptr;
	c->gd.rd = &c->rd;
	c->rd.scope.push_back(nullptr);
	c->gd.cur.push_back(nullptr);
	c->rd.err = &c->err;
	return c;
}

int PGL_Load(PGLM m, const char* sourceCode, int len)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	auto output = Parse(string(sourceCode, len), c.err);
	if(c.err.GetErrCnt())
	{
		return PGL_ERROR;
	}

	for(auto i : output.imports)
	{
		PGL_LoadLibrary(m, i.name.c_str());
	}

	output.CollectID(&c.rd);
	c.ws.push_back(output);
	return PGL_OK;
}

int PGL_LoadStdLibrary(PGLM m)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	PGLBasicLibrary::Include(&c.rd);
	PGLMathLibrary::Include(&c.rd);
	PGLOSLibrary::Include(&c.rd);
	PGLTimeLibrary::Include(&c.rd);
	return PGL_OK;
}

int PGL_LoadLibrary(PGLM m, const char* name)
{
	if(!m) return PGL_ERROR;
	if(!name) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	auto it = c.dll.find(name);
	if(it == c.dll.end())
	{
		string buf = name;
		buf += ".dll";
		HMODULE hMod = LoadLibraryA(buf.c_str());
		if(!hMod)
		{
			c.err.Push(Error(ErrorType::cannot_find_library, 0, name));
			return PGL_ERROR;
		}
		PGLLibraryInit mi = (PGLLibraryInit)GetProcAddress(hMod, "PGL_LibraryInit");
		if(mi(m) != PGL_OK)
		{
			c.err.Push(Error(ErrorType::wrong_library, 0, name));
			FreeLibrary(hMod);
			return PGL_ERROR;
		}
		c.dll[name] = hMod;
	}
	return PGL_OK;
}

int PGL_RegisterGlobal(PGLM m, const char* name, PGLData data)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	c.rd.RegisterGlobal(name, *d);
	return PGL_OK;
}

int PGL_RegisterGlobalNull(PGLM m, const char* name)
{
	auto d = make_shared<PGLVoidData>();
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalInteger(PGLM m, const char* name, int data)
{
	auto d = ConvertToPGLType(data);
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalReal(PGLM m, const char* name, float data)
{
	auto d = ConvertToPGLType(data);
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalString(PGLM m, const char* name, const char* data)
{
	auto d = ConvertToPGLType(string(data));
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalBoolean(PGLM m, const char* name, int data)
{
	auto d = ConvertToPGLType(!!data);
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalCFunction(PGLM m, const char* name, PGLCFunction data)
{
	auto d = ConvertToPGLType<void*>(data);
	return PGL_RegisterGlobal(m, name, &d);
}

int PGL_RegisterGlobalUserData(PGLM m, const char* name, void* data)
{
	auto d = ConvertToPGLType<void*>(data);
	return PGL_RegisterGlobal(m, name, &d);
}

PGLData PGL_GetGlobalValue(PGLM m, const char* name)
{
	if(!m) return nullptr;
	PGLC& c = *(PGLC*)m;
	if(!name) return nullptr;
	if(!c.vm) return nullptr;
	auto data = c.vm->GetGlobalValue(name);
	if(!data) return nullptr;
	return new shared_ptr<PGLCompoundData>(data);
}

int PGL_SetGlobalValue(PGLM m, const char* name, PGLData data)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	if(!name) return PGL_ERROR;
	if(!data) return PGL_ERROR;
	if(!c.vm) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	return c.vm->SetGlobalValue(name, *d);
}

int PGL_GetErrorMsg(PGLM m, char* buffer, int bufferSize)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	if(!buffer) return c.err.GetErrMsg().size() + 1;
	strcpy_s(buffer, bufferSize, c.err.GetErrMsg().c_str());
	return PGL_OK;
}

int PGL_Run(PGLM m)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	for(auto& e : c.ws)
	{
		e.ResolveIdentifier(&c.rd);
		if(c.err.GetErrCnt()) return PGL_ERROR;
		e.GenerateCode(&c.gd);
		if(c.err.GetErrCnt()) return PGL_ERROR;
	}
	c.vm = new PGLVM(PGLLink(&c.gd));
	return PGL_OK;
}

int PGL_Close(PGLM m)
{
	if(!m) return PGL_ERROR;
	PGLC* c = (PGLC*)m;
	if(c->vm) delete c->vm;
	delete c;
	return PGL_OK;
}

int PGL_GetDisassembly(PGLM m, const char* funcName, char* buffer, int bufferSize)
{
	if(!m) return PGL_ERROR;
	PGLC& c = *(PGLC*)m;
	if(!c.vm) return PGL_ERROR;
	if(!buffer) return c.vm->GetDisassembly(funcName).size() + 1;
	string asmb = c.vm->GetDisassembly(funcName);
	strncpy_s(buffer, bufferSize, asmb.c_str(), min((int)asmb.size(), bufferSize - 1));
	return PGL_OK;
}

PGLThread PGL_NewThread(PGLM m)
{
	if(!m) return nullptr;
	PGLC& c = *(PGLC*)m;
	if(!c.vm) return nullptr;
	auto th = c.vm->NewThread();
	th->SetUserData(m);
	return th;
}

PGLData PGLD_CreateNull()
{
	return new shared_ptr<PGLCompoundData>(new PGLVoidData);
}

PGLData PGLD_CreateInteger(int n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType(n));
}

PGLData PGLD_CreateReal(float n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType(n));
}

PGLData PGLD_CreateString(const char* n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType(string(n)));
}

PGLData PGLD_CreateBoolean(int n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType(!!n));
}

PGLData PGLD_CreateCFunction(PGLCFunction n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType<void*>(n));
}

PGLData PGLD_CreateUserData(void* n)
{
	return new shared_ptr<PGLCompoundData>(ConvertToPGLType(n));
}

PGLData PGLD_CreateArray()
{
	return new shared_ptr<PGLCompoundData>(new PGLArrayData);
}

PGLData PGLD_CreateDictionary()
{
	return new shared_ptr<PGLCompoundData>(new PGLDictionaryData);
}

int PGLD_Release(PGLData data)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	delete d;
	return PGL_OK;
}

int PGLD_Type(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	return (*d)->GetType();
}

int PGLD_GetInteger(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Integer:
		return static_pointer_cast<PGLIntData>(*d)->data;
	case PGLCompoundData::Real:
		return static_pointer_cast<PGLRealData>(*d)->data;
	default:
		return 0;
	}
}

float PGLD_GetReal(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Integer:
		return static_pointer_cast<PGLIntData>(*d)->data;
	case PGLCompoundData::Real:
		return static_pointer_cast<PGLRealData>(*d)->data;
	default:
		return 0;
	}
}

const char* PGLD_GetString(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::String:
		return static_pointer_cast<PGLStringData>(*d)->data.c_str();
	default:
		return 0;
	}
}

int PGLD_GetBoolean(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Boolean:
		return static_pointer_cast<PGLBooleanData>(*d)->data;
	default:
		return 0;
	}
}

PGLCFunction PGLD_GetCFunction(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CFunction:
		return (PGLCFunction)static_pointer_cast<PGLCFunctionData>(*d)->data;
	default:
		return 0;
	}
}

void* PGLD_GetUserData(PGLData data)
{
	if(!data) return 0;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CFunction:
		return (PGLCFunction)static_pointer_cast<PGLCFunctionData>(*d)->data;
	default:
		return 0;
	}
}

int PGLD_SetArrayValue(PGLData data, int idx, PGLData newData)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Array:
		if(idx >= static_pointer_cast<PGLArrayData>(*d)->data.size())
		{
			return PGL_ERROR;
		}
		static_pointer_cast<PGLArrayData>(*d)->data[idx] = *(shared_ptr<PGLCompoundData>*)newData;
		return PGL_OK;
	default:
		return PGL_ERROR;
	}
}

int PGLD_GetArrayLength(PGLData data)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Array:
		return static_pointer_cast<PGLArrayData>(*d)->data.size();
	default:
		return PGL_ERROR;
	}
}

int PGLD_ResizeArray(PGLData data, int length)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Array:
		static_pointer_cast<PGLArrayData>(*d)->data.resize(length);
		return PGL_OK;
	default:
		return PGL_ERROR;
	}
}

PGLData PGLD_GetArrayValue(PGLData data, int idx)
{
	if(!data) return nullptr;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	switch((*d)->GetType())
	{
	case PGLCompoundData::Array:
		if(idx >= static_pointer_cast<PGLArrayData>(*d)->data.size())
		{
			return nullptr;
		}
		return new shared_ptr<PGLCompoundData>(static_pointer_cast<PGLArrayData>(*d)->data[idx]);
	default:
		return nullptr;
	}
}

int PGLD_SetDictionaryValue(PGLData data, PGLData key, PGLData newValue)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	if((*d)->GetType() == PGLCompoundData::Dictionary)
	{
		auto& dict = static_pointer_cast<PGLDictionaryData>(*d);
		dict->data[*(shared_ptr<PGLFirstData>*)key] = *(shared_ptr<PGLCompoundData>*)newValue;
		return PGL_OK;
	}
	else
	{
		return PGL_ERROR;
	}
}

int PGLD_SetDictionaryValueByStr(PGLData data, const char* key, PGLData newValue)
{
	auto key2 = ConvertToPGLType(string(key));
	return PGLD_SetDictionaryValue(data, &key2, newValue);
}

int PGLD_HasDictionaryKey(PGLData data, PGLData key)
{
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	if((*d)->GetType() == PGLCompoundData::Dictionary)
	{
		auto& dict = static_pointer_cast<PGLDictionaryData>(*d);
		if(dict->data.find(*(shared_ptr<PGLFirstData>*)key) == dict->data.end())
		{
			return 0;
		}
		else return 1;
	}
	else
	{
		return PGL_ERROR;
	}
}

int PGLD_HasDictionaryKeyByStr(PGLData data, const char* key)
{
	auto key2 = ConvertToPGLType(string(key));
	return PGLD_HasDictionaryKey(data, &key2);
}

PGLData PGLD_GetDictionaryValue(PGLData data, PGLData key)
{
	if(!data) return nullptr;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	if((*d)->GetType() == PGLCompoundData::Dictionary)
	{
		auto& dict = static_pointer_cast<PGLDictionaryData>(*d);
		auto it = dict->data.find(*(shared_ptr<PGLFirstData>*)key);
		if(it == dict->data.end())
		{
			return nullptr;
		}
		else return new shared_ptr<PGLCompoundData>(it->second);
	}
	else
	{
		return nullptr;
	}
}

PGLData PGLD_GetDictionaryValueByStr(PGLData data, const char* key)
{
	auto key2 = ConvertToPGLType(string(key));
	return PGLD_GetDictionaryValue(data, &key2);
}

PGLData PGLD_IterationBegin(PGLData data)
{
	if(!data) return nullptr;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	if((*d)->GetType() == PGLCompoundData::Dictionary)
	{
		auto& dict = static_pointer_cast<PGLDictionaryData>(*d);
		return new shared_ptr<PGLCMapIterData>(make_shared<PGLCMapIterData>(*dict));
	}
	else if((*d)->GetType() == PGLCompoundData::Array)
	{
		auto& arr = static_pointer_cast<PGLArrayData>(*d);
		return new shared_ptr<PGLCVectorIterData>(make_shared<PGLCVectorIterData>(*arr));
	}
	else
	{
		return nullptr;
	}
}

int PGLD_IterationNext(PGLData iter)
{
	if(!iter) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)iter;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		(*d)->Next();
		return PGL_OK;
	default:
		return PGL_ERROR;
	}
}

int PGLD_IterationNotEnd(PGLData iter)
{
	if(!iter) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)iter;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		return (*d)->IsNotEnd();
	default:
		return PGL_ERROR;
	}
}

PGLData PGLD_GetIterationKey(PGLData iter)
{
	if(!iter) return nullptr;
	auto* d = (shared_ptr<PGLCompoundData>*)iter;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		return new shared_ptr<PGLCompoundData>((*d)->GetKey());
	default:
		return nullptr;
	}
}

PGLData PGLD_GetIterationValue(PGLData iter)
{
	if(!iter) return nullptr;
	auto* d = (shared_ptr<PGLCompoundData>*)iter;
	switch((*d)->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		return new shared_ptr<PGLCompoundData>((*d)->GetValue());
	default:
		return nullptr;
	}
}

PGLM PGLT_GetM(PGLThread thread)
{
	if(!thread) return nullptr;
	auto th = (PGLVMThread*)thread;
	return th->GetUserData();
}

int PGLT_Push(PGLThread thread, PGLData data)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	if(!data) return PGL_ERROR;
	auto* d = (shared_ptr<PGLCompoundData>*)data;
	th->Push(*d);
	return PGL_OK;
}

int PGLT_PushNull(PGLThread thread)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	th->PushNull();
	return PGL_OK;
}

int PGLT_PushInteger(PGLThread thread, int data)
{
	auto d = ConvertToPGLType(data);
	return PGLT_Push(thread, &d);
}

int PGLT_PushReal(PGLThread thread, float data)
{
	auto d = ConvertToPGLType(data);
	return PGLT_Push(thread, &d);
}

int PGLT_PushString(PGLThread thread, const char* data)
{
	auto d = ConvertToPGLType(string(data));
	return PGLT_Push(thread, &d);
}

int PGLT_PushBoolean(PGLThread thread, int data)
{
	auto d = ConvertToPGLType(!!data);
	return PGLT_Push(thread, &d);
}

int PGLT_PushCFunction(PGLThread thread, PGLCFunction data)
{
	auto d = ConvertToPGLType<void*>(data);
	return PGLT_Push(thread, &d);
}

int PGLT_PushUserData(PGLThread thread, void* data)
{
	auto d = ConvertToPGLType(data);
	return PGLT_Push(thread, &d);
}

int PGLT_Entry(PGLThread thread, int paramCnt)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	return th->Entry(paramCnt);
}

int PGLT_Resume(PGLThread thread)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	return th->Resume();
}

int PGLT_GetErrorMsg(PGLThread thread, char* buffer, int bufferSize)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	if(!buffer) return th->GetErrMsg().size() + 1;
	strcpy_s(buffer, bufferSize, th->GetErrMsg().c_str());
	return PGL_OK;
}

PGLData PGLT_GetParam(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return new shared_ptr<PGLCompoundData>(th->GetParameter(paramIdx));
}

int PGLT_GetParamByInteger(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetInteger(&th->GetParameter(paramIdx));
}

float PGLT_GetParamByReal(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetReal(&th->GetParameter(paramIdx));
}

const char* PGLT_GetParamByString(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetString(&th->GetParameter(paramIdx));
}

int PGLT_GetParamByBoolean(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetBoolean(&th->GetParameter(paramIdx));
}

PGLCFunction PGLT_GetParamByCFunction(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetCFunction(&th->GetParameter(paramIdx));
}

void* PGLT_GetParamByUserData(PGLThread thread, int paramIdx)
{
	if(!thread) return 0;
	auto th = (PGLVMThread*)thread;
	return PGLD_GetUserData(&th->GetParameter(paramIdx));
}

int PGLT_GetParamCnt(PGLThread thread)
{
	if(!thread) return -1;
	auto th = (PGLVMThread*)thread;
	return th->GetParameterCnt();
}

int PGLT_Throw(PGLThread thread)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	return th->Throw();
}

int PGLT_MakeArray(PGLThread thread, int n)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	return th->MakeArray(n);
}

int PGLT_MakeDictionary(PGLThread thread, int n)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	return th->MakeDictionary(n);
}

int PGLT_SetYieldCF(PGLThread thread, PGLCFunction func)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	auto d = ConvertToPGLType<void*>(func);
	th->SetYieldCF(d);
	return PGL_OK;
}

int PGLT_SetReturnCnt(PGLThread thread, int retCnt)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	th->SetReturnCnt(retCnt);
	return PGL_OK;
}

int PGLT_Pop(PGLThread thread, int n)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	th->Pop(n);
	return PGL_OK;
}

int PGLT_Close(PGLThread thread)
{
	if(!thread) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	delete th;
	return PGL_OK;
}

int PGLT_Receive(PGLThread thread, PGLThread from, int n)
{
	if(!thread) return PGL_ERROR;
	if(!from) return PGL_ERROR;
	auto th = (PGLVMThread*)thread;
	auto fr = (PGLVMThread*)from;
	th->Receive(fr, n);
	return PGL_OK;
}