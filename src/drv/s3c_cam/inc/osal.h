#ifndef _OSAL_H_ 
#define _OSAL_H_

#ifdef __KERNEL__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pm.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/time.h> 
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/errno.h> /* error codes */
#include <linux/swap.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/irq.h>
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
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <soc/io.h>
#include <soc/div64.h>
#include <soc/irq.h>
#include <soc/hardware.h>
#include <soc/uaccess.h>
#include <soc/arch/map.h>
#include <soc/page.h>
//#include <soc/semaphore.h>
#include <soc/memory.h>
#include <soc/io.h>
#include <soc/cacheflush.h>
#include <soc/page.h>
#include <soc/pgtable.h>
#include <media/v4l2-dev.h>

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,16)
#include <linux/config.h>
#include <soc/arch/registers.h>
#endif

#ifdef LOG_TAG
#define __E(fmt, args...) ({ printk(KERN_ERR "E/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args); })
#define __W(fmt, args...) ({ printk(KERN_ERR "W/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args); })
#define __I(fmt, args...) ({ printk(KERN_ERR "I/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args); })

#define __DEBUG

#ifdef  __DEBUG
#define __D(fmt, args...) ({ printk(KERN_ERR "D/%-10s -> %3d: %s(): " fmt, LOG_TAG, __LINE__, __FUNCTION__, ## args); })
#else
//#warning "osal: NO DEBUG !!!"
#define __D(fmt, args...)
#endif

#else
//#warning "osal: need #define LOG_TAG !!!"
#define __D(fmt, args...)
#define __E(fmt, args...)
#define __W(fmt, args...)
#define __I(fmt, args...)
#endif	/* LOG_TAG */


#else 	/* __KERNEL__ */
/* osal for app */


#define OSAL_NAME "OSAL"

#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>


#define LIKELY_EXPECT
#ifdef LIKELY_EXPECT
# define likely(x)	    __builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)
#else
# define likely(x)      (x)  
# define unlikely(x)    (x)
#endif

#ifdef LOG_TAG

#define __E(fmt, args...)  ({fprintf(stderr, "E/%-10s: %20s -> %3d: %s(): " fmt, OSAL_NAME, LOG_TAG, __LINE__, __FUNCTION__, ## args); })
#define __W(fmt, args...)  ({fprintf(stderr, "W/%-10s: %20s -> %3d: %s(): " fmt, OSAL_NAME, LOG_TAG, __LINE__, __FUNCTION__, ## args); })
#define __I(fmt, args...)  ({fprintf(stderr, "I/%-10s: %20s -> %3d: %s(): " fmt, OSAL_NAME, LOG_TAG, __LINE__, __FUNCTION__, ## args); })


#ifdef NDEBUG
#define __D(fmt, args...)
#else
#define __D(fmt, args...)  ({fprintf(stderr, "D/%-10s  : %20s -> %3d: %s(): " fmt, OSAL_NAME, LOG_TAG,  __LINE__, __FUNCTION__, ## args); })
#endif

#else

#define __D(fmt, args...)
#define __E(fmt, args...)
#define __W(fmt, args...)
#define __I(fmt, args...)


#endif	/* LOG_TAG */


#define BILLION             1000000000L

#define EXPORT_SYMBOL(symbol)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* align addr on a size boundary - adjust address up/down if needed */
#define SIZE_MASK(size)         (~((size)-1))
#define _ALIGN_UP(addr,size)	(((addr)+((size)-1))&SIZE_MASK(size))
#define _ALIGN_DOWN(addr,size)	((addr)&(SIZE_MASK(size)))
#define _ALIGN(addr,size)       _ALIGN_UP(addr,size)

typedef char              s8;
typedef int16_t           s16;
typedef int32_t           s32;
typedef int64_t           s64;
typedef unsigned char     u8;

typedef uint16_t          u16;
typedef uint32_t          u32;
typedef uint64_t          u64;
typedef volatile uint32_t v32;

typedef struct {
	u8    b00:1;
	u8    b01:1;
	u8    b02:1;
	u8    b03:1;
	u8    b04:1;
	u8    b05:1;
	u8    b06:1;
	u8    b07:1;
} bit8_t;

typedef struct {
	u8    b00:1;
	u8    b01:1;
	u8    b02:1;
	u8    b03:1;
	u8    b04:1;
	u8    b05:1;
	u8    b06:1;
	u8    b07:1;
	u8    b08:1;
	u8    b09:1;
	u8    b10:1;
	u8    b11:1;
	u8    b12:1;
	u8    b13:1;
	u8    b14:1;
	u8    b15:1;
} bit16_t;

typedef struct {
	u8    b00:1;
	u8    b01:1;
	u8    b02:1;
	u8    b03:1;
	u8    b04:1;
	u8    b05:1;
	u8    b06:1;
	u8    b07:1;
	u8    b08:1;
	u8    b09:1;
	u8    b10:1;
	u8    b11:1;
	u8    b12:1;
	u8    b13:1;
	u8    b14:1;
	u8    b15:1;
	u8    b16:1;
	u8    b17:1;
	u8    b18:1;
	u8    b19:1;
	u8    b20:1;
	u8    b21:1;
	u8    b22:1;
	u8    b23:1;
	u8    b24:1;
	u8    b25:1;
	u8    b26:1;
	u8    b27:1;
	u8    b28:1;
	u8    b29:1;
	u8    b30:1;
	u8    b31:1;
} bit32_t;

typedef struct buffer {
	void *  vaddr; /* start */
	u8*     paddr; /* paddr_start */
	size_t  length;
	s32     w;
	s32     h;
	s32     stride;
	s32     offset;
	s32     id;
	s32     used;
	void   *gf_surf;
} buffer_t;

static inline void rate_control(int fps)
{
	struct timespec         t_end;
	static struct timespec  t_start;
	struct timespec         t_sleep;
	long                    t_dif;
	long                    t_span = BILLION / fps;	/* 33333333 ,30fps */
	long                    t_tmp;
	
	clock_gettime(CLOCK_REALTIME, &t_end);
	
	t_dif = t_end.tv_nsec - t_start.tv_nsec;
	if(t_dif < t_span && t_dif >= 0) {
		t_tmp = t_span - t_dif;
		t_sleep.tv_sec  = 0;
		t_sleep.tv_nsec = t_tmp;
		
		nanosleep(&t_sleep, NULL);

		t_end.tv_nsec = (t_end.tv_nsec + t_tmp) % BILLION;
		
		__D("sleep %10d(ns) \n", (int)t_sleep.tv_nsec);
	} 

	t_start = t_end;
}

typedef enum {
	__GFP_ZERO,
	GFP_KERNEL,
	NR_GFP
} gfp_t;

static inline void * kzalloc(size_t size, gfp_t flags)
{
	s32   align  = 16;
	u8    value  = sizeof(value);
	u8   *p      = (typeof(p))malloc(size + align + value);
	if(p == NULL) return NULL;
	
	u8   *a      = (typeof(a))_ALIGN((unsigned long)p+value, align);
	(a-1)[0] = a - p;				/* set value of offset */
	
	memset(a, 0, size);
	__D("size = %d, alloc_addr = %p,  align%d_addr = %p \n", size, p, align, a);
	return a;
}

static inline void kfree(void * objp)
{
	u8   *a      = (typeof(a))objp;
	u8   *p      = a - (a-1)[0];
	
	__D("alloc_addr = %p,  align_addr = %p \n", p, a);
	free(p);
}

typedef struct completion {
	sem_t semaphore;
} completion_t;

static inline void init_completion(struct completion *x)
{
	int ipc_shared = 0;
	sem_init(&x->semaphore, ipc_shared, 0);
}
static inline void exit_completion(struct completion *x)
{
	sem_destroy(&x->semaphore);
}

static inline void wait_for_completion(completion_t* x)
{
	sem_wait(&x->semaphore);
}

static inline void complete(completion_t* x)
{
	sem_post(&x->semaphore);
}

typedef struct task_struct {
	pthread_t           thread;
	pthread_attr_t      attr;
	void               *thread_return;
	s32                 stop;
	completion_t        wait;
	s8                  name[64];
}task_struct_t;

static inline int kthread_should_stop(task_struct_t *k)
{
	return k->stop;
}

static inline int wake_up_process(task_struct_t *k)
{
	complete(&k->wait);
	return 0;
}

static inline struct task_struct *kthread_run(void* (*th)(void *data), void *data, const char name[], ...)
{
	task_struct_t      *t   = kzalloc(sizeof(*t), GFP_KERNEL);
	struct sched_param  param;
	int                 policy;
	pthread_attr_init(&t->attr);
	pthread_getschedparam(pthread_self(), &policy, &param);
	//pthread_attr_setinheritsched(&t->attr, PTHREAD_EXPLICIT_SCHED);
#if 1
	char               *str = NULL;
	if(str = strstr(name, "SCHED_")) {
		if(str = strstr(name, "RR")) {
			policy = SCHED_RR;
		} else if(str = strstr(name, "FIFO")) {
			policy = SCHED_FIFO;
		} else {
			//...;
		}
	} 	
	pthread_attr_setschedpolicy(&t->attr, policy);
	
	if(str = strstr(name, "=")) {
		int priority = 0;
		sscanf(str, "=%d", &priority);
		param.sched_priority = priority;
	}
#endif

	pthread_attr_setschedparam(&t->attr, &param);
	t->stop = 0;
	init_completion(&t->wait);
	pthread_create(&t->thread, &t->attr, th, data);
	
	return t;
}

static inline int kthread_stop(struct task_struct *k)
{
	k->stop = 1;
	exit_completion(&k->wait);
	//pthread_kill(k->thread, SIGKILL);
	pthread_join(k->thread, &k->thread_return);
	
	kfree(k);
	return 0;
}

typedef struct shmem_list {
	void *vaddr;
	u32   size;
	char  name[32];
} shmem_list_t;

static shmem_list_t shmem_list[8] = {{0}};
static inline void* shmem_open(char *name, int size)
{
    int fd = shm_open(name, O_RDWR | O_CREAT, S_IRWXU);
    if(fd == -1) {
        __E("error opening the shared memory object\n");
        return NULL;
    }
    ftruncate(fd, size);
    void *shmem = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED,
				 fd, 0);
    close(fd);

	s32 i;
	for(i = 0; i < ARRAY_SIZE(shmem_list); i++) {
		if(shmem_list[i].vaddr == 0) {
			shmem_list[i].vaddr = shmem;
			shmem_list[i].size  = size;
			strncpy(shmem_list[i].name, name,
					sizeof(shmem_list[i].name));
			break;
		}
	}
	
	return shmem;
}

static inline int shmem_close(void *shmem)
{
	char *name;
	int   size = 0;

	s32 i;
	for(i = 0; i < ARRAY_SIZE(shmem_list); i++) {
		if(shmem_list[i].vaddr == shmem) {
			size = shmem_list[i].size;
			name = shmem_list[i].name;
			shmem_list[i].vaddr = 0;
			break;
		}
	}
	
    munmap(shmem, size);
	return 0;
}

static inline ssize_t read_timeout(int fd, void *buf, size_t size, int sec)
{
	struct timeval timeout = {.tv_sec=sec, .tv_usec=0};

	fd_set readfd;
	FD_ZERO(&readfd);
	FD_SET(fd, &readfd);
		
	int ret = select(fd+1, &readfd, NULL, NULL, &timeout);
	if(ret <= 0) {
		/* is timeout */
		return 0;
	}
		
	if(FD_ISSET(fd, &readfd)) {
		ret = read(fd, buf, size);
	}
	
	return ret;
}

#endif	/* __KERNEL__ */

#endif	/* _OSAL_H_ */



