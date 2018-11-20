

#include "composite.h"


static struct list_head func_list;

static SCI_MUTEX_PTR	func_lock;

void function_init(void)
{
	func_lock = SCI_CreateMutex("func_mutex", SCI_INHERIT);
	INIT_LIST_HEAD(&func_list);
}

static struct usb_function *get_usb_function(const char *name)
{
	struct usb_function_driver *fd;
	struct usb_function *f;

	f = ERR_PTR(-ENOENT);
	mutex_lock(&func_lock);
	list_for_each_entry(fd, &func_list, list) {

		if (strcmp(name, fd->name))
			continue;
	
		f  = fd->alloc_func(name);
		if (IS_ERR(f))
			//module_put(fd->mod);
		else
			f->fd = fd;
		break;
	}
	mutex_unlock(&func_lock);
	return f;
}


void usb_put_function(struct usb_function *f)
{
	if (!f)
		return;

	f->free_func(f);
}

int usb_function_register(struct usb_function_driver *newf)
{
	struct usb_function_driver *fd;
	int ret;

	ret = -EEXIST;

	mutex_lock(&func_lock);
	list_for_each_entry(fd, &func_list, list) {
		if (!strcmp(fd->name, newf->name))
			goto out;
	}
	ret = 0;
	list_add_tail(&newf->list, &func_list);
out:
	mutex_unlock(&func_lock);
	return ret;
}

void usb_function_unregister(struct usb_function_driver *fd)
{
	mutex_lock(&func_lock);
	list_del(&fd->list);
	mutex_unlock(&func_lock);
}
