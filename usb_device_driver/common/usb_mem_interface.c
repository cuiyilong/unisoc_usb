

void *usb_dma_malloc(size_t size, dma_addr_t *dma_handle)
{
	void * addr;
	addr = SCI_ALLOC(size);
	dma_handle = (dma_addr_t);
	return addr;
}

void *usb_dma_mem_free(size_t size, dma_addr_t *dma_handle)
{
	void * addr;
	addr = SCI_ALLOC(size);
	dma_handle = (dma_addr_t);
	return addr;
}


void *usb_malloc(size_t size)
{
	return SCI_ALLOC(size);
}

void *usb_mem_free(size_t size)
{
	return SCI_FREE(size);
}

