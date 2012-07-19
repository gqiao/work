#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/memblock.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include <linux/module.h>

#include <asm/setup.h>

#include <plat/sram.h>
#include <plat/vram.h>
#include <plat/dma.h>


#define __E(fmt, args...) ({ printk(KERN_ERR   "E/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __W(fmt, args...) ({ printk(KERN_ERR   "W/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __I(fmt, args...) ({ printk(KERN_INFO  "I/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})
#define __D(fmt, args...) ({ printk(KERN_DEBUG "D/[%s]:%3d:%s(): " fmt, THIS_MODULE->name,  __LINE__, __FUNCTION__, ## args);})


#define DMA_STOP  0
#define	DMA_START 1
static int status;
static int lch;
static int cnt;
static int dma_test_pages;
static int msecs;
static unsigned long paddr;
static unsigned long start_jiffies;

static void dma_test_cb(int lch, u16 ch_status, void *data)
{
	//struct completion *compl = data;
	cnt++;
	
	if (status == DMA_STOP) {
		return;
	}

	msecs = jiffies_to_msecs(jiffies - start_jiffies);
	start_jiffies = jiffies;
	omap_start_dma(lch);
}

static int dma_test(unsigned long paddr, unsigned pages)
{
	struct completion compl;
	unsigned elem_count = pages * PAGE_SIZE / 4;
	unsigned frame_count = 1;
	int r;

	r = omap_request_dma(OMAP_DMA_NO_DEVICE, "DMA TEST",
			dma_test_cb,
			&compl, &lch);
	if (r) {
		__E("request_dma failed for dma test\n");
		return -EBUSY;
	}

	omap_set_dma_transfer_params(lch, OMAP_DMA_DATA_TYPE_S32,
								 elem_count, frame_count,
								 OMAP_DMA_SYNC_ELEMENT,
								 0, 0);

	omap_set_dma_dest_params(lch, 0, OMAP_DMA_AMODE_POST_INC,
							 paddr, 0, 0);

	omap_set_dma_color_mode(lch, OMAP_DMA_CONSTANT_FILL, 0x000000);

	start_jiffies = jiffies;
	omap_start_dma(lch);
	return 0;
}


int dma_test_stop(void)
{
	status = DMA_STOP;
	omap_stop_dma(lch);
	omap_free_dma(lch);
	free_pages(paddr, get_order(dma_test_pages * PAGE_SIZE));

	cnt            = 0;
	msecs          = 0;
	paddr          = 0;
	start_jiffies  = 0;
	dma_test_pages = 0;
	
	return 0;
}

int dma_test_start(unsigned pages)
{
	void *vaddr;

	if (paddr && dma_test_pages) {
		__I("default: paddr = 0x%p, pages = %d \n", (void*)paddr, dma_test_pages);
	} else {
		vaddr = (void *)__get_free_pages(GFP_KERNEL, get_order(pages * PAGE_SIZE));
		if(vaddr == NULL) {
			__E("cannot alloc %d pages \n", pages);
			return -1;
		}

		paddr          = virt_to_phys(vaddr);
		dma_test_pages = pages;
	} 
	
	dma_test(paddr, dma_test_pages);
	status = DMA_START;
	return 0;
}


static ssize_t dma_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t nr_read = 0;

	nr_read += snprintf(buf + nr_read, PAGE_SIZE,
						"0 : DMA_STOP   \n"
						"1 : DMA_START  \n"
						"Current is %d  \n",
						status);

    return nr_read;
}

static ssize_t dma_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	
	sscanf(buf, "%u", &i);

	switch(i) {
		
	case DMA_STOP: 
		dma_test_stop();
		break;

	
	case DMA_START:
		dma_test_start(dma_test_pages);
		break;

	default:
		__E("dma status write %d error \n", i);
	} //switch
	
	return count;
}

static DEVICE_ATTR(status,
				   S_IRUGO | S_IWUSR,
				   dma_status_show,
				   dma_status_store);


static ssize_t dma_pages_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t nr_read = 0;

	nr_read += snprintf(buf + nr_read, PAGE_SIZE,
						"%d",
						dma_test_pages);

    return nr_read;
}

static ssize_t dma_pages_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &dma_test_pages);
	return count;
}

static DEVICE_ATTR(pages,
				   S_IRUGO | S_IWUSR,
				   dma_pages_show,
				   dma_pages_store);



static ssize_t dma_msecs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t nr_read = 0;

	nr_read += snprintf(buf + nr_read, PAGE_SIZE,
						"%d",
						msecs);

    return nr_read;
}

static DEVICE_ATTR(msecs,
				   S_IRUGO | S_IWUSR,
				   dma_msecs_show,
				   NULL);



static ssize_t dma_paddr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t nr_read = 0;

	nr_read += snprintf(buf + nr_read, PAGE_SIZE,
						"0x%p",
						(void*)paddr);

    return nr_read;
}

static DEVICE_ATTR(paddr,
				   S_IRUGO | S_IWUSR,
				   dma_paddr_show,
				   NULL);



static ssize_t dma_cnt_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t nr_read = 0;

	nr_read += snprintf(buf + nr_read, PAGE_SIZE,
						"%d",
						cnt);

    return nr_read;
}

static DEVICE_ATTR(cnt,
				   S_IRUGO | S_IWUSR,
				   dma_cnt_show,
				   NULL);



static struct attribute *dma_attributes[] = {
	&dev_attr_status.attr,
	&dev_attr_pages.attr,
	&dev_attr_msecs.attr,
	&dev_attr_paddr.attr,
	&dev_attr_cnt.attr,
	NULL
};



static struct attribute_group dma_attribute_group = {
    .attrs = dma_attributes
};


static struct device *device;
int dma_init(struct device *dev)
{
	if (sysfs_create_group(&dev->kobj, &dma_attribute_group)) {
		__E("sysfs_create_group() failed\n");
		return -1;
	}
	device = dev;
	return 0;
}

int dma_exit(void)
{
	struct device *dev = device;
	sysfs_remove_group(&dev->kobj, &dma_attribute_group);
	return 0;
}

