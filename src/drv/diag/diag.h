#ifndef _DIAG_H_
#define _DIAG_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#define DIAG_NAME    "diag"
#define DIAG_DEVICE  "/dev/diag"
#define DIAG_MAJOR_NUMBER 'k'   // 'k' equal decimal 107.
#define DIAG_MINOR_NUMBER  0

typedef struct {
	unsigned pages;
	int      ret;
} diag_dma_test_t;

typedef struct {
    unsigned long  vaddr;
    unsigned long  paddr;
} diag_get_paddr_t;

typedef struct {
    int id;
    int irq;
    int cnt;
} diag_irq_t;

#define DIAG_GET_PADDR         _IOWR(DIAG_MAJOR_NUMBER, 0, uint32_t)
#define DIAG_IRQCNT_BEGIN      _IOWR(DIAG_MAJOR_NUMBER, 1, uint32_t)
#define DIAG_IRQCNT_END        _IOWR(DIAG_MAJOR_NUMBER, 2, uint32_t)
#define DIAG_IRQCNT_GET        _IOWR(DIAG_MAJOR_NUMBER, 3, uint32_t)
#define DIAG_DMA_TEST_START    _IOWR(DIAG_MAJOR_NUMBER, 4, uint32_t)
#define DIAG_IOC_NR            5


#ifdef __KERNEL__

#else
static int diag_fd;
static inline int diag_open(void)
{
    if(diag_fd > 0) {
        return diag_fd;
    }

    diag_fd = open(DIAG_DEVICE, O_RDWR);
    return diag_fd;
}

static inline void diag_close(void)
{
    if(diag_fd > 0) close(diag_fd);
    diag_fd = 0;
}

static inline unsigned long diag_get_paddr(unsigned long vaddr)
{
    diag_get_paddr_t a;
    int ret;
    int fd;
    
    if((fd = diag_open()) < 0) {
        return 0;
    }

    a.vaddr = vaddr;
    ret = ioctl(fd, DIAG_GET_PADDR, &a);
    if(ret < 0 || a.paddr == 0) {
        __E("cannot DIAG_GET_PADDR from vaddr: 0x%lx \n",
            a.vaddr);
        return 0;
    }
    
    return a.paddr;
}

static inline int diag_dma_test_start(unsigned test_pages)
{
	diag_dma_test_t diag_dma_test;
	int ret;
	int fd;
    
    if((fd = diag_open()) < 0) {
        return 0;
    }

	diag_dma_test.pages = test_pages;
	ret = ioctl(fd, DIAG_DMA_TEST_START, &diag_dma_test);
	if(ret < 0) {
		__E("cannot DIAG_DMA_TEST_START \n");
	}

	return diag_dma_test.ret;
}



#endif

#endif  // _DIAG_H_
