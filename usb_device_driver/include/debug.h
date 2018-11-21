#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "os_api.h"
#include "types.h"


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
