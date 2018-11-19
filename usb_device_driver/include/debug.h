#ifndef __DEBUG_H__
#define __DEBUG_H__

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void * __must_check ERR_PTR(long error)
{
	return (void *) error;
}

static inline long __must_check PTR_ERR(__force const void *ptr)
{
	return (long) ptr;
}

static inline bool __must_check IS_ERR(__force const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline bool __must_check IS_ERR_OR_NULL(__force const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)ptr);
}

#define dev_err(format, ...) SCI_TRACE_LOW("dev error:"format, ...)
#define dev_dbg(format, ...) SCI_TRACE_LOW("dev dbg:"format, ...)
#define dev_info(format, ...) SCI_TRACE_LOW("dev info:"format, ...)
#define dev_warn(format, ...) SCI_TRACE_LOW("dev arn:"format, ...)


#endif
