#pragma once

using namespace std;

class PGLCompoundData
{
public:
	enum Type
	{
		None,
		Integer,
		Real,
		String,
		Boolean,
		Array,
		Dictionary,
		Function,
		CFunction,
		Closure,
		Ref,
		CVectorIter,
		CMapIter,
		Thread,
		Time,
	};
	enum Cmp
	{
		EQ,
		NEQ,
		GT,
		GTE,
		LS,
		LSE,
	};
	virtual Type GetType() const = 0;
	virtual ~PGLCompoundData() {}

	virtual bool Compare(const PGLCompoundData* o) const = 0;
	virtual string DebugInfo() const = 0;

	virtual shared_ptr<PGLCompoundData> OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateSub(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateMul(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateMod(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperatePow(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateAnd(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateOr(
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateDot(
		const string& right) const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateSign() const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateNot() const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const {return nullptr;}
	virtual void Next() {}
	virtual bool IsNotEnd() const {return false;}
	virtual shared_ptr<PGLCompoundData> GetKey() const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> GetValue() const {return nullptr;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return nullptr;}
};

class PGLVMThread;

class PGLFirstData : public PGLCompoundData
{
public:
	virtual Type GetType() const = 0;
	virtual ~PGLFirstData() {}
};

class PGLRefData : public PGLCompoundData
{
public:
	shared_ptr<PGLCompoundData> data;
	PGLRefData(const shared_ptr<PGLCompoundData>& _d = 0) : data(_d) {}
	Type GetType() const override {return Ref;}
	virtual bool Compare(const PGLCompoundData* o) const {return false;}
	string DebugInfo() const {return "Ref" + data->DebugInfo();}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLRefData>(data->Copy());}
};

class PGLVoidData : public PGLFirstData
{
public:
	Type GetType() const override {return None;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	string DebugInfo() const {return "null";}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLVoidData>();}
};

class PGLIntData : public PGLFirstData
{
public:
	int data;
	PGLIntData(int _d = 0) : data(_d) {}
	Type GetType() const override {return Integer;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateSub(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateMul(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateMod(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperatePow(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateSign() const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	string DebugInfo() const {char buf[32]; sprintf_s(buf, 32, "%d", data); return buf;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLIntData>(data);}
};

class PGLRealData : public PGLFirstData
{
public:
	float data;
	PGLRealData(float _d = 0) : data(_d) {}
	Type GetType() const override {return Real;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateSub(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateMul(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateMod(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperatePow(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateSign() const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	string DebugInfo() const {char buf[32]; sprintf_s(buf, 32, "%f", data); return buf;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLRealData>(data);}
};

class PGLStringData : public PGLFirstData
{
public:
	string data;
	PGLStringData(string _d = string()) : data(_d) {}
	Type GetType() const override {return String;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	string DebugInfo() const {return "\"" + data + "\"";}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLStringData>(data);}
};

class PGLBooleanData : public PGLFirstData
{
public:
	bool data;
	PGLBooleanData(bool _d = false) : data(_d) {}
	Type GetType() const override {return Boolean;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateAnd(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateOr(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateNot() const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	string DebugInfo() const {return data ? "true" : "false";}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLBooleanData>(data);}
};

class PGLFunctionData : public PGLFirstData
{
protected:
public:
	int data;
	bool memberFunc;
	PGLFunctionData(int _d = false) : data(_d), memberFunc(false) {}
	Type GetType() const override {return Function;}
	virtual bool Compare(const PGLCompoundData* o) const;
	string DebugInfo() const {char buf[32]; sprintf_s(buf, 32, "%x", data); return buf;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLFunctionData>(data);}
};

class PGLCFunctionData : public PGLFirstData
{
protected:
public:
	void* data;
	PGLCFunctionData(void* _d = false) : data(_d) {}
	Type GetType() const override {return CFunction;}
	virtual bool Compare(const PGLCompoundData* o) const;
	string DebugInfo() const {char buf[32]; sprintf_s(buf, 32, "%x", data); return buf;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLCFunctionData>(data);}
};

class PGLTimeData : public PGLFirstData
{
public:
	time_t data;
	PGLTimeData(time_t _d = 0) : data(_d) {}
	Type GetType() const override {return Time;}
	virtual bool Compare(const PGLCompoundData* o) const;
	virtual shared_ptr<PGLCompoundData> OperateAdd(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateSub(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateMul(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateDiv(
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateCmp(Cmp cmp,
		const shared_ptr<PGLCompoundData>& right) const;
	virtual shared_ptr<PGLCompoundData> OperateDot(const string& right) const;
	string DebugInfo() const {char buf[64]; ctime_s(buf, 64, &data); return buf;}
	virtual shared_ptr<PGLCompoundData> Copy() const {return make_shared<PGLTimeData>(data);}
};

struct PGLDataCompare
{
	bool operator() (const shared_ptr<PGLCompoundData>& a, const shared_ptr<PGLCompoundData>& b) const;
};

class PGLArrayData : public PGLCompoundData
{
protected:
public:
	vector<shared_ptr<PGLCompoundData>> data;
	Type GetType() const override {return Array;}
	virtual bool Compare(const PGLCompoundData* o) const;
	string DebugInfo() const {return "Array";}
	virtual shared_ptr<PGLCompoundData> Copy() const;
};

class PGLDictionaryData : public PGLCompoundData
{
protected:
public:
	map<shared_ptr<PGLFirstData>,
		shared_ptr<PGLCompoundData>, PGLDataCompare> data;
	Type GetType() const override {return Dictionary;}
	virtual bool Compare(const PGLCompoundData* o) const;
	string DebugInfo() const {return "Dict";}
	virtual shared_ptr<PGLCompoundData> Copy() const;
};

class PGLClosureData : public PGLCompoundData
{
protected:
public:
	shared_ptr<PGLFunctionData> func;
	shared_ptr<PGLArrayData> context;
	Type GetType() const override {return Closure;}
	virtual bool Compare(const PGLCompoundData* o) const;
	string DebugInfo() const {return "Closure";}
	virtual shared_ptr<PGLCompoundData> Copy() const;
};

class PGLCVectorIterData : public PGLCompoundData
{
protected:
public:
	size_t n;
	vector<shared_ptr<PGLCompoundData>>::iterator it;
	vector<shared_ptr<PGLCompoundData>>::iterator end;
	PGLCVectorIterData(PGLArrayData& c) : n(0), it(c.data.begin()), end(c.data.end()) {}
	Type GetType() const override {return CVectorIter;}
	virtual bool Compare(const PGLCompoundData* o) const {return false;}
	string DebugInfo() const {return "CVectorIter";}
	void Next() {++it; ++n;}
	bool IsNotEnd() const {return it != end;}
	shared_ptr<PGLCompoundData> GetKey() const {return make_shared<PGLIntData>(n);}
	shared_ptr<PGLCompoundData> GetValue() const {return *it;}
};

class PGLCMapIterData : public PGLCompoundData
{
protected:
public:
	map<shared_ptr<PGLFirstData>,
		shared_ptr<PGLCompoundData>, PGLDataCompare>::iterator it;
	map<shared_ptr<PGLFirstData>,
		shared_ptr<PGLCompoundData>, PGLDataCompare>::iterator end;
	PGLCMapIterData(PGLDictionaryData& c) : it(c.data.begin()), end(c.data.end()) {}
	Type GetType() const override {return CMapIter;}
	virtual bool Compare(const PGLCompoundData* o) const {return false;}
	string DebugInfo() const {return "CMapIter";}
	void Next() {++it;}
	bool IsNotEnd() const {return it != end;}
	shared_ptr<PGLCompoundData> GetKey() const {return it->first;}
	shared_ptr<PGLCompoundData> GetValue() const {return it->second;}
};

class PGLThreadData : public PGLCompoundData
{
public:
	shared_ptr<PGLVMThread> data;
	PGLThreadData(const shared_ptr<PGLVMThread>& _d = false) : data(_d) {}
	Type GetType() const override {return Thread;}
	virtual shared_ptr<PGLCompoundData> OperateDot(const string& right) const;
	virtual bool Compare(const PGLCompoundData* o) const {return false;}
	string DebugInfo() const {return "Thread";}
};

template<typename _Ty>
struct C_PGLType
{
	typedef void type;
};

template<>
struct C_PGLType<void>
{
	typedef PGLVoidData type;
};

template<>
struct C_PGLType<int>
{
	typedef PGLIntData type;
};

template<>
struct C_PGLType<size_t>
{
	typedef PGLIntData type;
};

template<>
struct C_PGLType<float>
{
	typedef PGLRealData type;
};

template<>
struct C_PGLType<double>
{
	typedef PGLRealData type;
};

template<>
struct C_PGLType<string>
{
	typedef PGLStringData type;
};

template<>
struct C_PGLType<bool>
{
	typedef PGLBooleanData type;
};

template<>
struct C_PGLType<shared_ptr<PGLVMThread>>
{
	typedef PGLThreadData type;
};

template<>
struct C_PGLType<void*>
{
	typedef PGLCFunctionData type;
};

template<>
struct C_PGLType<time_t>
{
	typedef PGLTimeData type;
};



template<typename _Ty>
shared_ptr<typename C_PGLType<_Ty>::type> ConvertToPGLType(const _Ty& cdata)
{
	return make_shared<C_PGLType<_Ty>::type>(cdata);
}