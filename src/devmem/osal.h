#if !defined _OSAL_H_ 
#define _OSAL_H_

#ifdef __KERNEL__

#ifdef LOG_TAG

#ifdef LOG_MORE
#define MORE_FMT "%s()->"
#define MORE_ARG ({char b[KSYM_NAME_LEN]; strchr(sprint_sym(b, __builtin_return_address(0)), '+')[0] = '\0'; b;}),
#else
#define MORE_FMT
#define MORE_ARG
#endif

#define __E(fmt, args...) ({ printk(KERN_ERR "E/[%s]:" MORE_FMT "%3d:%s(): " fmt, THIS_MODULE->name, MORE_ARG  __LINE__, __FUNCTION__, ## args);})
#define __W(fmt, args...) ({ printk(KERN_ERR "W/[%s]:" MORE_FMT "%3d:%s(): " fmt, THIS_MODULE->name, MORE_ARG  __LINE__, __FUNCTION__, ## args);})
#define __I(fmt, args...) ({ printk(KERN_ERR "I/[%s]:" MORE_FMT "%3d:%s(): " fmt, THIS_MODULE->name, MORE_ARG  __LINE__, __FUNCTION__, ## args);})
#define __P(fmt, args...) ({ printk(KERN_ERR fmt, ## args);})

#ifdef __DEBUG
#define __D(fmt, args...) ({ printk(KERN_ERR "D/[%s]:" MORE_FMT "%3d:%s(): " fmt, THIS_MODULE->name, MORE_ARG  __LINE__, __FUNCTION__, ## args);})
#else
#warning "osal: NO DEBUG !!!"
#define __D(fmt, args...)
#endif

#else
#warning "osal: need #define LOG_TAG !!!"
#define __D(fmt, args...)
#define __E(fmt, args...)
#define __W(fmt, args...)
#define __I(fmt, args...)
#endif	/* LOG_TAG */

#include <linux/version.h>
#include <linux/init.h>
#include <asm/local.h>


#include <linux/kallsyms.h>
#define sprint_sym(buf, addr) ({sprint_symbol((buf), (unsigned long)(addr)); (buf);})
#define print_sym(fmt, addr)  ({print_symbol((fmt), (unsigned long)(addr));})
#define get_sym(addr)         ({static char sym_buf[KSYM_NAME_LEN]; sprint_sym((sym_buf), (addr));})


#include <linux/module.h>
typedef struct kernel_symbol kernel_symbol_t;
#if 1
//#define S(x) ({typeof(&x) addr = symbol_get(x); if(!addr) {__E("no symbol %s\n", #x);return -1;} symbol_put_addr(addr); addr;})
#define S(x) ({const kernel_symbol_t *sym = find_symbol( #x, NULL, NULL, true, true); if(!sym){__E("no symbol %s\n", #x);return -1;} (typeof(&x))sym->value;})
#else
#define S(x) x
#endif


typedef struct module module_t;
static inline int print_mod(module_t *ret)
{
	if(ret == NULL) {
		__I("module is NULL ! \n");
		return -1;
	}
	
	__I("name:        %s \n", ret->name);
	__I("state:       %d \n", ret->state);
	__I("core_size:   %d \n", ret->core_size);
	__I("module_core: %p \n", ret->module_core);
	print_sym("sym of module_core: %s \n", ret->module_core);
	__I("refs of %s is %d \n", ret->name, module_refcount(ret));
	
	return 0;
}

/*
 * module_address - get the module which contains an address.
 * @addr: the address.
 */
static inline module_t* module_address(unsigned long addr)
{
	module_t *ret;

	preempt_disable();
	ret = __module_address(addr);
	preempt_enable();
	
	return ret;
}

static inline module_t* module_text_address(unsigned long addr)
{
	module_t *ret;

	preempt_disable();
	ret = __module_text_address(addr);
	preempt_enable();
	
	return ret;
}

static inline int module_get(module_t *module)
{
	if(!module_is_live(module)) {
		return -1;
	}
	
	__module_get(module);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
typedef struct module_ref module_ref_t;
static inline module_ref_t* module_ref_addr(module_t *mod)
{
	unsigned int  cpu = get_cpu();
	module_ref_t *ret = per_cpu_ptr(mod->refptr, cpu);
	put_cpu();
	
	return ret;
}

#else

typedef local_t module_ref_t;
static inline module_ref_t* module_ref_addr(module_t *mod)
{
	unsigned int  cpu = get_cpu();
	module_ref_t *ret = __module_ref_addr(mod, cpu);
	put_cpu();
	
	return ret;
}
#endif

static inline void module_ref_inc(module_t *mod)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
	//module_ref_addr(mod)->incs++;
#else
	//local_inc(module_ref_addr(mod));
#endif
	module_get(mod);
}

static inline void module_ref_dec(module_t *mod)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
	//module_ref_addr(mod)->decs++;
#else
	//local_dec(module_ref_addr(mod));
#endif	
	module_put(mod);
}

static inline int module_name_is_live(const char *name)
{
	int       live = 0;
	module_t *mod  = find_module(name);

	if(mod) {
		live = module_is_live(mod);
	}
	
	return live;
}

#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>
typedef struct task_struct task_t;

#include <linux/wait.h>
static inline unsigned int sleep(unsigned int seconds)
{
	unsigned int msecs   = seconds * 1000;
	unsigned int timeout = msecs_to_jiffies(msecs);
	
	set_current_state(TASK_UNINTERRUPTIBLE);
	timeout = schedule_timeout(timeout);
	
	return timeout ? (jiffies_to_msecs(timeout)/1000) : 0;
}

#include <linux/list.h>



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
#include <linux/miscdevice.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <media/v4l2-dev.h>


#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,16)
#include <linux/config.h>
#include <asm/arch/registers.h>
#endif


MODULE_LICENSE("GPL");

/**
 * osal_virt_to_phys : translate user/virtual address to phy address
 * @virtp: user/virtual address
 *
 * This inline function is used to convert user space virtual address to
 * physical address.
 */
static inline u32 osal_virt_to_phys(u32 virtp)
{
	unsigned long physp = 0;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;

	vma = find_vma(mm, virtp);

	/* For kernel direct-mapped memory, take the easy way */
	if (virtp >= PAGE_OFFSET)
		physp = virt_to_phys((void *)virtp);
	else if (vma && (vma->vm_flags & VM_IO) && (vma->vm_pgoff))
		/**
		 * this will catch, kernel-allocated, mmaped-to-usermode
		 * addresses
		 */
		physp = (vma->vm_pgoff << PAGE_SHIFT) + (virtp - vma->vm_start);
	else {
		/* otherwise, use get_user_pages() for general userland pages */
		int res, nr_pages = 1;
			struct page *pages;

		down_read(&current->mm->mmap_sem);

		res = get_user_pages(current, current->mm,
				     virtp, nr_pages, 1, 0, &pages, NULL);
		up_read(&current->mm->mmap_sem);

		if (res == nr_pages)
			physp = __pa(page_address(&pages[0]) +
				     (virtp & ~PAGE_MASK));
		else {
			__E("get_user_pages failed\n");
			return 0;
		}
	}
	return physp;
}



#else  /* app */

#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/time.h> 
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

#define __E(fmt, args...)  ({fprintf(stderr, "E/%-10s: %3d: %s(): " fmt, LOG_TAG,  __LINE__, __FUNCTION__, ## args);})
#define __W(fmt, args...)  ({fprintf(stderr, "W/%-10s: %3d: %s(): " fmt, LOG_TAG,  __LINE__, __FUNCTION__, ## args);})
#define __I(fmt, args...)  ({fprintf(stderr, "I/%-10s: %3d: %s(): " fmt, LOG_TAG,  __LINE__, __FUNCTION__, ## args);})
#define __D(fmt, args...)  ({fprintf(stderr, "D/%-10s: %3d: %s(): " fmt, LOG_TAG,  __LINE__, __FUNCTION__, ## args);})

#else

#define __D(fmt, args...)
#define __E(fmt, args...)
#define __W(fmt, args...)
#define __I(fmt, args...)

#endif	/* LOG_TAG */

#define BILLION             1000000000L

#define EXPORT_SYMBOL(symbol)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define PAGE_SIZE  getpagesize()

/* align addr on a size boundary - adjust address up/down if needed */
#define SIZE_MASK(size)         (~((size)-1))
#define _ALIGN_UP(addr,size)	(((addr)+((size)-1))&SIZE_MASK(size))
#define _ALIGN_DOWN(addr,size)	((addr)&(SIZE_MASK(size)))
#define _ALIGN(addr,size)       _ALIGN_UP(addr,size)


typedef char              s8;
typedef unsigned char     u8;

typedef int16_t           s16;
typedef uint16_t          u16;

typedef int32_t           s32;
typedef uint32_t          u32;
typedef volatile u32      v32;

typedef int64_t           s64;
typedef uint64_t          u64;

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
	u8*     vaddr; /* start */
	u8*     paddr; /* paddr_start */
	size_t  length;
	s32     w;
	s32     h;
	s32     stride;
	s32     offset;
	s32     id;
	s32     used;
	void   *gf_surf;
}buffer_t;


static inline int be(char *argv[])
{
	return execv(argv[0], argv);
}

static inline char* valid_file(char* path[], char* filename)
{
	static char pathname[128] = "";
	int i;
	for(i = 0; path[i]; i++) {
		sprintf(pathname, "%s/%s", path[i], filename);
		if(access(pathname, R_OK) == 0) {
			return pathname;
		}
	}
		
	return NULL;
}


static void at_exit_sig(int signo, siginfo_t *info, void *context)
{
	exit(0);
}
	
static inline int at_exit(void (*function)(void))
{
	struct sigaction act, *a = &act;
	a->sa_flags     = SA_SIGINFO;
	a->sa_sigaction = at_exit_sig;
	
	sigemptyset(&a->sa_mask);
	sigaction(SIGINT,   a, NULL);
	sigaction(SIGTERM,  a, NULL);
 
	return atexit(function);
}

static inline void rate_control(int fps)
{
	struct timespec         t_end;
	static struct timespec  t_start;
	struct timespec         t_sleep;
	long                    t_dif;
	long                    t_span = BILLION / fps;	//33333333 ,30fps */
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
	a[-1] = a - p;				/* set value of offset */
	
	memset(a, 0, size);
	__D("size = %d, alloc_addr = %p,  align%d_addr = %p \n", (int)size, p, align, a);
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
	while(sem_wait(&x->semaphore) != 0);
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
	pthread_join(k->thread, &k->thread_return);
	
	kfree(k);
	return 0;
}

static inline int kthread_wait(struct task_struct *k)
{
	pthread_join(k->thread, &k->thread_return);
	exit_completion(&k->wait);
	kfree(k);
	return 0;
}

static inline s32 buffer_copy_execute(buffer_t *s, buffer_t *d)
{
	s32 i;
	
	for(i = 0; i < s->h; i++) {
		u8 *s_l = ((u8*)s->vaddr) + s->stride * i;
		u8 *d_l = ((u8*)d->vaddr) + d->stride * i;
		memcpy(d_l, s_l, s->w*2);
	}
	
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


typedef struct list_head {
	struct list_head *next, *prev;
} list_head_t;

#define LIST_HEAD(name)								\
	struct list_head name = { &(name), &(name) }

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *_new,
							  struct list_head *prev,
							  struct list_head *next)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

static inline void list_add(struct list_head *_new, struct list_head *head)
{
	__list_add(_new, head, head->next);
}

static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
	__list_add(_new, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_replace(struct list_head *old,
								struct list_head *_new)
{
	_new->next = old->next;
	_new->next->prev = _new;
	_new->prev = old->prev;
	_new->prev->next = _new;
}

static inline void list_replace_init(struct list_head *old,
									 struct list_head *_new)
{
	list_replace(old, _new);
	INIT_LIST_HEAD(old);
}

static inline void list_del_init(struct list_head *entry)
{
	list_del(entry);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
								  struct list_head *head)
{
	list_del(list);
	list_add_tail(list, head);
}

static inline int list_is_last(const struct list_head *list,
							   const struct list_head *head)
{
	return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct list_head *list,
								 struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

static inline void list_splice_init(struct list_head *list,
									struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

#define OFFSETOF(type, member) ((long)&((type*)0)->member)

#define list_entry(ptr, type, member)					\
	((type *)((char*)(ptr) - OFFSETOF(type,member)))

#define list_for_each(pos, head)								\
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head)							\
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/* usage: first_next = head->next */
#define list_for_each_entry(pos, head, member)						\
	for (pos = list_entry((head)->next, typeof(*pos), member);		\
		 &pos->member != (head);									\
		 pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_reverse(pos, head, member)				\
	for (pos = list_entry((head)->prev, typeof(*pos), member);		\
		 &pos->member != (head);									\
		 pos = list_entry(pos->member.prev, typeof(*pos), member))

#define list_prepare_entry(pos, head, member)			\
	((pos) ? : list_entry(head, typeof(*pos), member))

/* usage: first_next = pos->member.next */
#define list_for_each_entry_continue(pos, head, member)				\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
		 &pos->member != (head);									\
		 pos = list_entry(pos->member.next, typeof(*pos), member))

/* usage: first_next_list_entry = pos */
#define list_for_each_entry_from(pos, head, member)					\
	for (;															\
		 &pos->member != (head);									\
		 pos = list_entry(pos->member.next, typeof(*pos), member))


void cleanup_module(void);
int init_module(void);

#ifdef EXE
#define module_exit(fp) void cleanup_module(void) {fp();}
#define module_init(fp) int init_module(void){return fp();} int main(int argc, char *argv[]) {at_exit(main_exit); return init_module();}
#elif defined LIB
#define module_exit(fp) void cleanup_module(void) {fp();}
#define module_init(fp) int init_module(void){return fp();} 
#endif//if EXE

#endif//if KERNEL

#endif	/* _OSAL_H_ */
