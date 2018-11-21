#ifndef _TOOLS_TYPES_H_
#define _TOOLS_TYPES_H_

#include "sci_types.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


#include "os_api.h"



typedef signed char __s8;
typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

typedef signed long __s64;
typedef unsigned long __u64;


/*
 * We define u64 as uint64_t for every architecture
 * so that we can print it with "%"PRIx64 without getting warnings.
 *
 * typedef __u64 u64;
 * typedef __s64 s64;
 */
typedef __u64 u64;
typedef __s64 s64;

typedef __u32 u32;
typedef __s32 s32;

typedef __u16 u16;
typedef __s16 s16;

typedef __u8  u8;
typedef __s8  s8;

typedef __u16 __le16;
typedef __u32 __le32;
typedef __u64 __le64;

typedef  BOOLEAN bool;
#if 0
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

#define __force
#define __user
#define __must_check
#define __cold

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif
typedef struct {
	int counter;
} atomic_t;



#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((u32)(n))

#define __iomem /*  */


/* for arm default little endian */
#define cpu_to_le16(x) x
#define le16_to_cpu(x) x
#define __le16_to_cpu(x) x


#define cpu_to_le64(x)	(x)
#define cpu_to_le32(x)	(x)

static inline void le16_add_cpu(__le16 *var, u16 val)
{
	*var = cpu_to_le16(le16_to_cpu(*var) + val);
}

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif


#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })


/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))


#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

#define	EINPROGRESS 36	/* Operation now in progress */
#define	EOPNOTSUPP	45	/* Operation not supported on transport endpoint */
#define	ECONNRESET	54	/* Connection reset by peer */
#define	ESHUTDOWN	58	/* No send after transport endpoint shutdown */
#define	ETIMEDOUT	60	/* timed out */

#define	ECONNABORTED	61	/* timed out */
#define	EOVERFLOW 62

#define	ENULLPOINTER 63


#define	ENOTSUPP 512

enum {
	false	= 0,
	true	= 1
};




#define dev_err(format, args...) SCI_TRACE_LOW(format, ## args)
#define dev_dbg(format, args...) SCI_TRACE_LOW(format, ## args)
#define dev_info(format, args...) SCI_TRACE_LOW(format, ## args)
#define dev_warn(format, args...) SCI_TRACE_LOW(format, ## args)


#define WARN_ON(x) 
#define WARN_ON_ONCE(x)

struct va_format {
	const char *fmt;
	va_list *va;
};


#endif /* _TOOLS_LINUX_TYPES_H_ */
