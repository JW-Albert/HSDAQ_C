#ifndef WINDOWS2LINUX_HPP_
#define WINDOWS2LINUX_HPP_

//#define sprintf_s snprintf
#define INVALID_HANDLE_VALUE -1
#define _MAX_PATH 260 /* max. length of full pathname */
//#define HANDLE int
//#define HANDLE intptr_t
#define HANDLE uintptr_t
#define MAX_PATH 260
#define TRUE true
#define FALSE false
#define __stdcall
#define __declspec(x)
#define __cdecl
//#define max(a,b) (((a) > (b)) ? (a) : (b))
//#define min(a,b) (((a) < (b)) ? (a) : (b))

#ifndef _MSW_H
    typedef int BOOL;         
    typedef unsigned char BYTE;
    typedef unsigned char UCHAR;
    typedef unsigned short WORD;
    typedef unsigned int DWORD;
       
#endif      

typedef float FLOAT;
typedef FLOAT *PFLOAT;
typedef char CHAR; 

typedef unsigned char *PUCHAR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef unsigned short *PUSHORT;
typedef long LONG;
 
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef ULONGLONG *PULONGLONG;
typedef unsigned long ULONG;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int *PUINT;
typedef void VOID;
typedef char *LPSTR;
typedef char *LPTSTR;

typedef const char *LPCTSTR;
typedef const char *LPCSTR;
typedef wchar_t WCHAR;
typedef WCHAR *LPWSTR;
typedef const WCHAR *LPCWSTR;
typedef DWORD *LPDWORD;
typedef unsigned long UINT_PTR;
typedef UINT_PTR SIZE_T;
typedef LONGLONG USN;
typedef BYTE BOOLEAN;
typedef void *PVOID;

typedef void *LPVOID;


#define TCHAR wchar_t 

typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME;

typedef union _ULARGE_INTEGER {
	struct {
		DWORD LowPart;
		DWORD HighPart;
	};
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;
	ULONGLONG QuadPart;
} ULARGE_INTEGER,
	*PULARGE_INTEGER;


#endif /* WINDOWS2LINUX_HPP_ */
