#include "stdafx.h"
#include "PGLVM.h"
#include "PGLLibrary.h"
#include "PGLTree.h"

int PGLMathLibrary::_abs(PGLVMThread* th)
{
	auto p = th->GetParameter(0);
	switch(p->GetType())
	{
	case PGLCompoundData::Integer:
		th->Push(ConvertToPGLType(abs(static_pointer_cast<PGLIntData>(p)->data)));
		break;
	case PGLCompoundData::Real:
		th->Push(ConvertToPGLType(abs(static_pointer_cast<PGLRealData>(p)->data)));
		break;
	}
	
	th->SetReturnCnt(1);
	return 0;
}

pair<float, bool> PGLMathLibrary::ExtractReal(const shared_ptr<PGLCompoundData>& d)
{
	switch(d->GetType())
	{
	case PGLCompoundData::Integer:
		return make_pair((float)static_pointer_cast<PGLIntData>(d)->data, true);
	case PGLCompoundData::Real:
		return make_pair(static_pointer_cast<PGLRealData>(d)->data, true);
	default:
		return make_pair(0.f, false);
	}
}

#define PGLMATHLIB(func) int PGLMathLibrary::_##func(PGLVMThread* th)\
	{\
		auto a = ExtractReal(th->GetParameter(1));\
		if(!a.second) return PGL_RUNERR;\
		th->Push(ConvertToPGLType(func(a.first)));\
		th->SetReturnCnt(1);\
		return 0;\
	}\

#define PGLMATHLIB2(func) int PGLMathLibrary::_##func(PGLVMThread* th)\
	{\
		auto a = ExtractReal(th->GetParameter(1));\
		auto b = ExtractReal(th->GetParameter(2));\
		if(!a.second || !b.second) return PGL_RUNERR;\
		th->Push(ConvertToPGLType(func(a.first, b.first)));\
		th->SetReturnCnt(1);\
		return 0;\
	}\

float deg(float f)
{
	return f / 3.14159265f * 180;
}

float rad(float f)
{
	return f * 3.14159265f / 180;
}

PGLMATHLIB(acos);
PGLMATHLIB(asin);
PGLMATHLIB(atan);
PGLMATHLIB2(atan2);
PGLMATHLIB(ceil);
PGLMATHLIB(cos);
PGLMATHLIB(cosh);
PGLMATHLIB(deg);
PGLMATHLIB(exp);
PGLMATHLIB(floor);
PGLMATHLIB2(fmod);
//PGLMATHLIB(frexp);
PGLMATHLIB2(ldexp);
PGLMATHLIB(log);
//PGLMATHLIB2(modf);
PGLMATHLIB2(pow);
PGLMATHLIB(rad);
PGLMATHLIB(sin);
PGLMATHLIB(sinh);
PGLMATHLIB(sqrt);
PGLMATHLIB(tan);
PGLMATHLIB(tanh);

int PGLMathLibrary::_rand(PGLVMThread* th)
{
	th->Push(ConvertToPGLType(rand()));
	th->SetReturnCnt(1);
	return 0;
}

int PGLMathLibrary::_srand(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Integer)
	{
		srand(static_pointer_cast<PGLIntData>(p)->data);
	}
	else
	{
	}
	th->PushNull();
	th->SetReturnCnt(1);
	return 0;
}

int PGLMathLibrary::_random(PGLVMThread* th)
{
	float f = rand() / (float)RAND_MAX + rand() / powf(RAND_MAX, 2);
	switch(th->GetParameterCnt())
	{
	case 0:
		break;
	case 1:
		{
			auto a = ExtractReal(th->GetParameter(1));
			f *= a.first;
		}
		break;
	case 2:
		{
			auto a = ExtractReal(th->GetParameter(1));
			auto b = ExtractReal(th->GetParameter(2));
			f *= b.first - a.first;
			f += a.first;
		}
		break;
	}
	th->Push(ConvertToPGLType(f));
	th->SetReturnCnt(1);
	return 0;
}

void PGLMathLibrary::Include(ResolveData* rd)
{
	auto mdl = make_shared<PGLDictionaryData>();
#define LIBINSERT(func) mdl->data.insert(make_pair(ConvertToPGLType(string("@"#func)), ConvertToPGLType<void*>(_##func)))
	
	LIBINSERT(abs);
	LIBINSERT(acos);
	LIBINSERT(asin);
	LIBINSERT(atan);
	LIBINSERT(atan2);
	LIBINSERT(ceil);
	LIBINSERT(cos);
	LIBINSERT(cosh);
	LIBINSERT(deg);
	LIBINSERT(exp);
	LIBINSERT(floor);
	LIBINSERT(fmod);
	LIBINSERT(ldexp);
	LIBINSERT(log);
	LIBINSERT(pow);
	LIBINSERT(rad);
	LIBINSERT(sin);
	LIBINSERT(sinh);
	LIBINSERT(sqrt);
	LIBINSERT(tan);
	LIBINSERT(tanh);
	LIBINSERT(rand);
	LIBINSERT(random);
	LIBINSERT(srand);
	
	mdl->data.insert(make_pair(ConvertToPGLType(string("@pi")), ConvertToPGLType(3.14159265f)));
	mdl->data.insert(make_pair(ConvertToPGLType(string("@e")), ConvertToPGLType(2.71828183f)));
	rd->RegisterGlobal("math", mdl);
}