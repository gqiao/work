#if !defined _OSAL_H_ && defined __KERNEL__
#define _OSAL_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/errno.h> /* error codes */
#include <asm/div64.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/arch/map.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/semaphore.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <media/v4l2-dev.h>


#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,16)
#include <linux/config.h>
#include <asm/arch/registers.h>
#endif

#ifdef LOG_TAG
#define __E(fmt, args...) ({ printk(KERN_ERR "E/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args);})
#define __W(fmt, args...) ({ printk(KERN_ERR "W/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args);})
#define __I(fmt, args...) ({ printk(KERN_ERR "I/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args);})

#define __DEBUG

#ifdef __DEBUG
#define __D(fmt, args...) ({ printk(KERN_ERR "D/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args);})
#else
//#warning "osal: NO DEBUG !!!"
#define __D(fmt, args...)
#endif

#else
#warning "osal: need #define LOG_TAG !!!"
#define __D(fmt, args...)
#define __E(fmt, args...)
#define __W(fmt, args...)
#define __I(fmt, args...)
#endif	/* LOG_TAG */



#endif	/* _OSAL_H_ */
