#pragma once

/*
기본 함수

print : 콘솔에 출력
copy : 깊은 복사 수행
len : 길이 반환(문자열, 배열, 사전)
type : 타입 반환
*/

struct ResolveData;

class PGLBasicLibrary
{
public:
	static int _print(PGLVMThread* th);
	static int _copy(PGLVMThread* th);
	static int _len(PGLVMThread* th);
	static int _type(PGLVMThread* th);
	static int _thread(PGLVMThread* th);
	static int _int(PGLVMThread* th);
	static int _real(PGLVMThread* th);
	static int _bool(PGLVMThread* th);
	static int _string(PGLVMThread* th);
	static int resume(PGLVMThread* th);
	static int status(PGLVMThread* th);
	static void Include(ResolveData* rd);
};

/*
수학 함수

math.abs
math.acos
math.asin
math.atan
math.atan2
math.ceil
math.cos
math.cosh
math.deg
math.exp
math.floor
math.fmod
math.frexp
math.ldexp
math.log
math.max
math.min
math.modf
math.pi
math.pow
math.rad
math.rand
math.random
math.sin
math.sinh
math.srand
math.sqrt
math.tan
math.tanh

*/
class PGLMathLibrary
{
public:
	static pair<float, bool> ExtractReal(const shared_ptr<PGLCompoundData>& d);
	static int _abs(PGLVMThread* th);
	static int _acos(PGLVMThread* th);
	static int _asin(PGLVMThread* th);
	static int _atan(PGLVMThread* th);
	static int _atan2(PGLVMThread* th);
	static int _ceil(PGLVMThread* th);
	static int _cos(PGLVMThread* th);
	static int _cosh(PGLVMThread* th);
	static int _deg(PGLVMThread* th);
	static int _exp(PGLVMThread* th);
	static int _floor(PGLVMThread* th);
	static int _fmod(PGLVMThread* th);
	static int _frexp(PGLVMThread* th);
	static int _ldexp(PGLVMThread* th);
	static int _log(PGLVMThread* th);
	static int _modf(PGLVMThread* th);
	static int _pow(PGLVMThread* th);
	static int _rad(PGLVMThread* th);
	static int _rand(PGLVMThread* th);
	static int _random(PGLVMThread* th);
	static int _srand(PGLVMThread* th);
	static int _sin(PGLVMThread* th);
	static int _sinh(PGLVMThread* th);
	static int _sqrt(PGLVMThread* th);
	static int _tan(PGLVMThread* th);
	static int _tanh(PGLVMThread* th);
	static void Include(ResolveData* rd);
};

class PGLOSLibrary
{
public:
	static int _now(PGLVMThread* th);
	static int _locale(PGLVMThread* th);
	static void Include(ResolveData* rd);
};

class PGLTimeLibrary
{
public:
	static int second(PGLVMThread* th);
	static int minute(PGLVMThread* th);
	static int hour(PGLVMThread* th);
	static int day(PGLVMThread* th);
	static int date(PGLVMThread* th);
	static int gmdate(PGLVMThread* th);
	static int _seconds(PGLVMThread* th);
	static int _minutes(PGLVMThread* th);
	static int _hours(PGLVMThread* th);
	static int _days(PGLVMThread* th);
	static int _time(PGLVMThread* th);
	static void Include(ResolveData* rd);
};