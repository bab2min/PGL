#include "stdafx.h"
#include "PGLVMError.h"

const char* GetErrorMsg(PGLVMError e)
{
	switch(e)
	{
	case PGLVMError::none:
		return "Unknown Error.";
	case PGLVMError::not_callable:
		return "Address is not callable.";
	case PGLVMError::stack_underflow:
		return "Stack underflow.";
	case PGLVMError::unable_add:
		return "Unable to add.";
	case PGLVMError::unable_sub:
		return "Unable to sub.";
	case PGLVMError::unable_mul:
		return "Unable to mul.";
	case PGLVMError::unable_div:
		return "Unable to div.";
	case PGLVMError::unable_pow:
		return "Unable to pow.";
	case PGLVMError::unable_and:
		return "Unable to and.";
	case PGLVMError::unable_or:
		return "Unable to or.";
	case PGLVMError::unable_not:
		return "Unable to not.";
	case PGLVMError::unable_sign:
		return "Unable to sign.";
	case PGLVMError::unable_cmp:
		return "Unable to cmp.";
	case PGLVMError::expect_bool:
		return "Expected bool.";
	case PGLVMError::no_stackbase:
		return "Stackbase is broken.";
	case PGLVMError::compound_key:
		return "Compound data type cannot be used as dictionary key.";
	case PGLVMError::unable_dereference:
		return "Unable derefence.";
	case PGLVMError::not_found:
		return "A key has not been found.";
	case PGLVMError::unexpected_exception:
		return "Unexpected exception has occurred.";
	}
	return "";
}