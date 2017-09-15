#include "stdafx.h"
#include "PGLParser.h"

bool ishex(char c)
{
	if('0' <= c && c <= '9') return true;
	if('A' <= c && c <= 'F') return true;
	if('a' <= c && c <= 'f') return true;
	return false;
}

int hexToInt(char c)
{
	if('0' <= c && c <= '9') return c - '0';
	if('A' <= c && c <= 'F') return c - 'A' + 10;
	if('a' <= c && c <= 'f') return c - 'a' + 10;
	return -1;
}

/* '\'로 Escape되어있는 문자열을 원래값으로 해석하여 반환한다
it에서 시작해서 end에서 끝나는 문자열. Escape이 끝나는 지점으로 it로 옮기고, 실패했을 경우 error를 true설정한다*/
wchar_t DecodeEscape(string::const_iterator& it, string::const_iterator end, bool& error)
{
	if(it == end || *it != '\\')
	{
		error = true;
		return 0;
	}
	++it;
	error = false;
	switch(*it)
	{
	case '\\':
		++it;
		return '\\';
	case 't':
		++it;
		return '\t';
	case 'n':
		++it;
		return '\n';
	case 'r':
		++it;
		return '\r';
	case '\'':
		++it;
		return '\'';
	case '"':
		++it;
		return '\"';
	case '0':
		++it;
		return '\0';
	case 'x':
		{
			++it;
			int h = 0;
			for(int i = 0; i < 2; ++i)
			{
				if(!ishex(*it))
				{
					error = true;
					return 0;
				}
				h = h * 16 +  hexToInt(*it);
				++it;
			}
			return h;
		}
	}
	error = true;
	return '\\';
}

/* c 문자가 토큰을 경계짓는 문자이면 참을 반환, 아니면 거짓을 반환한다
*/
bool IsCutCharacter(wchar_t c)
{
	switch(c)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '<':
	case '>':
	case '[':
	case ']':
	case '=':
	case '!':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '^':
	case '?':
	case '.':
	case ':':
	case ';':
	case ',':
	case '{':
	case '}':
	case '(':
	case ')':
	case '|':
		return true;
	}
	return false;
}

vector<Token> Tokenize(const string& str)
{
	vector<Token> ret;

	auto it = str.begin();
	auto marker = str.begin();

	TokenType stage = TokenType::none;
	int line = 1;
	string s;

	for(;it != str.end();++it)
	{
		switch(stage)
		{
		case TokenType::none:
			switch(*it)
			{
			case '\n':
				++line;
			case ' ':
			case '\t':
			case '\r':
				break;
			case '\"':
				stage = TokenType::string;
				s.clear();
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				stage = TokenType::integer;
				marker = it;
				break;
			case '<':
				if(it + 1 != str.end() && it[1] == '=')
				{
					ret.push_back(Token("<=", TokenType::lessequal, line));
					++it;
				}
				else
				{
					ret.push_back(Token("<", TokenType::less, line));
				}
				break;
			case '>':
				if(it + 1 != str.end() && it[1] == '=')
				{
					ret.push_back(Token(">=", TokenType::greaterequal, line));
					++it;
				}
				else
				{
					ret.push_back(Token(">", TokenType::greater, line));
				}					
				break;
			case '[':
				ret.push_back(Token("[", TokenType::lsbracket, line));
				break;
			case ']':
				ret.push_back(Token("]", TokenType::rsbracket, line));
				break;
			case '!':
				if(it + 1 != str.end() && it[1] == '=')
				{
					ret.push_back(Token("!=", TokenType::notequal, line));
					++it;
				}
				else
				{
					ret.push_back(Token("!", TokenType::error, line));
				}
				break;
			case '=':
				if(it + 1 != str.end() && it[1] == '=')
				{
					ret.push_back(Token("==", TokenType::equal, line));
					++it;
				}
				else
				{
					ret.push_back(Token("=", TokenType::assign, line));
				}
				break;
			case '+':
				ret.push_back(Token(string(it, it + 1), TokenType::plus, line));
				break;
			case '-':
				ret.push_back(Token(string(it, it + 1), TokenType::minus, line));
				break;
			case '*':
				ret.push_back(Token(string(it, it + 1), TokenType::asterisk, line));
				break;
			case '/':
				if(it + 1 != str.end() && it[1] == '/')
				{
					stage = TokenType::comment1;
					++it;
				}
				else if(it + 1 != str.end() && it[1] == '*')
				{
					stage = TokenType::comment2;
					++it;
				}
				else
				{
					ret.push_back(Token(string(it, it + 1), TokenType::divide, line));
				}
				break;
			case '%':
				ret.push_back(Token(string(it, it + 1), TokenType::percent, line));
				break;
			case '^':
				ret.push_back(Token(string(it, it + 1), TokenType::hat, line));
				break;
			case '?':
				ret.push_back(Token(string(it, it + 1), TokenType::question, line));
				break;
			case '.':
				ret.push_back(Token(string(it, it + 1), TokenType::dot, line));
				break;
			case ':':
				ret.push_back(Token(string(it, it + 1), TokenType::colon, line));
				break;
			case ';':
				ret.push_back(Token(string(it, it + 1), TokenType::semicolon, line));
				break;
			case ',':
				ret.push_back(Token(string(it, it + 1), TokenType::comma, line));
				break;
			case '{':
				ret.push_back(Token(string(it, it + 1), TokenType::lbracket, line));
				break;
			case '}':
				ret.push_back(Token(string(it, it + 1), TokenType::rbracket, line));
				break;
			case '(':
				ret.push_back(Token(string(it, it + 1), TokenType::lparen, line));
				break;
			case ')':
				ret.push_back(Token(string(it, it + 1), TokenType::rparen, line));
				break;
			case '|':
				ret.push_back(Token(string(it, it + 1), TokenType::bar, line));
				break;
			default:
				stage = TokenType::identifier;
				marker = it;
				break;
			}
			break;
		case TokenType::string:
			for(;*it != '\"'; ++it)
			{
				if(it == str.end() || *it == '\n' || *it == '\r')
				{
					stage = TokenType::none;
					ret.push_back(Token(s, TokenType::error, line));
					break;
				}
				else if(*it == '\\')
				{
					bool err;
					s.push_back(DecodeEscape(it, str.end(), err));
					if(err)
					{
						stage = TokenType::none;
						ret.push_back(Token(s, TokenType::error, line));
						break;
					}
					--it;
				}
				else
				{
					s.push_back(*it);
				}
			}
			stage = TokenType::none;
			ret.push_back(Token(s, TokenType::string, line));
			break;
		case TokenType::integer:
			if(*it == '.')
			{
				stage = TokenType::real;
			}
			else if(IsCutCharacter(*it))
			{
				stage = TokenType::none;
				ret.push_back(Token(string(marker, it--), TokenType::integer, line));
			}
			else if(!isdigit(*it))
			{
				stage = TokenType::none;
				ret.push_back(Token(string(marker, it), TokenType::error, line));
				--it;
			}
			break;
		case TokenType::real:
			if(IsCutCharacter(*it))
			{
				stage = TokenType::none;
				ret.push_back(Token(string(marker, it--), TokenType::real, line));
			}
			else if(!isdigit(*it))
			{
				stage = TokenType::none;
				ret.push_back(Token(string(marker, it), TokenType::error, line));
				--it;
			}
			break;
		case TokenType::identifier:
			if(IsCutCharacter(*it))
			{
				stage = TokenType::none;
				string s(marker, it--);
				if(s == "and")
					ret.push_back(Token(s, TokenType::and_, line));
				else if(s == "or")
					ret.push_back(Token(s, TokenType::or_, line));
				else if(s == "not")
					ret.push_back(Token(s, TokenType::not_, line));
				else if(s == "function")
					ret.push_back(Token(s, TokenType::function_, line));
				else if(s == "var")
					ret.push_back(Token(s, TokenType::var_, line));
				else if(s == "as")
					ret.push_back(Token(s, TokenType::as_, line));
				else if(s == "if")
					ret.push_back(Token(s, TokenType::if_, line));
				else if(s == "else")
					ret.push_back(Token(s, TokenType::else_, line));
				else if(s == "for")
					ret.push_back(Token(s, TokenType::for_, line));
				else if(s == "while")
					ret.push_back(Token(s, TokenType::while_, line));
				else if(s == "do")
					ret.push_back(Token(s, TokenType::do_, line));
				else if(s == "true")
					ret.push_back(Token(s, TokenType::true_, line));
				else if(s == "false")
					ret.push_back(Token(s, TokenType::false_, line));
				else if(s == "null")
					ret.push_back(Token(s, TokenType::null_, line));
				else if(s == "return")
					ret.push_back(Token(s, TokenType::return_, line));
				else if(s == "break")
					ret.push_back(Token(s, TokenType::break_, line));
				else if(s == "continue")
					ret.push_back(Token(s, TokenType::continue_, line));
				else if(s == "done")
					ret.push_back(Token(s, TokenType::done_, line));
				else if(s == "in")
					ret.push_back(Token(s, TokenType::in_, line));
				else if(s == "yield")
					ret.push_back(Token(s, TokenType::yield_, line));
				else if(s == "this")
					ret.push_back(Token(s, TokenType::this_, line));
				else if(s == "throw")
					ret.push_back(Token(s, TokenType::throw_, line));
				else if(s == "try")
					ret.push_back(Token(s, TokenType::try_, line));
				else if(s == "catch")
					ret.push_back(Token(s, TokenType::catch_, line));
				else if(s == "import")
					ret.push_back(Token(s, TokenType::import, line));
				else
					ret.push_back(Token(s, TokenType::identifier, line));
			}
			break;
		case TokenType::comment1:
			if(*it == '\n')
			{
				++line;
				stage = TokenType::none;
			}
			break;
		case TokenType::comment2:
			switch(*it)
			{
			case '\n':
				++line;
				break;
			case '*':
				if(it + 1 != str.end() && it[1] == '/')
				{
					++it;
					stage = TokenType::none;
				}
				break;
			}
			break;
		}
	}
	ret.push_back(Token("", TokenType::eof, line));

	return ret;
}

/*
입력 받은 str 문자열을 정수로 변환한다. 
정수인 부분까지 변환하여 반환하고, 정수가 아닌 문자열일 경우 0을 반환한다.
*/
int ParseInt(string str)
{
	int sum = 0;
	for(auto it = str.begin(); it != str.end(); ++it)
	{
		if(!isdigit(*it)) return sum;
		sum = sum * 10 + (*it - '0');
	}
	return sum;
}


int P_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	return P_not_expr(begin, end, out, err);
}

int P_not_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> ret;

	if(begin[i].type == TokenType::not_)
	{
		++i;
		shared_ptr<PGL_expr> not;
		if((t = P_not_expr(begin + i, end, not, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
			return -1;
		}
		ret = make_shared<PGL_not_expr>(not);
	}
	else
	{
		if((t = P_or_expr(begin + i, end, ret, err)) >= 0)
		{
			i += t;
		}
		else
		{
			return -1;
		}
	}
	ret->line = begin->line;
	out = ret;
	return i;
}

int P_or_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_and_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
		
	while(begin[i].type == TokenType::or_)
	{
		++i;
		shared_ptr<PGL_expr> rexp;
		if((t = P_and_expr(begin + i, end, rexp, err)) >= 0)
		{
			lexp = make_shared<PGL_or_expr>(lexp, rexp);
			lexp->line = begin[i].line;
			i += t;
		}
		else
		{
			--i;
			break;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_and_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_cmp_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
		
	while(begin[i].type == TokenType::and_)
	{
		++i;
		shared_ptr<PGL_expr> rexp;
		if((t = P_cmp_expr(begin + i, end, rexp, err)) >= 0)
		{
			lexp = make_shared<PGL_and_expr>(lexp, rexp);
			lexp->line = begin[i].line;
			i += t;
		}
		else
		{
			--i;
			break;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_cmp_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_add_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
	
	TokenType s = begin[i].type;

	switch(s)
	{
	case TokenType::equal:
	case TokenType::notequal:
	case TokenType::greater:
	case TokenType::greaterequal:
	case TokenType::less:
	case TokenType::lessequal:
		++i;
		break;
	default:
		out = lexp;
		return i;
	}

	shared_ptr<PGL_expr> rexp;
	if((t = P_add_expr(begin + i, end, rexp, err)) >= 0)
	{
		i += t;
		switch(s)
		{
		case TokenType::equal:
			lexp = make_shared<PGL_equal_expr>(lexp, rexp);
			break;
		case TokenType::notequal:
			lexp = make_shared<PGL_notequal_expr>(lexp, rexp);
			break;
		case TokenType::greater:
			lexp = make_shared<PGL_greater_expr>(lexp, rexp);
			break;
		case TokenType::greaterequal:
			lexp = make_shared<PGL_greaterequal_expr>(lexp, rexp);
			break;
		case TokenType::less:
			lexp = make_shared<PGL_less_expr>(lexp, rexp);
			break;
		case TokenType::lessequal:
			lexp = make_shared<PGL_lessequal_expr>(lexp, rexp);
			break;
		}
	}
	else
	{
		err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
		return -1;
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_add_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_mul_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
	
	while(begin[i].type == TokenType::plus || begin[i].type == TokenType::minus)
	{
		TokenType s = begin[i].type;
		++i;
		shared_ptr<PGL_expr> rexp;
		if((t = P_mul_expr(begin + i, end, rexp, err)) >= 0)
		{
			switch(s)
			{
			case TokenType::plus:
				lexp = make_shared<PGL_add_expr>(lexp, rexp);
				break;
			case TokenType::minus:
				lexp = make_shared<PGL_sub_expr>(lexp, rexp);
				break;
			}
			lexp->line = begin[i].line;
			i += t;
		}
		else
		{
			--i;
			break;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_mul_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_sign_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
	
	while(begin[i].type == TokenType::asterisk || begin[i].type == TokenType::divide || begin[i].type == TokenType::percent)
	{
		TokenType s = begin[i].type;
		++i;
		shared_ptr<PGL_expr> rexp;
		if((t = P_sign_expr(begin + i, end, rexp, err)) >= 0)
		{
			switch(s)
			{
			case TokenType::asterisk:
				lexp = make_shared<PGL_mul_expr>(lexp, rexp);
				break;
			case TokenType::divide:
				lexp = make_shared<PGL_div_expr>(lexp, rexp);
				break;
			case TokenType::percent:
				lexp = make_shared<PGL_mod_expr>(lexp, rexp);
				break;
			}
			lexp->line = begin[i].line;
			i += t;
		}
		else
		{
			--i;
			break;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_sign_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> ret;

	if(begin[i].type == TokenType::minus)
	{
		++i;
		shared_ptr<PGL_expr> not;
		if((t = P_sign_expr(begin + i, end, not, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
			return -1;
		}
		ret = make_shared<PGL_sign_expr>(not);
	}
	else
	{
		if((t = P_pow_expr(begin + i, end, ret, err)) >= 0)
		{
			i += t;
		}
		else return -1;
	}
	ret->line = begin->line;
	out = ret;
	return i;
}

int P_pow_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_a_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
		
	while(begin[i].type == TokenType::hat)
	{
		++i;
		shared_ptr<PGL_expr> rexp;
		if((t = P_a_expr(begin + i, end, rexp, err)) >= 0)
		{
			lexp = make_shared<PGL_pow_expr>(lexp, rexp);
			lexp->line = begin[i].line;
			i += t;
		}
		else
		{
			--i;
			break;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}


int P_a_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> lexp;
	if((t = P_par_expr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
		
	while(begin[i].type == TokenType::dot || begin[i].type == TokenType::lsbracket || begin[i].type == TokenType::lparen)
	{
		if(begin[i].type == TokenType::dot)
		{
			++i;
			if(begin[i].type == TokenType::identifier)
			{
				lexp = make_shared<PGL_dot_expr>(lexp, begin[i].str);
				lexp->line = begin[i].line;
				++i;
			}
			else
			{
				err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
				return -1;
			}
		}
		else if(begin[i].type == TokenType::lsbracket)
		{
			++i;
			shared_ptr<PGL_expr> rexp;
			if((t = P_expr(begin + i, end, rexp, err)) >= 0)
			{
				i += t;
			}
			else
			{
				err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
				return -1;
			}
			
			if(begin[i].type == TokenType::rsbracket)
			{
				++i;
			}
			else
			{
				err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
				return -1;
			}
			lexp = make_shared<PGL_ref_expr>(lexp, rexp);
		}
		else
		{
			++i;
			vector<shared_ptr<PGL_expr>> params;
			shared_ptr<PGL_expr> p;

			if((t = P_expr(begin + i, end, p, err)) >= 0)
			{
				i += t;
				params.push_back(p);
				while(begin[i].type == TokenType::comma)
				{
					++i;
					if((t = P_expr(begin + i, end, p, err)) >= 0)
					{
						i += t;
						params.push_back(p);
					}
					else
					{
						err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
						return -1;
					}
				}
			}

			if(begin[i].type == TokenType::rparen)
			{
				++i;
			}
			else
			{
				err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
				return -1;
			}

			shared_ptr<PGL_functioncall_expr> ret = shared_ptr<PGL_functioncall_expr>(new PGL_functioncall_expr);
			ret->l = lexp;
			ret->params = params;
			lexp = ret;
			lexp->line = begin[i].line;
		}
	}
	lexp->line = begin->line;
	out = lexp;
	return i;
}

int P_par_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_expr> ret;
	if((t = P_function_expr(begin + i, end, ret, err)) >= 0)
	{
		i += t;
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else if((t = P_lambda_expr(begin + i, end, ret, err)) >= 0)
	{
		i += t;
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else if(begin[i].type == TokenType::lparen)
	{
		++i;
		if((t = P_expr(begin + i, end, ret, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}

		if(begin[i].type == TokenType::rparen)
		{
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
			return -1;
		}
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else if(begin[i].type == TokenType::identifier)
	{
		ret = make_shared<PGL_identifier>(begin[i].str);
		++i;
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else if(begin[i].type == TokenType::this_)
	{
		ret = make_shared<PGL_this>();
		++i;
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else if((t = P_literal(begin + i, end, ret, err)) >= 0)
	{
		i += t;
		ret->line = begin->line;
		out = ret;
		return i;
	}
	else
	{
		return -1;
	}
}

int P_simple_literal(Tokit begin, Tokit end, shared_ptr<PGLFirstData>& out, ErrorCnt& err)
{
	int i = 0;
	shared_ptr<PGLFirstData> data;
	if(begin[i].type == TokenType::integer)
	{
		data = make_shared<PGLIntData>(atoi(begin[i].str.c_str()));
	}
	else if(begin[i].type == TokenType::real)
	{
		data = make_shared<PGLRealData>((float)atof(begin[i].str.c_str()));
	}
	else if(begin[i].type == TokenType::string)
	{
		data = make_shared<PGLStringData>(begin[i].str);
	}
	else if(begin[i].type == TokenType::true_)
	{
		data = make_shared<PGLBooleanData>(true);
	}
	else if(begin[i].type == TokenType::false_)
	{
		data = make_shared<PGLBooleanData>(false);
	}
	else if(begin[i].type == TokenType::null_)
	{
		data = make_shared<PGLVoidData>();
	}
	else return -1;
	++i;
	out = data;
	return i;
}

int P_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGLFirstData> data;
	if((t = P_simple_literal(begin + i, end, data, err)) >=0 )
	{
		i += t;
		out = make_shared<PGL_literal>(data);
		out->line = begin->line;
	}
	else if((t = P_array_literal(begin + i, end, out, err)) >= 0)
	{
		i += t;
	}
	else if((t = P_dict_literal(begin + i, end, out, err)) >= 0)
	{
		i += t;
	}
	else return -1;

	return i;
}

int P_array_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	auto a = shared_ptr<PGL_array_expr>(new PGL_array_expr);
	if(begin[i].type == TokenType::lsbracket)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_expr> p;

	if((t = P_expr(begin + i, end, p, e)) >= 0)
	{
		i += t;
		a->elem.push_back(p);
		while(begin[i].type == TokenType::comma)
		{
			++i;
			if((t = P_expr(begin + i, end, p, e)) >= 0)
			{
				i += t;
				a->elem.push_back(p);
			}
			else
			{
				err.Merge(e);
				err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
				return -1;
			}
		}
	}

	if(begin[i].type == TokenType::rsbracket)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
		return -1;
	}
	out = a;
	out->line = begin->line;
	return i;
}

int P_key_val(Tokit begin, Tokit end, PGL_key_val& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	shared_ptr<PGLFirstData> key;
	if((t = P_simple_literal(begin + i, end, key, e)) >= 0)
	{
		i += t;
		out.key = make_shared<PGL_literal>(key);
	}
	else return -1;

	if(begin[i].type == TokenType::assign)
	{
		++i;
	}
	else return -1;

	if((t = P_expr(begin + i, end, out.val, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	return i;
}

int P_dict_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	auto a = shared_ptr<PGL_dict_expr>(new PGL_dict_expr);
	if(begin[i].type == TokenType::lbracket)
	{
		++i;
	}
	else return -1;

	PGL_key_val p;

	if((t = P_key_val(begin + i, end, p, e)) >= 0)
	{
		i += t;
		a->elem.push_back(p);
		while(begin[i].type == TokenType::comma)
		{
			++i;
			if((t = P_key_val(begin + i, end, p, e)) >= 0)
			{
				i += t;
				a->elem.push_back(p);
			}
			else
			{
				err.Merge(e);
				err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
				return -1;
			}
		}
	}

	if(begin[i].type == TokenType::rbracket)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
		return -1;
	}
	out = a;
	out->line = begin->line;
	return i;
}

int P_lambda_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	PGL_param p;
	auto ret = shared_ptr<PGL_function_expr>(new PGL_function_expr);
	if(begin[i].type == TokenType::lparen)
	{
		++i;
		if((t = P_param(begin + i, end, p, err)) >= 0)
		{
			i += t;
			ret->params.push_back(p);
			while(begin[i].type == TokenType::comma)
			{
				++i;
				if((t = P_param(begin + i, end, p, err)) >= 0)
				{
					i += t;
					ret->params.push_back(p);
				}
				else
				{
					err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
					return -1;
				}
			}
		}
		e = ErrorCnt();
		if(begin[i].type == TokenType::rparen)
		{
			++i;
		}
		else
		{
			e.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
			return -1;
		}
	}
	else if((t = P_param(begin + i, end, p, e)) >=0 )
	{
		i += t;
		ret->params.push_back(p);
	}
	else return -1;

	if(begin[i].type == TokenType::colon)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_expr> expr;
	if((t = P_expr(begin + i, end, expr, e)) >= 0)
	{
		i += t;
		auto s = shared_ptr<PGL_return>(new PGL_return);
		s->ret = expr;
		ret->sents.sents.push_back(s);
	}
	else
	{
		e.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_function_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	if(begin[i].type == TokenType::function_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else return -1;

	auto var = shared_ptr<PGL_function_expr>(new PGL_function_expr);
	PGL_param param;
	if((t = P_param(begin + i, end, param, e)) >= 0)
	{
		i += t;
		var->params.push_back(param);
		while(begin[i].type == TokenType::comma)
		{
			++i;
			if((t = P_param(begin + i, end, param, e)) >= 0)
			{
				i += t;
				var->params.push_back(param);
			}
			else
			{
				err.Merge(e);
				err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
				return -1;
			}
		}
	}

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::as_)
	{
		++i;
		if(begin[i].type == TokenType::identifier)
		{
			var->type = begin[i].str;
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if((t = P_block(begin + i, end, var->sents, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	out = var;
	out->line = begin->line;
	return i;
}

int P_sentence(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if((t = P_if(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_for(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_for_in(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_while(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_return(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_yield(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_throw(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_try(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_continue(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_break(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_decl_var_semicolon(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_decl_func(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_simple_sentence_semicolon(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
}

int P_simple_sentence(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if((t = P_assign(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else if((t = P_call(begin + i, end, out, err)) >= 0)
	{
		i += t;
		return i;
	}
	else
	{
		return -1;
	}
}

int P_simple_sentence_semicolon(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if((t = P_simple_sentence(begin + i, end, out, err)) >= 0)
	{
		i += t;
		if(begin[i].type == TokenType::semicolon)
		{
			++i;
			return i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
			return -1;
		}
	}
	return -1;
}

int P_block(Tokit begin, Tokit end, PGL_block& out, ErrorCnt& err)
{
	int i = 0;
	int t;

	ErrorCnt e;

	if(begin[i].type == TokenType::lbracket)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_sentence> ret;
	bool exit = false;
	while(!exit)
	{
		if(begin[i].type == TokenType::rbracket) break;
		t = P_sentence(begin + i, end, ret, e);
		if(t >= 0)
		{
			i += t;
			out.sents.push_back(ret);
			err.Merge(e);
			e = ErrorCnt();
		}
		else
		{
			int bracket = 1;
			for(;;++i)
			{
				if(begin[i].type == TokenType::lbracket)
				{
					++bracket;
				}
				else if(begin[i].type == TokenType::rbracket)
				{
					--bracket;
					if(bracket == 0)
					{
						exit = true;
						err.Merge(e);
						e = ErrorCnt();
						break;
					}
				}
				else if(begin[i].type == TokenType::semicolon)
				{
					if(bracket == 1)
					{
						++i;
						err.Merge(e);
						e = ErrorCnt();
						break;
					}
				}
				else if(begin[i].type == TokenType::eof)
				{
					exit = true;
					break;
				}
			}
		}
	}

	if(begin[i].type == TokenType::rbracket)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
		return -1;
	}
	return i;
}

int P_if(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;

	if(begin[i].type == TokenType::if_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	shared_ptr<PGL_expr> cond;
	if((t = P_expr(begin + i, end, cond, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	e = ErrorCnt();
	PGL_block block;
	if((t = P_block(begin + i, end, block, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = shared_ptr<PGL_if>(new PGL_if);
	ret->cond = cond;
	ret->trueSents = block;
	if(begin[i].type == TokenType::else_)
	{
		++i;
		if(begin[i].type == TokenType::if_)
		{
			shared_ptr<PGL_sentence> elif;
			if((t = P_if(begin + i, end, elif, err)) >= 0)
			{
				i += t;
				ret->falseSents.sents.push_back(elif);
			}
			else
			{
				err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
				return -1;
			}
		}
		else
		{
			if((t = P_block(begin + i, end, ret->falseSents, err)) >= 0)
			{
				i += t;
			}
			else
			{
				err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
				return -1;
			}
		}
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_try(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;

	if(begin[i].type == TokenType::try_)
	{
		++i;
	}
	else return -1;

	e = ErrorCnt();
	PGL_block block;
	if((t = P_block(begin + i, end, block, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = make_shared<PGL_try>();
	ret->trySents = block;
	if(begin[i].type == TokenType::catch_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	string name;
	if(begin[i].type == TokenType::identifier)
	{
		name = begin[i].str;
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	e = ErrorCnt();
	block = PGL_block();
	if((t = P_block(begin + i, end, block, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	ret->catchSents = block;
	ret->catchName = name;
	out = ret;
	out->line = begin->line;
	return i;
}


int P_while(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;

	if(begin[i].type == TokenType::while_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	shared_ptr<PGL_expr> cond;
	if((t = P_expr(begin + i, end, cond, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	e = ErrorCnt();
	PGL_block block;
	if((t = P_block(begin + i, end, block, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = shared_ptr<PGL_while>(new PGL_while);
	ret->cond = cond;
	ret->sents = block;

	if(begin[i].type == TokenType::done_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->doneS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if(begin[i].type == TokenType::else_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->elseS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}

	out = ret;
	out->line = begin->line;
	return i;
}

int P_for(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;
	shared_ptr<PGL_sentence> var;


	if(begin[i].type == TokenType::for_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if((t = P_decl_var(begin + i, end, var, e)) >= 0)
	{
		i += t;
	}
	
	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		/*err.Merge(e);
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));*/
		return -1;
	}
	
	shared_ptr<PGL_expr> cond;
	if((t = P_expr(begin + i, end, cond, e)) >= 0)
	{
		i += t;
	}

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	shared_ptr<PGL_sentence> loop;
	if((t = P_simple_sentence(begin + i, end, loop, e)) >= 0)
	{
		i += t;
	}

	auto ret = make_shared<PGL_for>();
	ret->var = static_pointer_cast<PGL_decl_var>(var);
	ret->cond = cond;
	ret->loop = static_pointer_cast<PGL_simple_sentence>(loop);

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	e = ErrorCnt();
	if((t = P_block(begin + i, end, ret->sents, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::done_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->doneS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if(begin[i].type == TokenType::else_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->elseS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_for_in(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	ErrorCnt e;

	if(begin[i].type == TokenType::for_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::var_)
	{
		++i;
	}
	else
	{
		return -1;
	}

	string key, val;

	if(begin[i].type == TokenType::identifier)
	{
		val = begin[i].str;
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	
	if(begin[i].type == TokenType::comma)
	{
		++i;
		key = move(val);
		if(begin[i].type == TokenType::identifier)
		{
			val = begin[i].str;
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if(begin[i].type == TokenType::in_)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}
	
	shared_ptr<PGL_expr> con;
	if((t = P_expr(begin + i, end, con, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	auto ret = make_shared<PGL_for_in>();
	ret->varKey = key;
	ret->varVal = val;
	ret->container = con;

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	e = ErrorCnt();
	if((t = P_block(begin + i, end, ret->sents, e)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Merge(e);
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::done_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->doneS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if(begin[i].type == TokenType::else_)
	{
		++i;
		if((t = P_block(begin + i, end, ret->elseS, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_return(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::return_)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_expr> r;
	if((t = P_expr(begin + i, end, r, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = shared_ptr<PGL_return>(new PGL_return);
	ret->ret = r;
	out = ret;
	out->line = begin->line;
	return i;
}

int P_yield(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::yield_)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_expr> r;
	if((t = P_expr(begin + i, end, r, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = make_shared<PGL_yield>();
	ret->ret = r;
	out = ret;
	out->line = begin->line;
	return i;
}

int P_throw(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::throw_)
	{
		++i;
	}
	else return -1;

	shared_ptr<PGL_expr> r;
	if((t = P_expr(begin + i, end, r, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = make_shared<PGL_throw>();
	ret->ret = r;
	out = ret;
	out->line = begin->line;
	return i;
}

int P_break(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::break_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = shared_ptr<PGL_break>(new PGL_break);
	out = ret;
	out->line = begin->line;
	return i;
}

int P_continue(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::continue_)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}

	auto ret = shared_ptr<PGL_continue>(new PGL_continue);
	out = ret;
	out->line = begin->line;
	return i;
}

int P_import(Tokit begin, Tokit end, PGL_import& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::import)
	{
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::string)
	{
		out.name = begin[i].str;
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_string, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}
	return i;
}

int P_decl_var(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::var_)
	{
		++i;
	}
	else return -1;

	auto var = shared_ptr<PGL_decl_var>(new PGL_decl_var);
	if(begin[i].type == TokenType::identifier)
	{
		var->name = begin[i].str;
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::as_)
	{
		++i;
		if(begin[i].type == TokenType::identifier)
		{
			var->type = begin[i].str;
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if(begin[i].type == TokenType::assign)
	{
		++i;
		if((t = P_expr(begin + i, end, var->init, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}
	}
	out = var;
	out->line = begin->line;
	return i;
}

int P_decl_var_semicolon(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_sentence> ret;
	if((t = P_decl_var(begin + i, end, ret, err)) >= 0)
	{
		i += t;
	}
	else return -1;

	if(begin[i].type == TokenType::semicolon)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_semicolon, begin[i].line, begin[i].str));
		return -1;
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_param(Tokit begin, Tokit end, PGL_param& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::identifier)
	{
		out.name = begin[i].str;
		++i;
	}
	else
	{
		return -1;
	}

	if(begin[i].type == TokenType::as_)
	{
		++i;
		if(begin[i].type == TokenType::identifier)
		{
			out.type = begin[i].str;
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
			return -1;
		}
	}

	return i;
}


int P_decl_func(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	if(begin[i].type == TokenType::function_)
	{
		++i;
	}
	else return -1;

	auto var = shared_ptr<PGL_decl_function>(new PGL_decl_function);
	if(begin[i].type == TokenType::identifier)
	{
		var->name = begin[i].str;
		++i;
	}
	else return -1;

	if(begin[i].type == TokenType::lparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	PGL_param param;
	if((t = P_param(begin + i, end, param, err)) >= 0)
	{
		i += t;
		var->params.push_back(param);
		while(begin[i].type == TokenType::comma)
		{
			++i;
			if((t = P_param(begin + i, end, param, err)) >= 0)
			{
				i += t;
				var->params.push_back(param);
			}
			else
			{
				err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
				return -1;
			}
		}
	}

	if(begin[i].type == TokenType::rparen)
	{
		++i;
	}
	else
	{
		err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
		return -1;
	}

	if(begin[i].type == TokenType::as_)
	{
		++i;
		if(begin[i].type == TokenType::identifier)
		{
			var->type = begin[i].str;
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_id, begin[i].line, begin[i].str));
			return -1;
		}
	}

	if((t = P_block(begin + i, end, var->sents, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	out = var;
	out->line = begin->line;
	return i;
}

int P_call(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;

	auto ret = shared_ptr<PGL_call>(new PGL_call);
	if((t = P_expr(begin + i, end, ret->l, err)) >= 0)
	{
		i += t;
	}
	else return -1;

	out = ret;
	out->line = begin->line;
	return i;
}

int P_lexpr(Tokit begin, Tokit end, shared_ptr<PGL_lexpr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_lexpr> lexp;
	if((t = P_par_lexpr(begin + i, end, lexp, err)) >= 0)
	{
		i += t;
	}
	else return -1;
		
	while(begin[i].type == TokenType::dot || begin[i].type == TokenType::lsbracket)
	{
		if(begin[i].type == TokenType::dot)
		{
			++i;
			if(begin[i].type == TokenType::identifier)
			{
				lexp = make_shared<PGL_dot_expr>(lexp, begin[i].str);
				i += t;
			}
			else
			{
				err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
				return -1;
			}
		}
		else
		{
			++i;
			shared_ptr<PGL_expr> rexp;
			if((t = P_expr(begin + i, end, rexp, err)) >= 0)
			{
				i += t;
			}
			else
			{
				err.Push(Error(ErrorType::none, begin[i].line, begin[i].str));
				return -1;
			}
			
			if(begin[i].type == TokenType::rsbracket)
			{
				++i;
			}
			else
			{
				err.Push(Error(ErrorType::expect_rbracket, begin[i].line, begin[i].str));
				return -1;
			}
			lexp = make_shared<PGL_ref_expr>(lexp, rexp);
		}
	}
	out = lexp;
	out->line = begin->line;
	return i;
}

int P_par_lexpr(Tokit begin, Tokit end, shared_ptr<PGL_lexpr>& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_lexpr> ret;
	if(begin[i].type == TokenType::lparen)
	{
		if((t = P_lexpr(begin + i, end, ret, err)) >= 0)
		{
			i += t;
		}
		else
		{
			err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
			return -1;
		}

		if(begin[i].type == TokenType::rparen)
		{
			++i;
		}
		else
		{
			err.Push(Error(ErrorType::expect_rparen, begin[i].line, begin[i].str));
			return -1;
		}

		out = ret;
		out->line = begin->line;
		return i;
	}
	else if(begin[i].type == TokenType::identifier)
	{
		ret = make_shared<PGL_identifier>(begin[i].str);
		++i;
		out = ret;
		out->line = begin->line;
		return i;
	}
	else if(begin[i].type == TokenType::this_)
	{
		ret = make_shared<PGL_this>();
		++i;
		out = ret;
		out->line = begin->line;
		return i;
	}
	else
	{
		return -1;
	}
}


int P_assign(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err)
{
	int i = 0;
	int t;

	auto ret = shared_ptr<PGL_assign>(new PGL_assign);
	if((t = P_lexpr(begin + i, end, ret->l, err)) >= 0)
	{
		i += t;
	}
	else return -1;

	if(begin[i].type == TokenType::assign)
	{
		++i;
	}
	else return -1;

	if((t = P_expr(begin + i, end, ret->r, err)) >= 0)
	{
		i += t;
	}
	else
	{
		err.Push(Error(ErrorType::unexpected, begin[i].line, begin[i].str));
		return -1;
	}
	out = ret;
	out->line = begin->line;
	return i;
}

int P_top(Tokit begin, Tokit end, PGL_whole& out, ErrorCnt& err)
{
	int i = 0;
	int t;
	shared_ptr<PGL_sentence> ret;
	PGL_import import;
	while(1)
	{
		if((t = P_sentence(begin + i, end, ret, err)) >= 0)
		{
			i += t;
			out.sents.push_back(ret);
			if(begin[i].type == TokenType::eof) break;
		}
		else if((t = P_import(begin + 1, end, import, err)) >= 0)
		{
			i += t;
			out.imports.push_back(import);
		}
		else
		{
			break;
		}
	}
	return i;
}

PGL_whole Parse(const string& str, ErrorCnt& err)
{
	auto toks = Tokenize(str);
	PGL_whole tree;
	P_top(toks.begin(), toks.end(), tree, err);
	return tree;
}