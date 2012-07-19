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

#include "mem.h"

static inline void *dlsym(const char* symbol)
{
    struct kernel_symbol *s;
    if((s = (typeof(s))find_symbol(symbol, NULL, NULL, true, true)) == NULL) {
        return NULL;
    }
    return (void*)s->value;
}

#define __E(fmt, args...) ({ printk(KERN_ERR   "E/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __W(fmt, args...) ({ printk(KERN_ERR   "W/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __I(fmt, args...) ({ printk(KERN_INFO  "I/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __D(fmt, args...) ({ printk(KERN_DEBUG "D/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})

unsigned long mem_pa(unsigned long vaddr)
{
    unsigned long          paddr = 0;
    struct mm_struct      *mm    = current->mm;
    struct vm_area_struct *vma   = find_vma(mm, vaddr);


    if(vaddr >= PAGE_OFFSET) {
        paddr = virt_to_phys((void *)vaddr);
    } else if(vma && (vma->vm_flags & VM_IO) && (vma->vm_pgoff)) {
        /* kernel-allocated, mmaped-to-usermode addresses */
        paddr = (vma->vm_pgoff << PAGE_SHIFT) + (vaddr - vma->vm_start);
    } else {
        /* for general userland pages */
        int res, nr_pages = 1;
        struct page *pages;

        down_read(&mm->mmap_sem);
        res = get_user_pages(current, mm, vaddr, nr_pages, 1, 0, &pages, NULL);
        up_read(&mm->mmap_sem);

        if (res != nr_pages) {
            __E("get_user_pages failed\n");
            return 0;
        }
        
        paddr = __pa(page_address(&pages[0]) + (vaddr & ~PAGE_MASK));
    }
    return paddr;
}


