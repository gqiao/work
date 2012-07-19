#ifndef __OSAL_H__
#define __OSAL_H__

#include "linux/sched.h"

typedef struct task_struct task_struct_t;

#define __D(f, a...) ({													\
			task_struct_t *th = current;								\
			printk(KERN_ERR "[ state=%d %d]\n", (int)th->state, __LINE__);		\
			printk(KERN_ERR "[ %s ] : [ thread_name:%s(pid:%d) -> %s() -> %d ] : " f, __FILE__, th->comm, (int)th->pid, __func__, __LINE__, ## a); \
		})


#endif	/* __OSAL_H__ */

