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

#include <usb/composite.h>


#define MAX_U_WCN_INTERFACES	3
#define MAX_FUNC_NAME_LEN 16

const char f_inf_name[MAX_U_WCN_INTERFACES][MAX_FUNC_NAME_LEN] = {
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

};
struct f_wcn_dev wcn_usb_dev;


/* utilities to allocate/free request and buffer */
struct usb_request *gs_alloc_req(struct usb_ep *ep, unsigned len, gfp_t flags);
void gs_free_req(struct usb_ep *, struct usb_request *req);

/* management of individual TTY ports */
int gserial_alloc_line(unsigned char *port_line);
void gserial_free_line(unsigned char port_line);

/* connect/disconnect is handled by individual functions */
int gserial_connect(struct gserial *, u8 port_num);
void gserial_disconnect(struct gserial *);

/* functions are bound to configurations by a config or gadget driver */
int gser_bind_config(struct usb_configuration *c, u8 port_num);
int obex_bind_config(struct usb_configuration *c, u8 port_num);

#endif /* __U_SERIAL_H */
