#include <linux/fs.h>
#include <linux/kprobes.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "diag.h"
#include "mem.h"
#include "irq.h"
#include "dma.h"

#define MKNOD 1


MODULE_AUTHOR       ("joesensport@gmail.com");
MODULE_DESCRIPTION  ("Diagnostic Driver");
MODULE_LICENSE      ("GPL");


static long diag_ioctl(struct file *file,
                      unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if(_IOC_TYPE(cmd) != 'k')
        return -ENOTTY;

    if(_IOC_NR(cmd) > DIAG_IOC_NR)
        return -ENOTTY;

    switch(cmd) {
    case DIAG_GET_PADDR:
    {
        diag_get_paddr_t a;
        
        if(copy_from_user((void*)&a, (void*)arg, sizeof(a)) != 0) {
            return -EFAULT;
        }

        a.paddr = (typeof(a.paddr))mem_pa((unsigned long)a.vaddr);
        return copy_to_user((void*)arg, (void*)&a, sizeof(a));
    }

    case DIAG_DMA_TEST_START:
    {
        diag_dma_test_t diag_dma_test;
        
        if(copy_from_user((void*)&diag_dma_test, (void*)arg, sizeof(diag_dma_test)) != 0) {
            return -EFAULT;
        }

        diag_dma_test.ret = dma_test_start(diag_dma_test.pages);
        return copy_to_user((void*)arg, (void*)&diag_dma_test, sizeof(diag_dma_test));
    }

    case DIAG_IRQCNT_BEGIN:
    {
        diag_irq_t r;
        
        printk("<1>diag: Beginning IRQ count\n");
        if(copy_from_user((void*)&r, (void*)arg, sizeof(r)) != 0) {
            return -EFAULT;
        }

        r.id = irq_cnt_begin(r.irq);
        return copy_to_user((void*)arg, (void*)&r, sizeof(r));
    }

    case DIAG_IRQCNT_END:
    {
        diag_irq_t r;
        
        printk("<1>diag: Ending IRQ count\n");
        if(copy_from_user((void*)&r, (void*)arg, sizeof(r)) != 0) {
            return -EFAULT;
        }
        
        if(r.irq < 0 || r.id < 0) {
            r.id = -1;
            return -EFAULT;
        }

        r.cnt = irq_get_cnt(r.id);
        irq_cnt_end(r.id);
        return copy_to_user((void*)arg, (void*)&r, sizeof(r));
    }

    case DIAG_IRQCNT_GET:
    {
        diag_irq_t r;

        printk("<1>diag: Getting IRQ count\n");
        if(copy_from_user((void*)&r, (void*)arg, sizeof(r)) != 0) {
            return -EFAULT;
        }

        if(r.irq < 0 || r.id < 0) {
            r.id = -1;
            return -EFAULT;
        }

        r.cnt = irq_get_cnt(r.id);
        printk("<1>diag: irqcnt = %d\n", r.cnt);
        return copy_to_user((void*)arg, (void*)&r, sizeof(r));
    }

    default:
        printk("<1>diag: error diag args\n");
    } //switch

    return ret;
}


static int diag_open(struct inode *inode, struct file *file)
{
    printk("<1>diag: Opening driver\n");
    return 0;
}

static int diag_release(struct inode *inode, struct file *file)
{
    printk("<1>diag: Releasing (closing) driver\n");
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .unlocked_ioctl   = diag_ioctl,
    .open    = diag_open,
    .release = diag_release,
};

static struct class *class;
static struct device *device;
int diag_init(void)
{
    int ret;

    if((ret = register_chrdev(DIAG_MAJOR_NUMBER, DIAG_NAME, &fops)) < 0) {
        printk("Failed to register Diagnostic kernel driver: %d\n", ret);
    }

    if(IS_ERR(class = class_create(THIS_MODULE, DIAG_NAME))) {
        printk("Failed to creating %s device class.\n", DIAG_NAME);
    }

#if (MKNOD == 1)
    if(IS_ERR(device = device_create(class, NULL, MKDEV(DIAG_MAJOR_NUMBER, 0), NULL,
                            DIAG_NAME))) {
        printk("Failed to creating %s device.\n", DIAG_NAME);
    }
	dma_init(device);
#endif

    printk("Diagnostic Driver installed\n");
    return 0;
}

void diag_exit(void)
{

#if (MKNOD == 1)
	dma_exit();
    device_destroy(class, MKDEV(DIAG_MAJOR_NUMBER, 0));
#endif

    class_destroy(class);

    unregister_chrdev(DIAG_MAJOR_NUMBER, DIAG_NAME);
    printk("Diagnostic Driver unloaded\n");
}


module_init(diag_init);
module_exit(diag_exit);

