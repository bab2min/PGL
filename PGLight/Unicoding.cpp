#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include "Unicoding.h"

using namespace std;

wstring utf8_to_utf16(const char* utf8)
{
	vector<wchar_t> utf16;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
	if(len > 1)
	{
		utf16.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &utf16[0], len);
	}
	return wstring(utf16.begin(), utf16.end());
}

string utf16_to_utf8(const wchar_t* utf16)
{
	vector<char> utf8;
	int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, nullptr, 0, nullptr, nullptr);
	if(len > 1)
	{
		utf8.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, utf16, -1, &utf8[0], len, nullptr, nullptr);
	}
	if(!utf8.empty() && utf8.back() == 0)utf8.pop_back();
	return string(utf8.begin(), utf8.end());
}