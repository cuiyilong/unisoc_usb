#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "os_api.h"
#include "types.h"

#define MAX_ERRNO	4095

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void * ERR_PTR(long error)
{
	//return (void *) error;
	return NULL;
}

static inline long PTR_ERR( const void *ptr)
{
	return (long) ptr;
}

static inline bool  IS_ERR( const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline bool  IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)ptr);
}

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

void trace_dwc3_gadget(struct va_format *vaf);
void trace_dwc3_writel(struct va_format *vaf);
void trace_dwc3_readl(struct va_format *vaf);
void trace_dwc3_ep0(struct va_format *vaf);

#endif
