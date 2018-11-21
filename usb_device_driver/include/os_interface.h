#include "os_api.h"
#include "types.h"
#include "list.h"

#define IS_ALIGNED(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)








/* int return value */
enum irqreturn {
	IRQ_NONE		= (0 << 0),
	IRQ_HANDLED		= (1 << 0),
	IRQ_WAKE_THREAD		= (1 << 1),
};
typedef enum irqreturn irqreturn_t;
#define IRQ_RETVAL(x)	((x) ? IRQ_HANDLED : IRQ_NONE)



static SCI_MUTEX_PTR inline mutex_lock_init(const char *name ,SCI_MUTEX_PTR mutex)
{
	return SCI_CreateMutex(name, SCI_INHERIT);
}

static void inline mutex_lock(SCI_MUTEX_PTR mutex)
{
	if(SCI_InThreadContext() && mutex)
		SCI_GetMutex(mutex, SCI_WAIT_FOREVER);
}

static void inline mutex_unlock(SCI_MUTEX_PTR mutex)
{
	if(SCI_InThreadContext() && mutex)
		SCI_PutMutex(mutex);
}

static inline void *usb_malloc(size_t size)
{
	return SCI_ALLOC(size);
}
static inline void *usb_mem_free(void *p)
{
	SCI_FREE(p);
}
static inline void *usb_dma_malloc(size_t size, dma_addr_t *dma_handle)
{
	void * addr;
	addr = SCI_ALLOC(size);
	dma_handle = (dma_addr_t *)addr;
	return addr;
}
static inline void *usb_dma_mem_free(void *p, dma_addr_t *dma_handle)
{
	SCI_FREE(p);
	dma_handle = NULL;
}

void enable_irq(u32 irq);
void disable_irq(u32 irq);

