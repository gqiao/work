/*
  FIMC3.x  Camera Header File

  Copyright (C) 2003 Samsung Electronics 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  *
  */


#ifndef __FIMC3X_CAMIF_H_
#define __FIMC3X_CAMIF_H_

#ifdef __KERNEL__

//#include "bits.h"
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <asm/types.h>
#include <linux/i2c.h>
#include <linux/video_decoder.h>

#endif  /* __KERNEL__ */

#ifndef O_NONCAP
#define O_NONCAP O_TRUNC
#endif

/* For UXGA .. SXGA Image */
#if defined (CONFIG_CPU_S3C2443)
#define MEM_SIZE        0x04000000
#elif (defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410))
#define MEM_SIZE        0x08000000
#endif

#define P_DEDICATED_MEM 	1
#define C_DEDICATED_MEM 	1
#define RESERVE_MEM  	15*1024*1024 /* only code of 2 pingpong */
#define YUV_MEM      	10*1024*1024
#define RGB_MEM      	(RESERVE_MEM - YUV_MEM)

#define CAMIF_NUM            2

/* Codec or Preview Status */
#define CAMIF_STARTED		(1<<1) 
#define CAMIF_STOPPED		(1<<2) 
#define CAMIF_INT_HAPPEN	(1<<3) 

/* Codec or Preview  : Interrupt FSM */
#define CAMIF_1st_INT         (1<<7) 
#define CAMIF_Xth_INT         (1<<8) 
#define CAMIF_Yth_INT         (1<<9)  
#define CAMIF_Zth_INT         (1<<10) 
#define CAMIF_NORMAL_INT      (1<<11) 
#define CAMIF_DUMMY_INT       (1<<12) 
#define CAMIF_PENDING_INT     0


/* CAMIF RESET Definition */
#define CAMIF_RESET           (1<<0) 
#define CAMIF_EX_RESET_AL     (1<<1)  	/* Active Low */
#define CAMIF_EX_RESET_AH     (1<<2) 	/* Active High */

#define SENSOR_INIT     (1<<0) 
#define USER_ADD        (1<<1) 
#define USER_EXIT       (1<<2) 
#define SENSOR_VGA      (1<<3) 
#define SENSOR_SVGA     (1<<10) 
#define SENSOR_SXGA     (1<<4) 
#define SENSOR_UXGA     (1<<11) 
#define SENSOR_ZOOMIN   (1<<5) 
#define SENSOR_ZOOMOUT  (1<<6) 
#define SENSOR_MIRROR   (1<<7) 
#define SENSOR_AF       (1<<8) 
#define SENSOR_WB       (1<<9) 
#define SENSOR_QVGA     (1<<12)     //bxl@hhtech


enum camif_itu_fmt {
	CAMIF_ITU601 = 1, //(1<<31),  
	CAMIF_ITU656 = 0
};

/* It is possbie to use two device simultaneously */
enum camif_dma_type {
	CAMIF_PREVIEW = (1<<0), 
	CAMIF_CODEC   = (1<<1), 
};

enum camif_order422 {
#if 0
	CAMIF_YCBYCR = 0,
	CAMIF_YCRYCB = (1<<14), 
	CAMIF_CBYCRY = (1<<15), 
	CAMIF_CRYCBY = (1<<14)|(1<<15),
#else
	CAMIF_YCBYCR = 0,
	CAMIF_YCRYCB = 1,
	CAMIF_CBYCRY = 2,
	CAMIF_CRYCBY = 3,
#endif	
};

enum flip_mode {
	CAMIF_FLIP   = 0,
	CAMIF_ROTATE_90 = (1<<13), 
	CAMIF_FLIP_X = (1<<14), 
	CAMIF_FLIP_Y = (1<<15), 
	CAMIF_FLIP_MIRROR = (1<<14)|(1<<15), 
	CAMIF_FLIP_ROTATE_270 = (1<<13)|(1<<14)|(1<<15), 
};

enum camif_fmt {
	CAMIF_YCBCR420  = (1<<0), /* Currently IN_YCBCR format fixed */
	CAMIF_YCBCR422  = (1<<1), 
       
	CAMIF_RGB16        = (1<<2), 
	CAMIF_RGB24        = (1<<3), 
	CAMIF_RGB32        = (1<<4), 
};

enum camif_capturing {
	CAMIF_BOTH_DMA_ON  = (1<<4), 
	CAMIF_DMA_ON   	   = (1<<3), 
	CAMIF_BOTH_DMA_OFF = (1<<1), 
	CAMIF_DMA_OFF      = (1<<0), 
	
	/*------------------------*/
	CAMIF_DMA_OFF_L_IRQ= (1<<5), 
};

enum image_effect {
	CAMIF_BYPASS   = 0,
	CAMIF_ARBITRARY_CB_CR,
	CAMIF_NEGATIVE,
	CAMIF_ART_FREEZE,
	CAMIF_EMBOSSING ,
	CAMIF_SILHOUETTE,
};


enum input_channel{
	CAMERA_INPUT   = 0,
	MSDMA_FROM_CODEC= 1,
	MSDMA_FROM_PREVIEW = 2,
};

enum output_channel{
	CAMIF_OUT_PP = 0,
	CAMIF_OUT_FIFO= 1,
};

enum field_status {
	FIELD_EVEN = 0,
	FIELD_ODD = 1,	
};

typedef struct camif_performance
{
	int	frames;
	int	framesdropped;
	__u64	bytesin;
	__u64	bytesout;
	__u32	reserved[4];
} camif_perf_t;


typedef struct {
	dma_addr_t  phys_y;
	dma_addr_t  phys_cb;
	dma_addr_t  phys_cr;
	u8          *virt_y;  
	u8          *virt_cb;
	u8          *virt_cr;
	dma_addr_t  phys_rgb;
	u8          *virt_rgb;
}img_buf_t;


/* this structure convers the CIWDOFFST, prescaler, mainscaler */
typedef struct {
	u32 modified_src_x;	/* After windows applyed to source_x */
	u32 modified_src_y;
	u32 hfactor;
	u32 vfactor;
	u32 shfactor;     	/* SHfactor = 10 - ( hfactor + vfactor ) */
	u32 prehratio;
	u32 prevratio;
	u32 predst_x;
	u32 predst_y;
	u32 scaleup_h;      	
	u32 scaleup_v;
	u32 mainhratio;
	u32 mainvratio;
	u32 scalerbypass;	/* only codec */
	u32 zoom_in_cnt;
} scaler_t;


enum v4l2_status {
	CAMIF_V4L2_INIT    = (1<<0), 
	CAMIF_v4L2_DIRTY   = (1<<1), 
};


/* Global Status Definition */
#define PWANT2START   	(1<<0) 
#define CWANT2START   	(1<<1) 
#define BOTH_STARTED  	(PWANT2START|CWANT2START)
#define P_NOT_WORKING  (1<<4) 
#define C_WORKING     	(1<<5) 
#define P_WORKING   	(1<<6) 
#define C_NOT_WORKING   (1<<7) 


#define FORMAT_FLAGS_DITHER       0x01
#define FORMAT_FLAGS_PACKED       0x02
#define FORMAT_FLAGS_PLANAR       0x04
#define FORMAT_FLAGS_RAW          0x08
#define FORMAT_FLAGS_CrCb         0x10


typedef struct {
	struct mutex 		     lock;
	enum camif_itu_fmt       ITU;               //itu_fmt;
	enum camif_order422      order422_CAM;      //order422
	u32                      win_hor_ofst;
	u32                      win_ver_ofst;
	u32                      win_hor_ofst2;
	u32                      win_ver_ofst2;
	u32                      camclk;            /* External Image Sensor Camera Clock */

	u32                      source_x;
	u32                      source_y;
	u32                      polarity_pclk;
	u32                      polarity_vsync;
	u32                      polarity_href;
	struct i2c_client       *sensor;
	u32                      user;             /* MAX 2 (codec, preview) */
	u32    			         irq_old_priority; /* BUS PRIORITY register */
	u32                      status;
	u32                      init_sensor;      /* initializing sensor */
	u32			             reset_type;	   /* External Sensor Reset  Type */
	u32		                 reset_udelay;	
	u32 			         zoom_in_cnt;
} camif_cis_t;			/* gobal control register */


/* when  App want to change v4l2 parameter,
 * we instantly store it into v4l2_t v2 
 * and then reflect it to hardware
 */	
typedef struct v4l2 {
	struct v4l2_fmtdesc     *fmtdesc;
	struct v4l2_framebuffer  frmbuf; /* current frame buffer */
	struct v4l2_input        *input;
	struct v4l2_input        *output;
	enum v4l2_status         status;
} v4l2_t;


typedef struct camif_c_t {
	struct video_device      *v;
	/* V4L2 param only for v4l2 driver */
	v4l2_t                    v2;
	camif_cis_t               *cis;          /* Common between Codec and Preview */
	/* logical parameter */
	wait_queue_head_t        waitq;
	u32                      status;         /* Start/Stop */
	u32                      fsm;            /* Start/Stop */
	u32                      open_count;     /* duplicated */
	int                      irq;
	char                     shortname[16];
	u32                      target_x;
	u32                      target_y;
	scaler_t                 sc;
	enum flip_mode           flip;
	enum image_effect        effect;
	enum camif_dma_type      dma_type;
	/* 4 pingpong Frame memory */
	u8                       *pp_virt_buf;
	dma_addr_t               pp_phys_buf;
	u32                      pp_totalsize;
	u32                      pp_num;         /* used pingpong memory number */
	img_buf_t                img_buf[4];
	enum camif_fmt     	 src_fmt;
	enum camif_fmt     	 dst_fmt;
	enum camif_capturing     capture_enable;
	camif_perf_t             perf;
	u32			 now_frame_num;
	u32                      auto_restart;   /* Only For Preview */
	int			 input_channel;
	int			 output_channel;
	void	  		 *other;                /* other camif_cfg_t */
	u32			interlace_capture;	        // 0 : Non interlace mode, 1 : interlace mode
	enum field_status	check_even_odd;	    // 0 : even frame, 1 : odd frame -> only for interlaced capture mode
	u32			msdma_status;	            // 0 : stop, 1 : start	
} camif_cfg_t;


#define CAMIF_DEBUG
#ifdef CAMIF_DEBUG
//#define DPRINTK(fmt, args...) printk(KERN_DEBUG "%s: " fmt, __FUNCTION__ , ## args)
#define DPRINTK(fmt, args...) printk(KERN_INFO "%s: " fmt, __FUNCTION__ , ## args) //lzcx
#else
#define DPRINTK(fmt, args...)
#endif


#ifdef CAMIF_DEBUG
#define assert(expr)									\
	if(!(expr)) {										\
        printk( "Assertion failed! %s,%s,%s,line=%d\n", \
				#expr,__FILE__,__FUNCTION__,__LINE__);	\
	}
#else
#define assert(expr)
#endif


/* 
 * IOCTL Command for Character Driver
 */ 

#define CMD_CAMERA_INIT   		(1<<0)
#define CMD_SENSOR_VGA_MODE             (1<<1)
#define CMD_SENSOR_SXGA_MODE    	(1<<2)  
#define CMD_SENSOR_SVGA_MODE    	(1<<10) 
#define CMD_SENSOR_UXGA_MODE    	(1<<11) 
#define CMD_PICTURE_MODE        	(1<<3) 
#define CMD_CAMCODER_MODE       	(1<<4) 
#define CMD_SENSOR_ZOOMIN       	(1<<5) 
#define CMD_SENSOR_ZOOMOUT      	(1<<6) 
#define CMD_SENSOR_MIRROR       	(1<<7) 
#define CMD_SENSOR_AF           	(1<<8) 
#define CMD_SENSOR_WB           	(1<<9) 
#define CMD_SENSOR_IMAGE_EFFECT      	(1<<12) 
#define CMD_POSTPROCESSING_INIT      	(1<<13) 


/*  Test Application Usage */
typedef struct {
	int src_x;
	int src_y;
	int dst_x;
	int dst_y;
	int src_fmt;
	int dst_fmt;
	int flip;
	int awb;
	int effect;
	int input_channel;
	int output_channel;
	unsigned int h_offset;
	unsigned int v_offset;
	unsigned int h_offset2;
	unsigned int v_offset2;
} camif_param_t;


extern int camif_capture_start(camif_cfg_t *);
extern int camif_capture_stop(camif_cfg_t *);
extern int camif_g_frame_num(camif_cfg_t *);
extern u8 * camif_g_frame(camif_cfg_t *);
extern int  camif_win_offset(camif_cis_t *);
extern void camif_hw_open(camif_cis_t *);
extern void camif_hw_close(camif_cfg_t *);
extern int camif_setup_fimc_controller(camif_cfg_t *);
extern int camif_dynamic_close(camif_cfg_t *);
extern void camif_reset(int,int);
extern void camif_setup_sensor(void);
extern int camif_g_fifo_status(camif_cfg_t *);
extern void camif_last_irq_en(camif_cfg_t *);
extern void camif_change_flip(camif_cfg_t *);
extern void camif_change_image_effect(camif_cfg_t *);
extern int camif_c_msdma_start(camif_cfg_t *, camif_param_t);

/* 
 *  API Interface function to both Character and V4L2 Drivers
 */
extern int camif_do_write(struct file *,const char *, size_t, loff_t *);
extern int camif_do_ioctl(struct inode *, struct file *,unsigned int, void *);


/* 
 * API for CIS module (3BA, 3AA.) 
 */
void camif_register_cis(struct i2c_client *);
void camif_unregister_cis(struct i2c_client*);

/* API for FSM */
#define INSTANT_SKIP 	0 
#define INSTANT_GO   	1 

extern ssize_t camif_start_1fsm(camif_cfg_t *);
extern ssize_t camif_start_2fsm(camif_cfg_t *);
extern ssize_t camif_start_preview(camif_cfg_t *);
extern ssize_t camif_stop_preview(camif_cfg_t *);
extern int camif_enter_p_4fsm(camif_cfg_t *);
extern int camif_enter_c_4fsm(camif_cfg_t *);
extern int camif_enter_2fsm(camif_cfg_t *);
extern int camif_enter_1fsm(camif_cfg_t *);
extern int camif_check_preview_in_CODEC(camif_cfg_t *);
extern int camif_callback_start(camif_cfg_t *);
extern int camif_source_fmt(camif_cis_t *);

extern void clear_camif_irq(int);


/* 
 *  V4L2 Part
 */
#define VID_HARDWARE_SAMSUNG_FIMC3X   236

extern camif_cfg_t fimc[CAMIF_NUM];

#endif


