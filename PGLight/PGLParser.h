#pragma once

#include "PGLTree.h"
#include "Error.h"
#include "Unicoding.h"

enum class TokenType
{
	none,
	eof,
	error,
	string,
	integer,
	real,
	identifier,
	question,
	plus,
	minus,
	asterisk,
	divide,
	percent,
	hat,
	dot,
	assign,
	equal,
	greater,
	greaterequal,
	less,
	lessequal,
	notequal,
	colon,
	semicolon,
	comma,
	lbracket,
	rbracket,
	lparen,
	rparen,
	lsbracket,
	rsbracket,
	bar,
	and_,
	or_,
	not_,
	function_,
	var_,
	as_,
	if_,
	else_,
	for_,
	do_,
	while_,
	return_,
	true_,
	false_,
	null_,
	continue_,
	break_,
	done_,
	in_,
	yield_,
	this_,
	throw_,
	try_,
	catch_,
	comment1,
	comment2,
	import,
};


struct Token
{
	string str;
	TokenType type;
	int line;
	Token(string s = string(), TokenType t = TokenType::none, int _line = 0) : str(s.c_str()), type(t), line(_line) {}
};

vector<Token> Tokenize(const string& str);
typedef vector<Token>::const_iterator Tokit;


/*P_* 함수들
begin에서 end범위의 토큰을 가지고 파싱을 시도한다. 성공했을 경우 out으로 트리를 내보내고, 사용한 토큰의 개수를 반환한다.
실패했을경우에는 음수를 반환한다.*/

PGL_whole Parse(const string& str, ErrorCnt& err);

int P_top(Tokit begin, Tokit end, PGL_whole& out, ErrorCnt& err);

int P_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_not_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_or_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_and_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_cmp_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_add_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_mul_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_sign_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_pow_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_a_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_dot_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_par_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_simple_literal(Tokit begin, Tokit end, shared_ptr<PGLFirstData>& out, ErrorCnt& err);
int P_array_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_key_val(Tokit begin, Tokit end, PGL_key_val& out, ErrorCnt& err);
int P_dict_literal(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_lambda_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_function_expr(Tokit begin, Tokit end, shared_ptr<PGL_expr>& out, ErrorCnt& err);
int P_lexpr(Tokit begin, Tokit end, shared_ptr<PGL_lexpr>& out, ErrorCnt& err);
int P_par_lexpr(Tokit begin, Tokit end, shared_ptr<PGL_lexpr>& out, ErrorCnt& err);

int P_block(Tokit begin, Tokit end, PGL_block& out, ErrorCnt& err);
int P_import(Tokit begin, Tokit end, PGL_import& out, ErrorCnt& err);
int P_sentence(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_simple_sentence(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_simple_sentence_semicolon(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_assign(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_call(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_for(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_for_in(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_if(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_while(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_return(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_yield(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_throw(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_try(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_continue(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_break(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_decl_var(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_decl_var_semicolon(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);
int P_param(Tokit begin, Tokit end, PGL_param& out, ErrorCnt& err);
int P_decl_func(Tokit begin, Tokit end, shared_ptr<PGL_sentence>& out, ErrorCnt& err);