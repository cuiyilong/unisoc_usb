

/* for dual-speed hardware, use deeper queues at high/super speed */
static inline int qlen(struct usb_gadget *gadget, unsigned qmult)
{
	if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
					    gadget->speed == USB_SPEED_SUPER))
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
}

static int gwcn_req_alloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
	unsigned		i;
	struct usb_request	*req;

	if (!n)
		return -ENOMEM;

	/* queue/recycle up to N requests */
	i = n;
	list_for_each_entry(req, list, list) {
		if (i-- == 0)
			goto extra;
	}
	while (i--) {
		req = usb_ep_alloc_request(ep);
		if (!req)
			return list_empty(list) ? -ENOMEM : 0;
		list_add(&req->list, list);
	}
	return 0;

extra:
	/* free extras */
	for (;;) {
		struct list_head	*next;

		next = req->list.next;
		list_del(&req->list);
		usb_ep_free_request(ep, req);

		if (next == list)
			break;

		req = container_of(next, struct usb_request, list);
	}
	return 0;
}




int gwcn_bt0_connect(struct f_wcn_bt0 *wcn_bt0)
{
	int	result = 0;
	/* enable eps */
	result = usb_ep_enable(wcn_bt0->int_in);
	if (result != 0) {

		goto fail0;
	}
	result = usb_ep_enable(wcn_bt0->bulk_out);
	if (result != 0) {

		goto fail0;
	}
	result = usb_ep_enable(wcn_bt0->bulk_in);
	if (result != 0) {

		goto fail0;
	}
	/* alloc requests */
	

	result = gwcn_req_alloc(wcn_bt0->int_tx_reqs,qlen(wcn_bt0->qmult));
	result = gwcn_req_alloc(wcn_bt0->bulk_tx_reqs,qlen(wcn_bt0->qmult));
	result = gwcn_req_alloc(wcn_bt0->rx_reqs,qlen(wcn_bt0->qmult));

	return result;

fail0:

	usb_ep_disable(wcn_bt0->int_in);
	usb_ep_disable(wcn_bt0->bulk_out);
	usb_ep_disable(wcn_bt0->bulk_in);
	result = -1;

	return result;
}

void gwcn_bt0_disconnect(struct f_wcn_bt0 *wcn_bt0)
{
	struct usb_request	*req;
	
	usb_ep_disable(wcn_bt0->int_in);
	while (!list_empty(&wcn_bt0->int_tx_reqs)) {
		req = container_of(wcn_bt0->tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(wcn_bt0->int_in, req);
	}
	wcn_bt0->int_in->desc = NULL;

	usb_ep_disable(wcn_bt0->bulk_out);
	while (!list_empty(&wcn_bt0->rx_reqs)) {
		req = container_of(wcn_bt0->rx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(wcn_bt0->bulk_out, req);
	}
	wcn_bt0->bulk_out->desc = NULL;

	usb_ep_disable(wcn_bt0->bulk_in);
	while (!list_empty(&wcn_bt0->bulk_tx_reqs)) {
		req = container_of(wcn_bt0->bulk_tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(wcn_bt0->bulk_in, req);
	}
	wcn_bt0->bulk_in->desc = NULL;
}


int gwcn_bt1_connect(struct f_wcn_bt0 *wcn_bt1)
{
	int	result = 0;

	/* enable eps */
	result = usb_ep_enable(wcn_bt1->isoc_in);
	if (result != 0) {

		goto fail0;
	}
	result = usb_ep_enable(wcn_bt1->isoc_out);
	if (result != 0) {

		goto fail0;
	}
	/* alloc requests */
	result = gwcn_req_alloc(wcn_bt1->tx_reqs,qlen(wcn_bt1->qmult));
	result = gwcn_req_alloc(wcn_bt1->rx_reqs,qlen(wcn_bt1->qmult));

	return result;

fail0:

	usb_ep_disable(wcn_bt1->isoc_in);
	usb_ep_disable(wcn_bt1->isoc_out);
	result = -1;

	return result;
}

void gwcn_bt1_disconnect(struct f_wcn_bt0 *wcn_bt1)
{
	struct usb_request	*req;
	
	usb_ep_disable(wcn_bt1->isoc_in);
	while (!list_empty(&wcn_bt1->tx_reqs)) {
		req = container_of(wcn_bt1->tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(wcn_bt1->isoc_in, req);
	}
	wcn_bt1->isoc_in->desc = NULL;

	usb_ep_disable(wcn_bt1->isoc_out);
	while (!list_empty(&wcn_bt1->rx_reqs)) {
		req = container_of(wcn_bt1->rx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(wcn_bt1->isoc_out, req);
	}
	wcn_bt1->isoc_out->desc = NULL;
}

int gwcn_wifi_connect(struct f_wcn_wifi *wcn_wifi)
{
	int	result = 0;
	int i;
	/* enable eps */
	for(i = 0; i < 3; i++) {
		result = usb_ep_enable(wcn_wifi->bulk_in[i]);
		if (result != 0) {

			goto fail0;
		}
		/* alloc requests */
		result = gwcn_req_alloc(wcn_wifi->tx_reqs[i],qlen(wcn_wifi->qmult));
		
	}
	for(i = 0; i < 7; i++) {
		result = usb_ep_enable(wcn_wifi->bulk_out[i]);
		if (result != 0) {

			goto fail0;
		}
		result = gwcn_req_alloc(wcn_wifi->rx_reqs,qlen(wcn_wifi->qmult));
	}
	
	return result;

fail0:

	for(i = 0; i < 3; i++) {
		result = usb_ep_disable(wcn_wifi->bulk_in[i]);
		wcn_wifi->bulk_in[i]->desc = NULL;
	}

	for(i = 0; i < 7; i++) {
		result = usb_ep_disable(wcn_wifi->bulk_out[i]);
		wcn_wifi->bulk_out[i]->desc = NULL;
	}


	result = -1;

	return result;
}

void gwcn_wifi_disconnect(struct f_wcn_wifi *wcn_wifi)
{
	struct usb_request	*req;
	int i;

	for(i = 0; i < 3; i++) {
		usb_ep_disable(wcn_wifi->bulk_in[i]);
		while (!list_empty(&wcn_wifi->tx_reqs[i])) {
			req = container_of(wcn_wifi->tx_reqs[i].next,
						struct usb_request, list);
			list_del(&req->list);
			usb_ep_free_request(wcn_wifi->bulk_in[i], req);
		}
		wcn_wifi->bulk_in[i]->desc = NULL;
	}

	for(i = 0; i < 7; i++) {
		usb_ep_disable(wcn_wifi->bulk_out[i]);
		while (!list_empty(&wcn_wifi->rx_reqs[i])) {
			req = container_of(wcn_wifi->rx_reqs[i].next,
						struct usb_request, list);
			list_del(&req->list);
		}
		wcn_wifi->bulk_out[i]->desc = NULL;
	}

}

