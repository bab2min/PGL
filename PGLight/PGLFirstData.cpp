#include "stdafx.h"
#include "PGLFirstData.h"
#include "PGLLibrary.h"

template<typename Ty1, typename Ty2> bool compare(Ty1 a, Ty2 b, PGLCompoundData::Cmp cmp)
{
	switch(cmp)
	{
	case PGLCompoundData::EQ:
		return a == b;
	case PGLCompoundData::NEQ:
		return a != b;
	case PGLCompoundData::GT:
		return a > b;
	case PGLCompoundData::GTE:
		return a >= b;
	case PGLCompoundData::LS:
		return a < b;
	case PGLCompoundData::LSE:
		return a <= b;
	}
	return false;
}

shared_ptr<PGLCompoundData> PGLVoidData::OperateCmp(PGLCompoundData::Cmp cmp,
	const shared_ptr<PGLCompoundData>& right) const
{
	switch(cmp)
	{
	case EQ:
		return make_shared<PGLBooleanData>(right->GetType() == None);
	case NEQ:
		return make_shared<PGLBooleanData>(right->GetType() != None);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>
			(data + ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>
			(data + ((PGLRealData*)right.get())->data);
	case String:
		{
			char buf[16];
			_itoa_s(data, buf, 16, 10);
			return make_shared<PGLStringData>
				(buf + ((PGLStringData*)right.get())->data);
		}
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateSub(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>
			(data - ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>
			(data - ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateMul(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>
			(data * ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>
			(data * ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>(data / ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(data / ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateMod(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>(data % ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(fmodf(data, ((PGLRealData*)right.get())->data));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperatePow(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLIntData>((int)pow(data, ((PGLIntData*)right.get())->data));
	case Real:
		return make_shared<PGLRealData>(powf(data, ((PGLRealData*)right.get())->data));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLIntData::OperateSign() const
{
	return make_shared<PGLIntData>(-data);
}

shared_ptr<PGLCompoundData> PGLIntData::OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLBooleanData>(compare(data, ((PGLIntData*)right.get())->data, cmp));
	case Real:
		return make_shared<PGLBooleanData>(compare(data, ((PGLIntData*)right.get())->data, cmp));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(data + ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(data + ((PGLRealData*)right.get())->data);
	case String:
		{
			char buf[16];
			sprintf_s(buf, 16, "%.3f", data);
			return make_shared<PGLStringData>(buf + ((PGLStringData*)right.get())->data);
		}
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateSub(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(data - ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(data - ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateMul(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(data * ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(data * ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(data / ((PGLIntData*)right.get())->data);
	case Real:
		return make_shared<PGLRealData>(data / ((PGLRealData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateMod(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(fmodf(data, ((PGLRealData*)right.get())->data));
	case Real:
		return make_shared<PGLRealData>(fmodf(data, ((PGLRealData*)right.get())->data));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperatePow(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLRealData>(pow(data, ((PGLIntData*)right.get())->data));
	case Real:
		return make_shared<PGLRealData>(powf(data, ((PGLRealData*)right.get())->data));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLRealData::OperateSign() const
{
	return make_shared<PGLRealData>(-data);
}

shared_ptr<PGLCompoundData> PGLRealData::OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLBooleanData>(compare(data, ((PGLIntData*)right.get())->data, cmp));
	case Real:
		return make_shared<PGLBooleanData>(compare(data, ((PGLIntData*)right.get())->data, cmp));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLStringData::OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case String:
		return make_shared<PGLStringData>(data + ((PGLStringData*)right.get())->data);
	case Integer:
		{
			char buf[16];
			_itoa_s(((PGLIntData*)right.get())->data, buf, 16, 10);
			return make_shared<PGLStringData>(data + buf);
		}
	case Real:
		{
			char buf[16];
			sprintf_s(buf, 16, "%.3f", ((PGLRealData*)right.get())->data);
			return make_shared<PGLStringData>(data + buf);
		}
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLStringData::OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case String:
		return make_shared<PGLBooleanData>(compare(data, ((PGLStringData*)right.get())->data, cmp));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLBooleanData::OperateAnd(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Boolean:
		return make_shared<PGLBooleanData>(data && ((PGLBooleanData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLBooleanData::OperateOr(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Boolean:
		return make_shared<PGLBooleanData>(data || ((PGLBooleanData*)right.get())->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLBooleanData::OperateNot() const
{
	return make_shared<PGLBooleanData>(!data);
}

shared_ptr<PGLCompoundData> PGLBooleanData::OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Boolean:
		return make_shared<PGLBooleanData>(compare(data, ((PGLBooleanData*)right.get())->data, cmp));
	default:
		return nullptr;
	}
}

bool PGLVoidData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return false;
}

bool PGLIntData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLIntData*)o)->data;
}

bool PGLRealData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLRealData*)o)->data;
}

bool PGLStringData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLStringData*)o)->data;
}

bool PGLBooleanData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLBooleanData*)o)->data;
}

bool PGLFunctionData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLFunctionData*)o)->data;
}

bool PGLCFunctionData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLCFunctionData*)o)->data;
}

bool PGLArrayData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLArrayData*)o)->data;
}

bool PGLDictionaryData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLDictionaryData*)o)->data;
}

bool PGLClosureData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	if(func->data < ((const PGLClosureData*)o)->func->data) return true;
	if(func->data > ((const PGLClosureData*)o)->func->data) return false;
	return context->Compare(((const PGLClosureData*)o)->context.get());
}

bool PGLDataCompare::operator()(const shared_ptr<PGLCompoundData>& a, const shared_ptr<PGLCompoundData>& b) const
{
	if(!a && !b) return false;
	if(!a) return true;
	if(!b) return false;
	return a->Compare(b.get());
}

shared_ptr<PGLCompoundData> PGLArrayData::Copy() const
{
	auto p = make_shared<PGLArrayData>();
	transform(data.begin(), data.end(), back_inserter(p->data), [](const shared_ptr<PGLCompoundData>& o)
	{
		return o->Copy();
	});
	return p;
}

shared_ptr<PGLCompoundData> PGLDictionaryData::Copy() const
{
	auto p = make_shared<PGLDictionaryData>();
	for(auto o : data)
	{
		p->data[o.first] = o.second->Copy();
	}
	return p;
}

shared_ptr<PGLCompoundData> PGLClosureData::Copy() const
{
	auto p = make_shared<PGLClosureData>();
	p->func = static_pointer_cast<PGLFunctionData>(func->Copy());
	p->context = static_pointer_cast<PGLArrayData>(context->Copy());
	return p;
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Time:
		return make_shared<PGLTimeData>
			(data + static_pointer_cast<PGLTimeData>(right)->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateSub(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Time:
		return make_shared<PGLTimeData>
			(data - static_pointer_cast<PGLTimeData>(right)->data);
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateMul(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLTimeData>
			(data * static_pointer_cast<PGLIntData>(right)->data);
	case Real:
		return make_shared<PGLTimeData>
			((time_t)(data * static_pointer_cast<PGLRealData>(right)->data));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Integer:
		return make_shared<PGLTimeData>
			(data / static_pointer_cast<PGLIntData>(right)->data);
	case Real:
		return make_shared<PGLTimeData>
			((time_t)(data / static_pointer_cast<PGLRealData>(right)->data));
	default:
		return nullptr;
	}
}

bool PGLTimeData::Compare(const PGLCompoundData* o) const
{
	if(GetType() < o->GetType()) return true;
	if(GetType() > o->GetType()) return false;
	return data < ((const PGLTimeData*)o)->data;
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const
{
	switch(right->GetType())
	{
	case Time:
		return ConvertToPGLType(compare(data, static_pointer_cast<PGLTimeData>(right)->data, cmp));
	default:
		return nullptr;
	}
}

shared_ptr<PGLCompoundData> PGLThreadData::OperateDot(const string& right) const
{
	if(right == "@resume")
	{
		return ConvertToPGLType<void*>(PGLBasicLibrary::resume);
	}
	if(right == "@status")
	{
		return ConvertToPGLType<void*>(PGLBasicLibrary::status);
	}
	return nullptr;
}

shared_ptr<PGLCompoundData> PGLTimeData::OperateDot(const string& right) const
{
	if(right == "@second")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::second);
	}
	if(right == "@minute")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::minute);
	}
	if(right == "@hour")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::hour);
	}
	if(right == "@day")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::day);
	}
	if(right == "@date")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::date);
	}
	if(right == "@gmdate")
	{
		return ConvertToPGLType<void*>(PGLTimeLibrary::gmdate);
	}
	return nullptr;
}