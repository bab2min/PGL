#include "StdAfx.h"
#include "Error.h"
#include "Unicoding.h"

void ErrorCnt::Push(Error err)
{
	errs.push_back(err);
}

void ErrorCnt::Merge(const ErrorCnt& ec)
{
	errs.insert(errs.end(), ec.errs.begin(), ec.errs.end());
}

const char* ErrorCnt::GetErrMsg(ErrorType e)
{
	static map<ErrorType, const char*> msg;
	if(msg.empty())
	{
		msg[ErrorType::unexpected] = "Unexpected token. %s";
		msg[ErrorType::expect_rparen] = "Expected ')'. %s";
		msg[ErrorType::unidentified] = "Unidentified id. %s";
		msg[ErrorType::break_withoutloop] = "A break is used without for/while. %s";
		msg[ErrorType::comma_no_exp] = "Expected expression. %s";
		msg[ErrorType::continue_withoutloop] = "A continue is used without for/while. %s";
		msg[ErrorType::expect_id] = "Expected identification. %s";
		msg[ErrorType::expect_rbracket] = "Expected ']'. %s";
		msg[ErrorType::expect_semicolon] = "Expected ';'. %s";
		msg[ErrorType::expect_string] = "Expected string type. %s";
		msg[ErrorType::redef_id] = "ID %s is already used.";
		msg[ErrorType::this_withoutfunc] = "'this' must be used in member function.";
		msg[ErrorType::cannot_find_library] = "Cannot find library '%s'.";
		msg[ErrorType::wrong_library] = "A library '%s' has wrong format.";
	}
	return msg[e] ? msg[e] : "";
}

string ErrorCnt::GetErrMsg() const
{
	string ret;
	char buf[256];
	for(auto& e : errs)
	{
		sprintf_s(buf, 256, "Error %04d (%d): ", e.type, e.line);
		ret += buf;
		sprintf_s(buf, 256, GetErrMsg(e.type), e.inf[0].c_str(),
			e.inf[1].c_str(),
			e.inf[2].c_str(),
			e.inf[3].c_str());
		ret += buf;
		sprintf_s(buf, 256, "\n");
		ret += buf;
	}
	return ret;
}