

/* Defines */

#define GS_VERSION_STR			"v2.4"
#define GS_VERSION_NUM			0x2400

#define GS_LONG_NAME			"Gadget Serial"
#define GS_VERSION_NAME			GS_LONG_NAME " " GS_VERSION_STR

/*-------------------------------------------------------------------------*/
USB_GADGET_COMPOSITE_OPTIONS();

/* Thanks to NetChip Technologies for donating this product ID.
*
* DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
* Instead:  allocate your own, using normal USB-IF procedures.
*/
#define GS_VENDOR_ID			0x0525	/* NetChip */
#define GS_PRODUCT_ID			0xa4a6	/* Linux-USB Serial Gadget */
#define GS_CDC_PRODUCT_ID		0xa4a7	/* ... as CDC-ACM */
#define GS_CDC_OBEX_PRODUCT_ID		0xa4a9	/* ... as CDC-OBEX */

/* string IDs are assigned dynamically */

#define STRING_DESCRIPTION_IDX		USB_GADGET_FIRST_AVAIL_IDX

static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "",
	[USB_GADGET_PRODUCT_IDX].s = GS_VERSION_NAME,
	[USB_GADGET_SERIAL_IDX].s = "",
	[STRING_DESCRIPTION_IDX].s = NULL /* updated; f(use_acm) */,
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength =		USB_DT_DEVICE_SIZE,
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB =		cpu_to_le16(0x0200),
	/* .bDeviceClass = f(use_acm) */
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	/* .bMaxPacketSize0 = f(hardware) */
	.idVendor =		cpu_to_le16(GS_VENDOR_ID),
	/* .idProduct =	f(use_acm) */
	.bcdDevice = cpu_to_le16(GS_VERSION_NUM),
	/* .iManufacturer = DYNAMIC */
	/* .iProduct = DYNAMIC */
	.bNumConfigurations =	1,
};

static const struct usb_descriptor_header *otg_desc[2];

/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/

static struct usb_configuration wcn_comp_config_driver[2] = {
{
	/* .label = f(use_acm) */
	.bConfigurationValue = 0,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes	= USB_CONFIG_ATT_SELFPOWER,
},
{
	/* .label = f(use_acm) */
	.bConfigurationValue = 1,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes	= USB_CONFIG_ATT_SELFPOWER,
}
};

static bool cfg_idx = 1;

static struct usb_function_instance *fi_wcn[MAX_U_WCN_INTERFACES];
static struct usb_function *f_wcn[MAX_U_WCN_INTERFACES];

static int wcn_comp_register_func(struct usb_composite_dev *cdev,
		struct usb_configuration *c, const char *f_name)
{
	int i;
	int ret;

	ret = usb_add_config_only(cdev, c);
	if (ret)
		goto out;

	for (i = 0; i < n_ports; i++) {

		fi_wcn[i] = usb_get_function_instance(f_name);
		if (IS_ERR(fi_serial[i])) {
			ret = PTR_ERR(fi_serial[i]);
			goto fail;
		}

		f_wcn[i] = usb_get_function(fi_serial[i]);
		if (IS_ERR(f_serial[i])) {
			ret = PTR_ERR(f_serial[i]);
			goto err_get_func;
		}

		ret = usb_add_function(c, f_serial[i]);
		if (ret)
			goto err_add_func;
	}

	return 0;

err_add_func:
	usb_put_function(f_serial[i]);
err_get_func:
	usb_put_function_instance(fi_serial[i]);

fail:
	i--;
	while (i >= 0) {
		usb_remove_function(c, f_serial[i]);
		usb_put_function(f_serial[i]);
		usb_put_function_instance(fi_serial[i]);
		i--;
	}
out:
	return ret;
}

static int wcn_composite_bind(struct usb_composite_dev *cdev)
{
	int			status;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		goto fail;
	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	status = strings_dev[STRING_DESCRIPTION_IDX].id;
	wcn_comp_config_driver.iConfiguration = status;

	if (gadget_is_otg(cdev->gadget)) {
		if (!otg_desc[0]) {
			struct usb_descriptor_header *usb_desc;

			usb_desc = usb_otg_descriptor_alloc(cdev->gadget);
			if (!usb_desc) {
				status = -ENOMEM;
				goto fail;
			}
			usb_otg_descriptor_init(cdev->gadget, usb_desc);
			otg_desc[0] = usb_desc;
			otg_desc[1] = NULL;
		}
		wcn_comp_config_driver.descriptors = otg_desc;
		wcn_comp_config_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	/* register our configuration */
	if (cfg_idx) {
		status  = wcn_comp_register_func(cdev, &wcn_comp_config_driver[cfg_idx],
				"default0");
		usb_ep_autoconfig_reset(cdev->gadget);
	} else
		status = serial_register_ports(cdev, &wcn_comp_config_driver[cfg_idx],
				"default1");
	if (status < 0)
		goto fail1;

	usb_composite_overwrite_options(cdev, &coverwrite);
	INFO(cdev, "%s\n", GS_VERSION_NAME);

	return 0;
fail1:
	kfree(otg_desc[0]);
	otg_desc[0] = NULL;
fail:
	return status;
}

static int wcn_composite_unbind(struct usb_composite_dev *cdev)
{
	int i;

	for (i = 0; i < n_ports; i++) {
		usb_put_function(f_serial[i]);
		usb_put_function_instance(fi_serial[i]);
	}

	kfree(otg_desc[0]);
	otg_desc[0] = NULL;

	return 0;
}

static struct usb_composite_driver wcn_composite_driver = {
	.name		= "wcn_composite",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.max_speed	= USB_SPEED_SUPER,
	.bind		= wcn_composite_bind,
	.unbind		= wcn_composite_unbind,
};

int wcn_copmosite_init(void)
{
	/* We *could* export two configs; that'd be much cleaner...
	 * but neither of these product IDs was defined that way.
	 */

	wcn_comp_config_driver[cfg_idx].label = "Wcn Cfg";
	wcn_comp_config_driver[cfg_idx].bConfigurationValue = 2;
	device_desc.bDeviceClass = USB_CLASS_VENDOR_SPEC;
	device_desc.idProduct =
			cpu_to_le16(GS_CDC_PRODUCT_ID);
	
	strings_dev[STRING_DESCRIPTION_IDX].s = wcn_comp_config_driver[cfg_idx].label;

	return usb_composite_probe(&wcn_composite_driver[cfg_idx]);
}


static void  wcn_copmosite_cleanup(void)
{
	usb_composite_unregister(&wcn_composite_driver[cfg_idx]);
}


