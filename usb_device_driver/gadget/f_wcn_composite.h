#include "composite.h"

struct list_head *wcn_usb_request_queue_get(u8 ep_addr);
struct usb_ep *wcn_ep_get(u8 ep_addr);
int wcn_copmosite_init(void);
int  wcn_func_init(u8 idx);
void  wcn_func_exit(u8 idx);