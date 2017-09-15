#include "stdafx.h"
#include "PGLVM.h"
#include "PGLLibrary.h"
#include "PGLTree.h"
#include "Unicoding.h"

int PGLTimeLibrary::_seconds(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Integer)
	{
		th->Push(ConvertToPGLType((time_t)static_pointer_cast<PGLIntData>(p)->data));
		th->SetReturnCnt(1);
	}
	else
	{
	}
	return 0;
}

int PGLTimeLibrary::_minutes(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Integer)
	{
		th->Push(ConvertToPGLType((time_t)static_pointer_cast<PGLIntData>(p)->data * 60));
		th->SetReturnCnt(1);
	}
	else
	{
	}
	return 0;
}

int PGLTimeLibrary::_hours(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Integer)
	{
		th->Push(ConvertToPGLType((time_t)static_pointer_cast<PGLIntData>(p)->data * 60 * 60));
		th->SetReturnCnt(1);
	}
	else
	{
	}
	return 0;
}

int PGLTimeLibrary::_days(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Integer)
	{
		th->Push(ConvertToPGLType((time_t)static_pointer_cast<PGLIntData>(p)->data * 60 * 60 * 24));
		th->SetReturnCnt(1);
	}
	else
	{
	}
	return 0;
}

int PGLTimeLibrary::second(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	th->Push(ConvertToPGLType((float)t));
	th->SetReturnCnt(1);
	return 0;
}

int PGLTimeLibrary::minute(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	th->Push(ConvertToPGLType((float)t / 60));
	th->SetReturnCnt(1);
	return 0;
}

int PGLTimeLibrary::hour(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	th->Push(ConvertToPGLType((float)t / 60 / 60));
	th->SetReturnCnt(1);
	return 0;
}

int PGLTimeLibrary::day(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	th->Push(ConvertToPGLType((float)t / 60 / 60 / 24));
	th->SetReturnCnt(1);
	return 0;
}

shared_ptr<PGLDictionaryData> tmToPGL(tm p)
{
	auto d = make_shared<PGLDictionaryData>();
	d->data.insert(make_pair(ConvertToPGLType(string("second")), ConvertToPGLType(p.tm_sec)));
	d->data.insert(make_pair(ConvertToPGLType(string("minute")), ConvertToPGLType(p.tm_min)));
	d->data.insert(make_pair(ConvertToPGLType(string("hour")), ConvertToPGLType(p.tm_hour)));
	d->data.insert(make_pair(ConvertToPGLType(string("mday")), ConvertToPGLType(p.tm_mday)));
	d->data.insert(make_pair(ConvertToPGLType(string("month")), ConvertToPGLType(p.tm_mon)));
	d->data.insert(make_pair(ConvertToPGLType(string("year")), ConvertToPGLType(p.tm_year)));
	d->data.insert(make_pair(ConvertToPGLType(string("wday")), ConvertToPGLType(p.tm_wday)));
	d->data.insert(make_pair(ConvertToPGLType(string("yday")), ConvertToPGLType(p.tm_yday)));
	d->data.insert(make_pair(ConvertToPGLType(string("isdst")), ConvertToPGLType(!!p.tm_isdst)));
	wchar_t buf[32];
	wcsftime(buf, 32, L"%z", &p);
	d->data.insert(make_pair(ConvertToPGLType(string("zone")), ConvertToPGLType(utf16_to_utf8(buf))));
	return d;
}

int PGLTimeLibrary::date(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	tm p;
	localtime_s(&p, &t);
	th->Push(tmToPGL(p));
	th->SetReturnCnt(1);
	return 0;
}

int PGLTimeLibrary::gmdate(PGLVMThread* th)
{
	time_t t = static_pointer_cast<PGLTimeData>(th->GetParameter(0))->data;
	tm p;
	gmtime_s(&p, &t);
	th->Push(tmToPGL(p));
	th->SetReturnCnt(1);
	return 0;
}

pair<int, bool> ExtractInt(const shared_ptr<PGLCompoundData>& d)
{
	switch(d->GetType())
	{
	case PGLCompoundData::Integer:
		return make_pair(static_pointer_cast<PGLIntData>(d)->data, true);
	case PGLCompoundData::Real:
		return make_pair((int)static_pointer_cast<PGLRealData>(d)->data, true);
	default:
		return make_pair(0, false);
	}
}

int PGLTimeLibrary::_time(PGLVMThread* th)
{
	auto p = th->GetParameter(1);
	if(p->GetType() == PGLCompoundData::Dictionary)
	{
		const auto& d = static_pointer_cast<PGLDictionaryData>(p)->data;
		auto sec = d.find(ConvertToPGLType(string("second")));
		auto min = d.find(ConvertToPGLType(string("minute")));
		auto hour = d.find(ConvertToPGLType(string("hour")));
		auto day = d.find(ConvertToPGLType(string("day")));
		auto mday = d.find(ConvertToPGLType(string("mday")));
		auto mon = d.find(ConvertToPGLType(string("month")));
		auto year = d.find(ConvertToPGLType(string("year")));
		auto isdst = d.find(ConvertToPGLType(string("isdst")));
		auto zone = d.find(ConvertToPGLType(string("zone")));

		// 아래 키들이 있는 경우는 시간 지점
		if(mday != d.end() || mon != d.end() || year != d.end())
		{
			tm t;
			t.tm_sec = sec != d.end() ? ExtractInt(sec->second).first : 0;
			t.tm_min = min != d.end() ? ExtractInt(min->second).first : 0;
			t.tm_hour = hour != d.end() ? ExtractInt(hour->second).first : 0;
			t.tm_mday = mday != d.end() ? ExtractInt(mday->second).first : 0;
			t.tm_mon = mon != d.end() ? ExtractInt(mon->second).first : 0;
			t.tm_year = year != d.end() ? ExtractInt(year->second).first : 0;
			t.tm_isdst = isdst != d.end() ? ExtractInt(isdst->second).first : 0;
			th->Push(ConvertToPGLType(mktime(&t)));
		}
		// 아닌 경우는 시간 길이
		else
		{
			int tsec = sec != d.end() ? ExtractInt(sec->second).first : 0;
			int tmin = min != d.end() ? ExtractInt(min->second).first : 0;
			int thour = hour != d.end() ? ExtractInt(hour->second).first : 0;
			int tday = day != d.end() ? ExtractInt(day->second).first : 0;
			time_t t = tsec + 60 * (tmin + 60 * (thour + 24 * tday));
			th->Push(ConvertToPGLType(t));
		}
		th->SetReturnCnt(1);
	}
	else
	{
	}
	return 0;
}

void PGLTimeLibrary::Include(ResolveData* rd)
{
	auto mdl = make_shared<PGLDictionaryData>();
#define LIBINSERT(func) mdl->data.insert(make_pair(ConvertToPGLType(string("@"#func)), ConvertToPGLType<void*>(_##func)))
	
	LIBINSERT(seconds);
	LIBINSERT(minutes);
	LIBINSERT(hours);
	LIBINSERT(days);
	LIBINSERT(time);
	rd->RegisterGlobal("time", mdl);
}