#include "os_interface.h"

void *usb_dma_malloc(size_t size, dma_addr_t *dma_handle)
{
	void * addr;
	addr = SCI_ALLOC(size);
	dma_handle = (dma_addr_t *)addr;
	return addr;
}

void *usb_dma_mem_free(void *p, dma_addr_t *dma_handle)
{
	SCI_FREE(p);
	dma_handle = NULL;
}

inline void *usb_malloc(size_t size)
{
	return SCI_ALLOC(size);
}

inline void *usb_mem_free(void *p)
{
	SCI_FREE(p);
}

void enable_irq(u32 irq)
{
	return;
}

void disable_irq(u32 irq)
{
	return;
}




