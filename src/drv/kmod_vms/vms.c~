#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

#include "osal.h"

static struct proc_dir_entry *proc_entry;
static unsigned long val = 0x12345678;

ssize_t simple_proc_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	__D(" %x\n", val);
	
	int len;
	if (off > 0) {
		*eof = 1;
		return 0;
	}

	len = sprintf(buf, "%08x\n", val);

	__D("buf = %s\n", buf);

	return len;
}

ssize_t simple_proc_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{
	__D(" %x\n", val);

#define MAX_UL_LEN 8
	char k_buf[MAX_UL_LEN];
	char *endp;
	unsigned long new;
	int count = min(MAX_UL_LEN, len);
	int ret;

	if (copy_from_user(k_buf, buff, count)) {
			ret = -EFAULT;
			goto err;
	} else {
			new = simple_strtoul(k_buf, &endp, 16);
			if (endp == k_buf) {
				ret = -EINVAL;
				goto err;
	}

	val = new;

	__D("simple_proc_write %x\n", val);

	return count;
	}

  err:
	return ret;
}

int __init simple_proc_init()
{
	__D("simple_proc_init\n");

	proc_entry = create_proc_entry("sim_proc", 0666, NULL);
	if (proc_entry == NULL) {
		printk(KERN_INFO "Couldn't create proc entry\n");
		goto err;
	} else {
		proc_entry->read_proc = simple_proc_read;
		proc_entry->write_proc = simple_proc_write;
		proc_entry->owner = THIS_MODULE;
	}

	return 0;

  err:
	return -ENOMEM;
}

void __exit simple_proc_exit()
{
	__D("simple_proc_exit\n");

	remove_proc_entry("sim_proc", &proc_entry);
}

module_init(simple_proc_init);
module_exit(simple_proc_exit);

MODULE_AUTHOR("Michelle, author@linuxdriver.cn");
MODULE_DESCRIPTION("A simpe Module for showing proc");
MODULE_VERSION("V1.0");
