#define IN_CHN_TO_EP_ADDR(chn) (chn - 0x10 + 0x80) 
#define OUT_CHN_TO_EP_ADDR(chn) chn 

static mchn_bus_ops usb_bus_ops;

/* mchn port to usb endpoint address: 0-15 out/16-31 in*/

static inline u8 wcn_chn_to_usb_intf(u8 chn,int inout)
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

static inline u8 wcn_chn_to_usb_func(u8 chn,int inout)
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

void usb_wcn_xmit_complete(struct usb_ep *ep, struct usb_request *req)
{
	
}
int usb_wcn_start_xmit(int chn, cpdu_t *head, cpdu_t *tail, int num)
{
	struct usb_ep *ep;
	struct usb_request *req;
	struct list_head *tx_reqs;
	
	int ret;
	u8 ep_addr;

	ep_addr = IN_CHN_TO_EP_ADDR(chn);
	ep = wcn_ep_get(ep_addr);
	if(!ep)
		return USB_CHN_ERR;

	if (list_empty(&tx_reqs)) {
		return WCN_TX_BUSY;
	}

	req = container_of(tx_reqs->next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty(&tx_reqs))
		netif_stop_queue(net);

	
	req->buf_n
	req->complete = usb_wcn_xmit_complete;
	ret= usb_ep_queue(ep,req);

	return ret;
}

int usb_chn_push_link(int chn, cpdu_t *head, cpdu_t *tail, int num)
{
	int ret = -1;
	mchn_info_t *mchn = mchn_info();
	switch (mchn_ops[chn]->inout)
	{
		case DIRECT_TX:
			ret = usb_wcn_start_xmit(chn, head, tail,num);
	
	}

}


void module_bus_init(void)
{
	usb_bus_ops.type = HW_TYPE_USB;
	usb_bus_ops.init = usb_dev_deinit;
	usb_bus_ops.deinit = usb_dev_deinit;
	usb_bus_ops.chn_init = usb_dev_chninit;
	usb_bus_ops.chn_deinit = usb_dev_chndeinit;
	usb_bus_ops.push_ink = usb_chn_push_link;
	module_ops_register(&usb_bus_ops);
}
