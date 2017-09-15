#pragma once

using namespace std;
enum class ErrorType
{
	none,
	unexpected,
	expect_rparen,
	comma_no_exp,
	expect_rbracket,
	expect_semicolon,
	expect_id,
	expect_string,
	redef_id,
	unidentified,
	break_withoutloop,
	continue_withoutloop,
	this_withoutfunc,
	cannot_find_library,
	wrong_library,
	MAX,
};

struct Error
{
	ErrorType type;
	int line;
	string inf[4];
	Error(ErrorType _type, int _line, string inf1 = string(),
		string inf2 = string(), string inf3 = string(), string inf4 = string())
		: type(_type), line(_line)
	{
		inf[0] = inf1;
		inf[1] = inf2;
		inf[2] = inf3;
		inf[3] = inf4;
	}
};

class ErrorCnt
{
protected:
	static const char* GetErrMsg(ErrorType e);
	vector<Error> errs;
public:
	void Push(Error err);
	void Merge(const ErrorCnt& ec);
	string GetErrMsg() const;
	size_t GetErrCnt() const {return errs.size();}
};
