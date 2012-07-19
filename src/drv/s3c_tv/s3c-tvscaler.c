#define LOG_TAG "s3c-tvscaler"
#include "osal.h"
#include "s3c.h"

/*
 * linux/drivers/tvenc/s3c-tvscaler.c
 *
 * Revision 1.0  
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    S3C TV Scaler driver 
 *
 */

#include <asm/arch/regs-tvscaler.h>
#include <asm/arch/regs-s3c6400-clock.h>
#include "s3c-tvscaler.h"

#define SINGLE_BUF	1		// Single buffer mode


static s3c_tvscaler_t __iomem    *base;
static s3c_tvscaler_t __iomem    *s3c_tvscaler_base;

static struct clk      *h_clk;
static struct clk      *tvscaler_clock;
static int              s3c_tvscaler_irq = NO_IRQ;
static struct resource *s3c_tvscaler_mem;

static irqreturn_t s3c_tvscaler_isr(int irq, void *dev_id)
{
	base->MODE.POSTINT = 0; /* Clear Source in POST Processor */

	return IRQ_HANDLED;
}

static inline void set_clk_src(scaler_clk_src_t clk_src)
{
	u32 rate = clk_get_rate(h_clk);

	h_clk = clk_get(NULL, "hclk");

	if(clk_src == HCLK) {
		if(rate > 66000000) {
			base->MODE.CLKDIR   = 1;
			base->MODE.CLKVAL_F = 1;
		} else {
			base->MODE.CLKDIR   = 0;
			base->MODE.CLKVAL_F = 0;
		}
		base->MODE.CLKVALUP = 0;
		
	} else if(clk_src == PLL_EXT) {
		/* do nothing */
	} else {
		base->MODE.CLKDIR   = 0;
		base->MODE.CLKVAL_F = 0;
		base->MODE.CLKVALUP = 0;
	}

	base->MODE.CLKSEL_F = clk_src;
}

static inline void set_fmt(cspace_t src,
								 cspace_t dst,
								 s3c_scaler_path_t in,
								 s3c_scaler_path_t out,
								 u32 *in_pixel_size,
								 u32 *out_pixel_size)
{
	base->MODE.R2YSel      = 1;
	base->MODE.Wide_Narrow = 2;

	if(in == POST_DMA) {

		switch(src) {
		case YC420:
			base->MODE.SRC420      = 1;
			base->MODE.InRGB       = 0;
			base->MODE.INTERLEAVE  = 0;
			base->MODE.InRGBFormat = 1;
			*in_pixel_size = 1;
			break;
		case CRYCBY:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 0;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 1;
			base->MODE.InYCbCrFormat_LSB = 0;
			base->MODE.InYCbCrFormat_MSB = 0;
			*in_pixel_size = 2;		
			break;
		case CBYCRY:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 0;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 1;
			base->MODE.InYCbCrFormat_LSB = 0;
			base->MODE.InYCbCrFormat_MSB = 1;
			*in_pixel_size = 2;	
			break;
		case YCRYCB:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 0;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 1;
			base->MODE.InYCbCrFormat_LSB = 1;
			base->MODE.InYCbCrFormat_MSB = 0;
			*in_pixel_size = 2;			
			break;
		case YCBYCR:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 0;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 1;
			base->MODE.InYCbCrFormat_LSB = 1;
			base->MODE.InYCbCrFormat_MSB = 1;
			*in_pixel_size = 2;			
			break;
		case RGB24:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 1;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 1;
			*in_pixel_size = 4;
			break;
		case RGB16:
			base->MODE.SRC420            = 0;
			base->MODE.InRGB             = 1;
			base->MODE.INTERLEAVE        = 1;
			base->MODE.InRGBFormat       = 0;
			*in_pixel_size = 2;			
			break;
		default:
			break;
		}

	} 
	else if(in == POST_FIFO) {
	}

	if(out == POST_DMA) {
		switch(dst) {
		case YC420:
			base->MODE.DST420 = 1;
			base->MODE.OutRGB = 0;
			*out_pixel_size = 1;			
			break;
		case CRYCBY:
			base->MODE.DST420         = 0;
			base->MODE.OutYCbCrFormat = 0;
			base->MODE.OutRGB         = 0;
			*out_pixel_size = 2;
			break;
		case CBYCRY:
			base->MODE.DST420         = 0;
			base->MODE.OutYCbCrFormat = 2;
			base->MODE.OutRGB         = 0;
			*out_pixel_size = 2;			
			break;
		case YCRYCB:
			base->MODE.DST420         = 0;
			base->MODE.OutYCbCrFormat = 1;
			base->MODE.OutRGB         = 0;
			*out_pixel_size = 2;			
			break;
		case YCBYCR:
			base->MODE.DST420         = 0;
			base->MODE.OutYCbCrFormat = 3;
			base->MODE.OutRGB         = 0;
			*out_pixel_size = 2;			
			break;
		case RGB24:
			base->MODE.OutRGB         = 1;
			base->MODE.OutRGBFormat   = 1;
			*out_pixel_size = 4;			
			break;
		case RGB16:
			base->MODE.OutRGB         = 1;
			base->MODE.OutRGBFormat   = 0;
			*out_pixel_size = 2;			
			break;
		default:
			break;
		}
	} else if(out == POST_FIFO) {
		if(dst == RGB24) {
			base->MODE.OutRGB         = 1;
			base->MODE.LCDPathEnable  = 1;
			
		} else if(dst == YCBYCR) {
			base->MODE.DST420         = 0;
			base->MODE.OutRGB         = 0;
			base->MODE.LCDPathEnable  = 1;
			
		} else {
		}
	}
}

static inline void set_path(s3c_scaler_path_t in, s3c_scaler_path_t out)
{
	base->MODE.Interlace = 0;
	base->MODE.ExtFIFOIn = in;
	base->MODE.LCDPathEnable = out;
}

static inline void set_addr(scaler_params_t *sp,
							u32 in_pixel_size,
							u32 out_pixel_size)
{
	if(sp->InPath == POST_DMA) {
		base->Offset_Y    = (sp->SrcFullWidth - sp->SrcWidth) * in_pixel_size;

		base->ADDRStart_Y = sp->SrcFrmSt
			+ (sp->SrcFullWidth * sp->SrcStartY + sp->SrcStartX) * in_pixel_size;
		
		base->ADDREnd_Y   = base->ADDRStart_Y
			+ sp->SrcWidth * sp->SrcHeight * in_pixel_size
			+ base->Offset_Y * (sp->SrcHeight - 1);

		
		if(sp->SrcCSpace == YC420) {
			base->Offset_Cb = ((sp->SrcFullWidth - sp->SrcWidth) / 2) * in_pixel_size;
			base->Offset_Cr = base->Offset_Cb;
			
			base->ADDRStart_Cb = sp->SrcFrmSt
				+ sp->SrcFullWidth * sp->SrcFullHeight * 1
				+ (sp->SrcFullWidth * sp->SrcStartY / 2 + sp->SrcStartX) /2 * 1;
			
			base->ADDREnd_Cb = base->ADDRStart_Cb
				+ sp->SrcWidth/2 * sp->SrcHeight/2 * in_pixel_size
				+ (sp->SrcHeight/2 -1) * base->Offset_Cb;

			base->ADDRStart_Cr = sp->SrcFrmSt
				+ sp->SrcFullWidth * sp->SrcFullHeight * 1
				+ sp->SrcFullWidth * sp->SrcFullHeight/4 * 1
				+ (sp->SrcFullWidth * sp->SrcStartY/2 + sp->SrcStartX)/2*1;
			
			base->ADDREnd_Cr = base->ADDRStart_Cr
				+ sp->SrcWidth/2 * sp->SrcHeight/2 * in_pixel_size
				+ (sp->SrcHeight/2 - 1) * base->Offset_Cr;
		}
	}
	
	if(sp->OutPath == POST_DMA) {
		base->Offset_RGB = (sp->DstFullWidth - sp->DstWidth)*out_pixel_size;
		
		base->ADDRStart_RGB = sp->DstFrmSt
			+ (sp->DstFullWidth * sp->DstStartY + sp->DstStartX) * out_pixel_size;

		base->ADDREnd_RGB = base->ADDRStart_RGB
			+ sp->DstWidth * sp->DstHeight * out_pixel_size
			+ base->Offset_RGB * (sp->DstHeight - 1);
		
		if(sp->DstCSpace == YC420) {
			base->Offset_oCb = ((sp->DstFullWidth - sp->DstWidth)/2)*out_pixel_size;
			base->Offset_oCr = base->Offset_oCb;

			base->ADDRStart_oCb = sp->DstFrmSt
				+ sp->DstFullWidth*sp->DstFullHeight*1
				+ (sp->DstFullWidth*sp->DstStartY/2 + sp->DstStartX)/2*1;
			
			base->ADDREnd_oCb = base->ADDRStart_oCb
				+ sp->DstWidth/2*sp->DstHeight/2*out_pixel_size
				+ (sp->DstHeight/2 -1)*base->Offset_oCr;
			
			base->ADDRStart_oCr = sp->DstFrmSt
				+ sp->DstFullWidth*sp->DstFullHeight*1
				+ (sp->DstFullWidth*sp->DstFullHeight/4)*1
				+ (sp->DstFullWidth*sp->DstStartY/2 +sp->DstStartX)/2*1;
			
			base->ADDREnd_oCr = base->ADDRStart_oCr
				+ sp->DstWidth/2*sp->DstHeight/2*out_pixel_size
				+ (sp->DstHeight/2 -1)*base->Offset_oCb;
		}
	}
}

#if 0
static void s3c_tvscaler_set_fifo_in(s3c_scaler_path_t in_path)
{
	base->MODE.ExtFIFOIn = in_path; //1:FIFO  0:DMA
}
#endif

void s3c_tvscaler_set_interlace(u32 on_off)
{
	base->MODE.Interlace = on_off;
}
EXPORT_SYMBOL(s3c_tvscaler_set_interlace);

static inline void set_size(scaler_params_t *sp)
{
	u32 pre_h_ratio, pre_v_ratio, h_shift, v_shift, sh_factor;
	u32 pre_dst_width, pre_dst_height, dx, dy;

	if(sp->SrcWidth >= (sp->DstWidth<<6)) {
		__E("Out of PreScalar range !!!\n");
		return;
	} else if(sp->SrcWidth >= (sp->DstWidth<<5)) {
		pre_h_ratio = 32;
		h_shift = 5;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<4)) {
		pre_h_ratio = 16;
		h_shift = 4;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<3)) {
		pre_h_ratio = 8;
		h_shift = 3;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<2)) {
		pre_h_ratio = 4;
		h_shift = 2;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<1)) {
		pre_h_ratio = 2;
		h_shift = 1;		
	} else {
		pre_h_ratio = 1;
		h_shift = 0;		
	}

	pre_dst_width = sp->SrcWidth / pre_h_ratio;
	dx = (sp->SrcWidth<<8) / (sp->DstWidth<<h_shift);


	if(sp->SrcHeight >= (sp->DstHeight<<6)) {
		__E("Out of PreScalar range !!!\n");
		return;
	} else if(sp->SrcHeight >= (sp->DstHeight<<5)) {
		pre_v_ratio = 32;
		v_shift = 5;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<4)) {
		pre_v_ratio = 16;
		v_shift = 4;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<3)) {
		pre_v_ratio = 8;
		v_shift = 3;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<2)) {
		pre_v_ratio = 4;
		v_shift = 2;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<1)) {
		pre_v_ratio = 2;
		v_shift = 1;		
	} else {
		pre_v_ratio = 1;
		v_shift = 0;		
	}	

	pre_dst_height = sp->SrcHeight / pre_v_ratio;
	dy = (sp->SrcHeight<<8) / (sp->DstHeight<<v_shift);
	sh_factor = 10 - (h_shift + v_shift);

	base->PreScale_Ratio.PreScale_H_Ratio = pre_h_ratio;
	base->PreScale_Ratio.PreScale_V_Ratio = pre_v_ratio;

	base->PreScaleImgSize.PreScale_DSTHeight = pre_dst_height;
	base->PreScaleImgSize.PreScale_DSTWidth  = pre_dst_width;

	base->PreScale_SHFactor = sh_factor;
	base->MainScale_H_Ratio = dx;
	base->MainScale_V_Ratio = dy;

	base->SRCImgSize.SRCWidth  = sp->SrcWidth;
	base->SRCImgSize.SRCHeight = sp->SrcHeight;

	base->DSTImgSize.DSTWidth  = sp->DstWidth;
	base->DSTImgSize.DSTHeight = sp->DstHeight;
}

static inline void set_auto_load(scaler_params_t *sp)
{
	base->MODE.AutoLoadEnable = sp->Mode; // 1:FREE_RUN  0:ONE_SHOT
}

void s3c_tvscaler_set_base_addr(void __iomem * base_addr)
{
	base = base_addr;
}
EXPORT_SYMBOL(s3c_tvscaler_set_base_addr);

void s3c_tvscaler_free_base_addr(void)
{
	base = NULL;
}
EXPORT_SYMBOL(s3c_tvscaler_free_base_addr);

void s3c_tvscaler_int_enable(u32 int_type)
{
	base->MODE.IRQ_Level = int_type; // 0:Edge triggering   1:level triggering
	base->MODE.INTEN = 1;
}
EXPORT_SYMBOL(s3c_tvscaler_int_enable);

void s3c_tvscaler_int_disable(void)
{
	base->MODE.INTEN = 0;
}
EXPORT_SYMBOL(s3c_tvscaler_int_disable);


void s3c_tvscaler_start(void)
{
	base->POSTENVID.enable = 1;
}
EXPORT_SYMBOL(s3c_tvscaler_start);

void s3c_tvscaler_stop_freerun(void)
{
	__D("\n");

	base->MODE.AutoLoadEnable = 0;
}
EXPORT_SYMBOL(s3c_tvscaler_stop_freerun);


void s3c_tvscaler_config(scaler_params_t *sp)
{
	u32 in_pixel_size = 0; 
	u32 out_pixel_size = 0;
	//u32 loop = 0;

	__D("\n");

	base->POSTENVID.enable = 0;

#ifdef SINGLE_BUF
	base->MODE_2.ADDR_CH_DIS = 1;
#else
	base->MODE_2.ADDR_CH_DIS = 0;
#endif

	base->MODE_2.BC_SEL = 1;
	base->MODE_2.hardware_trigger = 0;
	
// peter mod.	start	
	sp->DstStartX = sp->DstStartY = 0;	
	sp->DstWidth  = sp->DstFullWidth;
	sp->DstHeight = sp->DstFullHeight;	
// peter mod. end		

	sp->DstFrmSt = (POST_BUFF_BASE_ADDR + PRE_BUFF_SIZE);

	set_clk_src(HCLK);

	set_path(sp->InPath, sp->OutPath);

	set_fmt(sp->SrcCSpace,
			sp->DstCSpace,
			sp->InPath, 
			sp->OutPath,
			&in_pixel_size, &out_pixel_size);

	set_size(sp);

	set_addr(sp, in_pixel_size, out_pixel_size);

	set_auto_load(sp);

}
EXPORT_SYMBOL(s3c_tvscaler_config);

void s3c_tvscaler_set_param(scaler_params_t *sp)
{
	__D("\n");
#if 0	
	param.SrcFullWidth	= sp->SrcFullWidth;
	param.SrcFullHeight	= sp->SrcFullHeight;
	param.SrcStartX		= sp->SrcStartX;	
	param.SrcStartY		= sp->SrcStartY;
	param.SrcWidth		= sp->SrcWidth;
	param.SrcHeight		= sp->SrcHeight;
	param.SrcFrmSt		= sp->SrcFrmSt;
	param.SrcCSpace		= sp->SrcCSpace;
	param.DstFullWidth	= sp->DstFullWidth;
	param.DstFullHeight	= sp->DstFullHeight;
	param.DstStartX		= sp->DstStartX;
	param.DstStartY		= sp->DstStartY;
	param.DstWidth		= sp->DstWidth;
	param.DstHeight		= sp->DstHeight;
	param.DstFrmSt		= sp->DstFrmSt;
	param.DstCSpace		= sp->DstCSpace;
	param.SrcFrmBufNum	= sp->SrcFrmBufNum;
	param.DstFrmSt		= sp->DstFrmSt;
	param.Mode		= sp->Mode;	
	param.InPath		= sp->InPath;
	param.OutPath		= sp->OutPath;
#endif	
}
EXPORT_SYMBOL(s3c_tvscaler_set_param);

void s3c_tvscaler_init(void)
{
	s3c_clk_src_t *p;
	__I("\n");

	// Use DOUTmpll source clock as a scaler clock
	p = S3C_CLK_SRC; 
	p->SCALER_SEL = 1;
}
EXPORT_SYMBOL(s3c_tvscaler_init);


static int s3c_tvscaler_probe(struct platform_device *pdev)
{
	struct resource *res;

	__D("\n");

	/* find the IRQs */
	if((s3c_tvscaler_irq = platform_get_irq(pdev, 0)) <= 0) {
		__E("failed to get irq resouce\n");
		return -ENOENT;
	}
	__I("s3c_tvscaler_irq = %d(13) \n", s3c_tvscaler_irq);
	
	/* get the memory region for the tv scaler driver */
	if((res = platform_get_resource(pdev, IORESOURCE_MEM, 0)) == NULL) {
		__E("failed to get memory region resouce\n");
		return -ENOENT;
	}
	__I("res->start = %x(0x76300000), res->end = %x(start+1M) \n", res->start, res->end);
	
	if((s3c_tvscaler_mem = request_mem_region(res->start,
											  res->end - res->start + 1,
											  pdev->name)) == NULL) {
		__E("failed to reserve memory region\n");
		return -ENOENT;
	}

	if((base = (typeof(base))ioremap(s3c_tvscaler_mem->start,
					   s3c_tvscaler_mem->end - s3c_tvscaler_mem->start + 1)) == NULL) {
		__E("failed ioremap\n");
		goto err1;
	}
	s3c_tvscaler_base = base;

	if((tvscaler_clock = clk_get(&pdev->dev, "tv_encoder")) == NULL) {
		__E("failed to find tvscaler clock source\n");
		goto err2;
	}

	clk_enable(tvscaler_clock);

	
	if((h_clk = clk_get(&pdev->dev, "hclk")) == NULL) {
		__E("failed to find h_clk clock source\n");
		goto err3;
	}

	//init_waitqueue_head(&waitq);
	
	if(request_irq(s3c_tvscaler_irq,
				   s3c_tvscaler_isr,
				   SA_INTERRUPT,
				   "TV_SCALER", NULL)) {
		__E("request_irq(TV_SCALER) failed.\n");
		goto err4;
	}
	
	printk(" Success\n");
	return 0;

  err4:
	clk_put(h_clk);

  err3:
	clk_disable(tvscaler_clock);
	clk_put(tvscaler_clock);
	
  err2:
	iounmap(s3c_tvscaler_base);

  err1:
	release_mem_region(s3c_tvscaler_mem->start,
					   s3c_tvscaler_mem->end - s3c_tvscaler_mem->start + 1);
	
	return -ENOENT;
}

static int s3c_tvscaler_remove(struct platform_device *dev)
{
	__I("s3c_tvscaler_remove called !\n");

	clk_put(h_clk);
	
	clk_disable(tvscaler_clock);
	clk_put(tvscaler_clock);
	
	free_irq(s3c_tvscaler_irq, NULL);

	__D("s3-tvscaler: releasing s3c_tvscaler_mem\n");

	iounmap(s3c_tvscaler_base);
	release_mem_region(s3c_tvscaler_mem->start,
					   s3c_tvscaler_mem->end - s3c_tvscaler_mem->start + 1);

	return 0;
}

static int s3c_tvscaler_suspend(struct platform_device *dev, pm_message_t state)
{
	__D("\n");
	clk_disable(tvscaler_clock);
	return 0;
}

static int s3c_tvscaler_resume(struct platform_device *pdev)
{
	__D("\n");
	clk_enable(tvscaler_clock);
	return 0;
}

static struct platform_driver s3c_tvscaler_driver = {
	.probe          = s3c_tvscaler_probe,
	.remove         = s3c_tvscaler_remove,
	.suspend        = s3c_tvscaler_suspend,
	.resume         = s3c_tvscaler_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-tvscaler",
	},
};

int __init  s3c_tvscaler_pre_init(void)
{
	__D("S3C6410 TV scaler Driver, (c) 2008 Samsung Electronics\n");

	if(platform_driver_register(&s3c_tvscaler_driver) != 0) {
   		__E("platform device register Failed \n");
   		return -1;
  	}
	
	__I(" S3C6410 TV scaler Driver module init OK. \n");

	return 0;
}

void  s3c_tvscaler_exit(void)
{
	__D("\n");
	platform_driver_unregister(&s3c_tvscaler_driver);
 	__I("S3C: tvscaler module exit\n");
}

module_init(s3c_tvscaler_pre_init);
module_exit(s3c_tvscaler_exit);


MODULE_AUTHOR("Peter, Oh");
MODULE_DESCRIPTION("S3C TV Controller Device Driver");
MODULE_LICENSE("GPL");
