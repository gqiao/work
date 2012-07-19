#define LOG_TAG "__module_address.c"

#include "osal.h"

int a_module(void)
{
	return 0;
}

int __init __module_address_init(void)
{
	struct module *ret;
	unsigned long addr = (typeof(addr))a_module;

	preempt_disable();
	ret = __module_address(addr);
	preempt_enable();

	if(ret != NULL) {
		__I("ret->name:      %s \n", ret->name);
		__I("ret->state:     %d \n", ret->state);
		__I("ret->core_size: %d \n", ret->core_size);
		__I("refs of %s is %d   \n", ret->name, module_refcount(ret));
	} else {
		__I("return NULL ! \n");
	}

	return 0;
}

void __exit __module_address_exit(void)
{
	__I("exit ok! \n");
}


module_init(__module_address_init);
module_exit(__module_address_exit);
