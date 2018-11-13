
/*
 * f_serial.c - generic USB serial function driver
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */




/* notification endpoint uses smallish and infrequent fixed-size messages */

#define NOTIFY_INTERVAL_MS		32
#define NOTIFY_MAXPACKET		10	/* notification + 2 bytes */

#define MAX_SINGLE_DIR_EPS 15;


/*
 * This function packages a simple "generic serial" port with no real
 * control mechanisms, just raw data transfer over two bulk endpoints.
 *
 * Because it's not standardized, this isn't as interoperable as the
 * CDC ACM driver.  However, for many purposes it's just as functional
 * if you can arrange appropriate host side drivers.
 */

struct wcn_func_bt0{
	struct usb_function		func;
	struct usb_ep			*int_in;
	struct usb_ep			*bulk_out;
	struct usb_ep			*bulk_in;
};

struct wcn_func_bt1{
	struct usb_function		func;
	struct usb_ep			*isoc_out;
	struct usb_ep			*isoc_in;
};

struct wcn_func_wifi{
	struct usb_function		func;
	struct usb_ep			*in;
	struct usb_ep			*out;
};


struct f_wcn_dev{
	/*inf 0*/
	struct wcn_func_bt0 *wcn_bt0;
	/*inf 1*/
	struct wcn_func_bt1 *wcn_bt1;
	/*inf 2*/
	struct wcn_func_wifi *wcn_wifi;

	struct usb_ep	 *in_ep[MAX_SINGLE_DIR_EPS];
	struct usb_ep	 *out_ep[MAX_SINGLE_DIR_EPS];

};
struct f_wcn_dev wcn_usb_dev;

/*-------------------------------------------------------------------------*/

/* interface descriptor: */

static struct usb_interface_descriptor bt0_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	3,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

static struct usb_interface_descriptor bt1_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

static struct usb_interface_descriptor wifi_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	9,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};


/* bt 0 interface */
/* full speed support: */
static struct usb_endpoint_descriptor bt0_fs_int_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(NOTIFY_MAXPACKET),
	.bInterval =		USB_MS_TO_HS_INTERVAL(NOTIFY_INTERVAL_MS),
};

static struct usb_endpoint_descriptor bt0_fs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor bt0_fs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *bt0_fs_function[] = {
	(struct usb_descriptor_header *) &bt0_interface_desc,
	(struct usb_descriptor_header *) &bt0_fs_int_in_desc,
	(struct usb_descriptor_header *) &bt0_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &bt0_fs_bulk_in_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor bt0_hs_int_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(NOTIFY_MAXPACKET),
	.bInterval =		USB_MS_TO_HS_INTERVAL(NOTIFY_INTERVAL_MS),
};

static struct usb_endpoint_descriptor bt0_hs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};
static struct usb_endpoint_descriptor bt0_hs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *bt0_hs_function[] = {
	(struct usb_descriptor_header *) &bt0_interface_desc,
	(struct usb_descriptor_header *) &bt0_hs_int_in_desc,
	(struct usb_descriptor_header *) &bt0_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &bt0_hs_bulk_in_desc,
	NULL,
};

static struct usb_endpoint_descriptor bt0_ss_int_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor bt0_ss_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor bt0_ss_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};


static struct usb_ss_ep_comp_descriptor bt0_ss_bulk_comp_desc = {
	.bLength =              sizeof bt0_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *bt0_ss_function[] = {
	(struct usb_descriptor_header *) &bt0_interface_desc,
	(struct usb_descriptor_header *) &bt0_ss_int_in_desc,
	(struct usb_descriptor_header *) &bt0_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &bt0_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &bt0_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &bt0_ss_bulk_in_desc,
	(struct usb_descriptor_header *) &bt0_ss_bulk_comp_desc,
	NULL,
};

/* bt 1 interface */

/* full speed support: */
static struct usb_endpoint_descriptor bt1_fs_isoc_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};

static struct usb_endpoint_descriptor bt1_fs_isoc_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};


static struct usb_descriptor_header *bt1_fs_function[] = {
	(struct usb_descriptor_header *) &bt1_interface_desc,
	(struct usb_descriptor_header *) &bt1_fs_isoc_out_desc,
	(struct usb_descriptor_header *) &bt1_fs_isoc_in_desc,
	NULL,
};

/* high speed support: */
static struct usb_endpoint_descriptor bt1_hs_isoc_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};

static struct usb_endpoint_descriptor bt1_hs_isoc_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};


static struct usb_descriptor_header *bt1_hs_function[] = {
	(struct usb_descriptor_header *) &bt1_interface_desc,
	(struct usb_descriptor_header *) &bt1_hs_isoc_out_desc,
	(struct usb_descriptor_header *) &bt1_hs_isoc_in_desc,
	NULL,
};

/* super speed support: */

static struct usb_endpoint_descriptor bt1_ss_isoc_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};

static struct usb_endpoint_descriptor bt1_ss_isoc_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
};


static struct usb_ss_ep_comp_descriptor bt1_ss_bulk_comp_desc = {
	.bLength =              sizeof bt1_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *bt1_ss_function[] = {
	(struct usb_descriptor_header *) &bt1_interface_desc,
	(struct usb_descriptor_header *) &bt1_ss_isoc_out_desc,
	(struct usb_descriptor_header *) &bt1_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &bt1_ss_isoc_in_desc,
	(struct usb_descriptor_header *) &bt1_ss_bulk_comp_desc,
	NULL,
};


/* wifi interface */
/* full speed support: */
static struct usb_endpoint_descriptor wifi_fs_bulk_out_desc[7] = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor wifi_fs_bulk_in_desc[3] = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *wifi_fs_function[] = {
	(struct usb_descriptor_header *) &wifi_interface_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc[0],
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc[1],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[0],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[1],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[2],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[3],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[4],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[5],
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc[2],
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc[6],
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor wifi_hs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor wifi_hs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *wifi_hs_function[] = {
	(struct usb_descriptor_header *) &wifi_interface_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc[0],
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc[1],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[0],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[1],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[2],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[3],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[4],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[5],
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc[2],
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc[6],
	NULL,
};


static struct usb_endpoint_descriptor wifi_ss_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor wifi_ss_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};


static struct usb_ss_ep_comp_descriptor wifi_ss_bulk_comp_desc[10] = {
	.bLength =              sizeof wifi_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *wifi_ss_function[] = {
	(struct usb_descriptor_header *) &wifi_interface_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc[0],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc[1],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[0],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[1],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[2],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[3],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[4],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[5],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc[2],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc[6],
	(struct usb_descriptor_header *) &wifi_ss_bulk_comp_desc,
	NULL,
};

/* string descriptors: */

static struct usb_string wcn_string_defs[] = {
	[0].s = "Wcn Composite",
	{  } /* end of list */
};

static struct usb_gadget_strings wcn_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		wcn_string_defs,
};

static struct usb_gadget_strings *wcn_strings[] = {
	&wcn_string_table,
	NULL,
};


static inline struct f_gser *func_to_dev(struct usb_function *f)
{
	return container_of(f, struct f_gser, port.func);
}


/*-------------------------------------------------------------------------*/

inline struct usb_ep *wcn_ep_get(u8 ep_addr)
{
	if(ep_addr & USB_DIR_IN)
		return wcn_usb_dev->in_ep[UE_GET_ADDR(ep_addr)];
	else
		return wcn_usb_dev->out_ep[UE_GET_ADDR(ep_addr)];
}
static int wcn_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_gser		*gser = func_to_gser(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt == 0, so this is an activation or a reset */

	if (gser->port.in->enabled) {
		dev_dbg(&cdev->gadget->dev,
			"reset generic ttyGS%d\n", gser->port_num);
		gserial_disconnect(&gser->port);
	}
	if (!gser->port.in->desc || !gser->port.out->desc) {
		dev_dbg(&cdev->gadget->dev,
			"activate generic ttyGS%d\n", gser->port_num);
		if (config_ep_by_speed(cdev->gadget, f, gser->port.in) ||
		    config_ep_by_speed(cdev->gadget, f, gser->port.out)) {
			gser->port.in->desc = NULL;
			gser->port.out->desc = NULL;
			return -EINVAL;
		}
	}
	gserial_connect(&gser->port, gser->port_num);
	return 0;
}

static void wcn_disable(struct usb_function *f)
{
	struct f_gser	*gser = func_to_gser(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	dev_dbg(&cdev->gadget->dev,
		"generic ttyGS%d deactivated\n", gser->port_num);
	gserial_disconnect(&gser->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static void wcn_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
}

/*
 * To compatible with our company's usb-to-serial driver, we need to handle the
 * request defining the bRequestType = 0x21 and bRequest = 0x22 to make the
 * usb-to-serial driver work well.
 */
static int wcn_setup(struct usb_function *f,
		      const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	u16 w_length = le16_to_cpu(ctrl->wLength);
	int value = -EOPNOTSUPP;

	/* Handle Bulk-only class-specific requests */
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS) {
		switch (ctrl->bRequest) {
		case 0x22:
			value = 0;
			break;
		default:
			break;
		}
	}

	/* Respond with data transfer or status phase? */
	if (value >= 0) {
		int rc;

		cdev->req->zero = value < w_length;
		cdev->req->length = value;
		cdev->req->complete = gser_setup_complete;
		rc = usb_ep_queue(cdev->gadget->ep0, cdev->req, GFP_ATOMIC);
		if (rc < 0)
			dev_err(&cdev->gadget->dev,
				"setup response queue error\n");
	}

	if (value == -EOPNOTSUPP) {
		dev_warn(&cdev->gadget->dev,
			 "unknown class-specific control req "
			 "%02x.%02x v%04x i%04x l%u\n",
			 ctrl->bRequestType, ctrl->bRequest,
			 le16_to_cpu(ctrl->wValue), le16_to_cpu(ctrl->wIndex),
			 le16_to_cpu(ctrl->wLength));
	}

	return value;
}

static int wcn_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	int			status, i;
	struct usb_ep		*ep;
	u8 addr;

	/* REVISIT might want instance-specific strings to help
	 * distinguish instances ...
	 */

	/* maybe allocate device-global string ID */
	if (wcn_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		wcn_string_defs[0].id = status;
	}

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;

	if(!strncmp(f->name,"wcn_bt0",strlen("wcn_bt0"))){
		
		bt0_interface_desc.bInterfaceNumber = status;
		/* allocate instance-specific endpoints */
		ep = usb_ep_autoconfig(cdev->gadget, &bt0_fs_int_in_desc);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_bt0->int_in = ep;
		addr = ep->address & 0xf;
		wcn_usb_dev->in_ep[addr] = ep;

		ep = usb_ep_autoconfig(cdev->gadget, &bt0_fs_bulk_out_desc);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_bt0->bulk_out = ep;
		addr = ep->address;
		wcn_usb_dev->out_ep[addr] = ep;
		
		ep = usb_ep_autoconfig(cdev->gadget, &bt0_fs_bulk_in_desc);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_bt0->bulk_in = ep;
		addr = ep->address & 0xf;
		wcn_usb_dev->in_ep[addr] = ep;

		/* support all relevant hardware speeds... we expect that when
		 * hardware is dual speed, all bulk-capable endpoints work at
		 * both speeds
		 */
		bt0_hs_int_in_desc.bEndpointAddress = bt0_fs_int_in_desc.bEndpointAddress;
		bt0_hs_bulk_out_desc.bEndpointAddress = bt0_fs_bulk_out_desc.bEndpointAddress;
		bt0_hs_bulk_in_desc.bEndpointAddress = bt0_fs_bulk_in_desc.bEndpointAddress;

		bt0_ss_int_in_desc.bEndpointAddress = bt0_fs_int_in_desc.bEndpointAddress;
		bt0_ss_bulk_out_desc.bEndpointAddress = bt0_fs_bulk_out_desc.bEndpointAddress;
		bt0_ss_bulk_in_desc.bEndpointAddress = bt0_fs_bulk_in_desc.bEndpointAddress;


		status = usb_assign_descriptors(f, bt0_fs_function, bt0_hs_function,
				bt0_ss_function);
		if (status)
			goto fail;
	}else if(!strncmp(name,"wcn_bt1",strlen("wcn_bt1"))){
		bt1_interface_desc.bInterfaceNumber = status;
		/* allocate instance-specific endpoints */
		ep = usb_ep_autoconfig(cdev->gadget, &bt1_fs_isoc_out_desc);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_bt1->isoc_out = ep;
		addr = ep->address;
		wcn_usb_dev->out_ep[addr] = ep;

		ep = usb_ep_autoconfig(cdev->gadget, &bt1_fs_isoc_in_desc);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_bt0->isoc_in = ep;
		addr = ep->address & 0xf;
		wcn_usb_dev->in_ep[addr] = ep;

		/* support all relevant hardware speeds... we expect that when
		 * hardware is dual speed, all bulk-capable endpoints work at
		 * both speeds
		 */
		bt1_hs_isoc_out_desc.bEndpointAddress = bt1_fs_isoc_out_desc.bEndpointAddress;
		bt1_hs_isoc_in_desc.bEndpointAddress = bt1_fs_isoc_in_desc.bEndpointAddress;

		bt1_ss_isoc_out_desc.bEndpointAddress = bt0_fs_int_in_desc.bEndpointAddress;
		bt1_ss_isoc_in_desc.bEndpointAddress = bt0_fs_bulk_out_desc.bEndpointAddress;

		status = usb_assign_descriptors(f, bt1_fs_function, bt1_hs_function,
				bt1_ss_function);
		if (status)
			goto fail;
	}
	else {
		wifi_interface_desc.bInterfaceNumber = status;
		for(i = 0; i < 2; i++)
		{
			ep = usb_ep_autoconfig(cdev->gadget, &wifi_fs_bulk_in_desc[i]);
			if (!ep)
				goto fail;
			addr = ep->address & 0xf;
			wcn_usb_dev->in_ep[addr] = ep;

			/* support all relevant hardware speeds... we expect that when
			 * hardware is dual speed, all bulk-capable endpoints work at
			 * both speeds
			 */
			wifi_hs_bulk_in_desc[i].bEndpointAddress = wifi_fs_bulk_in_desc[i].bEndpointAddress;
			wifi_ss_bulk_in_desc[i].bEndpointAddress = wifi_fs_bulk_in_desc[i].bEndpointAddress;
		}

		for(i = 0; i < 6; i++)
		{
			ep = usb_ep_autoconfig(cdev->gadget, &wifi_fs_bulk_out_desc[i]);
			if (!ep)
				goto fail;
			addr = ep->address;
			wcn_usb_dev->out_ep[addr] = ep;
			wifi_hs_bulk_out_desc[i].bEndpointAddress = wifi_fs_bulk_out_desc[i].bEndpointAddress;
			wifi_ss_bulk_out_desc[i].bEndpointAddress = wifi_fs_bulk_out_desc[i].bEndpointAddress;
		}
		
		/* allocate instance-specific endpoints */
		ep = usb_ep_autoconfig(cdev->gadget, &wifi_fs_bulk_in_desc[2]);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_wifi->int_in = ep;
		addr = ep->address & 0xf;
		wcn_usb_dev->in_ep[addr] = ep;
		wifi_hs_bulk_out_desc[2].bEndpointAddress = wifi_fs_bulk_in_desc[2].bEndpointAddress;
		wifi_ss_bulk_out_desc[2].bEndpointAddress = wifi_fs_bulk_in_desc[2].bEndpointAddress;

		ep = usb_ep_autoconfig(cdev->gadget, &wifi_fs_bulk_out_desc[6]);
		if (!ep)
			goto fail;
		wcn_usb_dev->wcn_wifi->bulk_out = ep;
		addr = ep->address;
		wcn_usb_dev->out_ep[addr] = ep;
		wifi_hs_bulk_out_desc[6].bEndpointAddress = wifi_fs_bulk_out_desc[6].bEndpointAddress;
		wifi_ss_bulk_out_desc[6].bEndpointAddress = wifi_fs_bulk_out_desc[6].bEndpointAddress;
		
	
		status = usb_assign_descriptors(f, wifi_fs_function, wifi_hs_function,
				wifi_ss_function);
		if (status)
			goto fail;
	}


	status = -ENODEV;

	return 0;

fail:
	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void wcn_free(struct usb_function *f)
{
	struct f_gser *serial;

	serial = func_to_gser(f);
	kfree(serial);
}

static void wcn_unbind(struct usb_configuration *c, struct usb_function *f)
{
	usb_free_all_descriptors(f);
}

static struct usb_function *wcn_func_alloc(const char * name)
{
	struct wcn_func_bt0 *wcn_bt0;
	struct wcn_func_bt1 *wcn_bt1;
	struct wcn_func_wifi *wcn_wifi;

	struct usb_function	*func;

	/* allocate and initialize one new instance */
	if(!strncmp(name,"wcn_bt0",strlen("wcn_bt0"))){
		
		wcn_bt0= usb_malloc(sizeof(struct wcn_func_bt0));
		if (!wcn_bt0)
			return ERR_PTR(-ENOMEM);
		wcn_usb_dev.wcn_bt0 = wcn_bt0;
		func = &wcn_bt0->func;
	} else if(!strncmp(name,"wcn_bt1",strlen("wcn_bt1"))){
		
		wcn_bt1= usb_malloc(sizeof(struct wcn_func_wifi));
		if (!wcn_bt1)
			return ERR_PTR(-ENOMEM);
		wcn_usb_dev.wcn_bt1 = wcn_bt1;
		func = &wcn_bt1->func;
	} else {
		wcn_wifi= usb_malloc(sizeof(struct wcn_func_wifi));
		if (!wcn_wifi)
			return ERR_PTR(-ENOMEM);
		wcn_usb_dev.wcn_wifi = wcn_wifi;
		func = &wcn_wifi->func;
	}
	func->name = func->fd->name;
	func->strings = wcn_strings;
	func->bind = wcn_bind;
	func->unbind = wcn_unbind;
	func->set_alt = wcn_set_alt;
	func->setup = wcn_setup;
	func->disable = wcn_disable;
	func->free_func = wcn_free;

	return func;
}

static struct usb_function_driver wcn_usb_func[MAX_U_WCN_INTERFACES] = {
	{
	.name = __stringify("wcn_bt0"),
	.alloc_func = wcn_func_alloc,
	},
	{
	.name = __stringify("wcn_bt1"),
	.alloc_func = wcn_func_alloc,
	},
	{
	.name = __stringify("wcn_wifi"),
	.alloc_func = wcn_func_alloc,
	},
};	


static int  wcn_func_init(u8 idx)
{
	return usb_function_register(&wcn_usb_func[idx]);
}								\
static void  wcn_func_exit(u8 idx)
{
	usb_function_unregister(&wcn_usb_func[idx]);
}
