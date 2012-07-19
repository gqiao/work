#define LOG_TAG "s3c_camif_fsm.c"
#include <osal.h>

#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/semaphore.h>


//#define CAMIF_DEBUG 

//#include "s3c_camif.h"
#include "s3c_camif_fsm.h"
/* 
 * FSM function is the place where Synchronization in not necessary
 * because IRS calls this functions.
 */

ssize_t camif_start_1fsm(camif_cfg_t *cfg)
{
	__D("\n");
	printk(KERN_INFO "camif_start_1fsm \n");
	//camif_reset(CAMIF_RESET,0);
	cfg->capture_enable = CAMIF_DMA_ON;
	camif_capture_start(cfg);
	camif_last_irq_en(cfg);
	cfg->status = CAMIF_STARTED;
	cfg->fsm = CAMIF_Yth_INT;
	cfg->perf.frames = 0;
	return 0;
}


ssize_t camif_start_2fsm(camif_cfg_t *cfg)
{
	__D("\n");
	camif_reset(CAMIF_RESET,0);/* FIFO Count goes to zero */
	cfg->capture_enable = CAMIF_DMA_ON;
	camif_capture_start(cfg);
	cfg->status = CAMIF_STARTED;
	cfg->fsm = CAMIF_1st_INT;
	cfg->perf.frames = 0;
	return 0;
}


ssize_t camif_start_preview(camif_cfg_t *cfg)
{
	__D("\n");
	camif_reset(CAMIF_RESET,0); /* FIFO Count goes to zero */
	cfg->capture_enable = CAMIF_DMA_ON;
	camif_capture_start(cfg);
	cfg->status = CAMIF_STARTED;
	cfg->fsm = CAMIF_1st_INT;
	cfg->perf.frames = 0;

#if 0	
	if(cfg->input_channel == MSDMA_FROM_PREVIEW){
		camif_preview_msdma_start(cfg);
	}
#endif
	return 0;
}

ssize_t camif_pc_stop(camif_cfg_t *cfg)
{
	__D("\n");
	cfg->capture_enable = CAMIF_BOTH_DMA_OFF;
//	cfg->status = CAMIF_STOPPED;
	camif_capture_stop(cfg);
	cfg->perf.frames = 0;	/* Dupplicated ? */
	return 0;
}

/* Policy:  
   cfg->perf.frames  set in camif_fsm.c
   cfg->status set in video-driver.c
*/

/* 
 * Don't insert camif_reset(CAM_RESET, 0 ) into this func 
 */

ssize_t camif_stop_preview(camif_cfg_t *cfg)
{
	__D("\n");
	cfg->capture_enable = CAMIF_DMA_OFF;
	cfg->status = CAMIF_STOPPED;
	camif_capture_stop(cfg);
	cfg->perf.frames = 0;	/* Dupplicated ? */
	return 0;
}

ssize_t camif_stop_codec(camif_cfg_t *cfg)
{
	__D("\n");
	cfg->capture_enable = CAMIF_DMA_OFF;
	cfg->status = CAMIF_STOPPED;
	camif_capture_stop(cfg);
	cfg->perf.frames = 0;	/* Duplicated ? */
	return 0;
}

/* When C working, P asks C to play togehter */
/* Only P must call this function */
static void camif_start_p_with_c(camif_cfg_t *cfg);

void camif_start_c_with_p(camif_cfg_t *cfg, camif_cfg_t *other)
{
	__D("\n");
//	cfg->other = get_camif(CODEC_MINOR);
	cfg->other = other;
	camif_start_p_with_c(cfg);
}

static void camif_start_p_with_c(camif_cfg_t *cfg)
{   
	camif_cfg_t *other = (camif_cfg_t *)cfg->other;

	__D("\n");
	
	/* Preview Stop */
	cfg->capture_enable = CAMIF_DMA_OFF;
	camif_capture_stop(cfg);
	
	/* Start P and C */
	camif_reset(CAMIF_RESET, 0);
	cfg->capture_enable =CAMIF_BOTH_DMA_ON;
	
	camif_capture_start(cfg);
	cfg->fsm = CAMIF_1st_INT; /* For Preview */
	
	if(!other) panic("s3c_camif_fsm.c : Unexpected Error - other is null \n");
	
	switch (other->pp_num) {
	case 4:
		other->fsm = CAMIF_1st_INT; /* For Codec */
		break;	
	case 1:
		other->fsm = CAMIF_Yth_INT;
		break;
	default:
		panic("Not Supporting pp_num");
		break;
	}
}

static void camif_auto_restart(camif_cfg_t *cfg)
{
	__D("\n");	
//	if (cfg->dma_type & CAMIF_CODEC) return;
	if (cfg->auto_restart)
		camif_start_p_with_c(cfg);
}


/* Supposed that PREVIEW already running 
 * request PREVIEW to start with Codec 
 */
static int camif_check_global(camif_cfg_t *cfg)
{
	int ret = 0;
 	 
	__D("\n");

        if (mutex_lock_interruptible(&cfg->cis->lock)) 
				return -ERESTARTSYS;
        if ( cfg->cis->status & CWANT2START ) {
		cfg->cis->status &= ~CWANT2START;
		cfg->auto_restart = 1;
		ret = 1;
	}
	else {
		ret = 0; /* There is no codec */
		cfg->auto_restart = 0; /* Duplicated ..Dummy */
	}

	mutex_unlock(&cfg->cis->lock);

        return ret;
}

/*
 *    1st INT : Start Interrupt
 *    Xth INT : enable Last IRQ : pingpong get the valid data
 *    Yth INT : Stop Codec or Preview : pingpong get the valid data
 *    Zth INT : Last IRQ : valid data 
 */
#define CHECK_FREQ 5

int camif_enter_p_4fsm(camif_cfg_t *cfg)
{
	int ret = 0;

	__D("\n");

	cfg->perf.frames++;
	if (cfg->fsm == CAMIF_NORMAL_INT)
		if (cfg->perf.frames % CHECK_FREQ == 0) 
			ret = camif_check_global(cfg);
	if (ret > 0) cfg->fsm = CAMIF_Xth_INT; /* Codec wait for Preview */
	
	switch (cfg->fsm) {
	case CAMIF_1st_INT:           /* Start IRQ */
		cfg->fsm = CAMIF_NORMAL_INT;
		ret = INSTANT_SKIP;
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : 1st INT \n");	
		break;
		
	case CAMIF_NORMAL_INT:	
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_NORMAL_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : NORMAL INT \n");	
		break;
		
	case CAMIF_Xth_INT:	
		camif_last_irq_en(cfg);/* IRQ for Enabling LAST IRQ */
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_Yth_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : Xth INT \n");	
		break;	
		
	case CAMIF_Yth_INT:        /* Capture Stop */
		cfg->capture_enable = CAMIF_DMA_OFF;
		cfg->status = CAMIF_INT_HAPPEN;
		camif_capture_stop(cfg);
		cfg->fsm = CAMIF_Zth_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : Yth INT \n");	
		break;
		
	case CAMIF_Zth_INT: 	/*  LAST IRQ (Dummy IRQ */
		cfg->fsm = CAMIF_DUMMY_INT;
		cfg->status = CAMIF_INT_HAPPEN;
		ret = INSTANT_GO;
		camif_auto_restart(cfg);  /* Automatically Restart Camera */
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : Zth INT \n");
		break;	
		
	case CAMIF_DUMMY_INT:
		cfg->status = CAMIF_STOPPED; /* Dupplicate ? */
		ret = INSTANT_SKIP;
		DPRINTK(KERN_INFO "camif_enter_p_4fsm() : Dummy INT \n"); 
		break;

	default:
		printk(KERN_INFO "camif_enter_p_4fsm() : Unexpected INT %d \n",cfg->fsm); 
		ret = INSTANT_SKIP;
		break;
	}
	return ret;
}


/* 
 * NO autorestart included in this function 
 */
int camif_enter_c_4fsm(camif_cfg_t *cfg)
{
	int ret;

	__D("\n");
	   
	cfg->perf.frames++;
#if 0
	if (   (cfg->fsm==CAMIF_NORMAL_INT)
		   && (cfg->perf.frames>cfg->restart_limit-1)
		) 
		cfg->fsm = CAMIF_Xth_INT;
#endif
	switch (cfg->fsm) {
	case CAMIF_1st_INT:           /* Start IRQ */
		cfg->fsm = CAMIF_NORMAL_INT;
//		cfg->status = CAMIF_STARTED; /* need this to meet auto-restart */
		ret = INSTANT_SKIP;
		DPRINTK(KERN_INFO "1st INT \n");
		break;
		
	case CAMIF_NORMAL_INT:	
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_NORMAL_INT;
		ret = INSTANT_GO;
//		DPRINTK(KERN_INFO "NORMALd INT \n");
		break;
		
	case CAMIF_Xth_INT:	
		camif_last_irq_en(cfg);/* IRQ for Enabling LAST IRQ */
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_Yth_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "Xth INT \n");
		break;	
		
	case CAMIF_Yth_INT:        /* Capture Stop */
		cfg->capture_enable = CAMIF_DMA_OFF;
		cfg->status = CAMIF_INT_HAPPEN;
		camif_capture_stop(cfg);
		cfg->fsm = CAMIF_Zth_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "Yth INT \n");
		break;
		
	case CAMIF_Zth_INT: 	/*  LAST IRQ (Dummy IRQ */
		cfg->fsm = CAMIF_DUMMY_INT;
		cfg->status = CAMIF_INT_HAPPEN;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "Zth INT \n");
		break;	
		
	case CAMIF_DUMMY_INT:
		cfg->status = CAMIF_STOPPED; /* Dupplicate ? */
		ret = INSTANT_SKIP;
		break;
		
	default:
		printk(KERN_INFO "Unexpected INT %d \n",cfg->fsm); 
		ret = INSTANT_SKIP;
		break;
	}
	return ret;
}

/* 4 interrupts State Machine is for two pingpong  
 *    1st INT : Start Interrupt
 *    Xth INT : enable Last IRQ : pingpong get the valid data
 *    Yth INT : Stop Codec or Preview : pingpong get the valid data
 *    Zth INT : Last IRQ : valid data 
 *    
 * Note:
 *    Before calling this func, you must call camif_reset
 */

int camif_enter_2fsm(camif_cfg_t *cfg) /* Codec FSM */
{
	int ret;

	__D("\n");

	cfg->perf.frames++;
	switch (cfg->fsm) {
	case CAMIF_1st_INT:           /* Start IRQ */
		cfg->fsm = CAMIF_Xth_INT;
		ret = INSTANT_SKIP;
		DPRINTK("camif_enter_2fsm() : 1st INT \n");	
		break;

	case CAMIF_Xth_INT:	
		camif_last_irq_en(cfg);/* IRQ for Enabling LAST IRQ */
		cfg->now_frame_num = 0;
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_Yth_INT;
		ret = INSTANT_GO;
		DPRINTK("camif_enter_2fsm() : Xth INT \n");	
		break;	

	case CAMIF_Yth_INT:        /* Capture Stop */
		cfg->capture_enable = CAMIF_DMA_OFF;
		cfg->now_frame_num = 1;
		cfg->status = CAMIF_INT_HAPPEN;
		camif_capture_stop(cfg);
		cfg->fsm = CAMIF_Zth_INT;
		ret = INSTANT_GO;
		DPRINTK("camif_enter_2fsm() : Yth INT \n");	
		break;

	case CAMIF_Zth_INT: 	/*  LAST IRQ (Dummy IRQ */
		cfg->now_frame_num = 0;
//		cfg->fsm = CAMIF_DUMMY_INT;
		cfg->status = CAMIF_INT_HAPPEN;
		ret = INSTANT_GO;
		DPRINTK("camif_enter_2fsm() : Zth INT \n");
		break;	

	case CAMIF_DUMMY_INT:
		cfg->status = CAMIF_STOPPED; /* Dupplicate ? */
		ret = INSTANT_SKIP;
		DPRINTK("camif_enter_2fsm() : Dummy INT \n"); 
		break;
		
	default:  /* CAMIF_PENDING_INT */
		printk("camif_enter_2fsm() : Unexpected INT \n"); 
		ret = INSTANT_SKIP;
		break;
	}
	return ret;
}


/* 2 Interrups State Machine is for one pingpong  
 *    1st INT : Stop Codec or Preview : pingpong get the valid data
 *    2nd INT : Last IRQ : dummy data 
 */
int camif_enter_1fsm(camif_cfg_t *cfg) /* Codec FSM */
{
	int ret;

	__D("\n");
	
	cfg->perf.frames++;
	switch (cfg->fsm) {
	case CAMIF_Yth_INT:	/* IRQ for Enabling LAST IRQ */
		cfg->capture_enable = CAMIF_DMA_OFF;
		camif_capture_stop(cfg);
		cfg->status = CAMIF_INT_HAPPEN;
		cfg->fsm = CAMIF_Zth_INT;
		ret = INSTANT_GO;
		DPRINTK(KERN_INFO "camif_enter_1fsm() : Yth INT \n");	
		break;	
		
	case CAMIF_Zth_INT: 	/*  LAST IRQ (Dummy IRQ */
		cfg->fsm = CAMIF_DUMMY_INT;
		cfg->status = CAMIF_STOPPED; 
//		cfg->status = CAMIF_INT_HAPPEN;
		ret = INSTANT_SKIP;
		DPRINTK(KERN_INFO "camif_enter_1fsm() : Zth INT \n");	
		break;
		
	case CAMIF_DUMMY_INT:
		cfg->status = CAMIF_STOPPED; /* Dupplicate ? */
		ret = INSTANT_SKIP;
		DPRINTK(KERN_INFO "camif_enter_1fsm() : Dummy INT \n"); 
		break;
		
	default:
		printk(KERN_INFO "camif_enter_1fsm() : Unexpect INT :fsm %d \n",cfg->fsm); 
		ret = INSTANT_SKIP;
		break;
	}
	return ret;
}


/*
 *  GLOBAL STATUS CONTROL FUNCTION
 *
 */


/* Supposed that PREVIEW already running 
 * request PREVIEW to start with Codec 
 */
int camif_callback_start(camif_cfg_t *cfg)
{
	int  doit = 1;

	__D("\n");
	
	while (doit) {
		if (mutex_lock_interruptible(&cfg->cis->lock)) {
				return -ERESTARTSYS;
			}
		cfg->cis->status = CWANT2START;
		cfg->other = cfg;
		mutex_unlock(&cfg->cis->lock);
		doit = 0;
	}
	return 0;
}


int camif_check_codec_in_PREVIEW(camif_cfg_t *cfg)
{
        int  doit = 1;

	__D("\n");

        while (doit) {
                if (mutex_lock_interruptible(&cfg->cis->lock)) {
                        return -ERESTARTSYS;
                }
                cfg->cis->status = PWANT2START;
                cfg->other = cfg;
                mutex_unlock(&cfg->cis->lock);
                doit = 0;
        }
        return 0;
}
/*
 * Return status of Preview Machine
 ret value :
 0: Preview is not working
 X: Codec must follow PREVIEW start
*/
int camif_check_preview_in_CODEC(camif_cfg_t *cfg)
{
	int ret = 0;

	__D("\n");
	
	if (mutex_lock_interruptible(&cfg->cis->lock)) {
		ret = -ERESTARTSYS;
		return ret;
	}
	if (cfg->cis->user == 1) ret = 0;
	else ret = 1;
	
	/* for yuv_photo.c */
	if (cfg->cis->status & P_NOT_WORKING) ret = 0;
	mutex_unlock(&cfg->cis->lock);
	return ret;
}

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
