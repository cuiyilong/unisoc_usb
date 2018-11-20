

/* for arm default little endian */
#define cpu_to_le16(x) x
#define le16_to_cpu(x) x



/* int return value */
enum irqreturn {
	IRQ_NONE		= (0 << 0),
	IRQ_HANDLED		= (1 << 0),
	IRQ_WAKE_THREAD		= (1 << 1),
};
typedef enum irqreturn irqreturn_t;
#define IRQ_RETVAL(x)	((x) ? IRQ_HANDLED : IRQ_NONE)



static void inline mutex_lock(SCI_MUTEX_PTR mutex)
{
	if(SCI_InThreadContext() && mutex)
		SCI_GetMutex(mutex, SCI_WAIT_FOREVER)
}

static void inline mutex_unlock(SCI_MUTEX_PTR mutex)
{
	if(SCI_InThreadContext() && mutex)
		SCI_PutMutex(mutex)
}



