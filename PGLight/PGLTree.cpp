#include "stdafx.h"
#include "PGLTree.h"
#include "PGLVM.h"

shared_ptr<PGLID> ResolveData::RegisterGlobal(string name, const shared_ptr<PGLCompoundData>& data)
{
	auto it = literalList.find(data);
	if(it == literalList.end())
	{
		PGLID idd;
		idd.ref = false;
		idd.constant = false;
		idd.addr = globListCount++;
		idd.belong = nullptr;
		idd.raddr = 0;
		auto p = literalList[data] = make_shared<PGLID>(idd);
		if(!name.empty()) glob.insert(make_pair(name, p));
	}
	else
	{
		if(!name.empty()) glob.insert(make_pair(name, it->second));
	}
	if(name.empty()) return nullptr;
	else return glob.find(name)->second;
}

string ResolveData::GetScopePrefix() const
{
	return accumulate(scope.begin() + 1, scope.end(), string(), [](const string& a, const PGL_closeUnit* b) -> string
	{
		return a + b->GetName() + ".";
	});
}

void PGL_decl_var::CollectID(ResolveData* rd)
{
	PGLID idd;
	idd.ref = false;
	idd.constant = false;
	idd.addr = rd->globListCount++;
	idd.belong = nullptr;
	idd.raddr = 0;
	rd->glob.insert(make_pair(name, make_shared<PGLID>(idd)));
	pid = rd->glob.find(name)->second;
}

void PGL_decl_function::CollectID(ResolveData* rd)
{
	faddr = shared_ptr<PGLFunctionData>(new PGLFunctionData);
	faddr->data = (int)this;
	rd->RegisterGlobal(name, faddr)->constant = true;
}

void PGL_decl_var::ResolveIdentifier(ResolveData* rd)
{
	// 전역변수가 아니라면 현재 스코프에 id 추가
	if(rd->scope.back())
	{
		PGLID idd;
		idd.ref = false;
		idd.constant = false;
		idd.addr = rd->GetIDList().id.size();
		idd.belong = rd->GetIDList().belong;
		idd.raddr = 0;
		rd->GetIDList().id.insert(make_pair(name, make_shared<PGLID>(idd)));
		pid = rd->GetIDList().id.find(name)->second;
	}
	if(init)
	{
		init->ResolveIdentifier(rd);
	}
}

void PGL_decl_var::EraseIdentifier(ResolveData* rd)
{
	// 전역변수가 아니라면 현재 스코프에서 id 제거
	if(rd->scope.back())
	{
		rd->GetIDList().id.erase(name);
	}
}

void PGL_decl_function::ResolveIdentifier(ResolveData* rd)
{
	// 전역변수가 아니라면 현재 스코프에 id 추가
	if(rd->scope.back())
	{
		PGLID idd;
		idd.constant = true;
		idd.ref = false;
		idd.addr = rd->GetIDList().id.size();
		idd.belong = rd->GetIDList().belong;
		idd.raddr = 0;
		faddr = shared_ptr<PGLFunctionData>(new PGLFunctionData);
		faddr->data = (int)this;
		idd.globFunc = rd->RegisterGlobal(rd->GetScopePrefix() + name, faddr);
		idd.globFunc->constant = true;
		rd->GetIDList().id.insert(make_pair(name, make_shared<PGLID>(idd)));
		pid = rd->GetIDList().id.find(name)->second;
	}

	// 새 스코프를 만들고, 그 스코프에 매개변수의 id들을 추가한다
	PGLIDList& il = rd->idList[this];
	il.belong = this;
	for(auto& p : params)
	{
		if(il.id.find(p.name) != il.id.end())
		{
			rd->err->Push(Error(ErrorType::redef_id, 0, p.name));
		}
		else
		{
			PGLID idd;
			idd.constant = false;
			idd.ref = false;
			idd.addr = il.id.size();
			idd.belong = this;
			idd.raddr = 0;
			il.id.insert(make_pair(p.name, make_shared<PGLID>(idd)));
			p.pid = il.id.find(p.name)->second;
		}
	}
	rd->scope.push_back(this);
	sents.ResolveIdentifier(rd);
	// 스코프 제거
	rd->scope.pop_back();
}

void PGL_decl_function::EraseIdentifier(ResolveData* rd)
{
	// 전역변수가 아니라면 현재 스코프에서 id 제거
	if(rd->scope.back())
	{
		rd->GetIDList().id.erase(name);
	}
}

void PGL_function_expr::ResolveIdentifier(ResolveData* rd)
{
	// 새 스코프를 만들고, 그 스코프에 매개변수의 id들을 추가한다
	PGLIDList il;
	il.belong = this;
	for(auto& p : params)
	{
		if(il.id.find(p.name) != il.id.end())
		{
			rd->err->Push(Error(ErrorType::redef_id, 0, p.name));
		}
		else
		{
			PGLID idd;
			idd.constant = false;
			idd.ref = false;
			idd.addr = il.id.size();
			idd.belong = this;
			idd.raddr = 0;
			il.id.insert(make_pair(p.name, make_shared<PGLID>(idd)));
		}
	}
	rd->scope.push_back(this);
	rd->idList.insert(make_pair(this, move(il)));
	sents.ResolveIdentifier(rd);

	// 스코프 제거
	rd->scope.pop_back();
}

void PGL_literal::ResolveIdentifier(ResolveData* rd)
{
	// void 타입일 경우에는 목록에 추가하지 않는다
	if(l->GetType() == PGLCompoundData::None) return;

	// 전역 리터럴 목록에서 찾아서 없을 경우에는 새로 추가
	auto it = rd->literalList.find(l);
	if(it == rd->literalList.end())
	{
		PGLID idd;
		idd.constant = true;
		idd.ref = false;
		idd.addr = rd->globListCount++;
		idd.belong = nullptr;
		idd.raddr = 0;
		rd->literalList.insert(make_pair(l, make_shared<PGLID>(idd)));
	}
}

void PGL_dot_expr::ResolveIdentifier(ResolveData* rd)
{
	l->ResolveIdentifier(rd);

	auto rr = ConvertToPGLType("@" + r);
	// 전역 리터럴 목록에서 찾아서 없을 경우에는 새로 추가
	auto it = rd->literalList.find(rr);
	if(it == rd->literalList.end())
	{
		PGLID idd;
		idd.constant = true;
		idd.ref = false;
		idd.addr = rd->globListCount++;
		idd.belong = nullptr;
		idd.raddr = 0;
		rd->literalList.insert(make_pair(rr, make_shared<PGLID>(idd)));
		pid = rd->literalList[rr];
	}
	else
	{
		pid = it->second;
	}
}

void PGL_identifier::ResolveIdentifier(ResolveData* rd)
{
	// 제일 안쪽 스코프에서부터 바깥으로 나가며 id를 조사한다
	for(auto it = rd->scope.rbegin(); it != rd->scope.rend(); ++it)
	{
		auto itid = rd->idList[*it].id.find(id);
		if(itid != rd->idList[*it].id.end())
		{
			pid = itid->second;
			break;
		}
	}

	// 속하는 스코프가 다를 경우에는 바깥 변수 참조이므로
	// 캡쳐 리스트에 추가
	if(pid && rd->scope.size() >= 2 && pid->belong == *(rd->scope.end() - 2))
	{
		pid->ref = true;
		PGLID idd;
		idd.addr = rd->scope.back()->captureList.size();
		idd.belong = pid->belong;
		idd.ref = true;
		idd.constant = pid->constant;
		idd.raddr = pid->addr;
		rd->scope.back()->captureList.insert(make_pair(id, make_shared<PGLID>(idd)));
		pid = rd->scope.back()->captureList.find(id)->second;
	}

	// 발견되지 않았을 경우에는 global id목록에서 찾는다
	if(!pid)
	{
		auto itid = rd->glob.find(id);
		if(itid != rd->glob.end())
		{
			pid = itid->second;
		}
	}
	if(!pid)
	{
		rd->err->Push(Error(ErrorType::unidentified, line, id));
	}
}

void PGL_this::ResolveIdentifier(ResolveData* rd)
{
	if(rd->scope.empty() || !rd->scope.back())
	{
		rd->err->Push(Error(ErrorType::this_withoutfunc, line, ""));
	}
	else
	{
		rd->scope.back()->memberFunc = true;
	}
}

void PGL_for::ResolveIdentifier(ResolveData* rd)
{
	if(var) var->ResolveIdentifier(rd);
	if(cond) cond->ResolveIdentifier(rd);
	if(loop) loop->ResolveIdentifier(rd);
	sents.ResolveIdentifier(rd);
	if(var) var->EraseIdentifier(rd);
	doneS.ResolveIdentifier(rd);
	elseS.ResolveIdentifier(rd);
}


void PGL_try::ResolveIdentifier(ResolveData* rd)
{
	trySents.ResolveIdentifier(rd);
	
	// 예외를 받을 인수를 스코프에 추가
	PGLID idd;
	idd.ref = false;
	idd.constant = false;
	idd.addr = rd->GetIDList().id.size();
	idd.belong = rd->GetIDList().belong;
	idd.raddr = 0;
	rd->GetIDList().id.insert(make_pair(catchName, make_shared<PGLID>(idd)));
	pid = rd->GetIDList().id.find(catchName)->second;

	catchSents.ResolveIdentifier(rd);

	// 예외를 받을 인수를 스코프에서 제거
	rd->GetIDList().id.erase(catchName);
}

void PGL_for_in::ResolveIdentifier(ResolveData* rd)
{
	PGLID idd;
	idd.ref = false;
	idd.constant = false;
	idd.belong = rd->GetIDList().belong;
	idd.raddr = 0;

	char itName[32];
	char contName[32];
	char keyName[256];
	sprintf_s(contName, 32, "\\Cont%x", this);
	sprintf_s(itName, 32, "\\Iter%x", this);
	if(varKey.empty()) sprintf_s(keyName, 32, "\\Key%x", this);
	else sprintf_s(keyName, 32, "%s", varKey.c_str());

	idd.addr = rd->GetIDList().id.size();
	rd->GetIDList().id.insert(make_pair(contName, make_shared<PGLID>(idd)));
	pidCont = rd->GetIDList().id.find(contName)->second;

	idd.addr = rd->GetIDList().id.size();
	rd->GetIDList().id.insert(make_pair(itName, make_shared<PGLID>(idd)));
	pidIt = rd->GetIDList().id.find(itName)->second;

	idd.addr = rd->GetIDList().id.size();
	rd->GetIDList().id.insert(make_pair(keyName, make_shared<PGLID>(idd)));
	pidKey = rd->GetIDList().id.find(keyName)->second;

	idd.addr = rd->GetIDList().id.size();
	rd->GetIDList().id.insert(make_pair(varVal, make_shared<PGLID>(idd)));
	pidVal = rd->GetIDList().id.find(varVal)->second;

	container->ResolveIdentifier(rd);
	sents.ResolveIdentifier(rd);
	rd->GetIDList().id.erase(varVal);
	rd->GetIDList().id.erase(keyName);
	rd->GetIDList().id.erase(itName);
	rd->GetIDList().id.erase(contName);

	doneS.ResolveIdentifier(rd);
	elseS.ResolveIdentifier(rd);
}

void PGL_literal::GenerateCode(GenerateData* gd)
{
	if(l->GetType() == PGLCompoundData::None)
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHNULL);
		
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
		gd->GetCurrentCode().push_back(gd->rd->literalList[l]->addr);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
}

void PGL_identifier::GenerateLCode(GenerateData* gd)
{
	// 클로져일 경우, 베이스스택 + 0가 캡쳐변수들이므로,
	// 나머지 지역변수들의 위치를 뒤로 밀어줘야함
	int shift = (!gd->cur.back() || gd->cur.back()->captureList.empty()) ? 0 : 1;
	// 멤버 함수일 경우 나머지 지역변수를 하나씩 더 밀어줘야함
	shift += (!gd->cur.back() || !gd->cur.back()->memberFunc) ? 0 : 1;

	if(pid->belong == nullptr) // 전역변수
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::STORE);
		gd->GetCurrentCode().push_back(pid->addr);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else if(pid->belong == gd->cur.back()) // 지역변수
	{
		// 공유당함
		if(pid->ref) gd->GetCurrentCode().push_back((short)PGLOpcode::WRITEDEREF);
		else gd->GetCurrentCode().push_back((short)PGLOpcode::WRITE);
		gd->GetCurrentCode().push_back(pid->addr + shift);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else // 캡쳐된 변수
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
		gd->GetCurrentCode().push_back(0);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		// 공유당함
		if(pid->ref) gd->GetCurrentCode().push_back((short)PGLOpcode::SETDDEREF);
		else gd->GetCurrentCode().push_back((short)PGLOpcode::SETD);
		gd->GetCurrentCode().push_back(pid->addr);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
}

void PGL_identifier::GenerateCode(GenerateData* gd)
{
	// 클로져일 경우, 베이스스택 + 0가 캡쳐변수들이므로,
	// 나머지 지역변수들의 위치를 뒤로 밀어줘야함
	int shift = (!gd->cur.back() || gd->cur.back()->captureList.empty()) ? 0 : 1;
	// 멤버 함수일 경우 나머지 지역변수를 하나씩 더 밀어줘야함
	shift += (!gd->cur.back() || !gd->cur.back()->memberFunc) ? 0 : 1;

	if(pid->belong == nullptr) // 전역변수
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
		gd->GetCurrentCode().push_back(pid->addr);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else if(pid->belong == gd->cur.back()) // 지역변수
	{
		 // 공유당함
		if(pid->ref) gd->GetCurrentCode().push_back((short)PGLOpcode::COPYDEREF);
		else gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
		gd->GetCurrentCode().push_back(pid->addr + shift);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else // 캡쳐된 변수
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
		gd->GetCurrentCode().push_back(0);
		gd->GetCurrentCode().push_back((short)PGLOpcode::ATD);
		gd->GetCurrentCode().push_back(pid->addr);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		if(pid->ref)
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::DEREF);

			gd->GetCurrentDebugInfo().push_back(this);
		}
	}
}

void PGL_this::GenerateLCode(GenerateData* gd)
{

}

void PGL_this::GenerateCode(GenerateData* gd)
{
	// 클로져일 경우, 베이스스택 + 0가 캡쳐변수들이므로,
	// 나머지 지역변수들의 위치를 뒤로 밀어줘야함
	int shift = (!gd->cur.back() || gd->cur.back()->captureList.empty()) ? 0 : 1;
	gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
	gd->GetCurrentCode().push_back(shift);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_dot_expr::GenerateLCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
	gd->GetCurrentCode().push_back(pid->addr);
	gd->GetCurrentCode().push_back((short)PGLOpcode::SET);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_ref_expr::GenerateLCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::SET);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_not_expr::GenerateCode(GenerateData* gd)
{
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::NOT);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_and_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::AND);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_or_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::OR);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_equal_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::EQ);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_notequal_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::NEQ);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_greater_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::GT);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_greaterequal_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::GTE);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_less_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::LS);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_lessequal_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::LSE);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_add_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::ADD);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_sub_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::SUB);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_mul_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::MUL);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_div_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::DIV);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_mod_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::MOD);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_pow_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::POW);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_sign_expr::GenerateCode(GenerateData* gd)
{
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::SIGN);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_dot_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
	gd->GetCurrentCode().push_back(pid->addr);
	gd->GetCurrentCode().push_back((short)PGLOpcode::FIND);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_ref_expr::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	r->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::FIND);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_functioncall_expr::GenerateCode(GenerateData* gd)
{
	if(l->GetType() == "dot_expr")
	{
		auto ll = static_pointer_cast<PGL_dot_expr>(l);
		ll->l->GenerateCode(gd);
		gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
		gd->GetCurrentCode().push_back(ll->pid->addr);
		gd->GetCurrentCode().push_back((short)PGLOpcode::FINDP);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		for(auto& r : params)
		{
			r->GenerateCode(gd);
		}
		gd->GetCurrentCode().push_back((short)PGLOpcode::THISCALL);
		gd->GetCurrentCode().push_back(params.size());
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	else
	{
		l->GenerateCode(gd);
		for(auto& r : params)
		{
			r->GenerateCode(gd);
		}
		gd->GetCurrentCode().push_back((short)PGLOpcode::CALL);
		gd->GetCurrentCode().push_back(params.size());

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
}

void PGL_array_expr::GenerateCode(GenerateData* gd)
{
	for(auto&r : elem)
	{
		r->GenerateCode(gd);
	}
	gd->GetCurrentCode().push_back((short)PGLOpcode::ASSEMBLE);
	gd->GetCurrentCode().push_back(elem.size());

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_dict_expr::GenerateCode(GenerateData* gd)
{
	for(auto&r : elem)
	{
		r.key->GenerateCode(gd);
		r.val->GenerateCode(gd);
	}
	gd->GetCurrentCode().push_back((short)PGLOpcode::NEWDICT);
	gd->GetCurrentCode().push_back(elem.size());

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}


void PGL_assign::GenerateCode(GenerateData* gd)
{
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHNULL);
	gd->GetCurrentDebugInfo().push_back(this);

	r->GenerateCode(gd);
	l->GenerateLCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::POPTO);
	gd->GetCurrentCode().push_back(gd->cur.back() ? gd->cur.back()->stacked : 0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_call::GenerateCode(GenerateData* gd)
{
	l->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::POPTO);
	gd->GetCurrentCode().push_back(gd->cur.back() ? gd->cur.back()->stacked : 0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_block::GenerateCode(GenerateData* gd)
{
	PGL_whole::GenerateCode(gd);
	int ps = CountPopStack();
	if(ps)
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::POP);
		gd->GetCurrentCode().push_back(ps);

		gd->GetCurrentDebugInfo().push_back(nullptr);
		gd->GetCurrentDebugInfo().push_back(nullptr);

		gd->cur.back()->stacked -= ps;
	}
}

void PGL_whole::GenerateCode(GenerateData* gd)
{
	for(auto& r : sents)
	{
		r->GenerateCode(gd);
	}
}

void PGL_if::GenerateCode(GenerateData* gd)
{
	cond->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::UNLESSJMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	size_t delta = gd->GetCurrentCode().size() - 1;
	size_t s = gd->GetCurrentCode().size();
	trueSents.GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	size_t jmpPos = gd->GetCurrentCode().size();
	gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - s;
	falseSents.GenerateCode(gd);
	gd->GetCurrentCode()[jmpPos - 1] = gd->GetCurrentCode().size() - jmpPos;
}

void PGL_try::GenerateCode(GenerateData* gd)
{
	gd->GetCurrentCode().push_back((short)PGLOpcode::TRY);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	size_t tryPos = gd->GetCurrentCode().size();
	trySents.GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::TRYEXIT);
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->GetCurrentCode()[tryPos - 1] = gd->GetCurrentCode().size() - tryPos;
	size_t jmpPos = gd->GetCurrentCode().size();
	catchSents.GenerateCode(gd);
	gd->GetCurrentCode()[jmpPos - 1] = gd->GetCurrentCode().size() - jmpPos;
}

void PGL_break::GenerateCode(GenerateData* gd)
{
	if(gd->loopScope.empty())
	{
		gd->rd->err->Push(Error(ErrorType::break_withoutloop, 0, 0));
		return;
	}
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->loopScope.back()->statBreak.push_back(gd->GetCurrentCode().size() - 1);
}

void PGL_continue::GenerateCode(GenerateData* gd)
{
	if(gd->loopScope.empty())
	{
		gd->rd->err->Push(Error(ErrorType::continue_withoutloop, 0, 0));
		return;
	}
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->loopScope.back()->statContinue.push_back(gd->GetCurrentCode().size() - 1);
}

void PGL_loopUnit::Solve(GenerateData* gd, size_t continuePos, size_t elsePos)
{
	for(auto s : statBreak)
	{
		gd->GetCurrentCode()[s] = elsePos - s - 1;
	}
	for(auto s : statContinue)
	{
		gd->GetCurrentCode()[s] = continuePos - s - 1;
	}
}

void PGL_while::GenerateCode(GenerateData* gd)
{
	gd->loopScope.push_back(this);
	size_t startPos = gd->GetCurrentCode().size();
	cond->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::UNLESSJMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	size_t delta = gd->GetCurrentCode().size() - 1;
	size_t s = gd->GetCurrentCode().size();
	sents.GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->GetCurrentCode().back() = startPos - (int)gd->GetCurrentCode().size();
	gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - s;
	gd->loopScope.pop_back();
	
	// 루프 완료시 구문
	doneS.GenerateCode(gd);
	size_t elsePos = gd->GetCurrentCode().size();
	// 루프 탈출시 구문
	if(!elseS.sents.empty())
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
		gd->GetCurrentCode().push_back(0);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		size_t delta = gd->GetCurrentCode().size() - 1;
		elsePos = gd->GetCurrentCode().size();
		elseS.GenerateCode(gd);
		gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - elsePos;
	}
	Solve(gd, startPos, elsePos);
}

void PGL_for::GenerateCode(GenerateData* gd)
{
	// 초기화 코드 생성
	if(var) var->GenerateCode(gd);

	// 조건 및 반복 코드 생성
	gd->loopScope.push_back(this);
	size_t t = gd->GetCurrentCode().size();
	size_t delta, s;
	if(cond)
	{
		cond->GenerateCode(gd);
		gd->GetCurrentCode().push_back((short)PGLOpcode::UNLESSJMP);
		gd->GetCurrentCode().push_back(0);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		delta = gd->GetCurrentCode().size() - 1;
		s = gd->GetCurrentCode().size();
	}
	sents.GenerateCode(gd);

	size_t contPos = gd->GetCurrentCode().size();
	// 증감 코드 생성
	if(loop) loop->GenerateCode(gd);

	// 조건으로 되돌아감
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);
	
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->GetCurrentCode().back() = t - (int)gd->GetCurrentCode().size();
	if(cond)
	{
		gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - s;
	}
	gd->loopScope.pop_back();

	// 루프 완료시 구문
	doneS.GenerateCode(gd);
	size_t elsePos = gd->GetCurrentCode().size();
	// 루프 탈출시 구문
	if(!elseS.sents.empty())
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
		gd->GetCurrentCode().push_back(0);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		size_t delta = gd->GetCurrentCode().size() - 1;
		elsePos = gd->GetCurrentCode().size();
		elseS.GenerateCode(gd);
		gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - elsePos;
	}
	Solve(gd, contPos, elsePos);
}

void PGL_for_in::GenerateCode(GenerateData* gd)
{
	// 초기화 코드 생성
	container->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHBEGIN);

	gd->GetCurrentDebugInfo().push_back(this);

	gd->cur.back()->stacked += 2;

	// 조건 및 반복 코드 생성
	gd->loopScope.push_back(this);
	size_t t = gd->GetCurrentCode().size();
	size_t delta, s;
	gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
	gd->GetCurrentCode().push_back(pidIt->addr);
	gd->GetCurrentCode().push_back((short)PGLOpcode::ISNOTEND);
	gd->GetCurrentCode().push_back((short)PGLOpcode::UNLESSJMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	delta = gd->GetCurrentCode().size() - 1;
	s = gd->GetCurrentCode().size();

	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHKEYVAL);

	gd->GetCurrentDebugInfo().push_back(this);

	gd->cur.back()->stacked += 2;
	// 루프 변수가 캡쳐되어있을 경우
	if(pidKey->ref)
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
		gd->GetCurrentCode().push_back(pidKey->addr);
		gd->GetCurrentCode().push_back((short)PGLOpcode::REF);
		gd->GetCurrentCode().push_back((short)PGLOpcode::WRITE);
		gd->GetCurrentCode().push_back(pidKey->addr);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);
	}
	if(pidVal->ref)
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::REF);

		gd->GetCurrentDebugInfo().push_back(this);
	}
	sents.GenerateCode(gd);
	gd->cur.back()->stacked -= 2;
	size_t contPos = gd->GetCurrentCode().size();
	// 증감 코드 생성
	gd->GetCurrentCode().push_back((short)PGLOpcode::POP);
	gd->GetCurrentCode().push_back(2);
	gd->GetCurrentCode().push_back((short)PGLOpcode::NEXT);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	// 조건으로 되돌아감
	gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
	gd->GetCurrentCode().push_back(0);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->GetCurrentCode().back() = t - (int)gd->GetCurrentCode().size();
	gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - s;
	gd->loopScope.pop_back();

	// 루프 완료시 구문
	gd->GetCurrentCode().push_back((short)PGLOpcode::POP);
	gd->GetCurrentCode().push_back(2);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	gd->cur.back()->stacked -= 2;
	doneS.GenerateCode(gd);
	size_t elsePos = gd->GetCurrentCode().size();
	// 루프 탈출시 구문
	if(!elseS.sents.empty())
	{
		gd->GetCurrentCode().push_back((short)PGLOpcode::JMP);
		gd->GetCurrentCode().push_back(0);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		size_t delta = gd->GetCurrentCode().size() - 1;
		elsePos = gd->GetCurrentCode().size();
		gd->GetCurrentCode().push_back((short)PGLOpcode::POP);
		gd->GetCurrentCode().push_back(2);

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		gd->cur.back()->stacked -= 2;
		elseS.GenerateCode(gd);
		gd->GetCurrentCode()[delta] = gd->GetCurrentCode().size() - elsePos;
	}
	Solve(gd, contPos, elsePos);
}

void PGL_return::GenerateCode(GenerateData* gd)
{
	ret->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::RETURN);
	gd->GetCurrentCode().push_back(1);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_yield::GenerateCode(GenerateData* gd)
{
	ret->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::YIELD);
	gd->GetCurrentCode().push_back(1);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_throw::GenerateCode(GenerateData* gd)
{
	ret->GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::THROW);

	gd->GetCurrentDebugInfo().push_back(this);
}

void PGL_decl_var::GenerateCode(GenerateData* gd)
{
	if(pid->belong) // 지역변수
	{
		if(init)
		{
			init->GenerateCode(gd);
		}
		else
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHNULL);

			gd->GetCurrentDebugInfo().push_back(this);
		}
		
		// 캡쳐되어 공유하도록 설정된 경우
		if(pid->ref)
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::REF);

			gd->GetCurrentDebugInfo().push_back(this);
		}
		++gd->cur.back()->stacked;
	}
	else // 전역변수
	{
		if(init)
		{
			init->GenerateCode(gd);
			gd->GetCurrentCode().push_back((short)PGLOpcode::STORE);
			gd->GetCurrentCode().push_back(pid->addr);

			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
		}
	}
}

void PGL_decl_function::GenerateCode(GenerateData* gd)
{
	// 지역 함수일 경우
	if(gd->cur.back())
	{
		// 주소를 추가한다
		gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
		gd->GetCurrentCode().push_back(pid->globFunc->addr);
		
		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		// 클로저인 경우
		if(!captureList.empty())
		{
			// 캡쳐 변수 목록을 추가하고
			vector<short> untaddr(captureList.size());
			for(auto& c : captureList)
			{
				untaddr[c.second->addr] = c.second->raddr;
			}
			for(auto& c : untaddr)
			{
				gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
				gd->GetCurrentCode().push_back(c);

				gd->GetCurrentDebugInfo().push_back(this);
				gd->GetCurrentDebugInfo().push_back(this);
			}

			// 배열로 묶는다
			gd->GetCurrentCode().push_back((short)PGLOpcode::ASSEMBLE);
			gd->GetCurrentCode().push_back(captureList.size());

			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);

			// 주소와 캡쳐리스트를 묶어 클로져로 만든다
			gd->GetCurrentCode().push_back((short)PGLOpcode::MAKECLOSURE);

			gd->GetCurrentDebugInfo().push_back(this);
			//이 값을 지역 변수로 설정
		}
		
		// 공유 당할 경우
		if(pid->ref)
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::REF);

			gd->GetCurrentDebugInfo().push_back(this);
		}
		++gd->cur.back()->stacked;
	}

	// 새로운 함수 블럭을 추가
	gd->cur.push_back(this);
	stacked = params.size();
	if(!captureList.empty()) ++stacked;
	if(memberFunc) ++stacked;

	for(auto& p : params)
	{
		// 캡쳐된 매개변수 공유 설정
		if(p.pid->ref && !p.pid->constant)
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
			gd->GetCurrentCode().push_back(p.pid->addr);
			gd->GetCurrentCode().push_back((short)PGLOpcode::REF);
			gd->GetCurrentCode().push_back((short)PGLOpcode::WRITE);
			gd->GetCurrentCode().push_back(p.pid->addr);

			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
		}
	}

	// 코드 생성
	sents.GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHNULL);
	gd->GetCurrentCode().push_back((short)PGLOpcode::RETURN);
	gd->GetCurrentCode().push_back(1);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	// 함수 블럭 제거
	gd->cur.pop_back();
}

void PGL_function_expr::GenerateCode(GenerateData* gd)
{
	// 리터럴에 함수 정보를 추가
	faddr = shared_ptr<PGLFunctionData>(new PGLFunctionData);
	faddr->data = (int)this;
	gd->rd->RegisterGlobal("", faddr);

	pid = gd->rd->literalList[faddr];

	// 새로운 함수 블럭을 추가
	gd->cur.push_back(this);
	stacked = params.size();
	if(!captureList.empty()) ++stacked;
	if(memberFunc) ++stacked;

	// 코드 생성
	sents.GenerateCode(gd);
	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSHNULL);
	gd->GetCurrentCode().push_back((short)PGLOpcode::RETURN);
	gd->GetCurrentCode().push_back(1);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	// 함수 블럭 제거
	gd->cur.pop_back();

	gd->GetCurrentCode().push_back((short)PGLOpcode::PUSH);
	gd->GetCurrentCode().push_back(pid->addr);

	gd->GetCurrentDebugInfo().push_back(this);
	gd->GetCurrentDebugInfo().push_back(this);

	// 클로져일 경우
	if(!captureList.empty())
	{
		// 캡쳐 변수 목록을 추가하고
		vector<short> untaddr(captureList.size());
		for(auto& c : captureList)
		{
			untaddr[c.second->addr] = c.second->raddr;
		}
		for(auto& c : untaddr)
		{
			gd->GetCurrentCode().push_back((short)PGLOpcode::COPY);
			gd->GetCurrentCode().push_back(c);

			gd->GetCurrentDebugInfo().push_back(this);
			gd->GetCurrentDebugInfo().push_back(this);
		}

		// 배열로 묶는다
		gd->GetCurrentCode().push_back((short)PGLOpcode::ASSEMBLE);
		gd->GetCurrentCode().push_back(captureList.size());

		gd->GetCurrentDebugInfo().push_back(this);
		gd->GetCurrentDebugInfo().push_back(this);

		// 주소와 캡쳐리스트를 묶어 클로져로 만든다
		gd->GetCurrentCode().push_back((short)PGLOpcode::MAKECLOSURE);

		gd->GetCurrentDebugInfo().push_back(this);
	}
}

void PGL_decl_function::Link(int pos)
{
	faddr->data = pos;
	faddr->memberFunc = memberFunc;
}

void PGL_function_expr::Link(int pos)
{
	faddr->data = pos;
	faddr->memberFunc = memberFunc;
}

FinalData PGLLink(GenerateData* gd)
{
	FinalData fd;
	for(auto& c : gd->codes)
	{
		// 전역 코드가 아닐 경우 재배치 작업을 한다
		if(c.first) c.first->Link(fd.code.size());

		fd.code.insert(fd.code.end(), c.second.begin(), c.second.end());
		
		// 전역 코드일 경우 끝에 RETURN 코드를 추가한다
		if(!c.first)
		{
			fd.code.push_back((short)PGLOpcode::RETURN);
			fd.code.push_back(0);
		}

		for(auto p : gd->debugInfo[c.first])
		{
			PGLVMDebugInfo di;
			di.line = p ? p->line : 0;
			fd.debugInfo.push_back(di);
		}
	}

	fd.globLiteral.resize(gd->rd->globListCount);
	for(auto& c : gd->rd->literalList)
	{
		fd.globLiteral[c.second->addr] = c.first;
	}
	
	fd.globDebugInfo.resize(gd->rd->globListCount);
	for(auto& c : gd->rd->glob)
	{
		fd.globDebugInfo[c.second->addr] = c.first;
	}

	for(size_t i = 0; i < fd.globDebugInfo.size(); ++i)
	{
		if(!fd.globDebugInfo[i].empty()) continue;
		if(!fd.globLiteral[i]) continue;
		fd.globDebugInfo[i] = fd.globLiteral[i]->DebugInfo();
	}
	return fd;
}