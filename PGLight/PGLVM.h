#pragma once

#include "PGLFirstData.h"
#include "PGLVMError.h"

class PGLVMThread;

enum class PGLOpcode : short
{
	NOP, // 아무 것도 하지 않음
	PUSH, // 리터럴/전역변수 %1을 스택에 넣는다
	STORE, // 스택에서 자료를 빼서 전역변수 %1로 설정한다
	COPY, // 베이스스택 + %1위치에 있는 자료를 스택에 넣는다
	WRITE, // 스택에서 자료 하나를 빼서 베이스스택 + %1위치에 덮어쓴다
	POP, // 스택에서 %1개의 자료를 빼낸다
	CALL, // 스택에서 주소를 꺼내고, %1개의 인자를 사용하여 함수 호출
	THISCALL, // 스택에서 주소를 꺼내고 this와 %1개의 인자를 사용하여 함수 호출
	RETURN, // 반환값을 %1개로 설정하고, 이전 주소로 복귀
	YIELD, // 반환값을 %1개로 설정하고, 현 스레드 실행을 중지하고, 부모 스레드에게 권한을 넘긴다
	ADD, // 스택에서 2개를 뽑아 더한 결과를 스택에 넣는다
	SUB, // 스택에서 2개를 뽑아 뺀 결과를 스택에 넣는다
	MUL, // 스택에서 2개를 뽑아 곱한 결과를 스택에 넣는다
	DIV, // 스택에서 2개를 뽑아 나눈 결과를 스택에 넣는다
	MOD,
	POW, // 스택에서 2개를 뽑아 n승한 결과를 스택에 넣는다
	SIGN, // 스택에서 1개를 뽑아 부호를 반전하여 스택에 넣는다
	EQ,
	NEQ,
	GT,
	GTE,
	LS,
	LSE,
	AND,
	OR,
	NOT,
	JMP, // %1변위로 점프한다
	UNLESSJMP, // 스택에서 1개를 뽑아, 그 값이 거짓이면,  %1변위로 점프한다
	PUSHNULL, // 스택에 null을 넣는다
	ASSEMBLE, // 스택에서 %1개를 뽑아 배열로 만들어 스택에 넣는다
	NEWDICT, // 스택에서 2* %1개를 뽑아, 차례로 키, 값으로 하는 사전을 만들어 스택에 넣는다
	AT, // 스택에서 배열 혹은 사전과 키를 뽑아, 키에 해당하는 값을 스택에 넣는다
	LEN, // 스택에서 배열 혹은 사전을 뽑고 그 크기를 구하여 스택에 넣는다
	ATD, // 스택에서 배열을 뽑고, %1번째 원소를 스택에 넣는다
	MAKECLOSURE, // 스택에서 함수 주소와 배열을 꺼내 클로져로 묶고, 결과를 스택에 넣는다
	SET, // 스택에서 자료와 배열 혹은 사전과 키를 뽑아, 키에 해당하는 값을 자료로 설정한다
	SETD, // 스택에서 자료와 배열을 뽑아, 그 배열의 %1번째 원소로 설정한다
	REF, // 스택에서 자료를 뽑아, 레퍼런스하여 스택에 넣는다
	DEREF, // 스택에서 자료를 뽑아, 디레퍼런스하여 스택에 넣는다
	COPYDEREF, // COPY + DEREF
	SETDDEREF, // 스택에서 자료와 배열을 뽑아, 그 배열의 %1번째 원소의 디레퍼런스로 설정한다
	WRITEDEREF, // 스택에서 자료 하나를 빼서 베이스스택 + %1위치의 디레퍼런스로 설정한다
	PUSHBEGIN, // 스택 꼭대기에 위치한 배열 혹은 사전에서 순회 첫 지점의 정보를 담는 자료를 스택에 넣는다
	ISNOTEND, // 스택 꼭대기에 위치한 순회 지점이 끝에 다다랐으면 거짓, 아니면 참을 스택에 넣는다
	NEXT, // 스택 꼭대기에 위치한 순회 지점을 다음으로 설정한다
	PUSHKEYVAL, // 스택 꼭대기에 위치한 순회 지점에서 키와 값을 뽑아 스택에 넣는다
	FIND, // 스택에서 배열 혹은 사전과 키를 뽑에 키에 해당하는 값을 스택에 넣는다. 값이 없을 경우 런타임 에러
	FINDP, // 스택에서 배열 혹은 사전과 키를 뽑에 키에 해당하는 값을 스택에 넣고 배열 혹은 사전을 다시 넣는다. 값이 없을 경우 런타임 에러.
	THROW, // 스택 꼭대기를 예외 객체로 하여 예외를 던진다
	TRY, // %1변위를 예외 처리루틴으로 등록한다
	TRYEXIT, // 예외 처리루틴을 해제한다
	POPTO, // 베이스스택 기준 %1개의 자료가 남도록 나머지를 빼낸다.
};

#define PGL_ERROR -2
#define PGL_OK 0
#define PGL_ENTRY 1
#define PGL_YIELD 2
#define PGL_RUNERR 3
#define PGL_RUNNING 4
#define PGL_THROW 5

typedef int (*PGL_CFunc)(void*);

struct PGLVMDebugInfo
{
	int line;
};

struct FinalData
{
	vector<shared_ptr<PGLCompoundData>> globLiteral;
	vector<string> globDebugInfo;
	vector<short> code;
	vector<PGLVMDebugInfo> debugInfo;
};

class PGLVM
{
	friend class PGLVMThread;
protected:
	vector<short> m_codes;
	vector<PGLVMDebugInfo> m_debugInfo;
	vector<shared_ptr<PGLCompoundData>> m_literal;
	vector<string> m_globDebugInfo;
public:
	PGLVM(const FinalData& fd);
	~PGLVM();
	PGLVMThread* NewThread();
	string _GetDisassembly(const short* codes, size_t size, size_t rel);
	string GetDisassembly(const char* funcName = nullptr);
	shared_ptr<PGLCompoundData> GetGlobalValue(const char* name);
	int SetGlobalValue(const char* name, const shared_ptr<PGLCompoundData>& data);
};

class PGLVMThread
{
protected:
	int m_pc;
	vector<shared_ptr<PGLCompoundData>> m_stack;
	vector<int> m_stackBase;
	vector<int> m_callstack;
	vector<int> m_catchAddr;
	PGLVM* m_mainVM;
	string m_err;
	size_t m_retCnt;
	shared_ptr<PGLCompoundData> m_yieldCF;
	void* m_userPtr;
	int m_state;

	int CallProc(const shared_ptr<PGLCompoundData>& func, int operand, bool thiscall = false);
public:
	int ProcNOP();
	int ProcPUSH();
	int ProcSTORE();
	int ProcCOPY();
	int ProcWRITE();
	int ProcPOP();
	int ProcCALL();
	int ProcTHISCALL();
	int ProcRETURN();
	int ProcYIELD();
	int ProcADD();
	int ProcSUB();
	int ProcMUL();
	int ProcDIV();
	int ProcMOD();
	int ProcPOW();
	int ProcSIGN();
	int ProcEQ();
	int ProcNEQ();
	int ProcGT();
	int ProcGTE();
	int ProcLS();
	int ProcLSE();
	int ProcAND();
	int ProcOR();
	int ProcNOT();
	int ProcJMP();
	int ProcUNLESSJMP();
	int ProcPUSHNULL();
	int ProcASSEMBLE();
	int ProcNEWDICT();
	int ProcAT();
	int ProcLEN();
	int ProcATD();
	int ProcMAKECLOSURE();
	int ProcSET();
	int ProcSETD();
	int ProcREF();
	int ProcDEREF();
	int ProcCOPYDEREF();
	int ProcSETDDEREF();
	int ProcWRITEDEREF();
	int ProcPUSHBEGIN();
	int ProcISNOTEND();
	int ProcNEXT();
	int ProcPUSHKEYVAL();
	int ProcFIND();
	int ProcFINDP();
	int ProcTHROW();
	int ProcTRY();
	int ProcTRYEXIT();
	int ProcPOPTO();

	PGLVMThread(PGLVM* vm);
	~PGLVMThread() {}
	PGLVM* GetVM() const {return m_mainVM;}
	int Entry(int paramCnt);
	int Resume();
	int Receive(PGLVMThread* from, int cnt);
	int GetState() const {return m_state;}
	void PushError(PGLVMError e, int addr);
	shared_ptr<PGLCompoundData> GetParameter(int i);
	size_t GetParameterCnt() const;
	void SetReturnCnt(size_t i);
	size_t GetReturnCnt() const {return m_retCnt;}
	void PushNull();
	void Push(const shared_ptr<PGLCompoundData>& data);
	void Pop(int n) {for(int i = 0; i < n; ++i) m_stack.pop_back();}
	int Throw() {return ProcTHROW();}
	int MakeArray(int n);
	int MakeDictionary(int n);
	string GetErrMsg() const {return m_err;}
	void SetUserData(void* ptr) {m_userPtr = ptr;}
	void* GetUserData() const {return m_userPtr;}
	void SetYieldCF(const shared_ptr<PGLCompoundData>& f) {m_yieldCF = f;}
};