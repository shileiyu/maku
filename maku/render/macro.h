#ifndef GOLD_COMMON_MACRO_H_
#define GOLD_COMMON_MACRO_H_

#include <tchar.h>

#ifndef SAFE_DELETE
#	define SAFE_DELETE(p) {if (NULL != (p)) {delete (p); (p) = NULL;}}
#endif

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE(p) {if (NULL != (p)) {(p)->Release(); (p) = NULL;}}
#endif	

#ifndef SAFE_ADDREF
#	define SAFE_ADDREF(p) {if (NULL != (p)) {(p)->AddRef();}}
#endif	

#ifndef SAFE_FREE
#	define SAFE_FREE(p) {if (NULL != (p)) {free((p)); (p) = NULL;}}
#endif	

#ifndef OUTPUT_DEBUGSTR
#define OUTPUT_DEBUGSTR(str_format,p1,p2,p3,p4) \
	{\
        TCHAR _str_fmt[250]={0};\
        _stprintf_s((TCHAR*)_str_fmt,250-1, _T("Gold->%s"), str_format);\
		TCHAR _str[250]={0};\
		_stprintf_s((TCHAR*)_str,250-1,_str_fmt,p1,p2,p3,p4);\
		OutputDebugString(_str);\
	}
#endif

#ifndef OUTPUT_MSGBOX
#define OUTPUT_MSGBOX(str_format,param) \
	{\
	TCHAR _str[250]={0};\
	_stprintf_s((TCHAR*)_str,250-1,str_format,param);\
	MessageBox(0, _str, _T("DebugMessage"), MB_OK);\
	}
#endif

#ifndef tstring
#	ifdef _UNICODE
#		define tstring wstring
#	else
#		define tstring string
#	endif
#endif

#endif

