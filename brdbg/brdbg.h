/***********************************************************************************
 *	Debugging and printing related definitions and prototypes
 ***********************************************************************************/
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

/* Put platform dependent declaration here */
/* For example, linux type definition */
typedef char INT8;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef short INT16;
typedef int INT32;
typedef long long INT64;

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;

typedef unsigned char *PUINT8;
typedef unsigned short *PUINT16;
typedef unsigned int *PUINT32;
typedef unsigned long long *PUINT64;
typedef int *PINT32;
typedef long long *PINT64;

/* modified for fixing compile warning on Sigma 8634 platform */
//typedef char STRING;
typedef char RTMP_STRING;

typedef signed char CHAR;

typedef signed short SHORT;
typedef signed int INT;
typedef signed long LONG;
typedef signed long long LONGLONG;

typedef unsigned long long ULONGLONG;

typedef unsigned char BOOLEAN;

typedef void VOID;
typedef VOID *PVOID;
typedef CHAR *PCHAR;
typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef LONG *PLONG;
typedef ULONG *PULONG;
typedef UINT *PUINT;