#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/system.h>
//#include <asm-arm/memory.h>
#include <asm/irq.h>
//#include <linux/completion.h>
//#include "ipera_k.h"
#include "Omap3_PMU.h"

//#ifndef IPERA_DMA_CHANNEL_ANY
//#define IPERA_DMA_CHANNEL_ANY -1
//#endif

static int major = 15;

#ifdef USE_UDEV
static struct class *ipera_class;
#endif	// USE_UDEV

static DECLARE_MUTEX(ipera_mutex);
//static int        master_ch;
//struct completion edmacompletion;

/* Forward declaration of system calls */
static int ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long args);
static int open(struct inode *inode, struct file *filp);
static int release(struct inode *inode, struct file *filp);



//ASM function declaration in Omap3_PMU_asm.s
extern void init_pmu_asm(void);



static unsigned long ipera_get_phys(unsigned long virtp)
{
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    struct mm_struct *mm = current->mm;

    pgd = pgd_offset(mm, virtp);
    if (!(pgd_none(*pgd) || pgd_bad(*pgd))) {
        pmd = pmd_offset(pgd, virtp);

        if (!(pmd_none(*pmd) || pmd_bad(*pmd))) {
            pte = pte_offset_kernel(pmd, virtp);

            if (pte_present(*pte)) {
                return __pa(page_address(pte_page(*pte)) +
                            (virtp & ~PAGE_MASK));
            }
        }
    }

    return 0;
}

static struct file_operations ipera_fops = {
	.ioctl   = ioctl,
	.open    = open,
	.release = release,
};

static int ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long args)
{
	int                    ret;
    unsigned int __user   *argp = (unsigned int __user *) args;
	unsigned long          physp;
	unsigned long          virtp;
	unsigned int           type;
	
	
	//__D("ioctl %d received. \n", cmd);
	
    switch (cmd) {
	case IPERA_INIT_PMU_STATICS:
		init_pmu_asm();
		__D("IPERA_INIT_PMU_STATICS : returning\n");
		break;

	case IPERA_START_PMU_CACHES_STATICS:
		//memset(&type, 0, sizeof(type));
		//ret = copy_from_user(&type, argp, sizeof(type));

		//set_pmu_event_asm(PMU_ICACHE_EXEC, EVENT_COUNTER0);
		//set_pmu_event_asm(PMU_ICACHE_MISS, EVENT_COUNTER1);
		//set_pmu_event_asm(PMU_DCACHE_ACCESS, EVENT_COUNTER2);
		//set_pmu_event_asm(PMU_DCACHE_MISS, EVENT_COUNTER3);
		//start_pmu_asm();
		//__D("IPERA_START_PMU_CACHES_STATICS : returning\n");
		break;

	case IPERA_END_PMU_CACHES_STATICS:
		//memset(&type, 0, sizeof(type));
		//ret = copy_from_user(&type, argp, sizeof(type));
		
		//stop_pmu_asm();
		//pmu_statics[type].pmu_count		+= 1; 
		//pmu_statics[type].pmu_cycles    += get_clock_counter_asm();
		//pmu_statics[type].pmu_instr_exec    += get_pmnx_counter_asm(EVENT_COUNTER0);
		//pmu_statics[type].pmu_icache_miss    += get_pmnx_counter_asm(EVENT_COUNTER1);
		//pmu_statics[type].pmu_dcache_access    += get_pmnx_counter_asm(EVENT_COUNTER2);
		//pmu_statics[type].pmu_dcache_miss    += get_pmnx_counter_asm(EVENT_COUNTER3);
		//__D("IPERA_END_PMU_CACHES_STATICS : returning\n");
		break;
		
	case IPERA_GET_STATICS:
		//memset(&type, 0, sizeof(type));
		//ret = copy_from_user(&type, argp, sizeof(type));
		
		//ret_get_cycles = pmu_statics[type].pmu_cycles;
		//__D("IPERA_GET_CYCLES : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_count);
		//__D("IPERA_GET_CYCLES : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_cycles);
		//__D("IPERA_GET_ICACHE_EXEC : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_instr_exec);
		//__D("IPERA_GET_ICACHE_MISS : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_icache_miss);
		//__D("IPERA_GET_DCACHE_ACCESS : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_dcache_access);
		//__D("IPERA_GET_DCACHE_MISS : returning %#lx\n", (unsigned long)pmu_statics[type].pmu_dcache_miss);
		//ret = copy_to_user(argp, &pmu_statics[type], sizeof(pmu_statics[type]));
		break;

	case IPERA_GET_PHYS:
		get_user(virtp, argp);
		physp = ipera_get_phys(virtp);
		put_user(physp, argp);
		//__D("IPERA_GET_PHYS : returning %#lx\n", physp);
		break;

		
#if 0
	case IPERA_GET_CYCLES:
		__D("IPERA_GET_CYCLES : received.\n");
		cur_cycles = get_cycles();
		copy_to_user(argp, &cur_cycles, sizeof(cur_cycles));
		__D("IPERA_GET_CYCLES : returning %#lx\n", cur_cycles);
		break;

	case IPERA_GET_PHYS:
		__D("IPERA_GET_PHYS : received.\n");
		get_user(virtp, argp);
		physp = get_phys(virtp);
		put_user(physp, argp);
		__D("IPERA_GET_PHYS : returning %#lx\n", physp);
		break;

    case IPERA_DMACPY:
        __D("IPERA_DMACPY : received.\n");
        if (copy_from_user(&dma, argp, sizeof(dma))) {
            return -EFAULT;
        }
        err = davinci_request_dma(DM350_DMA_CHANNEL_ANY, "EDMA memcpy", memcpy_dma_irq_handler, NULL, &master_ch, &tcc, EVENTQ_1);
        if (err < 0) {
            __E("Error in requesting Master channel %d = 0x%x\n", master_ch, err);
            return err;
        } else if(master_ch != 25)  __E("get channel %d \n", master_ch);
        davinci_stop_dma(master_ch);

        init_completion(&edmacompletion);
        davinci_set_dma_src_params(master_ch, (unsigned long) edmaparams.src, edmaparams.srcmode, edmaparams.srcfifowidth);
        davinci_set_dma_dest_params(master_ch, (unsigned long) edmaparams.dst, edmaparams.dstmode, edmaparams.dstfifowidth);
        davinci_set_dma_src_index(master_ch, edmaparams.srcbidx, edmaparams.srccidx);
        davinci_set_dma_dest_index(master_ch, edmaparams.dstbidx, edmaparams.dstcidx);
        davinci_set_dma_transfer_params(master_ch, edmaparams.acnt, edmaparams.bcnt, edmaparams.ccnt, edmaparams.bcntrld, edmaparams.syncmode);
        davinci_get_dma_params(master_ch, &paramentry);
        davinci_set_dma_params(master_ch, &paramentry);
        davinci_start_dma(master_ch);
        wait_for_completion(&edmacompletion);
        //printk("Dma completed... \n");
        davinci_stop_dma(master_ch);
        davinci_free_dma(master_ch);
        break;
#endif

    default:
        __E("Unknown ioctl received = %d.\n", cmd);
        return -EINVAL;
    }
    return 0;
}


static int open(struct inode *inode, struct file *filp)
{
    __D("open: called.\n");
    return 0;
}

static int release(struct inode *inode, struct file *filp)
{
    __D("close: called.");
    return 0;
}

static int __init ipera_init(void)
{
	int ret;
	
    __D("** " __FILE__ " kernel module built: " __DATE__ " " __TIME__ "\n");

    ret = register_chrdev(major, K_DEV, &ipera_fops);
    if (ret < 0) {
        __E("Failed to allocate major number.\n");
        return -ENODEV;
    }
    __D("Allocated major number: %d\n", major);

#ifdef USE_UDEV
    ipera_class = class_create(THIS_MODULE, K_DEV);
    if (IS_ERR(ipera_class)) {
        __E("Error creating ipera device class.\n");
        return -EIO;
    }
    //class_device_create(ipera_class, NULL, MKDEV(major, 0), NULL, K_DEV);
#endif                          // USE_UDEV

    __D("Successfully initialized module\n");

    return 0;
}

static void __exit ipera_exit(void)
{
    __D("...\n");

#ifdef USE_UDEV
	//class_device_destroy(ipera_class, MKDEV(major, 0));
    class_destroy(ipera_class);
#endif                          // USE_UDEV
    __D("Unregistering character device ipera\n");

    unregister_chrdev(major, K_DEV);
    __D("ipera unregistered\n");
}

module_init(ipera_init);
module_exit(ipera_exit);

MODULE_AUTHOR("Ipera");
MODULE_DESCRIPTION("Iperatech export to userland");
MODULE_LICENSE("GPL");
