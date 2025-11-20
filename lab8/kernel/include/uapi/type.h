#ifndef _UAPI_TYPE_H
#define _UAPI_TYPE_H

typedef unsigned int 		uint32_t;
typedef unsigned long long 	uint64_t;
typedef int 				int32_t;
typedef long long			int64_t;
typedef unsigned char		byte;
typedef unsigned long long	size_t;
typedef int					bool;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;

#define TRUE	1
#define FALSE	0

#define INT64_MAX	(9223372036854775807LL)
#define INT64_MIN	(-9223372036854775807LL - 1)

#define UINT64_MAX	0xFFFFFFFFFFFFFFFF

#ifndef NULL
#define NULL ((void*)0)
#endif



#endif