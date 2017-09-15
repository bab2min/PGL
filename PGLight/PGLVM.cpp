#include "stdafx.h"
#include "PGLVM.h"


PGLVM::PGLVM(const FinalData& fd)
{
	m_codes = fd.code;
	m_literal = fd.globLiteral;
	m_globDebugInfo = fd.globDebugInfo;
	m_debugInfo = fd.debugInfo;
}

PGLVM::~PGLVM()
{
}

PGLVMThread* PGLVM::NewThread()
{
	return new PGLVMThread(this);
}

string PGLVM::GetDisassembly(const char* funcName)
{
	if(funcName)
	{
		auto it = find(m_globDebugInfo.begin(), m_globDebugInfo.end(), funcName);
		if(it != m_globDebugInfo.end())
		{
			int s = it - m_globDebugInfo.begin();
			if(m_literal[s]->GetType() == PGLCompoundData::Function)
			{
				int start = static_pointer_cast<PGLFunctionData>(m_literal[s])->data;
				return _GetDisassembly(&m_codes[start], m_codes.size() - start, start);
			}
		}
		return string();
	}
	else
	{
		return _GetDisassembly(&m_codes[0], m_codes.size(), 0);
	}
}

shared_ptr<PGLCompoundData> PGLVM::GetGlobalValue(const char* name)
{
	auto it = find(m_globDebugInfo.begin(), m_globDebugInfo.end(), name);
	if(it != m_globDebugInfo.end())
	{
		int s = it - m_globDebugInfo.begin();
		return m_literal[s];
	}
	else
	{
		return nullptr;
	}
}

int PGLVM::SetGlobalValue(const char* name, const shared_ptr<PGLCompoundData>& data)
{
	auto it = find(m_globDebugInfo.begin(), m_globDebugInfo.end(), name);
	if(it != m_globDebugInfo.end())
	{
		int s = it - m_globDebugInfo.begin();
		m_literal[s] = data;
		return PGL_OK;
	}
	else
	{
		return PGL_ERROR;
	}
}

PGLVMThread::PGLVMThread(PGLVM* vm) : m_mainVM(vm)
{
	m_stackBase.push_back(0);
	m_catchAddr.push_back(0);
	m_pc = 0;
	m_retCnt = 0;
	m_state = PGL_OK;
}

void PGLVMThread::PushError(PGLVMError e, int addr)
{
	char buf[256];
	if(m_mainVM->m_debugInfo.size() > addr)
	{
		sprintf_s(buf, 256, "Runtime Error %04d(line %d) %s\n", e, m_mainVM->m_debugInfo[addr].line, GetErrorMsg(e));
	}
	else
	{
		sprintf_s(buf, 256, "Runtime Error %04d(??) %s\n", e, GetErrorMsg(e));
	}

	m_err += buf;
}

int PGLVMThread::ProcNOP()
{
	return -1;
}

int PGLVMThread::ProcPUSH()
{
	short operand = m_mainVM->m_codes[m_pc++];
	m_stack.push_back(m_mainVM->m_literal[operand]);
	return -1;
}

int PGLVMThread::ProcSTORE()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	m_mainVM->m_literal[operand] = m_stack.back();
	m_stack.pop_back();
	return -1;
}

int PGLVMThread::ProcCOPY()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stackBase.empty())
	{
		PushError(PGLVMError::no_stackbase, m_pc - 2);
		return PGL_RUNERR;
	}
	auto v = m_stack[m_stackBase.back() + operand];
	m_stack.push_back(v);
	return -1;
}

int PGLVMThread::ProcWRITE()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stackBase.empty())
	{
		PushError(PGLVMError::no_stackbase, m_pc - 2);
		return PGL_RUNERR;
	}
	m_stack[m_stackBase.back() + operand] = m_stack.back();
	m_stack.pop_back();
	return -1;
}

int PGLVMThread::ProcPOP()
{
	short operand = m_mainVM->m_codes[m_pc++];
	for(int i = 0; i < operand; ++i) m_stack.pop_back();
	return -1;
}

int PGLVMThread::CallProc(const shared_ptr<PGLCompoundData>& call, int operand, bool thiscall)
{
	shared_ptr<PGLFunctionData> func;
	switch(call->GetType())
	{
	case PGLFirstData::Function:
		func = static_pointer_cast<PGLFunctionData>(call);
		break;
	case PGLFirstData::Closure:
		func = static_pointer_cast<PGLClosureData>(call)->func;
		break;
	case PGLFirstData::CFunction:
		{
			m_retCnt = 0;
			m_yieldCF.reset();
			int ret = ((PGL_CFunc)(((PGLCFunctionData*)call.get())->data))(this);
			if(ret == PGL_OK || !m_yieldCF)
			{
				// 함수가 일반 종료 혹은 재진입없는 yield로 끝난 경우
				m_callstack.pop_back();
				m_catchAddr.pop_back();
				// 스택 정리, return 될 값들만 남기고 모두 지운다
				m_stack.erase(m_stack.begin() + m_stackBase.back() - 1, m_stack.end() - m_retCnt);
				m_stackBase.pop_back();
			}
			else if(ret == PGL_YIELD)
			{
				// 재진입 있는 yield로 끝난 경우
			}

			if(ret != PGL_OK) return ret;
		}
		return -1;
	default:
		PushError(PGLVMError::not_callable, m_pc - 2);
		return PGL_RUNERR;
	}

	if(call->GetType() == PGLFirstData::Closure)
	{
		m_stack.insert(m_stack.end() - operand - (thiscall ? 1 : 0), ((PGLClosureData*)call.get())->context);
	}

	m_pc = func->data;

	// this 인자가 필요없는 경우 제거한다
	if(thiscall && !func->memberFunc)
	{
		m_stack.erase(m_stack.end() - operand - 1);
	}
	return -1;
}

int PGLVMThread::ProcCALL()
{
	short operand = m_mainVM->m_codes[m_pc++];
	auto call = *(m_stack.end() - operand - 1);
	m_stackBase.push_back(m_stack.size() - operand);
	m_callstack.push_back(m_pc);
	m_catchAddr.push_back(0);
	return CallProc(call, operand);
}

int PGLVMThread::ProcTHISCALL()
{
	short operand = m_mainVM->m_codes[m_pc++];
	auto call = *(m_stack.end() - operand - 2);
	m_stackBase.push_back(m_stack.size() - operand - 1);
	m_callstack.push_back(m_pc);
	m_catchAddr.push_back(0);
	return CallProc(call, operand, true);
}

int PGLVMThread::ProcRETURN()
{
	short operand = m_mainVM->m_codes[m_pc++];
	//되돌아갈 곳이 없으면 정상 종료
	if(m_callstack.empty()) return PGL_OK;
	m_pc = m_callstack.back();
	m_callstack.pop_back();
	m_catchAddr.pop_back();
	
	// 스택 정리, return 될 값들만 남기고 모두 지운다
	m_stack.erase(m_stack.begin() + m_stackBase.back() - 1, m_stack.end() - operand);
	m_stackBase.pop_back();
	return -1;
}

int PGLVMThread::ProcYIELD()
{
	short operand = m_mainVM->m_codes[m_pc++];
	m_retCnt = operand;
	return PGL_YIELD;
}

int PGLVMThread::ProcADD()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateAdd(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_add, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcSUB()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateSub(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_sub, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcMUL()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateMul(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_mul, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcDIV()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateDiv(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_div, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcMOD()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateMod(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_mod, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}
int PGLVMThread::ProcPOW()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperatePow(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_pow, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcAND()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateAnd(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_and, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcOR()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateOr(m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_or, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcEQ()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::EQ, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcNEQ()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::NEQ, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcGT()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::GT, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcGTE()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::GTE, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcLS()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::LS, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcLSE()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& a = *(m_stack.end()-2);
	a = a->OperateCmp(PGLCompoundData::LSE, m_stack.back());
	m_stack.pop_back();
	if(!a)
	{
		PushError(PGLVMError::unable_cmp, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcSIGN()
{
	if(m_stack.size() < 1)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	m_stack.back() = m_stack.back()->OperateSign();
	if(!m_stack.back())
	{
		PushError(PGLVMError::unable_sign, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcNOT()
{
	if(m_stack.size() < 1)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	m_stack.back() = m_stack.back()->OperateNot();
	if(!m_stack.back())
	{
		PushError(PGLVMError::unable_not, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcJMP()
{
	short operand = m_mainVM->m_codes[m_pc++];
	m_pc += operand;
	return -1;
}

int PGLVMThread::ProcUNLESSJMP()
{
	short operand = m_mainVM->m_codes[m_pc++];
	auto value = m_stack.back();
	m_stack.pop_back();
	if(value->GetType() == PGLFirstData::Boolean)
	{
		if(static_pointer_cast<PGLBooleanData>(value)->data == false)
		{
			m_pc += operand;
		}
	}
	else
	{
		PushError(PGLVMError::expect_bool, m_pc - 2);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcPUSHNULL()
{
	m_stack.push_back(shared_ptr<PGLFirstData>(new PGLVoidData));
	return -1;
}

int PGLVMThread::ProcASSEMBLE()
{
	short operand = m_mainVM->m_codes[m_pc++];
	MakeArray(operand);
	return -1;
}

int PGLVMThread::ProcNEWDICT()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(MakeDictionary(operand) == PGL_RUNERR) return PGL_RUNERR;
	return -1;
}

int PGLVMThread::ProcAT()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto key = m_stack.back();
	m_stack.pop_back();
	auto cont = m_stack.back();
	m_stack.pop_back();

	switch(key->GetType())
	{
	case PGLCompoundData::Array:
	case PGLCompoundData::Dictionary:
	case PGLCompoundData::Closure:
		PushError(PGLVMError::compound_key, m_pc - 1);
		return PGL_RUNERR;
	}

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		if(key->GetType() != PGLCompoundData::Integer)
		{
			PushError(PGLVMError::compound_key, m_pc - 1);
			return PGL_RUNERR;
		}
		m_stack.push_back(
			((PGLArrayData*)(cont.get()))->data[((PGLIntData*)(key.get()))->data]);
		break;
	case PGLCompoundData::Dictionary:
		{
			auto p = ((PGLDictionaryData*)(cont.get()))->data[static_pointer_cast<PGLFirstData>(key)];
			if(!p) p = shared_ptr<PGLFirstData>(new PGLVoidData);
			m_stack.push_back(p);
		}
		break;
	default:
		if(key->GetType() == PGLCompoundData::String)
		{
			auto p = cont->OperateDot(static_pointer_cast<PGLStringData>(key)->data);
			if(!p)
			{
				PushError(PGLVMError::unable_dereference, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(p);
		}
		else
		{
			PushError(PGLVMError::unable_dereference, m_pc - 1);
			return PGL_RUNERR;
		}
	}
	return -1;
}

int PGLVMThread::ProcLEN()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto cont = m_stack.back();
	m_stack.pop_back();

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		m_stack.push_back(make_shared<PGLIntData>(
			((PGLArrayData*)(cont.get()))->data.size()));
		break;
	case PGLCompoundData::Dictionary:
		m_stack.push_back(make_shared<PGLIntData>(
			((PGLDictionaryData*)(cont.get()))->data.size()));
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcATD()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	auto cont = m_stack.back();
	m_stack.pop_back();

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		m_stack.push_back(
			((PGLArrayData*)(cont.get()))->data[operand]);
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 2);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcMAKECLOSURE()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto caps = m_stack.back();
	m_stack.pop_back();
	auto func = m_stack.back();
	m_stack.pop_back();

	if(caps->GetType() != PGLCompoundData::Array)
	{
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	if(func->GetType() != PGLCompoundData::Function)
	{
		PushError(PGLVMError::not_callable, m_pc - 1);
		return PGL_RUNERR;
	}

	auto cls = make_shared<PGLClosureData>();
	cls->func = static_pointer_cast<PGLFunctionData>(func);
	cls->context = static_pointer_cast<PGLArrayData>(caps);
	m_stack.push_back(cls);
	return -1;
}

int PGLVMThread::ProcSET()
{
	if(m_stack.size() < 3)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto key = m_stack.back();
	m_stack.pop_back();
	auto cont = m_stack.back();
	m_stack.pop_back();
	auto value = m_stack.back();
	m_stack.pop_back();

	switch(key->GetType())
	{
	case PGLCompoundData::Array:
	case PGLCompoundData::Dictionary:
	case PGLCompoundData::Closure:
		PushError(PGLVMError::compound_key, m_pc - 1);
		return PGL_RUNERR;
	}

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		if(key->GetType() != PGLCompoundData::Integer)
		{
			PushError(PGLVMError::compound_key, m_pc - 1);
			return PGL_RUNERR;
		}
		((PGLArrayData*)(cont.get()))->data[((PGLIntData*)(key.get()))->data] = value;
		break;
	case PGLCompoundData::Dictionary:
		{
			((PGLDictionaryData*)(cont.get()))->data[static_pointer_cast<PGLFirstData>(key)] = value;
		}
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcSETD()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	auto cont = m_stack.back();
	m_stack.pop_back();
	auto value = m_stack.back();
	m_stack.pop_back();

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		{
			auto& v = ((PGLArrayData*)(cont.get()))->data[operand];
			v = value;
		}
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 2);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcREF()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto value = m_stack.back();
	m_stack.pop_back();
	m_stack.push_back(make_shared<PGLRefData>(value));
	return -1;
}

int PGLVMThread::ProcDEREF()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto value = m_stack.back();
	m_stack.pop_back();
	
	if(value->GetType() != PGLCompoundData::Ref)
	{
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	m_stack.push_back(static_pointer_cast<PGLRefData>(value)->data);
	return -1;
}

int PGLVMThread::ProcCOPYDEREF()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stackBase.empty())
	{
		PushError(PGLVMError::no_stackbase, m_pc - 2);
		return PGL_RUNERR;
	}
	auto v = m_stack[m_stackBase.back() + operand];
	if(v->GetType() == PGLCompoundData::Ref)
	{
		m_stack.push_back(static_pointer_cast<PGLRefData>(v)->data);
	}
	else
	{
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcSETDDEREF()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	auto cont = m_stack.back();
	m_stack.pop_back();
	auto value = m_stack.back();
	m_stack.pop_back();

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		{
			auto& v = ((PGLArrayData*)(cont.get()))->data[operand];
			if(v->GetType() == PGLCompoundData::Ref)
			{
				static_pointer_cast<PGLRefData>(v)->data = value;
				break;
			}
		}
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 2);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcWRITEDEREF()
{
	short operand = m_mainVM->m_codes[m_pc++];
	if(m_stackBase.empty())
	{
		PushError(PGLVMError::no_stackbase, m_pc - 2);
		return PGL_RUNERR;
	}
	auto value = m_stack.back();
	m_stack.pop_back();
	auto& v = m_stack[m_stackBase.back() + operand];
	if(v->GetType() == PGLCompoundData::Ref)
	{
		static_pointer_cast<PGLRefData>(v)->data = value;
	}
	else
	{
		PushError(PGLVMError::unable_dereference, m_pc - 2);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcPUSHBEGIN()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto value = m_stack.back();
	switch(value->GetType())
	{
	case PGLCompoundData::Array:
		m_stack.push_back(make_shared<PGLCVectorIterData>(*static_pointer_cast<PGLArrayData>(value)));
		break;
	case PGLCompoundData::Dictionary:
		m_stack.push_back(make_shared<PGLCMapIterData>(*static_pointer_cast<PGLDictionaryData>(value)));
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcISNOTEND()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto value = m_stack.back();
	m_stack.pop_back();
	switch(value->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		m_stack.push_back(make_shared<PGLBooleanData>(value->IsNotEnd()));
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcNEXT()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto& value = m_stack.back();
	switch(value->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		value->Next();
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcPUSHKEYVAL()
{
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}

	auto value = m_stack.back();
	switch(value->GetType())
	{
	case PGLCompoundData::CVectorIter:
	case PGLCompoundData::CMapIter:
		m_stack.push_back(value->GetKey());
		m_stack.push_back(value->GetValue());
		break;
	default:
		PushError(PGLVMError::unable_dereference, m_pc - 1);
		return PGL_RUNERR;
	}
	return -1;
}

int PGLVMThread::ProcFIND()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto key = m_stack.back();
	m_stack.pop_back();
	auto cont = m_stack.back();
	m_stack.pop_back();

	switch(key->GetType())
	{
	case PGLCompoundData::Array:
	case PGLCompoundData::Dictionary:
	case PGLCompoundData::Closure:
		PushError(PGLVMError::compound_key, m_pc - 1);
		return PGL_RUNERR;
	}

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		if(key->GetType() != PGLCompoundData::Integer)
		{
			PushError(PGLVMError::compound_key, m_pc - 1);
			return PGL_RUNERR;
		}
		else
		{
			int i = static_pointer_cast<PGLIntData>(key)->data;
			if(i >= static_pointer_cast<PGLArrayData>(cont)->data.size())
			{
				PushError(PGLVMError::not_found, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(static_pointer_cast<PGLArrayData>(cont)->data[i]);
		}
		break;
	case PGLCompoundData::Dictionary:
		{
			auto k = static_pointer_cast<PGLFirstData>(key);
			auto it = static_pointer_cast<PGLDictionaryData>(cont)->data.find(k);
			if(it == static_pointer_cast<PGLDictionaryData>(cont)->data.end())
			{
				PushError(PGLVMError::not_found, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(it->second);
		}
		break;
	default:
		if(key->GetType() == PGLCompoundData::String)
		{
			auto p = cont->OperateDot(static_pointer_cast<PGLStringData>(key)->data);
			if(!p)
			{
				PushError(PGLVMError::unable_dereference, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(p);
		}
		else
		{
			PushError(PGLVMError::unable_dereference, m_pc - 1);
			return PGL_RUNERR;
		}
	}
	return -1;
}

int PGLVMThread::ProcFINDP()
{
	if(m_stack.size() < 2)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 1);
		return PGL_RUNERR;
	}
	auto key = m_stack.back();
	m_stack.pop_back();
	auto cont = m_stack.back();
	m_stack.pop_back();

	switch(key->GetType())
	{
	case PGLCompoundData::Array:
	case PGLCompoundData::Dictionary:
	case PGLCompoundData::Closure:
		PushError(PGLVMError::compound_key, m_pc - 1);
		return PGL_RUNERR;
	}

	switch(cont->GetType())
	{
	case PGLCompoundData::Array:
		if(key->GetType() != PGLCompoundData::Integer)
		{
			PushError(PGLVMError::compound_key, m_pc - 1);
			return PGL_RUNERR;
		}
		else
		{
			int i = static_pointer_cast<PGLIntData>(key)->data;
			if(i >= static_pointer_cast<PGLArrayData>(cont)->data.size())
			{
				PushError(PGLVMError::not_found, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(static_pointer_cast<PGLArrayData>(cont)->data[i]);
		}
		break;
	case PGLCompoundData::Dictionary:
		{
			auto k = static_pointer_cast<PGLFirstData>(key);
			auto it = static_pointer_cast<PGLDictionaryData>(cont)->data.find(k);
			if(it == static_pointer_cast<PGLDictionaryData>(cont)->data.end())
			{
				PushError(PGLVMError::not_found, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(it->second);
		}
		break;
	default:
		if(key->GetType() == PGLCompoundData::String)
		{
			auto p = cont->OperateDot(static_pointer_cast<PGLStringData>(key)->data);
			if(!p)
			{
				PushError(PGLVMError::unable_dereference, m_pc - 1);
				return PGL_RUNERR;
			}
			m_stack.push_back(p);
		}
		else
		{
			PushError(PGLVMError::unable_dereference, m_pc - 1);
			return PGL_RUNERR;
		}
	}
	m_stack.push_back(cont);
	return -1;
}

int PGLVMThread::ProcTHROW()
{
	m_retCnt = 1;
	int instAddr = m_pc - 1;
	if(m_stack.empty())
	{
		PushError(PGLVMError::stack_underflow, instAddr);
		return PGL_RUNERR;
	}

	while(!m_catchAddr.empty())
	{
		if(m_catchAddr.back() == 0)
		{
			if(!m_callstack.empty()) m_callstack.pop_back();
			m_catchAddr.pop_back();
	
			// 스택 정리, throw 될 값들만 남기고 모두 지운다
			if(m_stackBase.back() > 0) m_stack.erase(m_stack.begin() + m_stackBase.back() - 1, m_stack.end() - m_retCnt);
			m_stackBase.pop_back();
		}
		else
		{
			m_pc = m_catchAddr.back();
			return -1;
		}
	}
	PushError(PGLVMError::unexpected_exception, instAddr);
	return PGL_RUNERR;
}

int PGLVMThread::ProcTRY()
{
	short operand = m_mainVM->m_codes[m_pc++];
	m_catchAddr.back() = m_pc + operand;
	return -1;
}

int PGLVMThread::ProcTRYEXIT()
{
	m_catchAddr.back() = 0;
	return -1;
}

int PGLVMThread::ProcPOPTO()
{
	short operand = m_mainVM->m_codes[m_pc++];
	int n = m_stack.size() - (m_stackBase.back() + operand);
	if(n < 0)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	for(int i = 0; i < n; ++i) m_stack.pop_back();
	return -1;
}

int PGLVMThread::Entry(int paramCnt)
{
	auto call = *(m_stack.end() - paramCnt - 1);
	//m_stackBase.push_back(m_stack.size() - operand);
	m_stackBase.back() = m_stack.size() - paramCnt;
	int ret = CallProc(call, paramCnt);
	if(ret > 0) return ret;
	m_state = PGL_ENTRY;
	return PGL_OK;
}

int PGLVMThread::Resume()
{
	m_state = PGL_RUNNING;
	// 재진입 지점이 설정되어 있다면
	if(m_yieldCF)
	{
		auto copy = m_yieldCF;
		int ret = CallProc(copy, 0);
		if(ret >= 0) return m_state = ret;
	}

#define PROC(p) case PGLOpcode::p: ret = Proc##p(); if(ret >= 0) return m_state = ret; break;
	int ret;
	while(m_pc < m_mainVM->m_codes.size())
	{
		switch(m_mainVM->m_codes[m_pc++])
		{
		PROC(NOP)
		PROC(PUSH)
		PROC(STORE)
		PROC(COPY)
		PROC(WRITE)
		PROC(POP)
		PROC(THISCALL)
		PROC(CALL)
		PROC(RETURN)
		PROC(YIELD)
		PROC(ADD)
		PROC(SUB)
		PROC(MUL)
		PROC(DIV)
		PROC(MOD)
		PROC(POW)
		PROC(SIGN)
		PROC(EQ)
		PROC(NEQ)
		PROC(GT)
		PROC(GTE)
		PROC(LS)
		PROC(LSE)
		PROC(AND)
		PROC(OR)
		PROC(NOT)
		PROC(JMP)
		PROC(UNLESSJMP)
		PROC(PUSHNULL)
		PROC(ASSEMBLE)
		PROC(NEWDICT)
		PROC(AT)
		PROC(LEN)
		PROC(ATD)
		PROC(MAKECLOSURE)
		PROC(SET)
		PROC(SETD)
		PROC(REF)
		PROC(DEREF)
		PROC(COPYDEREF)
		PROC(SETDDEREF)
		PROC(WRITEDEREF)
		PROC(PUSHBEGIN)
		PROC(ISNOTEND)
		PROC(NEXT)
		PROC(PUSHKEYVAL)
		PROC(FIND)
		PROC(FINDP)
		PROC(THROW)
		PROC(TRY)
		PROC(TRYEXIT)
		PROC(POPTO)
		}
	}
	m_state = PGL_OK;
	return PGL_OK;
}

shared_ptr<PGLCompoundData> PGLVMThread::GetParameter(int i)
{
	return m_stack[m_stackBase.back() + i];
}

size_t PGLVMThread::GetParameterCnt() const
{
	return m_stack.size() - m_stackBase.back();
}

void PGLVMThread::SetReturnCnt(size_t i)
{
	m_retCnt = i;
}

void PGLVMThread::PushNull()
{
	ProcPUSHNULL();
}

void PGLVMThread::Push(const shared_ptr<PGLCompoundData>& data)
{
	m_stack.push_back(data);
}

int PGLVMThread::Receive(PGLVMThread* from, int cnt)
{
	move(from->m_stack.end() - cnt, from->m_stack.end(), back_inserter(m_stack));
	from->m_stack.erase(from->m_stack.end() - cnt, from->m_stack.end());
	return PGL_OK;
}

int PGLVMThread::MakeArray(int n)
{
	if(m_stack.size() < n)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	auto arr = make_shared<PGLArrayData>();
	move(m_stack.end() - n, m_stack.end(), back_inserter(arr->data));
	m_stack.erase(m_stack.end() - n, m_stack.end());
	m_stack.push_back(arr);
	return PGL_OK;
}

int PGLVMThread::MakeDictionary(int n)
{
	if(m_stack.size() < 2*n)
	{
		PushError(PGLVMError::stack_underflow, m_pc - 2);
		return PGL_RUNERR;
	}
	auto dict = make_shared<PGLDictionaryData>();
	for(int i = 0; i < n; ++i)
	{
		auto val = m_stack.back();
		m_stack.pop_back();
		auto key = m_stack.back();
		m_stack.pop_back();

		switch(key->GetType())
		{
		case PGLCompoundData::Array:
		case PGLCompoundData::Dictionary:
		case PGLCompoundData::Closure:
			PushError(PGLVMError::compound_key, m_pc - 2);
			return PGL_RUNERR;
		}
		dict->data[static_pointer_cast<PGLFirstData>(key)] = val;
	}
	m_stack.push_back(dict);
	return PGL_OK;
}

string PGLVM::_GetDisassembly(const short* codes, size_t size, size_t rel)
{
	string ret;
	char buf[256];
#define CASEOP(op) case PGLOpcode::op: sprintf_s(buf, 256, "(%04d) 0x%04X: " ## #op, (i + rel < m_debugInfo.size()) ? m_debugInfo[i + rel].line : 0, i + rel); ret += buf;
	for(size_t i = 0; i < size; ++i)
	{
		size_t g;
		switch(codes[i])
		{
		CASEOP(NOP)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(PUSH)
			g = codes[++i];
			sprintf_s(buf, 256,  "  %d\t; %s", g, m_globDebugInfo[g].c_str()); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(STORE)
			g = codes[++i];
			sprintf_s(buf, 256,  "  %d\t; %s", g, m_globDebugInfo[g].c_str()); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(COPY)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(WRITE)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(POP)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(THISCALL)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(CALL)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(RETURN)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(YIELD)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(ADD)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(SUB)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(MUL)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(DIV)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(MOD)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(POW)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(SIGN)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(EQ)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(NEQ)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(GT)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(GTE)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(LS)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(LSE)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(AND)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(OR)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(NOT)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(JMP)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(UNLESSJMP)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(PUSHNULL)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(ASSEMBLE)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(NEWDICT)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(AT)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(LEN)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(ATD)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(MAKECLOSURE)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(SET)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(SETD)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(REF)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(DEREF)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(COPYDEREF)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(SETDDEREF)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(WRITEDEREF)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(PUSHBEGIN)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(ISNOTEND)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(NEXT)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(PUSHKEYVAL)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(FIND)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(FINDP)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(THROW)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(TRY)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(TRYEXIT)
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		CASEOP(POPTO)
			sprintf_s(buf, 256,  "  %d", codes[++i]); ret += buf;
			sprintf_s(buf, 256,  "\n"); ret += buf;
			break;
		}
	}
	return ret;
}