/*
 * u_serial.h - interface to USB gadget "serial port"/TTY utilities
 *
 * Copyright (C) 2008 David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef __U_WCN_COMPOSITE_H
#define __U_WCN_COMPOSITE_H

#include "composite.h"


#define MAX_U_WCN_INTERFACES	3
#define MAX_FUNC_NAME_LEN 16


#define IN_CHN_TO_EP_ADDR(chn) (chn - 0x10 + 0x80) 
#define OUT_CHN_TO_EP_ADDR(chn) chn 

#define IN_EP_ADDR_TO_CHN(addr) (addr - 0x80 + 0x10) 
#define OUT_EP_ADDR_TO_CHN(addr) addr

#define MAX_SINGLE_DIR_EPS 15


enum {
	USB_CHN_ERR = -1,
	USB_TX_BUSY = -2,
	USB_RX_BUSY = -3,
};


static const char f_inf_name[MAX_U_WCN_INTERFACES][MAX_FUNC_NAME_LEN] = {
"wcn_bt0",
"wcn_bt1",
"wcn_wifi"
};

struct f_wcn_bt0{
	struct usb_function		func;
	struct usb_ep			*int_in;
	struct usb_ep			*bulk_out;
	struct usb_ep			*bulk_in;
	struct list_head	int_tx_reqs, bulk_tx_reqs, rx_reqs;
	unsigned		qmult;/* default 2 */
};

struct f_wcn_bt1{
	struct usb_function		func;
	struct usb_ep			*isoc_out;
	struct usb_ep			*isoc_in;
	struct list_head	tx_reqs, rx_reqs;
	unsigned		qmult;/* default 2 */
};

struct f_wcn_wifi{
	struct usb_function		func;
	struct usb_ep			*bulk_in[3];
	struct usb_ep			*bulk_out[7];
	struct list_head	tx_reqs[3], rx_reqs[7];
	unsigned		qmult;/* default 10 */
};


struct f_wcn_dev{
	struct usb_gadget	*gadget;
	/* inf 0 */
	struct f_wcn_bt0 *wcn_bt0;
	/* inf 1 */
	struct f_wcn_bt1 *wcn_bt1;
	/* inf 2 */
	struct f_wcn_wifi *wcn_wifi;

	struct usb_ep	 *in_ep[MAX_SINGLE_DIR_EPS];
	struct usb_ep	 *out_ep[MAX_SINGLE_DIR_EPS];

	struct list_head	*in_req[MAX_SINGLE_DIR_EPS];
	struct list_head	*out_req[MAX_SINGLE_DIR_EPS];
	/* func rx buf list */
	struct list_head	rx_buf_head[MAX_U_WCN_INTERFACES];
};



struct rx_buf_node {
	struct list_head	list;
	cpdu_t	*buf_head;
	cpdu_t	*buf_tail;
	unsigned	buf_num;
	unsigned chn;
};

extern struct f_wcn_dev *wcn_usb_dev;

int gwcn_bt0_connect(struct f_wcn_bt0 *wcn_bt0);
void gwcn_bt0_disconnect(struct f_wcn_bt0 *wcn_bt0);

int gwcn_bt1_connect(struct f_wcn_bt1 *wcn_bt1);
void gwcn_bt1_disconnect(struct f_wcn_bt1 *wcn_bt1);

int gwcn_wifi_connect(struct f_wcn_wifi *wcn_wifi);
void gwcn_wifi_disconnect(struct f_wcn_wifi *wcn_wifi);

int usb_wcn_start_xmit(int chn, cpdu_t *head, cpdu_t *tail, int num);

int gwcn_task_init(void);


/* utilities to allocate/free request and buffer */

#endif /* __U_WCN_COMPOSITE_H */
