#define LOG_TAG "s3c-tvenc"
#include "osal.h"
#include "s3c.h"

/*
 * linux/drivers/tvenc/s3c-tvenc.c
 *
 * Revision 1.0  
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    S3C TV Encoder driver 
 *
 */

#include <asm/arch/regs-tvenc.h>
#include <asm/arch/regs-lcd.h>
#include <media/v4l2-common.h>
#include "s3c-tvenc.h"

#define PFX "s3c_tvenc"

static struct clk *tvenc_clock;
static struct clk *h_clk;
static int s3c_tvenc_irq = NO_IRQ;
static struct resource *s3c_tvenc_mem;

static s3c_tvenc_t __iomem       *base;
static s3c_tvenc_t __iomem       *s3c_tvenc_base;

static tv_out_params_t tv_param = {0,};

/* Backup SFR value */
static u32 backup_reg[2];


/* Structure that declares the access functions*/
static inline void tvenc_switch(tv_enc_switch_t sw)
{
	base->TVCTRL.TVONOFF = sw;	// {OFF, ON};
}

static inline void set_image_size(u32 width, u32 height)
{
	base->InImageSize.ImageWidth  = width;
	base->InImageSize.ImageHeight = height;
}

#if 0
static void s3c_tvenc_enable_macrovision(tv_standard_t tvmode, macro_pattern_t pattern)
{
	switch(pattern) {
	case AGC4L :
		break;
	case AGC2L :
		break;
	case N01 :
		break;
	case N02 :
		break;
	case P01 :
		break;
	case P02 :	
		break;
	default :
		break;
	}
}

static void s3c_tvenc_disable_macrovision(void)
{
	__raw_writel(__raw_readl(base + S3C_MACROVISION0) 
		&~0xff, base + S3C_MACROVISION0);
	base->MacroBurstCTRL = ;
}
#endif

static inline void set_tv_mode(tv_standard_t mode, tv_conn_type_t out)
{
	//u32 signal_type = 0, output_type = 0;
	
	switch(mode) {
	case PAL_N :
		base->TVCTRL.TVOUTFMT = 0;
		base->PEDCTRL = 0;
		goto pal_comm;
	case PAL_NC :
		base->TVCTRL.TVOUTFMT = 4;
		base->FscAuxCTRL.Phalt = 1;
		base->FscAuxCTRL.Fdrst = 0;
		base->PEDCTRL = 1;
		goto pal_comm;
	case PAL_BGHID :
		base->TVCTRL.TVOUTFMT = 2;
		base->FscAuxCTRL.Phalt = 1;
		base->FscAuxCTRL.Fdrst = 0;
		base->PEDCTRL = 1;
	pal_comm:
		base->VBPORCH.VEFBPD = 0x14F;
		base->VBPORCH.VOFBPD = 0x16;
		base->HBPORCH.HBPD   = 0x108;
		base->HBPORCH.HSPW   = 0x80;
		base->HEnhOffset.DTOffset = 0x4;
		base->HEnhOffset.HEOV = 0x1A;
		base->YCFilterBW.YBW = 3;
		base->YCFilterBW.CBW = 3;
		base->SyncSizeCTRL = 0x3E;
		base->BurstCTRL.BuEnd = 0x6A;
		base->BurstCTRL.BuSt  = 0x4A;
		base->MacroBurstCTRL  = 0x42;
		base->ActVidPoCTRL.AvonEnd = 0x352; 
		base->ActVidPoCTRL.AvonSt  = 0x82; 
		break;

	case NTSC_443 :
		base->TVCTRL.TVOUTFMT = 0;
		base->PEDCTRL = 0;
		base->YCFilterBW.YBW = 3;
		goto ntsc_comm;
	case NTSC_J :
		base->TVCTRL.TVOUTFMT = 1;
		base->FscAuxCTRL.Phalt = 0;
		base->FscAuxCTRL.Fdrst = 1;
		base->PEDCTRL = 1;
		base->YCFilterBW.YBW = 4;
		goto ntsc_comm;
	case PAL_M 	:	
		base->TVCTRL.TVOUTFMT = 3;
		base->FscAuxCTRL.Phalt = 1;
		base->FscAuxCTRL.Fdrst = 0;
		base->PEDCTRL = 0;
		base->YCFilterBW.YBW = 4;
		goto ntsc_comm;
	case NTSC_M	:
		base->TVCTRL.TVOUTFMT = 0;
		base->FscAuxCTRL.Phalt = 0;
		base->FscAuxCTRL.Fdrst = 1;
		base->PEDCTRL = 0;
		base->YCFilterBW.YBW = 4;
		goto ntsc_comm;
	default :
		base->TVCTRL.TVOUTFMT = 0;
		base->PEDCTRL = 0;
		base->YCFilterBW.YBW = 4;
		
	ntsc_comm:
		base->VBPORCH.VEFBPD = 0x11C;
		base->VBPORCH.VOFBPD = 0x15;
		base->HBPORCH.HSPW   = 0x80;
		base->HBPORCH.HBPD   = 0xF4;
		base->HEnhOffset.DTOffset = 0x4;
		base->HEnhOffset.HEOV = 0x1A;
		base->YCFilterBW.CBW = 3;
		base->SyncSizeCTRL = 0x3D;
		base->BurstCTRL.BuEnd = 0x69;
		base->BurstCTRL.BuSt  = 0x49;
		base->MacroBurstCTRL  = 0x41;
		base->ActVidPoCTRL.AvonEnd = 0x348; 
		base->ActVidPoCTRL.AvonSt  = 0x78; 
		break;
	}

	if(out == S_VIDEO) {
		base->YCFilterBW.YBW = 0;
		base->YCFilterBW.CBW = 3;
		base->TVCTRL.TVOUTTYPE = 1;
	} else {
		base->TVCTRL.TVOUTTYPE = 0;
	}
	
}

#if 0
static void s3c_tvenc_set_pedestal(tv_enc_switch_t sw)
{
	if(sw)
		__raw_writel(EPC_PED_ON, base + S3C_PEDCTRL);
	else
		__raw_writel(EPC_PED_OFF, base + S3C_PEDCTRL);
}

static void s3c_tvenc_set_sub_carrier_freq(u32 freq)
{
	__raw_writel(FSC_CTRL(freq), base + S3C_FSCCTRL);
}

static void s3c_tvenc_set_fsc_dto(u32 val)
{
	unsigned int temp;

	temp = (0x1<<31)|(val&0x7fffffff);
	__raw_writel(temp, base + S3C_FSCDTOMANCTRL);
}

static void s3c_tvenc_disable_fsc_dto(void)
{
	__raw_writel(__raw_readl(base + S3C_FSCDTOMANCTRL)&~(1<<31), 
		base + S3C_FSCDTOMANCTRL);
}

static void s3c_tvenc_set_bg(u32 soft_mix, u32 color, u32 lum_offset)
{
	unsigned int bg_color;
	switch(color) {
	case 0 :
		bg_color = BGC_BGCS_BLACK;
		break;
	case 1 :
		bg_color = BGC_BGCS_BLUE;
		break;
	case 2 :
		bg_color = BGC_BGCS_RED;
		break;
	case 3 :
		bg_color = BGC_BGCS_MAGENTA;
		break;
	case 4 :
		bg_color = BGC_BGCS_GREEN;
		break;
	case 5 :
		bg_color = BGC_BGCS_CYAN;
		break;
	case 6 :
		bg_color = BGC_BGCS_YELLOW;
		break;
	case 7 :
		bg_color = BGC_BGCS_WHITE;
		break;	
	}
	if(soft_mix)
		__raw_writel(BGC_SME_ENA|bg_color|BGC_BGYOFS(lum_offset), 
		base + S3C_BGCTRL);
	else
		__raw_writel(BGC_SME_DIS|bg_color|BGC_BGYOFS(lum_offset), 
		base + S3C_BGCTRL);
		
}

static void s3c_tvenc_set_bg_vav_hav(u32 hav_len, u32 vav_len, u32 hav_st, u32 vav_st)
{
	__raw_writel(BVH_BG_HL(hav_len)|BVH_BG_HS(hav_st)|BVH_BG_VL(vav_len)|BVH_BG_VS(vav_st), 
		base + S3C_BGHVAVCTRL);
}
#endif

#if 0
static u32 s3c_tvenc_get_hue_phase(void)
{
	return __raw_readl(base + S3C_HUECTRL)&0xff;
}
#endif


#if 0
static u32 s3c_tvenc_get_contrast(void)
{
	return (__raw_readl(base + S3C_CONTRABRIGHT)&0xff);
}
#endif


#if 0
static u32 s3c_tvenc_get_bright(void)
{
	return ((__raw_readl(base + S3C_CONTRABRIGHT)&(0xff<<16))>>16);
}


static void s3c_tvenc_set_cbgain(u32 cbgain)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CBCRGAINCTRL);

	__raw_writel((temp &~0xff)|cbgain, 
		base + S3C_CBCRGAINCTRL);
}


static u32 s3c_tvenc_get_cbgain(void)
{
	return (__raw_readl(base + S3C_CBCRGAINCTRL)&0xff);
}

static void s3c_tvenc_set_crgain(u32 crgain)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CBCRGAINCTRL);

	__raw_writel((temp &~(0xff<<16))| (crgain<<16), 
		base + S3C_CBCRGAINCTRL);
}

static u32 s3c_tvenc_get_crgain(void)
{
	return ((__raw_readl(base + S3C_CBCRGAINCTRL)&(0xff<<16))>>16);
}
#endif


#if 0
static u32 s3c_tvenc_get_gamma_gain(void)
{
	return ((__raw_readl(base + S3C_GAMMACTRL)&(0x7<<8))>>8);
}

static void s3c_tvenc_enable_mute_control(tv_enc_switch_t enable)
{
	u32 temp;

	temp = __raw_readl(base + S3C_GAMMACTRL);
	if(enable == ON)
		temp |= (1<<12);
	else
		temp &= ~(1<<12);

	__raw_writel(temp, base + S3C_GAMMACTRL);
}

static void s3c_tvenc_set_mute(u32 y, u32 cb, u32 cr)
{
	u32 temp;

	temp = __raw_readl(base + S3C_MUTECTRL);

	temp &=~(0xffffff<<8);
	temp |= (cr & 0xff)<<24;
	temp |= (cb & 0xff)<<16;
	temp |= (y & 0xff)<<8;

	__raw_writel(temp, base + S3C_MUTECTRL);
}

static void s3c_tvenc_get_mute(u32 *y, u32 *cb, u32 *cr)
{
	u32 temp;

	temp = __raw_readl(base + S3C_MUTECTRL);

	*y = (temp&(0xff<<8))>>8;	
	*cb = (temp&(0xff<<16))>>16;	
	*cr = (temp&(0xff<<24))>>24;	
}
#endif


// LCD display controller configuration functions
static void s3c_lcd_set_output_path(lcd_local_output_t out)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_set_output_path(out);
#else	// peter for 2.6.24 kernel
	s3cfb_set_output_path(out);
#endif
}

static void s3c_lcd_set_clkval(u32 clkval)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_set_clkval(clkval);
#else	// peter for 2.6.24 kernel
	s3cfb_set_clock(clkval);
#endif
}

static void s3c_lcd_enable_rgbport(u32 on_off)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_enable_rgbport(on_off);
#else	// peter for 2.6.24 kernel
	s3cfb_enable_rgbport(on_off);
#endif
}

static void s3c_lcd_start(void)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_start_lcd();
#else	// peter for 2.6.24 kernel
	s3cfb_start_lcd();
#endif
}

static void s3c_lcd_stop(void)
{
#if 0	// peter for 2.6.21 kernel		
	s3c_fb_stop_lcd();
#else	// peter for 2.6.24 kernel
	s3cfb_stop_lcd();
#endif
}


static void s3c_lcd_set_config(void)
{
	backup_reg[0] = __raw_readl(S3C_VIDCON0);
	backup_reg[1] = __raw_readl(S3C_VIDCON2);

	s3c_lcd_set_output_path(LCD_TV);
	tv_param.lcd_output_mode = LCD_TV;
	
	s3c_lcd_set_clkval(4);
	s3c_lcd_enable_rgbport(1);	
}

static void s3c_lcd_exit_config(void)
{
	__raw_writel(backup_reg[0], S3C_VIDCON0);
	__raw_writel(backup_reg[1], S3C_VIDCON2);
	tv_param.lcd_output_mode = LCD_RGB;
}

static int scaler_test_start(void)
{
	tv_param.sp.DstFullWidth = 640;
	tv_param.sp.DstFullHeight= 480;
	tv_param.sp.DstCSpace = RGB16;
	
	s3c_tvscaler_config(&tv_param.sp);

	s3c_tvscaler_int_enable(1);

	s3c_tvscaler_start();

	return 0;
}

static int scaler_test_stop(void)
{
	s3c_tvscaler_int_disable();

	return 0;
}


static int tvout_start(void)
{
	tv_param.sp.DstFullWidth *= 2; // For TV OUT
	tv_param.sp.Mode          = FREE_RUN;
	tv_param.sp.DstCSpace     = YCBYCR;

	/* Set TV-SCALER parameter */
	switch(tv_param.v2.input->type) {

	case V4L2_INPUT_TYPE_FIFO:		// LCD FIFO-OUT
		/* Display controller setting */
		s3c_lcd_stop();
		s3c_lcd_set_config();
		s3c_lcd_start();
		break;
		
	case V4L2_INPUT_TYPE_MSDMA:		// MSDMA		
		break;

	default:
		return -EINVAL;
	}
	
	set_tv_mode(tv_param.sig_type, tv_param.connect);	
	set_image_size(tv_param.sp.DstFullWidth, tv_param.sp.DstFullHeight);	
	tvenc_switch(ON);
	
	s3c_tvscaler_config(&tv_param.sp);	// for setting DstStartX/Y, DstWidth/Height
	s3c_tvscaler_set_interlace(1);

	if(tv_param.v2.input->type == V4L2_INPUT_TYPE_FIFO) {
		s3c_tvscaler_int_disable();
	} else {
		s3c_tvscaler_int_enable(1);
	}
	
	s3c_tvscaler_start();
	return 0;
}

static int tvout_stop(void)
{

	s3c_tvscaler_stop_freerun();
	s3c_tvscaler_int_disable();
	s3c_tvscaler_set_interlace(0);
	tvenc_switch(OFF);
	
	switch(tv_param.v2.input->type) {
	case V4L2_INPUT_TYPE_FIFO:	// LCD FIFO-OUT
		/* Display controller setting */
		s3c_lcd_stop();
		s3c_lcd_exit_config();
		s3c_lcd_start();
		break;
	default:
		break;
	}
	return 0;
}

/* ------------------------------------------ V4L2 SUPPORT ----------------------------------------------*/
/* ------------- In FIFO and MSDMA, v4l2_input supported by S3C TVENC controller ------------------*/
static struct v4l2_input tvenc_inputs[] = {
	{
		.index		= 0,
		.name		= "LCD FIFO_OUT",
		.type		= V4L2_INPUT_TYPE_FIFO,
		.audioset		= 1,
		.tuner		= 0, /* ignored */
		.std			= 0,
		.status		= 0,
	}, 
	{
		.index		= 1,
		.name		= "Memory input (MSDMA)",
		.type		= V4L2_INPUT_TYPE_MSDMA,
		.audioset		= 2,
		.tuner		= 0,
		.std			= 0,
		.status		= 0,
	} 
};

/* ------------ Out FIFO and MADMA, v4l2_output supported by S3C TVENC controller ----------------*/
static struct v4l2_output tvenc_outputs[] = {
	{
		.index		= 0,
		.name		= "TV-OUT",
		.type		= V4L2_OUTPUT_TYPE_ANALOG,
		.audioset		= 0,
		.modulator	= 0,
		.std			= V4L2_STD_PAL | V4L2_STD_NTSC_M,
	}, 
	{
		.index		= 1,
		.name		= "Memory output (MSDMA)",
		.type		= V4L2_OUTPUT_TYPE_MSDMA,
		.audioset		= 0,
		.modulator	= 0, 
		.std			= 0,
	}, 

};

const struct v4l2_fmtdesc tvenc_input_formats[] = {
	{
		.index    = 0,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.description = "16 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB565,
		.flags    = FORMAT_FLAGS_PACKED,
	},
	{
		.index    = 1,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags    = FORMAT_FLAGS_PACKED,
		.description = "24 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB24,
	},
	{
		.index     = 2,
		.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags     = FORMAT_FLAGS_PLANAR,
		.description = "4:2:2, planar, Y-Cb-Cr",
		.pixelformat = V4L2_PIX_FMT_YUV422P,

	},
	{
		.index    = 3,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags    = FORMAT_FLAGS_PLANAR,
		.description     = "4:2:0, planar, Y-Cb-Cr",
		.pixelformat   = V4L2_PIX_FMT_YUV420,
	}
};

const struct v4l2_fmtdesc tvenc_output_formats[] = {
	{
		.index    = 0,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.description = "16 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB565,
		.flags    = FORMAT_FLAGS_PACKED,
	},
	{
		.index    = 1,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags    = FORMAT_FLAGS_PACKED,
		.description = "24 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB24,
	},
	{
		.index     = 2,
		.type      = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags     = FORMAT_FLAGS_PLANAR,
		.description = "4:2:2, planar, Y-Cb-Cr",
		.pixelformat = V4L2_PIX_FMT_YUV422P,

	},
	{
		.index    = 3,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags    = FORMAT_FLAGS_PLANAR,
		.description     = "4:2:0, planar, Y-Cb-Cr",
		.pixelformat   = V4L2_PIX_FMT_YUV420,
	}
};

const struct v4l2_standard tvout_standards[] = {
	{
		.index    = 0,
		.id     = V4L2_STD_NTSC_M,
		.name = "NTSC type",
	},
	{
		.index    = 1,
		.id     = V4L2_STD_PAL,
		.name = "PAL type",
	}	
};

static inline int s_fmt(struct v4l2_format *f)
{	
	/* update our state informations */
	tv_param.v2.pixfmt= f->fmt.pix;

	switch(tv_param.v2.pixfmt.pixelformat) {

	case V4L2_PIX_FMT_RGB565:
		tv_param.sp.SrcCSpace = RGB16;
		tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width;
		tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height;
		tv_param.sp.SrcStartX = 0;
		tv_param.sp.SrcStartY = 0;	
		tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth;
		tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight;	
		__I("TV-OUT: LCD path operation set\n");
		break;

	case V4L2_PIX_FMT_RGB24:
		tv_param.sp.SrcCSpace = RGB24;
		break;

	case V4L2_PIX_FMT_YUV420:
		tv_param.sp.SrcCSpace = YC420;
#ifdef DIVX_TEST		// padded output 	
		tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width + 2*16;
		tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height + 2*16;
		tv_param.sp.SrcStartX = 16;
		tv_param.sp.SrcStartY = 16;
		tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth - 2*tv_param.sp.SrcStartX;
		tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight - 2*tv_param.sp.SrcStartY;
#else	// not padded output 		
		tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width;
		tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height;
		tv_param.sp.SrcStartX = 0;
		tv_param.sp.SrcStartY = 0;
		tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth;
		tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight;
#endif	
		__I("TV-OUT: MFC path operation set\n");
		break;

	case V4L2_PIX_FMT_YUV422P:
		tv_param.sp.SrcCSpace = YC422;
		break;

	default:
		return -EINVAL;
	}
	
	return 0;
}

static inline int s_control(struct v4l2_control *ctrl)
{
	switch(ctrl->id) {
		
	case V4L2_CID_MPEG_STREAM_PID_VIDEO:
		tv_param.sp.SrcFrmSt = ctrl->value;
		return 0;

	case V4L2_CID_CONNECT_TYPE:
		tv_param.connect = ctrl->value;// {0:COMPOSITE, 1:S-VIDEO}
		return 0;

	case V4L2_CID_BRIGHTNESS:
		if((ctrl->value > 0xff)||(ctrl->value < 0))
			return -EINVAL;

		base->ContraBright.BRIGHT = ctrl->value;
		return 0;

	case V4L2_CID_CONTRAST:
		if((ctrl->value > 0xff)||(ctrl->value < 0))
			return -EINVAL;
		
		base->ContraBright.CONTRAST = ctrl->value;
		return 0;

	case V4L2_CID_GAMMA:
		if((ctrl->value > 0x3)||(ctrl->value < 0))
			return -EINVAL;

		base->GammaCTRL.GamEn = ON;
		base->GammaCTRL.GamMode = ctrl->value;
		base->GammaCTRL.GamEn = OFF;
		return 0;

	case V4L2_CID_HUE:
		if((ctrl->value > 0xff)||(ctrl->value < 0))
			return -EINVAL;

		base->HUECTRL = ctrl->value;
		return 0;		

	case V4L2_CID_HCENTER:
		if((ctrl->value > 0xff)||(ctrl->value < 0)) 
			return -EINVAL;

		base->HEnhOffset.HACTWinCenCTRL = ctrl->value;

		return 0;				

	case V4L2_CID_VCENTER:
		if((ctrl->value > 0x3f)||(ctrl->value < 0)) 
			return -EINVAL;

		base->HEnhOffset.VACTWinCenCTRL = ctrl->value;
		return 0;				
	
	default:
		return -EINVAL;
	}
	return 0;
}

int s3c_tvenc_open(struct inode *inode, struct file *filp) 
{ 
	int err;

	err = video_exclusive_open(inode, filp);	// One function of V4l2 driver

	if(err < 0) 
		return err;
	filp->private_data = &tv_param;

	s3c_tvscaler_init();
	
	/* Success */
	return 0;
}

int s3c_tvenc_release(struct inode *inode, struct file *filp) 
{
	video_exclusive_release(inode, filp);
	
	/* Success */
	return 0;
}

static int s3c_tvenc_do_ioctl(struct inode *inode,struct file *filp,unsigned int cmd,void *arg)
{
	int ret;
	
	switch(cmd){
	case VIDIOC_QUERYCAP:
		{
			/* struct v4l2_capability cap = { */
			/* 	.driver       = "S3C TV-OUT driver", */
			/* 	.card         = tv_param.v->name, */
			/* 	.bus_info     = "ARM AHB BUS", */
			/* 	.capabilities = tv_param.v->type2, */
			/* }; */
			
			/* *(typeof(cap)*)arg = cap; */

			struct v4l2_capability *cap = arg;
			sprintf(cap->driver, "S3C TV-OUT driver");
			sprintf(cap->card, tv_param.v->name);
			sprintf(cap->bus_info, "ARM AHB BUS");
			cap->version = 0;
			cap->capabilities = tv_param.v->type2;
			return 0;
		}
	
	case VIDIOC_OVERLAY:
		{
			int on = *(int *)arg;
			
			__I("TV-OUT: VIDIOC_OVERLAY on:%d\n", on);
			if(on) {
				ret = tvout_start();
			} else {
				ret = tvout_stop();
			}
			return ret;
		}	

	case VIDIOC_ENUMINPUT:
		{
			struct v4l2_input *i = arg;
			__I("TV-OUT: VIDIOC_ENUMINPUT : index = %d\n", i->index);
			
			if ((i->index) >= ARRAY_SIZE(tvenc_inputs)) {
				return -EINVAL;
			}

			*i = tvenc_inputs[i->index];
			return 0;
		}

	case VIDIOC_S_INPUT:	// 0 -> LCD FIFO-OUT, 1 -> MSDMA
		{
			int index = *((int *)arg);
			__I("TV-OUT: VIDIOC_S_INPUT \n");
				
			if (index >= ARRAY_SIZE(tvenc_inputs)) {
				return -EINVAL;
			}
	
			tv_param.v2.input = &tvenc_inputs[index];	
			switch(tv_param.v2.input->type) {
			case V4L2_INPUT_TYPE_FIFO:		// LCD FIFO-OUT
				tv_param.sp.InPath = POST_FIFO;
				break;
			case V4L2_INPUT_TYPE_MSDMA:		// MSDMA
				tv_param.sp.InPath = POST_DMA;
				break;
			default:
				return -EINVAL;
			}
			return 0;
		}
	
	case VIDIOC_G_INPUT:
		{
			__I("TV-OUT: VIDIOC_G_INPUT \n");
			*(u32*)arg = tv_param.v2.input->type;
			return 0;
		}	
	
	case VIDIOC_ENUMOUTPUT:
		{
			struct v4l2_output *i = arg;
			__I("TV-OUT: VIDIOC_ENUMOUTPUT : index = %d\n", i->index);
			
			if ((i->index) >= ARRAY_SIZE(tvenc_outputs)) {
				return -EINVAL;
			}
			
			*i = tvenc_outputs[i->index];
			return 0;
		}	

	case VIDIOC_S_OUTPUT:	// 0 -> TV / FIFO , 1 -> MSDMA
		{
			int index = *((int *)arg);
			__I("TV-OUT: VIDIOC_S_OUTPUT \n");
		
			if (index >= ARRAY_SIZE(tvenc_outputs)) {
				return -EINVAL;
			}

			tv_param.v2.output = &tvenc_outputs[index];
			switch(tv_param.v2.output->type) {
			case V4L2_OUTPUT_TYPE_ANALOG:	// TV-OUT (FIFO-OUT)
				tv_param.sp.OutPath = POST_FIFO;
				break;
			case V4L2_OUTPUT_TYPE_MSDMA:	// MSDMA
				tv_param.sp.OutPath = POST_DMA;
				break;
			default:
				return -EINVAL;
			}
			return 0;
		}

	case VIDIOC_G_OUTPUT:
		{
			__I("VIDIOC_G_OUTPUT \n");
			*(u32*)arg = tv_param.v2.output->type;
			return 0;
		}

	case VIDIOC_ENUM_FMT:
		{
			struct v4l2_fmtdesc *f = arg;
			int index = f->index;

			__I("C: VIDIOC_ENUM_FMT : index = %d\n", index);
			if (index >= ARRAY_SIZE(tvenc_input_formats)) 
				return -EINVAL;
			
			switch (f->type) {
				case V4L2_BUF_TYPE_VIDEO_CAPTURE:
					break;
				case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				default:
					return -EINVAL;
			}

			*f = tv_param.v2.fmtdesc[index];
			return 0;
		}	
	
	case VIDIOC_G_FMT:
		{
			__I("C: VIDIOC_G_FMT \n");
			((struct v4l2_format*)arg)->fmt.pix = tv_param.v2.pixfmt;
			return 0;	
		}
	
	case VIDIOC_S_FMT:
		{	
			__I("C: VIDIOC_S_FMT \n");
			return s_fmt((struct v4l2_format*)arg);
		}
	
	case VIDIOC_S_CTRL:
		{
			__I("P: VIDIOC_S_CTRL \n");
			return s_control((struct v4l2_control*)arg);
		}

	case VIDIOC_ENUMSTD:
		{
			struct v4l2_standard *std = (typeof(std))arg;
			if (std->index >= ARRAY_SIZE(tvout_standards))
				return -EINVAL;

			v4l2_video_std_construct(std, tvout_standards[std->index].id,
									 (char *)tvout_standards[std->index].name);

			return 0;
		}
	
	case VIDIOC_G_STD:
		{
			*(v4l2_std_id*)arg = tvout_standards[0].id;
			return 0;
		}
	
	case VIDIOC_S_STD:
		{
			v4l2_std_id *id = arg;

			unsigned int i;
			for (i = 0; i < ARRAY_SIZE(tvout_standards); i++) {
				if (*id & tvout_standards[i].id)
					break;
			}
			if (i == ARRAY_SIZE(tvout_standards))
				return -EINVAL;

			switch(*id) {
			case V4L2_STD_NTSC_M:
				tv_param.sig_type = NTSC_M;		
				tv_param.sp.DstFullWidth = 720;		
				tv_param.sp.DstFullHeight = 480;					
				break;
			case V4L2_STD_PAL:
				tv_param.sig_type = PAL_BGHID;
				tv_param.sp.DstFullWidth = 720;
				tv_param.sp.DstFullHeight = 576;		
				break;
			default:
				return -EINVAL;
			}
			return 0;
		}

	case VIDIOC_S_TVOUT_ON:
		return tvout_start();

	case VIDIOC_S_TVOUT_OFF:
		return tvout_stop();
	
	case VIDIOC_S_SCALER_TEST:
			scaler_test_start();
			mdelay(1);
			scaler_test_stop();
			return 0;
			
	default:
 		return -EINVAL;
	}
	
	return 0;
}

static int s3c_tvenc_ioctl_v4l2(struct inode *inode, struct file *filp,
								unsigned int cmd, unsigned long arg)
{
	return video_usercopy(inode, filp, cmd, arg, s3c_tvenc_do_ioctl);
}

int s3c_tvenc_read(struct file *filp, char *buf, size_t count,
				   loff_t *f_pos)
{
	return 0;
}

int s3c_tvenc_write(struct file *filp, const char *buf, size_t 
					count, loff_t *f_pos)
{
	return 0;
}

int s3c_tvenc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	u32 size = vma->vm_end - vma->vm_start;
	u32 max_size;
	u32 page_frame_no;

	page_frame_no = __phys_to_pfn(POST_BUFF_BASE_ADDR); /* 0x55B00000 >> 12 */

	max_size = RESERVE_POST_MEM + PAGE_SIZE - (RESERVE_POST_MEM % PAGE_SIZE);

	if(size > max_size) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED;

	if( remap_pfn_range(vma, vma->vm_start, page_frame_no,
						size, vma->vm_page_prot)) {
		__E("%s: mmap_error\n", __FUNCTION__);
		return -EAGAIN;

	}

	return 0;
}

struct file_operations s3c_tvenc_fops = {
	.owner   = THIS_MODULE,
	.open    = s3c_tvenc_open,
	.ioctl   = s3c_tvenc_ioctl_v4l2,
	.release = s3c_tvenc_release,
	.read    = s3c_tvenc_read,
	.write   = s3c_tvenc_write,
	.mmap    = s3c_tvenc_mmap,
};

void s3c_tvenc_vdev_release (struct video_device *vdev) {
//	kfree(vdev);
}


irqreturn_t s3c_tvenc_isr(int irq, void *dev_id)
{
	// Clear FIFO under-run status pending bit
	base->TVCTRL.INTStatus = 1;

	return IRQ_HANDLED;
}

static int s3c_tvenc_probe(struct platform_device *pdev)
{
	struct resource *res;

	struct video_device tvencoder = {
		.name     = "TVENCODER",
		.type     = VID_TYPE_OVERLAY | VID_TYPE_CAPTURE | VID_TYPE_SCALES,
		.type2    = V4L2_CAP_VIDEO_OUTPUT| V4L2_CAP_VIDEO_CAPTURE,		/* V4L2 */
		.fops     = &s3c_tvenc_fops,
		.release  = s3c_tvenc_vdev_release,
		.minor    = TVENC_MINOR,
	};
	
	__D("\n");
	
	/* find the IRQs */
	if((s3c_tvenc_irq = platform_get_irq(pdev, 0)) <= 0) {
		__E(KERN_ERR "failed to get irq resouce\n");
		return -ENOENT;
	}

	/* get the memory region for the tv scaler driver */

	if((res = platform_get_resource(pdev, IORESOURCE_MEM, 0)) == NULL) {
		__E("failed to get memory region resouce\n");
		return -ENOENT;
	}
	__I("res->start = %x(0x76200000), res->end = %x(start+1M) \n", res->start, res->end);
	
	if((s3c_tvenc_mem = request_mem_region(res->start,
										   res->end - res->start + 1,
										   pdev->name)) == NULL) {
		__E("failed to reserve memory region\n");
		return -ENOENT;
	}


	if((base = (typeof(base))ioremap(s3c_tvenc_mem->start,
					   s3c_tvenc_mem->end - res->start + 1)) == NULL) {
		__E("failed ioremap\n");
		goto err1;
	}
								 
	s3c_tvenc_base = base;
	
	if((tvenc_clock = clk_get(&pdev->dev, "tv_encoder")) == NULL) {
		__E("failed to find tvenc clock source\n");
		goto err2;
	}

	clk_enable(tvenc_clock);

	if((h_clk = clk_get(&pdev->dev, "hclk")) == NULL) {
		__E("failed to find h_clk clock source\n");
		goto err3;
	}
		
	if(!(tv_param.v = video_device_alloc())) {
		__E("s3c-tvenc: video_device_alloc() failed\n");
		goto err4;
	}
	
	*tv_param.v = tvencoder;
	if(video_register_device(tv_param.v, VFL_TYPE_GRABBER, TVENC_MINOR) != 0) {
		__E("s3c_camera_driver.c : Couldn't register this codec driver.\n");
		goto err5;
	}
	
	if(request_irq(s3c_tvenc_irq,
				   s3c_tvenc_isr,
				   SA_INTERRUPT,
				   "TV_ENCODER", NULL)) {
		__E("request_irq(TV_ENCODER) failed.\n");
		goto err6;
	}
	
	__I("Success\n");
	return 0;

  err6:
	video_unregister_device(tv_param.v);
  err5:
	video_device_release(tv_param.v);
  err4:
	clk_put(h_clk);
  err3:
	clk_disable(tvenc_clock);
	clk_put(tvenc_clock);
  err2:
	iounmap(s3c_tvenc_base);
  err1:
	release_mem_region(s3c_tvenc_mem->start,
					   s3c_tvenc_mem->end - s3c_tvenc_mem->start + 1);

	return -ENOENT;
}

static int s3c_tvenc_remove(struct platform_device *dev)
{
	__I("s3c_tvenc_remove called !\n");

	free_irq(s3c_tvenc_irq, NULL);

	video_unregister_device(tv_param.v);
	video_device_release(tv_param.v);
	
	clk_put(h_clk);

	clk_disable(tvenc_clock);
	clk_put(tvenc_clock);
	
	__D("s3-tvenc: releasing s3c_tvenc_mem\n");

	iounmap(s3c_tvenc_base);

	release_mem_region(s3c_tvenc_mem->start,
					   s3c_tvenc_mem->end - s3c_tvenc_mem->start + 1);

	return 0;
}

static int s3c_tvenc_suspend(struct platform_device *dev, pm_message_t state)
{
	clk_disable(tvenc_clock);
	return 0;
}

static int s3c_tvenc_resume(struct platform_device *pdev)
{
	clk_enable(tvenc_clock);
	return 0;
}

static struct platform_driver s3c_tvenc_driver = {
       .probe          = s3c_tvenc_probe,
       .remove         = s3c_tvenc_remove,
       .suspend        = s3c_tvenc_suspend,
       .resume         = s3c_tvenc_resume,
       .driver	       = {
		.owner  	       = THIS_MODULE,
		.name	           = "s3c-tvenc",
	},
};

static int __init  s3c_tvenc_init(void)
{
	__D("S3C6410 TV encoder Driver, (c) 2008 Samsung Electronics\n");
	
 	if(platform_driver_register(&s3c_tvenc_driver) != 0)
  	{
   		__E("Platform Device Register Failed \n");
   		return -1;
  	}
	
	__I(" S3C6410 TV encoder Driver module init OK. \n");

	return 0;
}

static void __exit  s3c_tvenc_exit(void)
{
	platform_driver_unregister(&s3c_tvenc_driver);
	   
 	printk("S3C6410 TV encoder Driver module exit. \n");
}


module_init(s3c_tvenc_init);
module_exit(s3c_tvenc_exit);


MODULE_AUTHOR("Peter, Oh");
MODULE_DESCRIPTION("S3C TV Encoder Device Driver");
MODULE_LICENSE("GPL");
