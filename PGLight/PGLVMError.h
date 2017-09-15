#pragma once

enum class PGLVMError
{
	none = 0,
	not_callable,
	stack_underflow,
	unable_add,
	unable_sub,
	unable_mul,
	unable_div,
	unable_mod,
	unable_pow,
	unable_and,
	unable_or,
	unable_not,
	unable_sign,
	unable_cmp,
	expect_bool,
	no_stackbase,
	compound_key,
	unable_dereference,
	not_found,
	unexpected_exception,
	max,
};

const char* GetErrorMsg(PGLVMError e);