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

#include "irq.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define IRQF_SHARED SA_SHIRQ
#endif

typedef struct {
    int irq;
    int count;
} irqcount_t;

static irqcount_t irqcount[MAX_IRQ];


static irqreturn_t diag_irqcount(int irq, void *dev_id
                                 /*, struct pt_regs *regs */ )
{
    ((irqcount_t*)dev_id)->count++;
    return IRQ_HANDLED;
}

int irq_cnt_begin(int irq)
{
    int i;
    static int inited;
    if(inited == 0) {
        for(i = 0; i < ARRAY_SIZE(irqcount); i++) {
            irqcount[i].irq   = -1;
            irqcount[i].count = -1;
        }

        inited = 1;
    }
    
    for(i = 0; i < ARRAY_SIZE(irqcount); i++) {
        if(irqcount[i].irq < 0) {
            printk("<1>diag: Requesting IRQ %d\n", irq);
            if(request_irq(irq, diag_irqcount, IRQF_SHARED,
                           "diag", &irqcount[i]) < 0) {
                return -1;
            }
                
            irqcount[i].count = 0;
            irqcount[i].irq   = irq;
            return i;
        }
    }
    
    return -1;
}

int irq_get_cnt(int id)
{
    if(id < 0 || id >= ARRAY_SIZE(irqcount)) {
        return -1;
    }
    
    return irqcount[id].count;
}

void irq_cnt_end(int id)
{
    if(id < 0 || id >= ARRAY_SIZE(irqcount)) {
        return;
    }

    free_irq(irqcount[id].irq, &irqcount[id]);
    irqcount[id].irq   = -1;
    irqcount[id].count = -1;
}

