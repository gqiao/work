#ifndef _OMAP3_PMU_H_
#define _OMAP3_PMU_H_

/*******************************************************************

        Ipera Technology Proprietary & Confidential

        All Rights Reserved,  2006-2007(c)

	Author: Bob

	Date:2008-07-08

*******************************************************************/

#define K_DEV   "ipera"			/* full path is /dev/ipera */

//IOCTL number in ipera_k.c
#define IPERA_INIT_PMU_STATICS              0x1
#define IPERA_START_PMU_CACHES_STATICS      0x2
#define IPERA_END_PMU_CACHES_STATICS        0x3
#define IPERA_GET_STATICS                   0x4
#define IPERA_GET_PHYS                      0x5

//Define event number in ipera_k.c
#define	PMU_UNUSED                          0x0
#define	PMU_ICACHE_EXEC               0x08
#define	PMU_ICACHE_MISS               0x01
#define	PMU_DCACHE_ACCESS             0x04
#define	PMU_DCACHE_MISS               0x03

#define	PMU_ICACHE_FETCH_LATENCY            0x0001
#define	PMU_DBUFFER_STALL                   0x0908
#define	PMU_WRITEBACK_STATICS               0x0C02
#define	PMU_ITLB_EFFICIENCY                 0x0307
#define	PMU_DTLB_EFFICIENCY                 0x040A

//Define event counter in ipera_k.c
#define EVENT_COUNTER0		0
#define EVENT_COUNTER1		1
#define EVENT_COUNTER2		2
#define EVENT_COUNTER3		3

typedef enum
{
	PMU_ALG_GS = 0,
	PMU_ALG_MOSQUITO,
	PMU_ALG_PEAKING,
	PMU_ALG_CE,
	PMU_ALG_SCALING,
	MAX_PMU_ALG_CNT,
} pmu_alg_type_t;

typedef struct
{
	unsigned long long 	pmu_count;
	unsigned long long  pmu_cycles;
	
	unsigned long long 	pmu_instr_exec;
	unsigned long long 	pmu_icache_miss;
	unsigned long long 	pmu_dcache_access;
	unsigned long long 	pmu_dcache_miss;
} pmu_statics_t;

extern unsigned int set_pmu_event_asm(unsigned int EvtNumber, unsigned int EvtCounter);
extern void start_pmu_asm(void);
extern void stop_pmu_asm(void);
extern unsigned int get_clock_counter_asm(void);
extern unsigned int get_pmnx_counter_asm(unsigned int EvtCounter);

 pmu_statics_t pmu_statics[MAX_PMU_ALG_CNT];


///Function declaration
#ifdef __KERNEL__
//#warning this is kernel module

#ifdef __DEBUG
#define __D(fmt, args...) ({ printk(KERN_ERR "Ipera Debug: %s: " fmt, __FUNCTION__, ## args);	})
#else
#define __D(fmt, args...)
#endif

#define __E(fmt, args...) ({ printk(KERN_ERR "Ipera Error: %s: " fmt, __FUNCTION__, ## args);	})


#else  /* __APPLICATION__ */
//#warning this is application module
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>



#ifndef __D
#ifdef __DEBUG
#define __D(fmt, args...) ({ printf("Debug: %s: %s: " fmt, __FILE__, __FUNCTION__, ## args); })
#else
#define __D(fmt, args...)
#endif
#endif	/* __D */
	
#ifndef __E
#define __E(fmt, args...) ({ printf("Error: %s: %s: " fmt, __FILE__, __FUNCTION__, ## args); })
#endif	/* __E */
	
//#define SUCCESS               0
//#define ERROR                -1
	
static int k_fd;


static inline int open_pmu(void)
{
	int   flags     = O_RDWR;
	char  name[30];

	snprintf(name, sizeof(name), "/dev/%s", K_DEV);
	
    k_fd = open(name, flags);
    if(k_fd == -1) {
        __E("Failed to open %s\n", name);
        return ERROR;
    }
	return SUCCESS;
}

static inline int close_pmu(void)
{
	if(close(k_fd) == -1) {
		__E("Failed to close /dev/%s \n", K_DEV);
		return ERROR;
	}
	return SUCCESS;
}
	
static inline void init_pmu(void)
{
	memset(pmu_statics, 0, sizeof(pmu_statics_t) * MAX_PMU_ALG_CNT);
	if(ioctl(k_fd, IPERA_INIT_PMU_STATICS, NULL) == -1) {
		__E("Failed to IPERA_INIT_PMU_STATICS\n");
    }
}

static inline void start_pmu_caches(unsigned int type)
{
	set_pmu_event_asm(PMU_ICACHE_EXEC, EVENT_COUNTER0);
	set_pmu_event_asm(PMU_ICACHE_MISS, EVENT_COUNTER1);
	set_pmu_event_asm(PMU_DCACHE_ACCESS, EVENT_COUNTER2);
	set_pmu_event_asm(PMU_DCACHE_MISS, EVENT_COUNTER3);
	start_pmu_asm();
}

static inline void end_pmu_caches(unsigned int type)
{
	stop_pmu_asm();
	pmu_statics[type].pmu_count		+= 1; 
	pmu_statics[type].pmu_cycles    += get_clock_counter_asm();
	pmu_statics[type].pmu_instr_exec    += get_pmnx_counter_asm(EVENT_COUNTER0);
	pmu_statics[type].pmu_icache_miss    += get_pmnx_counter_asm(EVENT_COUNTER1);
	pmu_statics[type].pmu_dcache_access    += get_pmnx_counter_asm(EVENT_COUNTER2);
	pmu_statics[type].pmu_dcache_miss    += get_pmnx_counter_asm(EVENT_COUNTER3);
}

static inline pmu_statics_t get_statics(unsigned int type)
{
	return (pmu_statics_t)pmu_statics[type];
}

static inline unsigned long get_phys(void *virtp)
{
	unsigned long        physp = (unsigned long)virtp;
	
    if (ioctl(k_fd, IPERA_GET_PHYS, &physp) == -1) {
        __E("Failed to get physical address of virtp = %p\n", virtp);
        return ERROR;
    }
    return physp;
}

#endif	/* __KERNEL__ */

#endif	/* _OMAP3_PMU_H_ */
