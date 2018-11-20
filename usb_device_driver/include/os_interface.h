#include "os_api.h"
#include "list.h"

#define IS_ALIGNED (((x) & ((typeof(x))(a) - 1)) == 0)

/* for arm default little endian */
#define cpu_to_le16(x) x
#define le16_to_cpu(x) x

#define cpu_to_le64(x)	(x)
#define cpu_to_le32(x)	(x)


static inline void le16_add_cpu(__le16 *var, u16 val)
{
	*var = cpu_to_le16(le16_to_cpu(*var) + val);
}



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
		SCI_GetMutex(mutex, SCI_WAIT_FOREVER)
}

static void inline mutex_unlock(SCI_MUTEX_PTR mutex)
{
	if(SCI_InThreadContext() && mutex)
		SCI_PutMutex(mutex)
}



