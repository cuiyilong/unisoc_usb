#include "gadget.h"
#include "u_wcn_composite.h"
#include "f_wcn_composite.h"

#define DEFAULT_QLEN	2	/* double buffering by default */

uint32 gwcn_thread_id = SCI_INVALID_BLOCK_ID;
SCI_EVENT_GROUP_PTR usb_gwcn_event;



/* for dual-speed hardware, use deeper queues at high/super speed */
//static inline int qlen(struct usb_gadget *gadget, unsigned qmult)
static inline int qlen(unsigned qmult)
{
	#if 0
	if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
					    gadget->speed == USB_SPEED_SUPER))
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
	#endif
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
	

	result = gwcn_req_alloc(&wcn_bt0->int_tx_reqs, wcn_bt0->int_in, qlen(wcn_bt0->qmult));
	result = gwcn_req_alloc(&wcn_bt0->bulk_tx_reqs, wcn_bt0->bulk_in, qlen(wcn_bt0->qmult));
	result = gwcn_req_alloc(&wcn_bt0->rx_reqs, wcn_bt0->bulk_out, qlen(wcn_bt0->qmult));

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
		req = container_of(wcn_bt0->int_tx_reqs.next,
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


int gwcn_bt1_connect(struct f_wcn_bt1 *wcn_bt1)
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
	result = gwcn_req_alloc(&wcn_bt1->tx_reqs, wcn_bt1->isoc_in, qlen(wcn_bt1->qmult));
	result = gwcn_req_alloc(&wcn_bt1->rx_reqs, wcn_bt1->isoc_out, qlen(wcn_bt1->qmult));

	return result;

fail0:

	usb_ep_disable(wcn_bt1->isoc_in);
	usb_ep_disable(wcn_bt1->isoc_out);
	result = -1;

	return result;
}

void gwcn_bt1_disconnect(struct f_wcn_bt1 *wcn_bt1)
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
		result = gwcn_req_alloc(&wcn_wifi->tx_reqs[i], wcn_wifi->bulk_in[i], qlen(wcn_wifi->qmult));
		
	}
	for(i = 0; i < 7; i++) {
		result = usb_ep_enable(wcn_wifi->bulk_out[i]);
		if (result != 0) {

			goto fail0;
		}
		result = gwcn_req_alloc(&wcn_wifi->rx_reqs[i], wcn_wifi->bulk_out[i], qlen(wcn_wifi->qmult));
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

static void usb_wcn_rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct list_head *rx_reqs;
	int		status = req->status;

	struct list_head	*rx_buf_head;
	struct rx_buf_node	*rx_buf;

	struct f_wcn_dev *p_wcn_usb_dev = ep->driver_data;
	int chn = IN_EP_ADDR_TO_CHN(ep->address);



	rx_reqs = wcn_usb_request_queue_get(ep->address);
	

	switch (status) {

	/* normal completion */
	case 0:

		rx_buf_head = p_wcn_usb_dev->rx_buf_head[wcn_chn_to_usb_intf(chn)];
		rx_buf= usb_malloc(sizeof(rx_buf));
		if (!rx_buf)
			goto clean;
		rx_buf->chn = chn;
		/* add buf to rx list */
		list_add_tail(&rx_buf->list, rx_buf_head);
		

		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		DBG("rx shutdown, code %d\n", status);

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		DBG("rx %s reset\n", ep->name);

		goto clean;

	/* data overrun */
	case -EOVERFLOW:
		/* FALLTHROUGH */
	default:

		DBG("rx status %d\n", status);
		break;
	}

clean:
	//spin_lock(&dev->req_lock);
	list_add(&req->list, rx_reqs);
	//spin_unlock(&dev->req_lock);

	

	SCI_SetEvent(usb_gwcn_event, 0x1, SCI_OR);

}
int usb_wcn_rx_submit(struct usb_ep *ep)
{
	struct usb_request *req;
	struct list_head *rx_reqs;

	
	int ret;

	mchn_info_t *mchn = mchn_info();
	int chn = IN_EP_ADDR_TO_CHN(ep->address);
	
	ep->driver_data = &wcn_usb_dev;

	rx_reqs = wcn_usb_request_queue_get(ep->address);
	if (list_empty(rx_reqs)) {
		return USB_RX_BUSY;
	}

	while (!list_empty(rx_reqs)) {

		req = container_of(rx_reqs->next, struct usb_request, list);
		list_del_init(&req->list);
		/* get free buffer */
		ret = mchn->ops[chn]->push_link(&req->buf_head, &req->buf_tail, &req->buf_num);
		if (ret != 0 ) {
			/* no tx buf*/
		}

		req->complete = usb_wcn_rx_complete;
		//req->context = &cpdu;

		ret = usb_ep_queue(ep, req);

	}

	return ret;
}

static void usb_wcn_xmit_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct list_head *tx_reqs = (struct list_head *)req->context;

	int chn = IN_EP_ADDR_TO_CHN(ep->address);

	tx_reqs = wcn_usb_request_queue_get(ep->address);


	switch (req->status) {
	default:
		
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		break;
		
	}

	/* tx finish: pop back to module*/
	
	bus_hw_pop_link(chn, req->buf_head, req->buf_head, req->buf_num);
	req->buf_head = req->buf_head = NULL;
	req->buf_num = 0;

	list_add_tail(&req->list, tx_reqs);
	
}
int usb_wcn_start_xmit(int chn, cpdu_t *head, cpdu_t *tail, int num)
{
	struct usb_ep *ep;
	struct usb_request *req;
	struct list_head *tx_reqs;
	
	int ret;
	u8 ep_addr;

	mchn_info_t *mchn = mchn_info();

	ep_addr = IN_CHN_TO_EP_ADDR(chn);
	ep = wcn_ep_get(ep_addr);
	if(!ep)
		return USB_CHN_ERR;

	tx_reqs = wcn_usb_request_queue_get(ep_addr);
	if (!tx_reqs) {
		return USB_CHN_ERR;
	}

	if (list_empty(tx_reqs)) {
		return USB_TX_BUSY;
	}

	req = container_of(tx_reqs->next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty(tx_reqs)) {
	//	usb_stop_queue();

	}


	req->context = tx_reqs;
	req->cpdu.buf_head = head;
	req->cpdu.buf_tail = tail;
	req->buf_num = num;

	/* buf length :init later*/
	/* buf link or only buf */
	
	
	req->complete = usb_wcn_xmit_complete;
	ret= usb_ep_queue(ep,req);

	return ret;
}


static void gwcn_usb_trans_task(uint32 argc, void *data)
{
	uint32 usb_gwcn_event_flag = 0;
	
	struct usb_ep *ep;
	struct list_head	*rx_buf_head;
	struct rx_buf_node	*rx_buf;

	int chn;
	
	while(1) {
		SCI_GetEvent(usb_gwcn_event, 0xffff, SCI_OR_CLEAR, &usb_gwcn_event_flag, SCI_WAIT_FOREVER);
		if (usb_gwcn_event_flag != 0x1)
			continue;
		chn  = usb_gwcn_event_flag;

		rx_buf_head = wcn_usb_dev->rx_buf_head[wcn_chn_to_usb_intf(chn)];
		/* hand rx list */
		while (!list_empty(rx_buf_head)) {
			rx_buf = list_entry(rx_buf_head, struct rx_buf_node, list);
			bus_hw_pop_link(chn, rx_buf->buf_head, rx_buf->buf_head, rx_buf->buf_num);

			
		}
		ep = wcn_ep_get(IN_CHN_TO_EP_ADDR(chn));
		usb_wcn_rx_submit(ep);
		
	}
}

#define SCI_PRIOTY_USB 20
int gwcn_task_init(void)
{
	/* init rx task */
	gwcn_thread_id = SCI_CreateThread("usb_gwcn_task","usb_gwcn_queue",gwcn_usb_trans_task,0,NULL,
		2048,10,SCI_PRIOTY_USB,SCI_PREEMPT,SCI_AUTO_ACTIVATE);
	if (gwcn_thread_id == SCI_INVALID_BLOCK_ID) {
		SCI_ASSERT(0);
		return -1;
	}

	/* init event */
	usb_gwcn_event = SCI_CreateEvent("usb_gwcn_trans_event");
	if (usb_gwcn_event == SCI_NULL) {
		SCI_ASSERT(0);
		return -1;
	}

	return 0;
}
