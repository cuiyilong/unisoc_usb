
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



#include "u_serial.h"


/* notification endpoint uses smallish and infrequent fixed-size messages */

#define NOTIFY_INTERVAL_MS		32
#define NOTIFY_MAXPACKET		10	/* notification + 2 bytes */


/*
 * This function packages a simple "generic serial" port with no real
 * control mechanisms, just raw data transfer over two bulk endpoints.
 *
 * Because it's not standardized, this isn't as interoperable as the
 * CDC ACM driver.  However, for many purposes it's just as functional
 * if you can arrange appropriate host side drivers.
 */

struct f_wcn_bt0 {
	struct gserial			port;
	u8				data_id;
	u8				port_num;
};

static inline struct f_gser *func_to_gser(struct usb_function *f)
{
	return container_of(f, struct f_gser, port.func);
}


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
	struct wcn_func_bt0 wcn_bt0;
	/*inf 1*/
	struct wcn_func_bt1 wcn_bt1;
	/*inf 2*/
	struct wcn_func_wifi wcn_wifi;

};


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
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};
static struct usb_endpoint_descriptor bt0_hs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
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
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor bt0_ss_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor bt0_ss_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
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
static struct usb_endpoint_descriptor wifi_fs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor wifi_fs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *wifi_fs_function[] = {
	(struct usb_descriptor_header *) &wifi_interface_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_fs_bulk_out_desc,
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
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_hs_bulk_out_desc,
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


static struct usb_ss_ep_comp_descriptor wifi_ss_bulk_comp_desc = {
	.bLength =              sizeof wifi_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *wifi_ss_function[] = {
	(struct usb_descriptor_header *) &wifi_interface_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_in_desc,
	(struct usb_descriptor_header *) &wifi_ss_bulk_out_desc,
	NULL,
};

/* string descriptors: */

static struct usb_string gser_string_defs[] = {
	[0].s = "Generic Serial",
	{  } /* end of list */
};

static struct usb_gadget_strings gser_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		gser_string_defs,
};

static struct usb_gadget_strings *gser_strings[] = {
	&gser_string_table,
	NULL,
};

/*-------------------------------------------------------------------------*/

static int gser_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
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

static void gser_disable(struct usb_function *f)
{
	struct f_gser	*gser = func_to_gser(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	dev_dbg(&cdev->gadget->dev,
		"generic ttyGS%d deactivated\n", gser->port_num);
	gserial_disconnect(&gser->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static void gser_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
}

/*
 * To compatible with our company's usb-to-serial driver, we need to handle the
 * request defining the bRequestType = 0x21 and bRequest = 0x22 to make the
 * usb-to-serial driver work well.
 */
static int gser_setup(struct usb_function *f,
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
	struct f_gser		*gser = func_to_gser(f);
	int			status;
	struct usb_ep		*ep;

	/* REVISIT might want instance-specific strings to help
	 * distinguish instances ...
	 */

	/* maybe allocate device-global string ID */
	if (gser_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		gser_string_defs[0].id = status;
	}

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	gser->data_id = status;
	gser_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &gser_fs_in_desc);
	if (!ep)
		goto fail;
	gser->port.in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &gser_fs_out_desc);
	if (!ep)
		goto fail;
	gser->port.out = ep;

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	gser_hs_in_desc.bEndpointAddress = gser_fs_in_desc.bEndpointAddress;
	gser_hs_out_desc.bEndpointAddress = gser_fs_out_desc.bEndpointAddress;

	gser_ss_in_desc.bEndpointAddress = gser_fs_in_desc.bEndpointAddress;
	gser_ss_out_desc.bEndpointAddress = gser_fs_out_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, gser_fs_function, gser_hs_function,
			gser_ss_function);
	if (status)
		goto fail;
	dev_dbg(&cdev->gadget->dev, "generic ttyGS%d: %s speed IN/%s OUT/%s\n",
		gser->port_num,
		gadget_is_superspeed(c->cdev->gadget) ? "super" :
		gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
		gser->port.in->name, gser->port.out->name);
	return 0;

fail:
	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static inline struct f_serial_opts *to_f_serial_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct f_serial_opts,
			    func_inst.group);
}

static void serial_attr_release(struct config_item *item)
{
	struct f_serial_opts *opts = to_f_serial_opts(item);

	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations serial_item_ops = {
	.release	= serial_attr_release,
};

static ssize_t f_serial_port_num_show(struct config_item *item, char *page)
{
	return sprintf(page, "%u\n", to_f_serial_opts(item)->port_num);
}

CONFIGFS_ATTR_RO(f_serial_, port_num);

static struct configfs_attribute *acm_attrs[] = {
	&f_serial_attr_port_num,
	NULL,
};

static struct config_item_type serial_func_type = {
	.ct_item_ops	= &serial_item_ops,
	.ct_attrs	= acm_attrs,
	.ct_owner	= THIS_MODULE,
};

static void gser_free_inst(struct usb_function_instance *f)
{
	struct f_serial_opts *opts;

	opts = container_of(f, struct f_serial_opts, func_inst);
	gserial_free_line(opts->port_num);
	kfree(opts);
}

static struct usb_function_instance *wcn_alloc_inst(void)
{
	struct f_serial_opts *opts;
	int ret;

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);

	opts->func_inst.free_func_inst = gser_free_inst;
	ret = gserial_alloc_line(&opts->port_num);
	if (ret) {
		kfree(opts);
		return ERR_PTR(ret);
	}
	config_group_init_type_name(&opts->func_inst.group, "",
				    &serial_func_type);

	return &opts->func_inst;
}

static void gser_free(struct usb_function *f)
{
	struct f_gser *serial;

	serial = func_to_gser(f);
	kfree(serial);
}

static void wcn_unbind(struct usb_configuration *c, struct usb_function *f)
{
	usb_free_all_descriptors(f);
}

static struct usb_function *wcn_alloc(struct usb_function_instance *fi)
{
	struct f_gser	*gser;
	struct f_serial_opts *opts;

	/* allocate and initialize one new instance */
	gser = kzalloc(sizeof(*gser), GFP_KERNEL);
	if (!gser)
		return ERR_PTR(-ENOMEM);

	opts = container_of(fi, struct f_serial_opts, func_inst);

	gser->port_num = opts->port_num;

	f_wcn_dev->port.func.name = "gser";
	gser->port.func.strings = gser_strings;
	gser->port.func.bind = wcn_bind;
	gser->port.func.unbind = wcn_unbind;
	gser->port.func.set_alt = w_set_alt;
	gser->port.func.setup = gser_setup;
	gser->port.func.disable = gser_disable;
	gser->port.func.free_func = wcn_free;

	return &gser->port.func;
}

DECLARE_USB_FUNCTION_INIT(wcn_bt0, wcn_bt0_alloc_inst, wcn_bt0_alloc);


usb_dev_chninit(_name ## func_init);					\
usb_dev_chndeinit(_name ## func_exit)
