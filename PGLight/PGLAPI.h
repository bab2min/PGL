#pragma once

/**
@mainpage PGLight API
@section intro 소개
- PGLight는 PG어를 게임엔진에 쓸 목적으로 스크립트화하여 경량화시킨 버전입니다.
@subsection s1 자료 타입
- PGLight는 정수(부호 있는 32비트), 실수(32비트 부동소수점), 문자열(UTF-8 인코딩), 부울, 배열, 사전, 함수 등의 타입을 지원합니다.
타입은 동적으로 결정됩니다.
@subsection s2 지역 변수와 전역 변수, 스코프
- 
@subsection s3 익명 함수와 클로저
- 
@section CREATEINFO 작성정보
- 작성자  : Integralus (bab2min@gmail.com)
- 작성일  : 2013-07-08
@section DESCRIPTION PGLight 문법


@date 2013-07-08
*/

/**
@file PGLAPI.h
@author Integralus
@brief PGLight의 C API함수
*/

#if defined(_DYNAMIC)
	#define DLLEXPORT __declspec(dllexport)
#elif defined(_STATIC)
	#define DLLEXPORT
#else
	#define DLLEXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief PGLM 타입은 PGLight 소스 코드를 구동시키는 가장 기본적인 단위입니다.
이 핸들은 @ref PGL_Init 함수를 통해 새로 생성될 수 있습니다.
이 핸들은 PGL가상머신과 오류 메세지 버퍼 등을 포함합니다.
모든 PGLight 작업이 끝난 뒤에는 @ref PGL_Close 함수를 통해 해제되어야합니다.
*/
typedef void* PGLM;

/**
@brief PGLThread 타입은 PGLight 소스 코드를 구동시키는 작업의 단위입니다.
이 핸들은 @ref PGL_NewThread 함수를 통해서 새로 생성될 수 있습니다.
생성된 스레드는 @ref PGLT_Entry 함수를 통해서 진입 지점을 설정할 수 있습니다.
진입 지점이 설정되지 않은 경우는 전역 공간의 코드를 수행합니다.
각각의 스레드는 독자적인 스택을 가지고 있으며, (전역 변수를 제외하면) 서로 영향을 받지 않고 실행될 수 있습니다.
PGLThread는 @ref PGLT_Receive 함수를 통해 다른 PGLThread로부터 자료를 주고 받을 수 있습니다.
두 개 이상의 스레드가 부모-자식 관계로 생성된 경우 자식 스레드에서 yield를 사용하고, 부모 스레드에서 resume을 사용함으로써
유기적으로 코드 수행권을 주고 받을 수 있습니다.
*/
typedef void* PGLThread;

/**
@brief PGLData 타입은 PGLight의 자료를 관리하는 핸들입니다.
PGLight에서는 정수, 실수, 문자열, 부울, 배열, 사전, 함수 등의 다양한 자료 타입을 제공합니다.
PGLData 핸들을 통해 모든 타입의 PGL자료를 조작할 수 있습니다.
이 핸들은 PGLD_Create... 함수로 생성될 수도 있고, @ref PGLT_GetParam 이나
@ref PGLD_GetArrayValue, @ref PGLD_GetDictionaryValue 와 같은 함수를 통해서 얻을 수도 있습니다.
PGLight의 자료는 참조계수를 이용해 관리되므로, 얻어진 PGLData 핸들은 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
*/
typedef void* PGLData;

/**
@brief PGLCFunction 타입은 PGLight측에서 호출할 수 있는 C언어 함수 타입입니다.
c언어 측에서는 인수로 넘어오는 @ref PGLThread 값을 조작하여 PGL자료를 수정할 수 있습니다.
이 함수가 반환하는 int값은 PGL_OK, PGL_YIELD, PGL_RUNERR, PGL_THROW 중 하나여야 합니다.

- PGL_OK : 함수가 성공적으로 종료되었을 경우 이 값을 반환해야 합니다. 함수의 반환값은 스택에 입력되어 있어야 합니다.

- PGL_YIELD : 함수 내에서 yield을 하고자 할 경우 이 값을 반환해야 합니다. yield의 반환값은 스택에 입력되어 있어야 합니다.

- PGL_RUNERR : 함수 실행 도중 오류가 발생했을 경우 이 값을 반환해야 합니다. 결과적으로 가상머신은 중단하게 됩니다.

- PGL_THROW : 함수 내에서 @ref PGLT_Throw 를 호출해서 예외를 발생시켰을 경우 이 값을 반환해야합니다.
발생된 예외를 PGLight 코드 측에서 적절하게 처리하지 못할 경우 가상머신은 중단하게 됩니다.
*/
typedef int (*PGLCFunction)(PGLThread);

typedef int (*PGLLibraryInit)(PGLM);

#define PGL_ERROR -1
#define PGL_OK 0
#define PGL_ENTRY 1
#define PGL_YIELD 2
#define PGL_RUNERR 3
#define PGL_RUNNING 4

/**
@brief 이 값들은 @ref PGLD_Type 함수를 통해 PGL자료의 타입을 조사할 때 반환되는 값입니다.
*/
enum
{
	PGLTYPE_None,		///< void타입(null값만 해당됩니다)
	PGLTYPE_Integer,	///< 32비트 정수 타입 (c언어의 long과 호환)
	PGLTYPE_Real,		///< 32비트 부동소수점 타입 (c언어의 float와 호환)
	PGLTYPE_String,		///< null종료 문자열 타입, UTF-8로 인코딩되어 있습니다.
	PGLTYPE_Boolean,	///< 부울 타입
	PGLTYPE_Array,		///< 배열 타입
	PGLTYPE_Dictionary,	///< 사전 타입, 키로는 정수, 실수, 문자열만 사용할 수 있습니다.
	PGLTYPE_Function,	///< PGL 함수 타입
	PGLTYPE_CFunction,	///< C함수 타입 (@ref PGLCFunction 를 참조)
	PGLTYPE_Closure,	///< 클로저 타입 (내부적으로만 사용됩니다.)
	PGLTYPE_Ref,		///< 참조 타입 (내부적으로만 사용됩니다.)
	PGLTYPE_CVectorIter,///< 배열 순회 타입 (내부적으로만 사용됩니다.)
	PGLTYPE_CMapIter,	///< 사전 순회 타입 (내부적으로만 사용됩니다.)
	PGLTYPE_Thread,		///< 스레드 타입
	PGLTYPE_Time,		///< 시간 타입
};

/// @fn PGLM PGL_Init()
/// @brief PGL가상머신을 준비합니다.
/// @return 생성한 가상머신을 반환합니다.
/// @remark 생성된 PGLM은 반드시 @ref PGL_Close 로 닫아주어야 합니다.
DLLEXPORT PGLM PGL_Init();

/// @fn int PGL_Load(PGLM m, const char* sourceCode, int len)
/// @brief 가상머신에 소스 코드를 로드하여 컴파일합니다.
/// @param m 소스 코드를 로드할 가상머신의 핸들
/// @param sourceCode 소스 코드의 시작주소
/// @param len 소스 코드의 길이
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark sourceCode 인자로 주어지는 소스 코드는 UTF-8로 인코딩되어 있어야 합니다.
/// 실패 시 발생한 오류 메세지는 @ref PGL_GetErrorMsg 함수를 이용해 얻을 수 있습니다.
DLLEXPORT int PGL_Load(PGLM m, const char* sourceCode, int len);

/// @fn int PGL_LoadStdLibrary(PGLM m)
/// @brief 가상머신에 기본 라이브러리를 로드합니다.
/// @param m 라이브러리를 로드할 가상머신의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 기본 라이브러리를 사용하고자 할 경우, @ref PGL_Load 에 앞서 먼저 이 함수를 호출하여 기본 라이브러리를 로드해야 합니다.
DLLEXPORT int PGL_LoadStdLibrary(PGLM m);

/// @fn int PGL_LoadLibrary(PGLM m)
/// @brief 가상머신에 외부 라이브러리를 로드합니다.
/// @param m 라이브러리를 로드할 가상머신의 핸들
/// @param name 라이브러리의 상대경로 (확장자는 제외)
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 외부 라이브러리를 사용하고자 할 경우, @ref PGL_Load 에 앞서 먼저 이 함수를 호출하여 외부 라이브러리를 로드해야 합니다.
DLLEXPORT int PGL_LoadLibrary(PGLM m, const char* name);

/// @fn int PGL_RegisterGlobal(PGLM m, const char* name, PGLData data)
/// @brief 가상머신에 전역변수를 등록합니다.
/// @param m 전역변수를 등록할 가상머신의 핸들
/// @param name 등록할 전역변수의 이름
/// @param data 전역변수로 등록할 PGL자료의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 전역변수 등록은 @ref PGL_Run 으로 가상머신이 실행되기 전에 이뤄져야 합니다. 가상머신 실행 중에는 새로 전역변수를 등록할 수 없습니다.
/// 전역함수 역시 이 함수로 등록할 수 있습니다.
DLLEXPORT int PGL_RegisterGlobal(PGLM m, const char* name, PGLData data);

DLLEXPORT int PGL_RegisterGlobalNull(PGLM m, const char* name);
DLLEXPORT int PGL_RegisterGlobalInteger(PGLM m, const char* name, int data);
DLLEXPORT int PGL_RegisterGlobalReal(PGLM m, const char* name, float data);
DLLEXPORT int PGL_RegisterGlobalString(PGLM m, const char* name, const char* data);
DLLEXPORT int PGL_RegisterGlobalBoolean(PGLM m, const char* name, int data);
DLLEXPORT int PGL_RegisterGlobalCFunction(PGLM m, const char* name, PGLCFunction data);
DLLEXPORT int PGL_RegisterGlobalUserData(PGLM m, const char* name, void* data);

/// @fn int PGL_SetGlobalValue(PGLM m, const char* name, PGLData data)
/// @brief 가상머신의 등록된 전역변수의 값을 새로 설정합니다.
/// @param m 전역변수가 등록된 가상머신의 핸들
/// @param name 값을 새로 설정할 전역변수의 이름
/// @param data 설정될 새로운 PGL자료의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수는 @ref PGL_RegisterGlobal 함수로 등록되거나, PGLight 소스 코드에 명시된 전역변수의 값을 변경하는 데에 사용합니다.
/// 인수로 주어진 이름을 등록된 전역변수 목록에서 찾지 못할 경우 함수는 실패합니다.
DLLEXPORT int PGL_SetGlobalValue(PGLM m, const char* name, PGLData data);

/// @fn PGLData PGL_GetGlobalValue(PGLM m, const char* name)
/// @brief 가상머신의 등록된 전역변수의 값을 가져옵니다.
/// @param m 전역변수가 등록된 가상머신의 핸들
/// @param name 값을 가져올 전역변수의 이름
/// @return 성공 시에는 PGL자료의 핸들, 실패 시에는 0을 반환합니다.
/// @remark 이 함수는 @ref PGL_RegisterGlobal 함수로 등록되거나, PGLight 소스 코드에 명시된 전역변수의 값을 가져오는 데에 사용합니다.
/// 인수로 주어진 이름을 등록된 전역변수 목록에서 찾지 못할 경우 함수는 실패합니다.
/// 이 함수로 얻어진 PGL자료의 핸들은 반드시 @ref PGLD_Release 를 통해 해제되어야합니다.
DLLEXPORT PGLData PGL_GetGlobalValue(PGLM m, const char* name);

/// @fn int PGL_GetErrorMsg(PGLM m, char* buffer, int bufferSize)
/// @brief 작업이 실패했을 경우 오류 메세지를 얻어옵니다.
/// @param m 오류 메세지를 얻어올 가상머신의 핸들
/// @param buffer 오류 메세지를 받을 버퍼의 주소
/// @param bufferSize 오류 메세지를 받을 버퍼의 크기
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// buffer로 0을 넣는 경우 오류 메세지를 담는데 필요한 버퍼의 크기를 반환합니다.
/// @remark bufferSize는 오류 메세지를 전부 담을 정도로 충분해야 합니다. 만약 버퍼의 크기가 작을 경우 오류 메세지가 잘릴 수 있습니다.
DLLEXPORT int PGL_GetErrorMsg(PGLM m, char* buffer, int bufferSize);

/// @fn int PGL_Run(PGLM m)
/// @brief 불러들인 소스 코드를 바탕으로 가상머신을 시작합니다.
/// @param m 시작할 가상머신의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수는 반드시 @ref PGL_Load 로 소스 코드를 로드한 다음 호출되어야 합니다.
DLLEXPORT int PGL_Run(PGLM m);

/// @fn int PGL_Close(PGLM m)
/// @brief PGL가상머신을 종료합니다.
/// @param m 종료할 PGL가상머신의 핸들
/// @return 성공시에는 PGL_OK, 실패시에는 그 외의 값을 반환합니다.
/// @remark PGL가상머신이 종료하기에 앞서 그 가상머신에서 생성했던 모든 스레드를 @ref PGLT_Close 함수를 통해 종료시켜야 합니다.
DLLEXPORT int PGL_Close(PGLM m);

/// @fn int PGL_GetDisassembly(PGLM m, const char* funcName, char* buffer, int bufferSize)
/// @brief PGL가상머신의 어셈블리 코드를 문자열로 가져옵니다.
/// @param m 코드를 가져올 가상머신의 핸들
/// @param funcName 어셈블리 코드를 조사할 함수의 이름. 0을 넣으면 전체 코드를 조사합니다.
/// @param buffer 코드를 담을 버퍼의 주소
/// @param bufferSize 코드를 받을 버퍼의 크기
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// buffer로 0을 넣는 경우 코드를 담는데 필요한 버퍼의 크기를 반환합니다.
/// @remark 이 함수는 @ref PGL_Load 로 소스 코드가 적재되고, @ref PGL_Run 으로 정상적으로 컴파일된 이후에 호출되어야 합니다.
DLLEXPORT int PGL_GetDisassembly(PGLM m, const char* funcName, char* buffer, int bufferSize);

/// @fn PGLThread PGL_NewThread(PGLM m)
/// @brief 새로운 PGL스레드를 생성합니다.
/// @param m 새로운 스레드를 생성할 PGL가상머신
/// @return 성공 시에는 NULL이 아닌 스레드의 핸들 값을, 실패 시에는 NULL을 반환한다.
DLLEXPORT PGLThread PGL_NewThread(PGLM m);

/// @fn PGLData PGLD_CreateNull()
/// @brief 널 타입의 PGL자료를 생성합니다.
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateNull();

/// @fn PGLData PGLD_CreateInteger(int n)
/// @brief 정수 타입의 PGL자료를 생성합니다.
/// @param n 정수값
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateInteger(int n);

/// @fn PGLData PGLD_CreateReal(float n)
/// @brief 실수 타입의 PGL자료를 생성합니다.
/// @param n 실수값
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateReal(float n);

/// @fn PGLData PGLD_CreateString(const char* n)
/// @brief 문자열 타입의 PGL자료를 생성합니다.
/// @param n 널종료 문자열의 포인터
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateString(const char* n);

/// @fn PGLData PGLD_CreateBoolean(int n)
/// @brief 부울 타입의 PGL자료를 생성합니다.
/// @param n 부울값
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateBoolean(int n);

/// @fn PGLData PGLD_CreateCFunction(PGLCFunction n)
/// @brief C함수 타입의 PGL자료를 생성합니다.
/// @param n C함수 포인터값
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateCFunction(PGLCFunction n);

DLLEXPORT PGLData PGLD_CreateUserData(void* n);

/// @fn PGLData PGLD_CreateArray()
/// @brief 배열 타입의 PGL자료를 생성합니다.
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateArray();

/// @fn PGLData PGLD_CreateDictionary()
/// @brief 사전 타입의 PGL자료를 생성합니다.
/// @return PGL자료의 핸들을 반환합니다.
/// @remark PGLD_Create-함수로 생성된 PGL자료는 반드시 @ref PGLD_Release 함수를 이용해 해제해야 합니다.
DLLEXPORT PGLData PGLD_CreateDictionary();

/// @fn int PGLD_Release(PGLData data)
/// @brief PGL자료를 해제합니다.
/// @param data 해제할 PGL자료의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark PGL자료는 참조계수를 이용하며 메모리를 관리합니다.
/// 따라서 @ref PGLT_GetParam, @ref PGLD_GetArrayValue, @ref PGLD_GetDictionaryValue 나 PGLD_Create... 함수로 생성된 자료는 반드시 해제해주어야 합니다.
DLLEXPORT int PGLD_Release(PGLData data);

/// @fn int PGLD_Type(PGLData data)
/// @brief PGL자료의 타입을 조사합니다.
/// @param data 타입을 조사할 PGL자료의 핸들
/// @return PGL자료 타입에 맞는 PGLTYPE를 반환합니다.
DLLEXPORT int PGLD_Type(PGLData data);

/// @fn int PGLD_GetInteger(PGLData data)
/// @brief PGL자료의 정수 값을 가져옵니다.
/// @param data 값을 가져올 PGL자료의 핸들
/// @return data가 정수 혹은 실수 타입이면 int로 변환하여 반환하고, 그 외의 경우엔 0을 반환합니다.
DLLEXPORT int PGLD_GetInteger(PGLData data);

/// @fn float PGLD_GetReal(PGLData data)
/// @brief PGL자료의 실수 값을 가져옵니다.
/// @param data 값을 가져올 PGL자료의 핸들
/// @return data가 정수 혹은 실수 타입이면 float로 변환하여 반환하고, 그 외의 경우엔 0을 반환합니다.
DLLEXPORT float PGLD_GetReal(PGLData data);

/// @fn const char* PGLD_GetString(PGLData data)
/// @brief PGL자료의 문자열 값을 가져옵니다.
/// @param data 값을 가져올 PGL자료의 핸들
/// @return data가 문자열 타입이면 그 시작 주소를 반환하고, 그 외의 경우엔 0을 반환합니다.
/// @remark 반환된 문자열 포인터는 적어도 data가 @ref PGLD_Release 로 해제되기 전까지는 유지됩니다.
/// 하지만 @ref PGLD_Release 로 data가 해제된 이후에는 포인터가 가리키는 값이 제거될 수도 있습니다.
DLLEXPORT const char* PGLD_GetString(PGLData data);

/// @fn int PGLD_GetBoolean(PGLData data)
/// @brief PGL자료의 부울 값을 가져옵니다.
/// @param data 값을 가져올 PGL자료의 핸들
/// @return data가 부울 타입이면 그 값을 int로 변환하여 반환하고, 그 외의 경우엔 0을 반환합니다.
DLLEXPORT int PGLD_GetBoolean(PGLData data);

/// @fn PGLCFunction PGLD_GetCFunction(PGLData data)
/// @brief PGL자료의 C함수 값을 가져옵니다.
/// @param data 값을 가져올 PGL자료의 핸들
/// @return data가 C함수 타입이면 그 값을 @ref PGLCFunction 로 변환하여 반환하고, 그 외의 경우엔 0을 반환합니다.
DLLEXPORT PGLCFunction PGLD_GetCFunction(PGLData data);

DLLEXPORT void* PGLD_GetUserData(PGLData data);

/// @fn int PGLD_SetArrayValue(PGLData data, int idx, PGLData newData)
/// @brief PGL배열의 요소 값을 설정합니다.
/// @param data 값을 설정할 PGL배열의 핸들
/// @param idx 값을 설정할 요소의 인덱스
/// @param newData 요소의 새로운 값
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 배열의 크기를 초과하는 인덱스를 인수로 주면 함수는 실패합니다. 배열이 크기를 조사하는 데에는 @ref PGLD_GetArrayLength 함수를 사용하십시오.
DLLEXPORT int PGLD_SetArrayValue(PGLData data, int idx, PGLData newData);

/// @fn int PGLD_GetArrayLength(PGLData data)
/// @brief PGL배열의 크기를 조사합니다.
/// @param data 크기를 조사할 PGL배열의 핸들
/// @return 성공 시에는 배열의 크기, 실패 시에는 음수 값을 반환합니다.
DLLEXPORT int PGLD_GetArrayLength(PGLData data);

/// @fn int PGLD_ResizeArray(PGLData data, int length)
/// @brief PGL배열의 크기를 재설정합니다.
/// @param data 크기를 재설정할 PGL배열의 핸들
/// @param length 배열의 새로운 크기
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark PGL배열의 크기를 키울 경우 빈 자리는 NULL값으로 채워집니다.
DLLEXPORT int PGLD_ResizeArray(PGLData data, int length);

/// @fn PGLData PGLD_GetArrayValue(PGLData data, int idx)
/// @brief PGL배열의 요소 값을 가져옵니다.
/// @param data 값을 가져올 PGL배열의 핸들
/// @param idx 값을 가져올 요소의 인덱스
/// @return 성공 시에는 가져온 값의 PGL자료 핸들, 실패 시에는 0을 반환합니다.
/// @remark 배열의 크기를 초과하는 인덱스를 인수로 주면 함수는 실패합니다.
/// 이 함수로 얻어진 PGL자료는 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
DLLEXPORT PGLData PGLD_GetArrayValue(PGLData data, int idx);

/// @fn int PGLD_SetDictionaryValue(PGLData data, PGLData key, PGLData newValue)
/// @brief PGL사전의 값을 설정합니다.
/// @param data 값을 설정할 PGL사전의 핸들
/// @param key 새 값을 설정할 키
/// @param newValue 새로운 값
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 사전에 존재하지 않는 키는 새로 추가되고, 이미 존재하던 키는 덮어씌여집니다.
DLLEXPORT int PGLD_SetDictionaryValue(PGLData data, PGLData key, PGLData newValue);

DLLEXPORT int PGLD_SetDictionaryValueByStr(PGLData data, const char* key, PGLData newValue);

/// @fn int PGLD_HasDictionaryKey(PGLData data, PGLData key)
/// @brief PGL사전이 특정 키를 가지고 있는지 조사합니다.
/// @param data 조사할 PGL사전의 핸들
/// @param key 조사할 키
/// @return 사전이 키를 가지고 있을 경우는 0이 아닌 값, 가지고 있지 않을 경우는 0을 반환합니다.
/// @remark data나 key 인수로 부적적할 값이 주어진 경우 PGL_ERROR를 반환합니다.
DLLEXPORT int PGLD_HasDictionaryKey(PGLData data, PGLData key);

DLLEXPORT int PGLD_HasDictionaryKeyByStr(PGLData data, const char* key);

/// @fn PGLData PGLD_GetDictionaryValue(PGLData data, PGLData key)
/// @brief PGL사전의 값을 가져옵니다.
/// @param data 값을 가져올 PGL사전의 핸들
/// @param key 값을 가져올 키
/// @return 성공 시에는 가져온 PGL자료의 핸들, 실패 시에는 0을 반환합니다.
/// @remark 키가 존재하지 않는 경우 0을 반환합니다.
/// 이 함수로 얻어진 PGL자료는 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
DLLEXPORT PGLData PGLD_GetDictionaryValue(PGLData data, PGLData key);

DLLEXPORT PGLData PGLD_GetDictionaryValueByStr(PGLData data, const char* key);

/// @fn PGLData PGLD_IterationBegin(PGLData data)
/// @brief 사전이나 배열을 순회하기 위한 이터레이터를 생성합니다.
/// @param data 이터레이터를 생성할 사전이나 배열
/// @return 성공 시에는 생성된 이터레이터 PGL자료의 핸들, 실패 시에는 0을 반환합니다.
/// @remark 인수로 주어지는 data는 배열이나 사전 타입이어야만 합니다.
/// 이 함수로 얻어진 PGL자료는 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
DLLEXPORT PGLData PGLD_IterationBegin(PGLData data);

/// @fn int PGLD_IterationNext(PGLData iter)
/// @brief 이터레이터가 다음 대상을 가리키도록 합니다.
/// @param iter 다음을 가리키도록 할 이터레이터의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
DLLEXPORT int PGLD_IterationNext(PGLData iter);

/// @fn int PGLD_IterationNotEnd(PGLData iter)
/// @brief 이터레이터가 끝에 도달하지 않았는지를 조사합니다.
/// @param iter 조사할 이터레이터의 핸들
/// @return 이터레이터가 끝에 도달하지 않았을 경우 0이 아닌 값, 끝에 도달한 경우 0을 반환합니다.
/// 함수가 실패한 경우 PGL_ERROR를 반환합니다.
DLLEXPORT int PGLD_IterationNotEnd(PGLData iter);

/// @fn PGLData PGLD_GetIterationKey(PGLData iter)
/// @brief 이터레이터가 가리키는 키를 얻어옵니다.
/// @param iter 키를 얻어올 이터레이터의 핸들
/// @return 성공 시에는 얻어온 키의 PGL자료 핸들, 실패 시에는 0을 반환합니다.
/// @remark 배열의 이터레이터를 인수로 주었을 경우, 반환되는 값은 0부터 시작하는 정수형의 인덱스입니다.
/// 사전의 이터레이터를 인수로 주었을 경우, 반환되는 값은 현재 이터레이터가 가리키는 키-값 쌍 중 키입니다.
/// 이 함수를 통해 얻어진 PGL자료는 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
DLLEXPORT PGLData PGLD_GetIterationKey(PGLData iter);

/// @fn PGLData PGLD_GetIterationValue(PGLData iter)
/// @brief 이터레이터가 가리키는 키를 얻어옵니다.
/// @param iter 값을 얻어올 이터레이터의 핸들
/// @return 성공 시에는 얻어온 키의 PGL자료 핸들, 실패 시에는 0을 반환합니다.
/// @remark 이 함수를 통해 얻어진 PGL자료는 반드시 @ref PGLD_Release 를 통해 해제되어야 합니다.
DLLEXPORT PGLData PGLD_GetIterationValue(PGLData iter);

/// @fn PGLM PGLT_GetM(PGLThread thread)
/// @brief 스레드 핸들로부터 가상머신의 핸들을 얻어옵니다.
/// @param thread 가상머신의 핸들을 얻어올 스레드의 핸들
/// @return 성공 시에는 가상머신의 핸들을, 실패 시에는 0을 반환합니다.
DLLEXPORT PGLM PGLT_GetM(PGLThread thread);

/// @fn int PGLT_Push(PGLThread thread, PGLData data)
/// @brief 스레드 스택 꼭대기에 새로운 자료를 넣습니다.
/// @param thread 자료를 넣을 스레드의 핸들
/// @param data 넣을 PGL자료의 핸들
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
DLLEXPORT int PGLT_Push(PGLThread thread, PGLData data);

DLLEXPORT int PGLT_PushNull(PGLThread thread);
DLLEXPORT int PGLT_PushInteger(PGLThread thread, int data);
DLLEXPORT int PGLT_PushReal(PGLThread thread, float data);
DLLEXPORT int PGLT_PushString(PGLThread thread, const char* data);
DLLEXPORT int PGLT_PushBoolean(PGLThread thread, int data);
DLLEXPORT int PGLT_PushCFunction(PGLThread thread, PGLCFunction data);
DLLEXPORT int PGLT_PushUserData(PGLThread thread, void* data);

/// @fn int PGLT_Entry(PGLThread thread, int paramCnt)
/// @brief 아직 시작하지 않은 스레드의 진입할 지점을 설정합니다.
/// @param thread 자료를 넣을 스레드의 핸들
/// @param paramCnt 진입 함수로 입력되는 인수의 개수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수를 호출하기에 앞서 먼저 스레드 스택에 진입할 함수의 주소와 그 함수가 받을 인수가 차례로 입력되어 있어야 합니다.
/// 진입 지점을 설정하지 않은 스레드는 전역 코드를 실행합니다.
/// 이 함수는 스레드를 시작시키지 않습니다. 스레드를 시작시키기 위해서는 @ref PGLT_Resume 함수를 호출해야합니다.
DLLEXPORT int PGLT_Entry(PGLThread thread, int paramCnt);

/// @fn int PGLT_Resume(PGLThread thread)
/// @brief 스레드를 시작/재개시킵니다.
/// @param thread 시작할 스레드의 핸들
/// @return 성공 시에는 PGL_OK나 PGL_YIELD, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수를 호출하면 바로 코드가 실행됩니다. 진입 함수 수행을 성공적으로 마친 경우 PGL_OK를 반환하며 실행을 중단합니다.
/// 코드 수행 도중 yield를 만나 코드가 중단된 경우는 PGL_YIELD를 반환합니다. 이 경우는 @ref PGLT_Resume 을 다시 호출하여 실행을 재개할 수 있습니다.
/// 코드 수행 도중 오류가 발생한 경우 PGL_RUNERR를 반환합니다. 이 경우 오류 메세지는 @ref PGLT_GetErrorMsg 를 호출하여 가져올 수 있습니다.
DLLEXPORT int PGLT_Resume(PGLThread thread);

/// @fn int PGLT_GetErrorMsg(PGLThread thread, char* buffer, int bufferSize)
/// @brief 스레드 실행이 실패했을 경우 오류 메세지를 얻어옵니다.
/// @param thread 오류 메세지를 얻어올 스레드의 핸들
/// @param buffer 오류 메세지를 받을 버퍼의 주소
/// @param bufferSize 오류 메세지를 받을 버퍼의 크기
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// buffer로 0을 넣는 경우 오류 메세지를 담는데 필요한 버퍼의 크기를 반환합니다.
/// @remark bufferSize는 오류 메세지를 전부 담을 정도로 충분해야 합니다. 만약 버퍼의 크기가 작을 경우 오류 메세지가 잘릴 수 있습니다.
DLLEXPORT int PGLT_GetErrorMsg(PGLThread thread, char* buffer, int bufferSize);

/// @fn int PGLT_GetParamCnt(PGLThread thread)
/// @brief 스레드 스택에 몇 개의 자료가 쌓여있는지를 조사합니다.
/// @param thread 조사할 스레드의 핸들
/// @return 스택에 쌓여있는 자료의 갯수를 반환한다. 실패 시에는 음수를 반환합니다.
/// @remark PGL함수 호출시 스레드 스택에 쌓여있는 자료의 갯수는 그 함수의 인수 갯수와 일치합니다.
/// 따라서 이 함수는 인수의 갯수를 조사하는 데 사용할 수 있습니다.
DLLEXPORT int PGLT_GetParamCnt(PGLThread thread);

/// @fn PGLData PGLT_GetParam(PGLThread thread, int paramIdx)
/// @brief 스레드 스택에 있는 자료를 가져옵니다.
/// @param thread 자료를 가져올 스레드의 핸들
/// @param paramIdx 가져올 자료의 인덱스
/// @return 성공 시 PGL자료의 핸들, 실패 시 0을 반환합니다.
/// @remark 이 함수로 얻어진 PGL자료는 반드시 @ref PGLD_Release 로 해제되어야 합니다.
/// 자료의 인덱스는 함수 왼쪽에서 0부터 차례로 오른쪽으로 1씩 증가하며 매겨집니다.
DLLEXPORT PGLData PGLT_GetParam(PGLThread thread, int paramIdx);

DLLEXPORT int PGLT_GetParamByInteger(PGLThread thread, int paramIdx);
DLLEXPORT float PGLT_GetParamByReal(PGLThread thread, int paramIdx);
DLLEXPORT const char* PGLT_GetParamByString(PGLThread thread, int paramIdx);
DLLEXPORT int PGLT_GetParamByBoolean(PGLThread thread, int paramIdx);
DLLEXPORT PGLCFunction PGLT_GetParamByCFunction(PGLThread thread, int paramIdx);
DLLEXPORT void* PGLT_GetParamByUserData(PGLThread thread, int paramIdx);

/// @fn int PGLT_Throw(PGLThread thread)
/// @brief 예외를 발생시킵니다.
/// @param thread 예외를 발생시킬 스레드의 핸들
/// @return 예외 발생 후 예외 처리 진입에 성공한 경우는 음수, 예외 처리 진입에 실패하여 프로그램 수행이 중단된 경우에는 PGL_ERROR를 반환합니다.
/// @remark 이 함수를 호출하기에 앞서 예외로 던질 객체가 스택 꼭대기에 입력되어 있어야 합니다.
/// 이 함수는 예외를 던진 이후 예외 처리 진입 지점까지 코드 수행을 계속 이어갑니다.
/// 이 함수가 종료되었을 때, 스레드의 상태는 예외 처리를 시작하려는 경우이거나 예외처리가 불가능하여 종료된 경우 중 하나입니다.
DLLEXPORT int PGLT_Throw(PGLThread thread);

/// @fn int PGLT_MakeArray(PGLThread thread, int n)
/// @brief 스택 꼭대기에 배열을 만듭니다.
/// @param thread 스택을 조작할 스레드의 핸들
/// @param n 배열의 원소의 갯수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수를 호출하기에 앞서 배열의 원소가 될 n개의 자료들이 스택에 입력되어 있어야 합니다.
/// 이 함수는 스택에서 n개의 자료를 제거하고, 1개의 배열을 넣습니다.
DLLEXPORT int PGLT_MakeArray(PGLThread thread, int n);

/// @fn int PGLT_MakeDictionary(PGLThread thread, int n)
/// @brief 스택 꼭대기에 사전을 만듭니다.
/// @param thread 스택을 조작할 스레드의 핸들
/// @param n 사전의 키-값 쌍의 갯수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수를 호출하기에 앞서 배열의 키와 값이 될 n개의 자료들이 각각 스택에 입력되어 있어야 합니다. (총 2*n개)
/// 이 함수는 스택에서 2*n개의 자료를 제거하고, 1개의 사전을 넣습니다.
DLLEXPORT int PGLT_MakeDictionary(PGLThread thread, int n);

/// @fn int PGLT_Receive(PGLThread thread, PGLThread from, int n)
/// @brief 스레드 간에 자료를 주고 받습니다.
/// @param thread 자료를 받을 스레드
/// @param from 자료를 보낼 스레드
/// @param n 옮길 자료의 수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수는 from 스레드 스택에서 n개의 자료를 꺼내어, thread 스레드에 그대로 넣습니다. 자료의 순서는 뒤집히지 않습니다.
DLLEXPORT int PGLT_Receive(PGLThread thread, PGLThread from, int n);

/// @fn int PGLT_SetYieldCF(PGLThread thread, int paramCnt)
/// @brief 스레드가 yield로 중지된 후 다시 시작할 때 진입할 지점을 설정합니다.
/// @param thread 재진입 지점을 설정할 스레드의 핸들
/// @param func 재진입할 함수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수는 PGLight측에서 호출하는 PGLCFunction 함수 내에서 그 함수가 PGL_YIELD를 반환하며 종료될 때만 효과를 발휘합니다.
/// 재진입시 스택의 상태는 yield하기 전 스택의 상태와 동일하므로, yield하기 전에 사용하던 함수 인자와 지역 변수를 그대로 사용할 수 있습니다.
DLLEXPORT int PGLT_SetYieldCF(PGLThread thread, PGLCFunction func);

/// @fn int PGLT_Pop(PGLThread thread, int n)
DLLEXPORT int PGLT_Pop(PGLThread thread, int n);

/// @fn int PGLT_SetReturnCnt(PGLThread thread, int retCnt)
/// @brief 현재 C함수가 PGLight측으로 반환하는 인자의 수를 설정합니다.
/// @param thread 반환하는 인자의 수를 설정할 스레드의 핸들
/// @param retCnt 반환하는 인자의 수
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark 이 함수는 PGLight측에서 호출하는 PGLCFunction 함수 내에서 그 함수가 PGL_OK나 PGL_YIELD를 반환하며 종료될 때만 효과를 발휘합니다.
/// 이 함수를 통해 반환하는 인자의 수를 설정하지 않을 경우 기본값으로 0이 설정됩니다.
DLLEXPORT int PGLT_SetReturnCnt(PGLThread thread, int retCnt);

/// @fn int PGLT_Close(PGLThread thread)
/// @brief 스레드를 종료시킵니다.
/// @param thread 종료시킬 스레드
/// @return 성공 시에는 PGL_OK, 실패 시에는 그 외의 값을 반환합니다.
/// @remark PGL_Close로 가상머신을 종료하기 전에 PGL_NewThread로 생성된 스레드를 이 함수를 통해 모두 닫아야합니다.
DLLEXPORT int PGLT_Close(PGLThread thread);

#ifdef __cplusplus
};
#endif