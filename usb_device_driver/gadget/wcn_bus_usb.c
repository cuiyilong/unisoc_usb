
static mchn_bus_ops usb_bus_ops;

/* mchn port to usb endpoint address: 0-15 out/16-31 in*/

inline u8 wcn_chn_to_usb_intf(u8 chn,int inout)
{	u8 idx = 0;

	if(chn > 16)
		chn = chn - 16;

	if(chn <= 2)
		idx = 0;
	else if (chn == 3)
		idx = 1;
	else
		idx = 2;

	return idx;
}


int usb_dev_init()
{
	wcn_copmosite_init();

	gwcn_task_init();
}

int usb_dev_deinit()
{
	wcn_copmosite_init();
}
	
static int usb_dev_chninit(mchn_ops_t *ops)
{
	return wcn_func_init(wcn_chn_to_usb_intf(ops->chn, ops->inout));
}

static int usb_dev_chndeinit(mchn_ops_t *ops)
{
	wcn_func_exit(wcn_chn_to_usb_intf(ops->chn, ops->inout));
}


int usb_chn_push_link(int chn, cpdu_t *head, cpdu_t *tail, int num)
{
	int ret = -1;
	mchn_info_t *mchn = mchn_info();
	switch (mchn->ops[chn]->inout)
	{
		case DIRECT_TX:
			ret = usb_wcn_start_xmit(chn, head, tail,num);
		case DIRECT_RX:
	}

}


void module_bus_init(void)
{
	usb_bus_ops.type = HW_TYPE_USB;
	usb_bus_ops.init = usb_dev_init;
	usb_bus_ops.deinit = usb_dev_deinit;
	usb_bus_ops.chn_init = usb_dev_chninit;
	usb_bus_ops.chn_deinit = usb_dev_chndeinit;
	usb_bus_ops.push_ink = usb_chn_push_link;
	module_ops_register(&usb_bus_ops);
}
