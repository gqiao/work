#define LOG_TAG "s3c_camif"
#include "osal.h"
#include "s3c.h"

/*
 *   Copyright (C) 2004 Samsung Electronics 
 *   
 * This file is subject to the terms and conditions of the GNU General Public
 * License 2. See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/semaphore.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>

#include <asm/arch/map.h>
#include <asm/arch/regs-camif.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-s3c6410-clock.h>

#include "s3c_camif.h"
#include <linux/videodev.h>

//#define CAMIF_DEBUG

#define CONFIG_VIDEO_SAA7113

#if (defined (CONFIG_VIDEO_ADV7180)/* || defined (CONFIG_VIDEO_SAA7113)*/)
#define TOMTOM_INTERLACE_MODE
#define SW_IPC
#endif

static s3c_syscon_t  __iomem  *syscon = S3C_VA_SYSCON;
static s3c_cam_t     __iomem  *base   = S3C_VA_CAMIF;

static dma_addr_t shared_buffer_addr; 

static int camif_dma_burst(camif_cfg_t *);
static int camif_setup_scaler(camif_cfg_t *);
static int camif_setup_intput_path(camif_cfg_t *);
static int camif_setup_msdma_input(camif_cfg_t *);
static int camif_setup_camera_input(camif_cfg_t *);
static int camif_setup_output_path(camif_cfg_t *);
static int camif_setup_lcd_fifo_output(camif_cfg_t *);
static int camif_setup_memory_output(camif_cfg_t *);



#define CAMDIV_val     20

int s3c_camif_set_clock (unsigned int camclk)
{
	unsigned int camclk_div; //, val, hclkcon;
	struct clk *src_clk = clk_get(NULL, "hclkx2");

	__D("External camera clock is set to %dHz\n", camclk);

	camclk_div = clk_get_rate(src_clk) / camclk;
	__D("Parent clk = %ld, CAMDIV = %d\n", clk_get_rate(src_clk), camclk_div);

	// CAMIF HCLK Enable
	syscon->HCLK_GATE.HCLK_CAMIF = 1;
	
	/* CAMCLK Enable */
	syscon->SCLK_GATE.SCLK_CAM = 1;
	
	syscon->CLK_DIV0.CAM_RATIO = 0;
	
	if(camclk_div > 16) {
	    __D("Can't set to %dHZ, set to %dHZ instead!!!\n", 
			(int)camclk, (int)clk_get_rate(src_clk)/16);
	    camclk_div = 16;
	}

	/* CAM CLK DIVider Ratio = (EPLL clk)/(camclk_div) */
	syscon->CLK_DIV0.CAM_RATIO = camclk_div - 1;
	
	return 0;
}


static inline  int camif_malloc(camif_cfg_t *cfg)
{
	unsigned int t_size = 0;
	unsigned int area = 0;
	
	__D("\n");
	
	if(cfg->interlace_capture)
		area = cfg->target_x * cfg->target_y * 2;
	else
		area = cfg->target_x * cfg->target_y;

	if(cfg->dma_type & CAMIF_CODEC) {
		if (cfg->dst_fmt & CAMIF_YCBCR420) 
			t_size = area * 3 / 2;  /* CAMIF_YCBCR420 */
		
		else if (cfg->dst_fmt & CAMIF_YCBCR422) 
			t_size = area * 2;     /* CAMIF_YCBCR422 */ 
		
		else if (cfg->dst_fmt & CAMIF_RGB16) 
			t_size = area * 2;     /*  2 bytes per one pixel*/
		
		else if (cfg->dst_fmt & CAMIF_RGB24) 
			t_size = area * 4;     /* 4 bytes per one pixel */
		
		else 
			__E("CODEC: Invalid format !! \n");
		
		
		t_size = t_size * cfg->pp_num;
		__I("CODEC: Memory required : 0x%08X bytes\n",t_size);
		
		__I("CODEC: Using Dedicated High RAM\n");
		cfg->pp_phys_buf = PHYS_OFFSET + (MEM_SIZE - RESERVE_MEM);
		cfg->pp_virt_buf = ioremap_nocache(cfg->pp_phys_buf, YUV_MEM);
		shared_buffer_addr = (typeof(shared_buffer_addr))cfg->pp_virt_buf;

		if ( !cfg->pp_virt_buf ) {
			__E("s3c_camif.c : Failed to request YCbCr: size of memory %08x \n",t_size);
			return -ENOMEM;
		}
		cfg->pp_totalsize = t_size;
		return 0;
	}
	
	if ( cfg->dma_type & CAMIF_PREVIEW ) {
	
		if (cfg->dst_fmt & CAMIF_RGB16) {
			t_size = area * 2;      /*  2 bytes per two pixel*/
		}
		else if (cfg->dst_fmt & CAMIF_RGB24) {
			t_size = area * 4;      /* 4 bytes per one pixel */
		}
		else {
			__E("Error[s3c_camif.c]: Invalid format !\n");
		}
			
		t_size = t_size * cfg->pp_num;
		__I("Preview: Memory required : 0x%08X bytes\n",t_size);
		

		__I("Preview: Using Dedicated High RAM\n");
		if(cfg->interlace_capture == 1) {
			cfg->pp_phys_buf = PHYS_OFFSET + (MEM_SIZE - RESERVE_MEM);
			cfg->pp_virt_buf = (typeof(cfg->pp_virt_buf))shared_buffer_addr;
		} else {
			cfg->pp_phys_buf = PHYS_OFFSET + (MEM_SIZE - RESERVE_MEM ) + YUV_MEM;
			cfg->pp_virt_buf = ioremap_nocache(cfg->pp_phys_buf, RGB_MEM);
		}
		

		if ( !cfg->pp_virt_buf ) { 
			__E("Failed to request RGB: size of memory %08x\n",t_size);
			return -ENOMEM;
		}
		cfg->pp_totalsize = t_size;
		return 0;
	}

	return 0;		/* Never come. */
}


static int camif_demalloc(camif_cfg_t *cfg)
{   __D("\n");
#if defined(P_DEDICATED_MEM) || defined(C_DEDICATED_MEM)
	iounmap(cfg->pp_virt_buf);
	cfg->pp_virt_buf = 0;
#else
	if ( cfg->pp_virt_buf ) {
		consistent_free(cfg->pp_virt_buf,cfg->pp_totalsize,cfg->pp_phys_buf);
		cfg->pp_virt_buf = 0;
	}
#endif
	return 0;
}

/* 
 * advise a person to use this func in ISR 
 * index value indicates the next frame count to be used 
 */
int camif_g_frame_num(camif_cfg_t *cfg)
{
	int index = 0;

	__D("\n");

	if (cfg->dma_type & CAMIF_CODEC ) {
		index = base->CICOSTATUS.FrameCnt_Co;
	} else if (cfg->dma_type & CAMIF_PREVIEW) {
		index = base->CIPRSTATUS.FrameCnt_Pr;
	} else {
		__E("Failed! %d\n", cfg->dma_type);
	}
	
	cfg->now_frame_num = (index + 2) % 4;    /* When 4 PingPong */
	return index; /* meaningless */
}

static int camif_pp_codec_rgb(camif_cfg_t *cfg);
static int camif_pp_preview_msdma(camif_cfg_t *cfg);

static int camif_pp_codec(camif_cfg_t *cfg)
{   
	u32 i, cbcr_size = 0;    /* Cb,Cr size */
	u32 one_p_size;
	u32 one_line_size;
	u32 area = 0;

	__D("\n");
	
	if(cfg->interlace_capture)
		area = cfg->target_x * cfg->target_y * 2;
	else
		area = cfg->target_x * cfg->target_y;
	
	if (cfg->dst_fmt & CAMIF_YCBCR420) {
		cbcr_size = area /4;
	}
	else if(cfg->dst_fmt & CAMIF_YCBCR422){
		cbcr_size = area /2;
	}
	else if((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24)){ 
		camif_pp_codec_rgb(cfg);
		return 0;
	}
	else {
		__E("Invalid format No - %d \n", cfg->dst_fmt);	
	}  
	
	switch ( cfg->pp_num ) {
	case 1 :
		for ( i =0 ; i < 4; i=i+1) {
			cfg->img_buf[i].virt_y = cfg->pp_virt_buf;
			cfg->img_buf[i].phys_y = cfg->pp_phys_buf;
			cfg->img_buf[i].virt_cb = cfg->pp_virt_buf + area; 
			cfg->img_buf[i].phys_cb = cfg->pp_phys_buf + area;
			cfg->img_buf[i].virt_cr = cfg->pp_virt_buf + area + cbcr_size;
			cfg->img_buf[i].phys_cr = cfg->pp_phys_buf + area + cbcr_size;
			base->CICOYSA[i]  = cfg->img_buf[i].phys_y;
			base->CICOCBSA[i] = cfg->img_buf[i].phys_cb;
			base->CICOCRSA[i] = cfg->img_buf[i].phys_cr;
		}
		break;
			
	case 2:
#define  TRY   (( i%2 ) ? 1 :0)
		if(cfg->interlace_capture)
			one_p_size = cfg->target_x * 2;
		else
			one_p_size = area + 2*cbcr_size;
	for (i = 0; i < 4  ; i++) {
		cfg->img_buf[i].virt_y = cfg->pp_virt_buf + TRY * one_p_size;
		cfg->img_buf[i].phys_y = cfg->pp_phys_buf + TRY * one_p_size;
		cfg->img_buf[i].virt_cb = cfg->pp_virt_buf + area + TRY * one_p_size;
		cfg->img_buf[i].phys_cb = cfg->pp_phys_buf + area + TRY * one_p_size;
		cfg->img_buf[i].virt_cr = cfg->pp_virt_buf + area + cbcr_size + TRY * one_p_size;
		cfg->img_buf[i].phys_cr = cfg->pp_phys_buf + area + cbcr_size + TRY * one_p_size;
		base->CICOYSA[i]  = cfg->img_buf[i].phys_y;
		base->CICOCBSA[i] = cfg->img_buf[i].phys_cb;
		base->CICOCRSA[i] = cfg->img_buf[i].phys_cr;
	}
	break;
			
	case 4: 
		if(cfg->interlace_capture) {
			one_line_size = cfg->target_x * 2;
			one_p_size = area + 2*cbcr_size;
			for (i = 0; i < 4 ; i++) {
				cfg->img_buf[i].virt_y = cfg->pp_virt_buf + (i/2) * one_p_size + (i%2) * one_line_size;
				cfg->img_buf[i].phys_y = cfg->pp_phys_buf + (i/2) * one_p_size + (i%2) * one_line_size;
				cfg->img_buf[i].virt_cb = cfg->pp_virt_buf + area + (i/2) * one_p_size + (i%2) * one_line_size;
				cfg->img_buf[i].phys_cb = cfg->pp_phys_buf + area + (i/2) * one_p_size + (i%2) * one_line_size;
				cfg->img_buf[i].virt_cr = cfg->pp_virt_buf + area + cbcr_size + (i/2) * one_p_size + (i%2) * one_line_size;
				cfg->img_buf[i].phys_cr = cfg->pp_phys_buf + area + cbcr_size + (i/2) * one_p_size + (i%2) * one_line_size;
				base->CICOYSA[i]  = cfg->img_buf[i].phys_y;
				base->CICOCBSA[i] = cfg->img_buf[i].phys_cb;
				base->CICOCRSA[i] = cfg->img_buf[i].phys_cr;
			}
		} else {
			one_p_size = area + 2*cbcr_size;
			for (i = 0; i < 4 ; i++) {
				cfg->img_buf[i].virt_y = cfg->pp_virt_buf + i * one_p_size;
				cfg->img_buf[i].phys_y = cfg->pp_phys_buf + i * one_p_size;
				cfg->img_buf[i].virt_cb = cfg->pp_virt_buf + area + i * one_p_size;
				cfg->img_buf[i].phys_cb = cfg->pp_phys_buf + area + i * one_p_size;
				cfg->img_buf[i].virt_cr = cfg->pp_virt_buf + area + cbcr_size + i * one_p_size;
				cfg->img_buf[i].phys_cr = cfg->pp_phys_buf + area + cbcr_size + i * one_p_size;
				base->CICOYSA[i]  = cfg->img_buf[i].phys_y;
				base->CICOCBSA[i] = cfg->img_buf[i].phys_cb;
				base->CICOCRSA[i] = cfg->img_buf[i].phys_cr;
			}
		}
		break;
			
	default:
		__E("Invalid PingPong Number %d \n", cfg->pp_num);
	}
	return 0;
}

/* RGB Buffer Allocation */
static int camif_pp_preview(camif_cfg_t *cfg)
{
	int i;
	u32 cbcr_size = 0;    /* Cb,Cr size */
	u32 area = cfg->target_x * cfg->target_y;

	__D("\n");
	
	if(cfg->input_channel) {
		camif_pp_preview_msdma(cfg);
		return 0;
	}

	if (cfg->dst_fmt & CAMIF_YCBCR420) {
		cbcr_size = area /4;
		/* To Do */
	} else if(cfg->dst_fmt & CAMIF_YCBCR422){
		cbcr_size = area /2;
		/* To Do */
	} else if(cfg->dst_fmt & CAMIF_RGB24)  {
		area = area * 4 ;
	} else if (cfg->dst_fmt & CAMIF_RGB16) {
		area = area * 2;
	} else {
		__E("Invalid format No - %d \n", cfg->dst_fmt);	
	}  
		
	switch ( cfg->pp_num ) {
	case 1:
		for ( i = 0; i < 4 ; i++ ) {
			cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf ;
			cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf ;
			base->CIPRYSA[i] = cfg->img_buf[i].phys_rgb;
		}
		break;
	case 2:
		for ( i = 0; i < 4 ; i++) {
			cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + TRY * area;
			cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + TRY * area;
			base->CIPRYSA[i] = cfg->img_buf[i].phys_rgb;
		}
		break;
	case 4:
		for ( i = 0; i < 4 ; i++) {
			cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + i * area;
			cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + i * area;
			base->CIPRYSA[i] = cfg->img_buf[i].phys_rgb;
		}
		break;
	default:
		__E("Invalid PingPong Number %d \n", cfg->pp_num);
	}
	return 0;
}

/* RGB Buffer Allocation For CODEC path */
static int camif_pp_codec_rgb(camif_cfg_t *cfg)
{
	int i;
	u32 val;
	u32 area = cfg->target_x * cfg->target_y;

	__D("\n");
	
	if(cfg->dst_fmt & CAMIF_RGB24) { 
		area = area * 4 ;
	} else if (cfg->dst_fmt & CAMIF_RGB16){
		area = area * 2;
	} else {
		__E("Failed! %d\n", cfg->dst_fmt);
	}  

	if(cfg->input_channel == MSDMA_FROM_CODEC){  // Use MSDMA
		val = readl(S3C_VIDW00ADD0B0);
		for ( i = 0; i < 4 ; i++ ) {
			base->CICOYSA[i] = val;
		}
	} else{ 					// Do NOT use MSDMA 
		switch ( cfg->pp_num ) {
		case 1:
			for ( i = 0; i < 4 ; i++ ) {
				cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf ;
				cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf ;
				base->CICOYSA[i] = cfg->img_buf[i].phys_rgb;
			}
			break;
		case 2:
			for ( i = 0; i < 4 ; i++) {
				cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + TRY * area;
				cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + TRY * area;
				base->CICOYSA[i] = cfg->img_buf[i].phys_rgb;
			}
			break;
		case 4:
			for ( i = 0; i < 4 ; i++) {
				cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + i * area;
				cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + i * area;
				base->CICOYSA[i] = cfg->img_buf[i].phys_rgb;
			}
			break;
		default:
			__E("Invalid PingPong Number %d \n",cfg->pp_num);
			panic("s3c_camif.c : Halt !\n");
		}
	}
	return 0;
}

/* RGB Buffer Allocation into frame buffer */
static int camif_pp_preview_msdma(camif_cfg_t *cfg)
{   
	int i;
	
	u32 cbcr_size = 0;    /* Cb+Cr size */
	u32 val; //one_p_size, val;
	
	u32 area = cfg->cis->source_x * cfg->cis->source_y;

	__D("\n");
	
	val = readl(S3C_VIDW01ADD0B0);
	
	if(!((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24)))
		__E("Invalid Format\n");
	
	for ( i = 0; i < 4 ; i++ ) {
		base->CIPRYSA[i] = val;
	}
	
	if (cfg->src_fmt & CAMIF_YCBCR420) {
		cbcr_size = area/4;
		cfg->img_buf[0].virt_cb = cfg->pp_virt_buf + area; 
		cfg->img_buf[0].phys_cb = cfg->pp_phys_buf + area;
		cfg->img_buf[0].virt_cr = cfg->pp_virt_buf + area + cbcr_size;
		cfg->img_buf[0].phys_cr = cfg->pp_phys_buf + area + cbcr_size;

	} else if(cfg->src_fmt & CAMIF_YCBCR422){
		area = area * 2;
		cfg->img_buf[0].virt_cb = 0; 
		cfg->img_buf[0].phys_cb = 0;
		cfg->img_buf[0].virt_cr = 0;
		cfg->img_buf[0].phys_cr = 0;
		
	} 
	cfg->img_buf[0].virt_y = cfg->pp_virt_buf;
	cfg->img_buf[0].phys_y = cfg->pp_phys_buf;

	base->MSPRY0SA  = cfg->img_buf[0].phys_y;
	base->MSPRY0END = cfg->img_buf[0].phys_y+area;
	
	base->MSPRCB0SA  = cfg->img_buf[0].phys_cb;
	base->MSPRCB0END = cfg->img_buf[0].phys_cb+cbcr_size;
	
	base->MSPRCR0SA = cfg->img_buf[0].phys_cr;
	base->MSPRCR0END = cfg->img_buf[0].phys_cr+cbcr_size;

#ifdef SW_IPC
	base->MSCOWIDTH.AutoLoadEnable = 0; // AutoLoadDisable
#else
	base->MSCOWIDTH.AutoLoadEnable = 1; // AutoLoadEnable
#endif

	base->MSPRWIDTH.MSPRHEIGHT = cfg->cis->source_y;
	base->MSPRWIDTH.MSPRHEIGHT = cfg->cis->source_x;

	return 0;
}

static int camif_setup_memory_output(camif_cfg_t *cfg)
{   __D("\n");
	if (cfg->dma_type & CAMIF_CODEC ) {
		camif_pp_codec(cfg);
	}

	if ( cfg->dma_type & CAMIF_PREVIEW) {
		camif_pp_preview(cfg);
	}
	return 0;
}


static int camif_setup_lcd_fifo_output(camif_cfg_t *cfg) 
{   __D("\n");
	/* To Be Implemented */
	return 0;
}



/* 
 * After calling camif_g_frame_num,
 * this func must be called 
 */
u8 * camif_g_frame(camif_cfg_t *cfg)
{
	u8 * ret = NULL;
	int cnt = cfg->now_frame_num;

	__D("\n");

	if(cfg->dma_type & CAMIF_PREVIEW) {
		ret = cfg->img_buf[cnt].virt_rgb;
	}
	if (cfg->dma_type & CAMIF_CODEC) {
		if ((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24))
			ret = cfg->img_buf[cnt].virt_rgb;
		else
			ret = cfg->img_buf[cnt].virt_y;
	}
	return ret;
}


/* This function must be called in module initial time */
int camif_source_fmt(camif_cis_t *cis) 
{
	/* Configure CISRCFMT --Source Format */
	base->CISRCFMT.ITU601_656n = cis->ITU;
	base->CISRCFMT.SrcHsize_CAM = cis->source_x;
	base->CISRCFMT.SrcVsize_CAM = cis->source_y;
	base->CISRCFMT.Order422_CAM = cis->order422_CAM;

#ifndef TOMTOM_INTERLACE_MODE
	base->CISRCFMT.UVOffset = 0;
#endif

	return 0 ;
}
   

/* 
 * Codec Input YCBCR422 will be Fixed
 * cfg->flip removed because cfg->flip will be set in camif_change_flip func. 
 */
static int camif_target_fmt(camif_cfg_t *cfg)
{
	//u32 cmd = 0;

	__D("\n");
	
	if (cfg->dma_type & CAMIF_CODEC) {
		base->CICOTRGFMT.TargetHsize_Co = cfg->target_x;
		base->CICOTRGFMT.TargetVsize_Co = cfg->target_y;
		
		/* YCBCR setting */  
		if ( cfg->dst_fmt & CAMIF_YCBCR420 ) {
			base->CICOTRGFMT.OutFormat_Co = 0;  //Interleadve Off 
		} else if(cfg->dst_fmt & CAMIF_YCBCR422) {
			if(cfg->interlace_capture)
				base->CICOTRGFMT.OutFormat_Co = 2;
			else
				base->CICOTRGFMT.OutFormat_Co = 1;
		} else if((cfg->dst_fmt & CAMIF_RGB24) ||(cfg->dst_fmt & CAMIF_RGB16)) {
			base->CICOTRGFMT.OutFormat_Co = 3;
		} else {
			__E("camif_target_fmt() - Invalid Format\n");
		}
	} else if (cfg->dma_type & CAMIF_PREVIEW) {
		base->CIPRTRGFMT.TargetHsize_Pr = cfg->target_x;
		base->CIPRTRGFMT.TargetVsize_Pr = cfg->target_y;
				
		/* YCBCR setting */  
		if ( cfg->dst_fmt & CAMIF_YCBCR420 ) {
			base->CIPRTRGFMT.OutFormat_Pr = 0;
		} else if(cfg->dst_fmt & CAMIF_YCBCR422) {	
			base->CIPRTRGFMT.OutFormat_Pr = 1;
		} else if((cfg->dst_fmt & CAMIF_RGB24) ||(cfg->dst_fmt & CAMIF_RGB16)) {	
			base->CIPRTRGFMT.OutFormat_Pr = 3;
		} else {
			__E("Invalid Format\n");
		}		
	} else {
		__E("Failed! %d\n", cfg->dma_type);
	}
	
	return 0;
}

void camif_change_flip(camif_cfg_t *cfg)
{   
	__D("\n");
	
	if (cfg->dma_type & CAMIF_CODEC ) {
		/* Clear FLIP Mode */
		base->CICOTRGFMT.FlipMd_Co = 0; 
		base->CICOTRGFMT.FlipMd_Co = cfg->flip >> 14;
	} else {
		/* Clear FLIP + ROTATE Mode */	
		base->CIPRTRGFMT.FlipMd_Pr = 0;
		if(cfg->flip & 1 << 13) {
			base->CIPRTRGFMT.Rot90_Pr = 1;
		} else {
			base->CIPRTRGFMT.Rot90_Pr = 0; 
		}
		base->CIPRTRGFMT.FlipMd_Pr = cfg->flip >> 14;
	}
}


void camif_change_image_effect(camif_cfg_t *cfg)
{
	__D("\n");
	
	base->CIIMGEFF.FIN = 0;   /* Clear image effect */

	/* Enable effect in Preview & Codec */
	base->CIIMGEFF.IE_ON_Pr = 1;
	base->CIIMGEFF.IE_ON_Co = 1;

	switch(cfg->effect) {
	case CAMIF_SILHOUETTE:
		base->CIIMGEFF.FIN = 5;
		break;
			
	case CAMIF_EMBOSSING:
		base->CIIMGEFF.FIN = 4;
		break;
			
	case CAMIF_ART_FREEZE:
		base->CIIMGEFF.FIN = 3;
		break;
			
	case CAMIF_NEGATIVE:
		base->CIIMGEFF.FIN = 2;
		break;
			
	case CAMIF_ARBITRARY_CB_CR:
		base->CIIMGEFF.FIN = 1;
		break;
			
	case CAMIF_BYPASS:
	default:
		break;
	}
}


/* Must:
 * Before calling this function,
 * you must use "camif_setup_fimc_controller"
 * If you want to enable both CODEC and preview
 *  you must do it at the same time.
 */
int camif_capture_start(camif_cfg_t *cfg)
{
	//int i=0;
	//u32 val;

	__D("\n");
	
	switch(cfg->capture_enable) {
	case CAMIF_BOTH_DMA_ON:
		camif_reset(CAMIF_RESET,0); /* Flush Camera Core Buffer */

		// For Codec
		base->CICOSCCTRL.CoScalerStart = 1;
			
		// For Preview
		base->CIPRSCCTRL.PrScalerStart = 1;
			
		base->CIIMGCPT.ImgCptEn_CoSc = 1;
		base->CIIMGCPT.ImgCptEn_PrSc = 1;
		break;

	case CAMIF_DMA_ON:
		camif_reset(CAMIF_RESET,0); /* Flush Camera Core Buffer */	
			
		if (cfg->dma_type & CAMIF_CODEC) {
			base->CICOSCCTRL.CoScalerStart = 1;
			base->CIIMGCPT.ImgCptEn_CoSc = 1;
			base->CIIMGCPT.Cpt_FrEn_pr   = 1;
		} else {
			base->CIPRSCCTRL.PrScalerStart = 1;
			base->CIIMGCPT.ImgCptEn_PrSc = 1;
		}
			
		/* wait until Sync Time expires */
		/* First settting, to wait VSYNC fall  */
		/* By VESA spec,in 640x480 @60Hz 
		   MAX Delay Time is around 64us which "while" has.*/ 
		while(base->CICOSTATUS.VSYNC);
		break;
			
	default:
		break;
	}
	
	if (cfg->dma_type & CAMIF_CODEC) {
		if (cfg->dst_fmt & CAMIF_RGB24) 
			base->CIIMGCPT.Cpt_FrEn_Co = 3;     /* RGB24 Codec DMA->RGB 24 bit*/
		else if(cfg->dst_fmt & CAMIF_RGB16)
			base->CIIMGCPT.Cpt_FrEn_Co = 1;     /* RGB16 Codec DMA->RGB 16 bit */
		else if(cfg->dst_fmt & CAMIF_YCBCR420)
			base->CIIMGCPT.Cpt_FrEn_Co = 2;     /* YUV420 Codec DMA->YUV420 */
		else if(cfg->dst_fmt & CAMIF_YCBCR422)
			base->CIIMGCPT.Cpt_FrEn_Co = 0;     /* YUV422 Codec DMA->YUV422 */
	}

	base->CIIMGCPT.ImgCptEn = 1;
	return 0;
}


int camif_capture_stop(camif_cfg_t *cfg)
{   
	__D("\n");
	
	switch(cfg->capture_enable) {
	case CAMIF_BOTH_DMA_OFF:
		base->CICOSCCTRL.CoScalerStart = 0;
		base->CIPRSCCTRL.PrScalerStart = 0;
		memset(&base->CIIMGCPT, 0, sizeof(base->CIIMGCPT));
		break;
			
	case CAMIF_DMA_OFF_L_IRQ: /* fall thru */
	case CAMIF_DMA_OFF:
		if (cfg->dma_type & CAMIF_CODEC) {
			base->CICOSCCTRL.CoScalerStart = 0;
			base->CIIMGCPT.ImgCptEn_CoSc = 0;
			if (base->CIIMGCPT.ImgCptEn_PrSc == 0) {
				memset(&base->CIIMGCPT, 0, sizeof(base->CIIMGCPT));
			}
		} else {
			base->CIPRSCCTRL.PrScalerStart = 0;
			base->CIIMGCPT.ImgCptEn_PrSc = 0;
			if (!base->CIIMGCPT.ImgCptEn_CoSc)
				memset(&base->CIIMGCPT, 0, sizeof(base->CIIMGCPT));	
		}
		break;
	default:
		__E("Unexpected \n");
	}
	
	base->CIIMGCPT.Cpt_FrEn_pr = 0;

	if(cfg->capture_enable == CAMIF_DMA_OFF_L_IRQ) { /* Last IRQ  */
		if (cfg->dma_type & CAMIF_CODEC) {
			base->CICOCTRL.LastIRQEn_Co = 1;
		} else { 
			base->CIPRCTRL.LastIRQEn_Pr = 1;
		}
	} 

	return 0;
}

int camif_start_postprocessing(camif_cfg_t *cfg) 
{
	__D("\n");
	
	if (cfg->dst_fmt & CAMIF_YCBCR420) {
		base->MSCOCR0SA |= 1<<1;
	} else {
		base->MSCOCR0SA &= ~(1<<1);
	}

	// clear ENVID_MS
	base->MSCOCR0SA &= ~(1<<0);

	// Set ENVID_MS
	base->MSCOCR0SA |= 1<<0;

	__I("camif_start_postprocessing() started\n");
	
	// Waiting for EOF_MS
	while((base->MSCOCR0SA & (1<<6)) == 0);
	
	__I("camif_start_postprocessing() finished\n");
	return 0;
}

/* 2007-05-11 by YREOM PIP for S3C6400 */
int camif_set_codec_msdma(camif_cfg_t * cfg)
{
	int ret = 0;
	//u32 val, val_width;
	u32 addr_start_Y=0, addr_start_CB=0, addr_start_CR=0;
	u32 addr_end_Y=0, addr_end_CB=0, addr_end_CR=0;

	__D("\n");
	
	/* Codec path input data selection */	
	// Clear codec path input 
	base->MSCOCTRL.SEL_DMA_CAM_C = 0;
	
	// Set MSDMA input path
	base->MSCOCTRL.SEL_DMA_CAM_C = 1;
	
	/* MSDMAFormat */			
	switch(cfg->src_fmt) {
	default:
	case 420:
		base->MSCOCTRL.InFormat_M_C = 0;

		addr_start_Y = cfg->pp_phys_buf;
		addr_start_CB = addr_start_Y+(cfg->cis->source_x*cfg->cis->source_y);
		addr_start_CR = addr_start_CB+(cfg->cis->source_x*cfg->cis->source_y*1/4);

		addr_end_Y = addr_start_Y+(cfg->cis->source_x*cfg->cis->source_y);
		addr_end_CB = addr_start_CB+(cfg->cis->source_x*cfg->cis->source_y*1/4);
		addr_end_CR = addr_start_CR+(cfg->cis->source_x*cfg->cis->source_y*1/4);
		break;

	case 422:
		base->MSCOCTRL.InFormat_M_C = 2;

		addr_start_Y = cfg->pp_phys_buf;
		addr_start_CB = addr_start_Y+(cfg->cis->source_x*cfg->cis->source_y);
		addr_start_CR = addr_start_CB+(cfg->cis->source_x*cfg->cis->source_y*1/2);

		addr_end_Y = addr_start_Y+(cfg->cis->source_x*cfg->cis->source_y);
		addr_end_CB = addr_start_CB+(cfg->cis->source_x*cfg->cis->source_y*1/2);
		addr_end_CR = addr_start_CR+(cfg->cis->source_x*cfg->cis->source_y*1/2);
		break;

	case 16:
	case 24:
		/* To Do */
		break;
	}

    /* MSDMA memory */
	base->MSCOY0SA   = addr_start_Y;
	base->MSCOCB0SA  = addr_start_CB;
	base->MSCOCR0SA  = addr_start_CR;

	base->MSCOY0END  = addr_end_Y;
	base->MSCOCB0END = addr_end_CB;
	base->MSCOCR0END = addr_end_CR;
	
	/* MSDMA memory offset */
	base->MSCOYOFF  = 0;
	base->MSCOCBOFF = 0;
	base->MSCOCROFF = 0;

	/* MSDMA for codec source image width */
	base->MSCOWIDTH.AutoLoadEnable = 1;
	base->MSCOWIDTH.MSCOHEIGHT     = cfg->cis->source_y;
	base->MSCOWIDTH.MSCOWIDTH      = cfg->cis->source_x;

	return ret;
}

int camif_codec_msdma_start(camif_cfg_t * cfg)
{
	int ret = 0;
	//u32 val;

	__D("\n");
	
	/* MSDMA start */
	// Clear ENVID_MS
	base->MSCOCTRL.ENVID_M_C = 0;
	
	// Set ENVID_MS
	base->MSCOCTRL.ENVID_M_C = 1;
	
	return ret;
}
EXPORT_SYMBOL(camif_codec_msdma_start);

int camif_set_preview_msdma(camif_cfg_t * cfg)
{
	int ret = 0;
	//u32 val, val_width;
	u32 addr_start_Y=0, addr_start_CB=0, addr_start_CR=0;
	u32 addr_end_Y=0, addr_end_CB=0, addr_end_CR=0;

	__D("\n");
	
	/* Codec path input data selection */	

	// Clear preview path input 
	base->MSCOCR0SA &=  ~(1<<3);
	
	// Set MSDMA input path
	base->MSCOCR0SA |= (1<<3);
	
	/* MSDMAFormat */			
	switch(cfg->src_fmt) {
	default:
	case CAMIF_YCBCR420:
		base->MSCOCR0SA &= ~(3<<1);

		addr_start_Y = base->MSPRY0SA;
		addr_start_CB = addr_start_Y + (cfg->cis->source_x*cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x*cfg->cis->source_y*1/4);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x*cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x*cfg->cis->source_y*1/4);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x*cfg->cis->source_y*1/4);
		break;

	case CAMIF_YCBCR422:
		base->MSCOCR0SA &= ~(3<<1);
		base->MSCOCR0SA |=  2<<1;  //YCbCr 422 Interleave
		base->MSCOCR0SA &= ~(3<<4);
		base->MSCOCR0SA |= 3<<4;  //YCbYCr

		addr_start_Y = base->MSPRY0SA;
		addr_start_CB = addr_start_Y + (cfg->cis->source_x*cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x*cfg->cis->source_y*1/2);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x*cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x*cfg->cis->source_y*1/2);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x*cfg->cis->source_y*1/2);
		break;
			
	case CAMIF_RGB16:
	case CAMIF_RGB24:
		/* To Do */
		break;
	}

	/* MSDMA memory */
	base->MSPRY0SA = addr_start_Y;
	base->MSPRCB0SA = addr_start_CB;
	base->MSPRCR0SA = addr_start_CR;
	
	base->MSPRY0END    = addr_end_Y;
	base->MSPRCB0END = addr_end_CB;
	base->MSPRCR0END = addr_end_CR;

	/* MSDMA memory offset */
	base->MSPRYOFF    = 0;
	base->MSPRCBOFF = 0;
	base->MSPRCROFF = 0;

	/* MSDMA for codec source image width */
#ifdef SW_IPC
	base->MSPRWIDTH.AutoLoadEnable = 1;  // AutoLoadEnable
#else
	base->MSPRWIDTH.AutoLoadEnable = 0;  // AutoLoadDisable
#endif
	
	base->MSPRWIDTH.MSPRHEIGHT = cfg->cis->source_y;
	base->MSPRWIDTH.MSPRWIDTH  = cfg->cis->source_x;
	return ret;
}

int camif_preview_msdma_start(camif_cfg_t * cfg)
{
	__D("\n");
	
	/* MSDMA start */
	// Set ENVID_MS
	base->MSPRCTRL.ENVID_M_P = 1;
	while (base->MSPRCTRL.EOF_M_P == 0);
	return 0;
}
EXPORT_SYMBOL(camif_preview_msdma_start);

/* LastIRQEn is autoclear */
void camif_last_irq_en(camif_cfg_t *cfg)
{
	__D("\n");
	
	if(cfg->capture_enable == CAMIF_BOTH_DMA_ON) {
		base->CICOCTRL.LastIRQEn_Co = 1;
	} else {
		if (cfg->dma_type & CAMIF_CODEC) {
			base->CICOCTRL.LastIRQEn_Co = 1;
		} else {
			base->CIPRCTRL.LastIRQEn_Pr = 1;
		}
	}
}

static int camif_scaler_internal(u32 srcWidth, u32 dstWidth, u32 *ratio, u32 *shift)
{   __D("\n");
	if(srcWidth>=64*dstWidth) {
		__E("Out of prescaler range: srcWidth /dstWidth = %d(< 64)\n",
			srcWidth/dstWidth);
		return 1;
	} else if(srcWidth>=32*dstWidth) {
		*ratio=32;
		*shift=5;
	} else if(srcWidth>=16*dstWidth) {
		*ratio=16;
		*shift=4;
	} else if(srcWidth>=8*dstWidth) {
		*ratio=8;
		*shift=3;
	} else if(srcWidth>=4*dstWidth) {
		*ratio=4;
		*shift=2;
 	} else if(srcWidth>=2*dstWidth) {
		*ratio=2;
		*shift=1;
	} else {
		*ratio=1;
		*shift=0;
	}  
	return 0;
}


int camif_g_fifo_status(camif_cfg_t *cfg) 
{
	__D("\n");
	
	//u32 reg, val;
	if (cfg->dma_type == CAMIF_CODEC) {
		if (base->CICOSTATUS.OvFiY_Co || base->CICOSTATUS.OvFiCb_Co || base->CICOSTATUS.OvFiCr_Co) {
			/* FIFO Error Count ++  */
			base->CIWDOFST.ClrOvCoFiY  = 1;
			base->CIWDOFST.ClrOvCoFiCb = 1;
			base->CIWDOFST.ClrOvCoFiCr = 1;
				
			base->CIWDOFST.ClrOvCoFiY  = 0;
			base->CIWDOFST.ClrOvCoFiCb = 0;
			base->CIWDOFST.ClrOvCoFiCr = 0;
			return 1; /* Error */
		}
	} else if (cfg->dma_type == CAMIF_PREVIEW) {
		if (base->CIPRSTATUS.OvFiCb_Pr || base->CIPRSTATUS.OvFiCr_Pr) {
			/* FIFO Error Count ++  */
			base->CIWDOFST.ClrOvPrFiCb = 1;
			base->CIWDOFST.ClrOvPrFiCr = 1;
			
			base->CIWDOFST.ClrOvCoFiY  = 0;
			base->CIWDOFST.ClrOvCoFiCb = 0;
			base->CIWDOFST.ClrOvCoFiCr = 0;
			return 1; /* Error */
		}
	} else {
		__E("Unknown cfg->dma_type\n");
	}
	
	return 0;		/* No Error */
}


/* Policy:
 * if codec or preview define the win offset,
 *    other must follow that value.
 */
int camif_win_offset(camif_cis_t *cis)
{
	u32 h = cis->win_hor_ofst;	/* Camera input offset ONLY */
	u32 v = cis->win_ver_ofst;	/* Camera input offset ONLY */
	u32 h2 = cis->win_hor_ofst2; /* Camera input offset ONLY */
	u32 v2 = cis->win_ver_ofst2; /* Camera input offset ONLY */

	__D("\n");
	
	/*Clear Overflow */
	base->CIWDOFST.ClrOvCoFiY  = 1;
	base->CIWDOFST.ClrOvCoFiCb = 1;
	base->CIWDOFST.ClrOvCoFiCr = 1;
	base->CIWDOFST.ClrOvPrFiCb = 1;
	base->CIWDOFST.ClrOvPrFiCr = 1;
	memset(&base->CIWDOFST, 0, sizeof(base->CIWDOFST));

	if(!h && !v) {
		memset(&base->CIWDOFST2, 0, sizeof(base->CIWDOFST2));
		return 0;
	}
	
	base->CIWDOFST.WinOfsEn = 1;
	base->CIWDOFST.WinHorOfst   = h;
	base->CIWDOFST.WinVerOfst   = v;

	base->CIWDOFST2.WinHorOfst2 = h2;
	base->CIWDOFST2.WinVerOfst2 = v2;
	
	return 0;
}

/*  
 * when you change the resolution in a specific camera,
 * sometimes, it is necessary to change the polarity 
 *                                       -- SW.LEE
 */
static void camif_polarity(camif_cis_t *cis)
{
	/* clear polarity */
	base->CIGCTRL.InvPolPCLK  = 0;
	base->CIGCTRL.InvPolVSYNC = 0;
	base->CIGCTRL.InvPolHREF  = 0;
	
	if (cis->polarity_pclk)
		base->CIGCTRL.InvPolPCLK  = 1;
	
	if (cis->polarity_vsync)
		base->CIGCTRL.InvPolVSYNC = 1;
	
	if (cis->polarity_href)
		base->CIGCTRL.InvPolHREF  = 1;
}


int camif_setup_fimc_controller(camif_cfg_t *cfg)
{   __D("\n");
	
	if(camif_malloc(cfg) ) {
		__E("Instead of using consistent_alloc()\n"
			"    Let me use dedicated mem for DMA \n");
		return -1;
	}

	camif_setup_intput_path(cfg);
	
	if(camif_setup_scaler(cfg)) {
		__E("s3c_camif.c:Preview Scaler, Change WinHorOfset or Target Size\n");
		return -1;
	}
	
	camif_target_fmt(cfg);
	
	if (camif_dma_burst(cfg)) {
		__E("s3c_camif.c:DMA Busrt Length Error \n");
		return 1;
	}

	camif_setup_output_path(cfg);
	
	return 0;
}

int inline camif_dynamic_close(camif_cfg_t *cfg)
{   __D("\n");
	if(cfg->dma_type & CAMIF_PREVIEW) {
		if(cfg->interlace_capture == 1)
			return 0;			
	}
	
	camif_demalloc(cfg);
	return 0;
}


static int camif_setup_intput_path(camif_cfg_t *cfg) 
{   __D("\n");
	
	if(cfg->input_channel == CAMERA_INPUT ){  // Camera Input
		camif_setup_camera_input(cfg); 
	} else{  // MSDMA Input
		camif_setup_msdma_input(cfg); 
	}

	return 0;
}

static int camif_setup_camera_input(camif_cfg_t *cfg) 
{   
	__D("\n");
	
	camif_win_offset(cfg->cis);
	camif_polarity(cfg->cis);

	if (cfg->dma_type & CAMIF_CODEC) {  //  CODEC input path setup : External camera:0, MSDMA:1
		base->MSCOCTRL.SEL_DMA_CAM_C = 0;  // External camera
	} else if(cfg->dma_type & CAMIF_PREVIEW){  // Preview input path setup : External camera:0, MSDMA:1
		base->MSCOCR0SA &= ~(1<<3);   // External camera
	} else{
		__E("camif_setup_camera_input : CAMERA:DMA_TYPE Wrong \n");
	}

	return 0;
}


static int camif_setup_msdma_input(camif_cfg_t *cfg) 
{   __D("\n");

	if(cfg->input_channel == MSDMA_FROM_PREVIEW) // Preview MSDMA Input
		camif_set_preview_msdma(cfg);
	else // Codec MSDMA Input
		camif_set_codec_msdma(cfg); 
	return 0;
}


static int camif_setup_output_path(camif_cfg_t *cfg) 
{   __D("\n");
	if(cfg->output_channel == CAMIF_OUT_FIFO)
		camif_setup_lcd_fifo_output(cfg); 
	else
		camif_setup_memory_output(cfg);
	
	return 0;
	
}


static int camif_target_area(camif_cfg_t *cfg) 
{
	u32 rect = cfg->target_x * cfg->target_y;

	__D("\n");
	
	if (cfg->dma_type & CAMIF_CODEC) {
		base->CICOTAREA = rect;
	}
	
	if (cfg->dma_type & CAMIF_PREVIEW) {
		base->CIPRTAREA = rect;
	}
	
	return 0;
}

static int inline camif_hw_reg(camif_cfg_t *cfg)
{   
	__D("\n");
	
	if (cfg->dma_type & CAMIF_CODEC) {
		base->CICOSCPRERATIO.SHfactor_Co      = cfg->sc.shfactor;
		base->CICOSCPRERATIO.PreHorRatio_Co = cfg->sc.prehratio;
		base->CICOSCPRERATIO.PreVerRatio_Co = cfg->sc.prevratio;
		base->CICOSCPREDST.PreDstWidth_Co   = cfg->sc.predst_x;
		base->CICOSCPREDST.PreDstHeight_Co  = cfg->sc.predst_y;
				
		/* Differ from Preview */
		if (cfg->sc.scalerbypass)
			base->CICOSCCTRL.ScalerBypass_Co = 1;
		
		/* Differ from Codec */
		if (cfg->dst_fmt & CAMIF_RGB24) {
			base->CICOSCCTRL.OutRGB_FMT_Co = 2;
		} else { /* RGB16 */
			base->CICOSCCTRL.OutRGB_FMT_Co = 0;
		}	
		
		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v) {
			base->CICOSCCTRL.ScaleUp_H_Co = 1;
			base->CICOSCCTRL.ScaleUp_V_Co = 1;
		}
		
		base->CICOSCCTRL.MainHorRatio_Co = cfg->sc.mainhratio;
		base->CICOSCCTRL.MainVerRatio_Co = cfg->sc.mainvratio;
		
	}	else if (cfg->dma_type & CAMIF_PREVIEW) {
		base->CIPRSCPRERATIO.SHfactor_Pr    = cfg->sc.shfactor;
		base->CIPRSCPRERATIO.PreHorRatio_Pr = cfg->sc.prehratio;
		base->CIPRSCPRERATIO.PreVerRatio_Pr = cfg->sc.prevratio;
		base->CIPRSCPREDST.PreDstWidth_Pr   = cfg->sc.predst_x;
		base->CIPRSCPREDST.PreDstHeight_Pr  = cfg->sc.predst_y;
		
		/* Differ from Codec */
		if (cfg->dst_fmt & CAMIF_RGB24) {
			base->CIPRSCCTRL.OutRGB_FMT_Pr = 2;
		} else { /* RGB16 */
			base->CIPRSCCTRL.OutRGB_FMT_Pr = 0;
		}	

		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v) {
			base->CIPRSCCTRL.ScaleUp_H_Pr = 1;
			base->CIPRSCCTRL.ScaleUp_V_Pr = 1;
		}

		base->CIPRSCCTRL.MainHorRatio_Pr  = cfg->sc.mainhratio;
		base->CIPRSCCTRL. MainVerRatio_Pr = cfg->sc.mainvratio;
	} else {
		__E("s3c_camif.c : CAMERA:DMA_TYPE Wrong \n");
	}
	
	return 0;
}


/* Configure Pre-scaler control  & main scaler control register */
static int camif_setup_scaler(camif_cfg_t *cfg)
{
	int tx = cfg->target_x, ty=cfg->target_y;
	int sx, sy;

	__D("\n");
	
	if (tx <= 0 || ty<= 0) {
		__E("Invalid target size \n");
		return -1;
	}

	sx = cfg->cis->source_x - (cfg->cis->win_hor_ofst + cfg->cis->win_hor_ofst2);
	sy = cfg->cis->source_y - (cfg->cis->win_ver_ofst + cfg->cis->win_hor_ofst2);	

	if (sx <= 0 || sy<= 0) 	{
		__E("Invalid source size \n");
		return -1;
	}
	cfg->sc.modified_src_x = sx;
	cfg->sc.modified_src_y = sy;

	/* Pre-scaler control register 1 */
	camif_scaler_internal(sx, tx, &cfg->sc.prehratio,&cfg->sc.hfactor);
	camif_scaler_internal(sy, ty, &cfg->sc.prevratio,&cfg->sc.vfactor);

	if (cfg->dma_type & CAMIF_PREVIEW) {
		if ( (sx /cfg->sc.prehratio) <= 640 ) {
		} else {
			__I("Internal Preview line buffer is 640 pixels\n");
		}
	}

	cfg->sc.shfactor = 10-(cfg->sc.hfactor+cfg->sc.vfactor);
	/* Pre-scaler control register 2 */
	cfg->sc.predst_x = sx / cfg->sc.prehratio;
	cfg->sc.predst_y = sy / cfg->sc.prevratio;

	/* Main-scaler control register */
	cfg->sc.mainhratio = (sx << 8)/(tx << cfg->sc.hfactor);
	cfg->sc.mainvratio = (sy << 8)/(ty << cfg->sc.vfactor);

	__D(" sx %d, sy %d tx %d ty %d  \n",sx,sy,tx,ty);
	__D(" hfactor %d  vfactor %d \n",cfg->sc.hfactor,cfg->sc.vfactor);
	__D(" prehratio %d  prevration %d \n",cfg->sc.prehratio,cfg->sc.prevratio);
	__D(" mainhratio %d  mainvratio %d \n",cfg->sc.mainhratio,cfg->sc.mainvratio);

	cfg->sc.scaleup_h  = (sx <= tx) ? 1: 0;
	cfg->sc.scaleup_v  = (sy <= ty) ? 1: 0;
	
	if ( cfg->sc.scaleup_h != cfg->sc.scaleup_v) {
		__E("scaleup_h must be same to scaleup_v \n");
		__E(" sx %d, sy %d tx %d ty %d  \n",sx,sy,tx,ty);
		__E(" hfactor %d  vfactor %d \n",cfg->sc.hfactor,cfg->sc.vfactor);
		__E(" prehratio %d  prevration %d \n",cfg->sc.prehratio,cfg->sc.prevratio);
		__E(" mainhratio %d  mainvratio %d \n",cfg->sc.mainhratio,cfg->sc.mainvratio);
		__E(" scaleup_h %d  scaleup_v %d \n",cfg->sc.scaleup_h,cfg->sc.scaleup_v);
	}
	camif_hw_reg(cfg);
	camif_target_area(cfg);
	
	return 0;
}

/******************************************************
 CalculateBurstSize - Calculate the busrt lengths
 Description:
 - dstHSize: the number of the byte of H Size.
********************************************************/
static void camif_g_bsize(u32 hsize, u32 *mburst, u32 *rburst)
{
	u32 tmp;

	__D("\n");
	
	tmp = (hsize>>2) & 0xf;
	switch(tmp) {
	case 0:
		*mburst=16;
		*rburst=16;
		break;
	case 4:
		*mburst=16;
		*rburst=4;
		break;
	case 8:
		*mburst=16;
		*rburst=8;
		break;
	default:
		tmp=(hsize/4)%8;
		switch(tmp) {
		case 0:
			*mburst=8;
			*rburst=8;
			break;
		case 4:
			*mburst=8;
			*rburst=4;
		default:
			*mburst=4;
			tmp=(hsize/4)%4;
			*rburst= (tmp) ? tmp: 4;
			break;
		}
		break;
	}
}

/* UXGA 1600x1200 */
/* SXGA 1028x1024 */
/* XGA 1024x768 */
/* SVGA 800x600 */
/* VGA 640x480 */
/* CIF 352x288 */
/* QVGA 320x240 */
/* QCIF 176x144 */
/* ret val 
   1 : DMA Size Error 
*/

#define BURST_ERR 1 
static int camif_dma_burst(camif_cfg_t *cfg)
{
	//u32 val;
	u32 yburst_m, yburst_r;
	u32 cburst_m, cburst_r;
	int width = cfg->target_x;

	__D("\n");
	
	if (cfg->dma_type & CAMIF_CODEC ) {
		if((cfg->dst_fmt == CAMIF_RGB16)||(cfg->dst_fmt == CAMIF_RGB24)) {
			if(cfg->dst_fmt == CAMIF_RGB24) {
				if (width %2 != 0 )
					return BURST_ERR;   /* DMA Burst Length Error */
				camif_g_bsize(width*4,&yburst_m,&yburst_r);
			
			} else {	/* CAMIF_RGB16 */
				if ((width/2) %2 != 0 )
					return BURST_ERR; /* DMA Burst Length Error */
				camif_g_bsize(width*2,&yburst_m,&yburst_r);
			}
			
			if(cfg->dst_fmt ==CAMIF_RGB24) {
				base->CICOCTRL.Yburst1_Co = yburst_m/2;
				base->CICOCTRL.Yburst2_Co = yburst_r/4;
				base->CICOCTRL.Cburst1_Co = 4;
				base->CICOCTRL.Cburst2_Co = 2;
			} else {
				base->CICOCTRL.Yburst1_Co = yburst_m/2;
				base->CICOCTRL.Yburst2_Co = yburst_r/2;
				base->CICOCTRL.Cburst1_Co = 4;
				base->CICOCTRL.Cburst2_Co = 2;
			}
		} else {
			/* CODEC DMA WIDHT is multiple of 16 */
			if (width %16 != 0 )
				return BURST_ERR;   /* DMA Burst Length Error */

#ifdef TOMTOM_INTERLACE_MODE
			camif_g_bsize(width*2,&yburst_m,&yburst_r);
			yburst_m = yburst_m >> 1;
			yburst_r = yburst_r >> 1;
			cburst_m = yburst_m >> 1;
			cburst_r = yburst_r >> 1;
			if(cfg->interlace_capture)
				*(v32*)&base->CICOSCOSY = width;
#else
			camif_g_bsize(width,&yburst_m,&yburst_r);
			camif_g_bsize(width/2,&cburst_m,&cburst_r);
#endif
			base->CICOCTRL.Yburst1_Co = yburst_m;
			base->CICOCTRL.Cburst1_Co = cburst_m;
			base->CICOCTRL.Yburst2_Co = yburst_r;
			base->CICOCTRL.Cburst2_Co = cburst_r;
		}
	}

	if (cfg->dma_type & CAMIF_PREVIEW) {
		if(cfg->dst_fmt == CAMIF_RGB24) {
			if (width %2 != 0 )
				return BURST_ERR;   /* DMA Burst Length Error */
			camif_g_bsize(width*4,&yburst_m,&yburst_r);
		} else {		/* CAMIF_RGB16 */
			if ((width/2) %2 != 0 )  return BURST_ERR; /* DMA Burst Length Error */
			camif_g_bsize(width*2,&yburst_m,&yburst_r);
		}

		base->CICOCTRL.Yburst1_Co = yburst_m;
		base->CICOCTRL.Yburst2_Co = yburst_r;
	}
	
	return 0;
}


/* 
   Reset Camera IP in CPU
   Reset External Sensor 
*/
void camif_reset(int is, int delay)
{   
	__D("\n");
	
	switch (is) {
	case CAMIF_RESET:
		if(base->CISRCFMT.ITU601_656n) { // ITU-R BT 601
			base->CIGCTRL.SwRst     = 1;
			base->CIGCTRL.IRQ_Level = 1; /* For level interrupt */
			mdelay(1);
			base->CIGCTRL.SwRst = 0;
		} else{ // ITU-R BT 656
			base->CISRCFMT.ITU601_656n = 1;
			base->CIGCTRL.SwRst = 1;
			base->CIGCTRL.IRQ_Level = 1; /* For level interrupt */
			mdelay(1);
			base->CIGCTRL.SwRst = 0;
			base->CISRCFMT.ITU601_656n = 0;
		}
		break;
			
	case CAMIF_EX_RESET_AH: /*Active High */
		base->CIGCTRL.CamRst = 1;
		udelay(200);
		udelay(delay);
		base->CIGCTRL.CamRst = 1;
		break;
			
	case CAMIF_EX_RESET_AL:	/*Active Low */
		base->CIGCTRL.CamRst = 0;
		udelay(200);
		base->CIGCTRL.CamRst = 1;
		udelay(delay);
		base->CIGCTRL.CamRst = 0;
		break;
			
	default:
		break;
	}
}


void clear_camif_irq(int irq)
{
	__D("\n");
	
	if(irq == IRQ_CAMIF_C) {
		base->CIGCTRL.IRQ_CLR_c = 1;
	} else if (irq == IRQ_CAMIF_P) {
		base->CIGCTRL.IRQ_CLR_p = 1;
	}
}


/* Init external image sensor 
 *  Before make some value into image senor,
 *  you must set up the pixel clock.
 */
void camif_setup_sensor(void)
{
	__D("\n");
	camif_reset(CAMIF_RESET, 0);

	//gpio init
	s3c_gpio_cfgpin(S3C_GPF5, S3C_GPF5_CAMIF_YDATA0);
	s3c_gpio_cfgpin(S3C_GPF6, S3C_GPF6_CAMIF_YDATA1);
	s3c_gpio_cfgpin(S3C_GPF7, S3C_GPF7_CAMIF_YDATA2);
	s3c_gpio_cfgpin(S3C_GPF8, S3C_GPF8_CAMIF_YDATA03);
	s3c_gpio_cfgpin(S3C_GPF9, S3C_GPF9_CAMIF_YDATA4);
	s3c_gpio_cfgpin(S3C_GPF10, S3C_GPF10_CAMIF_YDATA5);
	s3c_gpio_cfgpin(S3C_GPF11, S3C_GPF11_CAMIF_YDATA06);
	s3c_gpio_cfgpin(S3C_GPF12, S3C_GPF12_CAMIF_YDATA7);
	s3c_gpio_cfgpin(S3C_GPF2, S3C_GPF2_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C_GPF4, S3C_GPF4_CAMIF_VSYNC);
	s3c_gpio_cfgpin(S3C_GPF1, S3C_GPF1_CAMIF_HREF);
	s3c_gpio_cfgpin(S3C_GPF0, S3C_GPF0_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C_GPF3, S3C_GPF3_CAMIF_RST);
	writel(0x0, S3C_GPFPU);

#if defined(CONFIG_VIDEO_SAMSUNG_S5K3AA)
	s3c_camif_set_clock(48000000);
#elif (defined(CONFIG_VIDEO_OV9650) || defined(CONFIG_VIDEO_SAA7113))
	s3c_camif_set_clock(24000000); 
#else
	s3c_camif_set_clock(19000000);
#endif

    /* Sometimes, Before loading I2C module, we need the reset signal */
	camif_reset(CAMIF_EX_RESET_AH,1000);

}

static s3c_priority_t *priority = S3C_PRIORITY;
static s3c_priority_t irq_old_priority; 

void camif_hw_close(camif_cfg_t *cfg)
{
	__D("\n");
	
	*priority = irq_old_priority;
	syscon->SCLK_GATE.SCLK_CAM = 0;	//disable clock
}

void camif_hw_open(camif_cis_t *cis)
{
	__D("\n");
	
	camif_source_fmt(cis);
	camif_win_offset(cis);

	irq_old_priority = *priority;
	priority->ARB_SEL0  = 0;
	priority->ARB_SEL0  = 1;/* Arbiter 1, REQ2 first */
	priority->ARB_MODE1 = 0;/* Disable Priority Rotate */
}

void camif_register_cis(struct i2c_client *ptr)
{
	s32 i;
	camif_cis_t *cis = (typeof(cis)) (ptr->data);

	for(i = 0; i < ARRAY_SIZE(fimc); i++) {
		fimc[i].cis = cis;
	}

	mutex_init(&cis->lock);		/* global lock for both Codec and Preview */
	cis->status |= P_NOT_WORKING;	/* Default Value */
	camif_hw_open(cis);

}

void camif_unregister_cis(struct i2c_client *ptr)
{   
	camif_cis_t *cis = (typeof(cis)) (ptr->data);
	cis->init_sensor = 0;
}

EXPORT_SYMBOL(camif_register_cis);
EXPORT_SYMBOL(camif_unregister_cis);
