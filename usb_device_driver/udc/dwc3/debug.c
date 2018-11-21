/**
 * debug.c - DesignWare USB3 DRD Controller Debug/Trace Support
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "debug.h"
#include "gadget.h"


void trace_dwc3_prepare_trb(struct dwc3_ep *dep,struct dwc3_trb *trb)
{

}

void trace_dwc3_complete_trb(struct dwc3_ep *dep,struct dwc3_trb *trb)
{

}

void trace_dwc3_free_request(struct dwc3_ep *dep,struct dwc3_trb *trb)
{

}

void trace_dwc3_ctrl_req(struct usb_ctrlrequest *ctrl)
{

}
void trace_dwc3_ep0(struct va_format *vaf)
{

}

void trace_dwc3_gadget(struct va_format *vaf)
{

}

void trace_dwc3_readl(struct va_format *vaf)
{

}

void trace_dwc3_writel(struct va_format *vaf)
{

}

void dwc3_trace(void (*trace)(struct va_format *), const char *fmt, ...)
{
	
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	trace(&vaf);

	va_end(args);
}
