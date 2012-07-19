#define LOG_TAG "s3c_mfc.c"
#include "osal.h"
#include "s3c.h"

/* linux/driver/media/video/mfc/s3c_mfc.c
 *
 * Multi Function Codec - FIMV driver 
 *
 */

#include <asm-arm/arch-s3c2410/regs-s3c6400-clock.h>
#include <linux/pm.h>
#include <asm/plat-s3c24xx/pm.h>

#include "regs-mfc.h"
#include "media.h"

#define S3C_MFC_PHYS_BUFFER_SET

#include "s3c_mfc_base.h"
#include "s3c_mfc_config.h"
#include "s3c_mfc_init_hw.h"
#include "s3c_mfc_instance.h"
#include "s3c_mfc_inst_pool.h"
#include "s3c_mfc.h"
#include "s3c_mfc_yuv_buf_manager.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_sfr.h"
#include "s3c_mfc_intr_noti.h"
#include "s3c_mfc_params.h"

static struct clk	*s3c_mfc_hclk;
static struct clk	*s3c_mfc_sclk;
static struct clk	*s3c_mfc_pclk;

static int s3c_mfc_openhandle_count = 0;

//static struct mutex *mutex = NULL;
static struct mutex mutex;
unsigned int s3c_mfc_intr_type = 0;

#define S3C_MFC_SAVE_START_ADDR 0x100
#define S3C_MFC_SAVE_END_ADDR	0x200

extern int s3c_mfc_get_config_params(s3c_mfc_inst_context_t *pMfcInst, s3c_mfc_args_t *args);
extern int s3c_mfc_set_config_params(s3c_mfc_inst_context_t *pMfcInst, s3c_mfc_args_t *args);

typedef struct _MFC_HANDLE {
	s3c_mfc_inst_context_t *mfc_inst;

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
	unsigned char *pMV;
	unsigned char *pMBType;
#endif
} s3c_mfc_handle_t;


#ifdef CONFIG_S3C6400_PDFW
int s3c_mfc_before_pdoff(void)
{   __D("\n");
	mfc_debug("mfc context saving before pdoff\n");
	return 0;
}

int s3c_mfc_after_pdon(void)
{   __D("\n");
	mfc_debug("mfc initialization after pdon\n");
	return 0;
}

struct pm_devtype s3c_mfc_pmdev = {
	.name         = "mfc",
	.state        = DEV_IDLE,
	.before_pdoff =	s3c_mfc_before_pdoff,
	.after_pdon   = s3c_mfc_after_pdon,
};
#endif


DECLARE_WAIT_QUEUE_HEAD(s3c_mfc_wait_queue);

static struct resource *mfc_mem;
void __iomem *mfc_base;

dma_addr_t s3c_mfc_phys_buffer;

static irqreturn_t s3c_mfc_irq(int irq, void *dev_id)
{
	unsigned int	intReason;
	s3c_mfc_inst_context_t		*pMfcInst;

	pMfcInst = (s3c_mfc_inst_context_t *)dev_id;

	intReason	= s3c_mfc_intr_reason();

	/* if PIC_RUN, buffer full and buffer empty interrupt */
	if (intReason & S3C_MFC_INTR_ENABLE_RESET) {
		s3c_mfc_intr_type = intReason;
		wake_up_interruptible(&s3c_mfc_wait_queue);
	}

	s3c_mfc_clear_intr();

	return IRQ_HANDLED;
}

static int s3c_mfc_open(struct inode *inode, struct file *file)
{  
	s3c_mfc_handle_t		*handle;

	 __D("\n");

	mutex_lock(&mutex);

	clk_enable(s3c_mfc_hclk);
	clk_enable(s3c_mfc_sclk);
	clk_enable(s3c_mfc_pclk);

	s3c_mfc_openhandle_count++;
	if (s3c_mfc_openhandle_count == 1) {
		if (MFC_HW_Init() == FALSE) 
			return -ENODEV;	
	}


	handle = (typeof(handle))kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle) {
		__E("mfc open error\n");
		goto err1;
	}

	/* 
	 * MFC Instance creation
	 */
	handle->mfc_inst = s3c_mfc_inst_create();
	if (handle->mfc_inst == NULL) {
		__E("fail to mfc instance allocation\n");
		goto err2;
	}

	/*
	 * MFC supports multi-instance. so each instance have own data structure
	 * It saves file->private_data
	 */
	file->private_data = (void *)handle;
	mutex_unlock(&mutex);
	__D("mfc open success\n");
	return 0;

  err2:
	kfree(handle);
  err1:
	mutex_unlock(&mutex);
	return -EPERM;
}


static int s3c_mfc_release(struct inode *inode, struct file *file)
{   
	s3c_mfc_handle_t *handle = NULL;

	__D("\n");
	
	mutex_lock(&mutex);

	handle = (typeof(handle))file->private_data;
	if (handle->mfc_inst == NULL) {
		mutex_unlock(&mutex);
		return -EPERM;
	};

	__D("deleting instance number = %d\n", handle->mfc_inst->inst_no);

	s3c_mfc_inst_del(handle->mfc_inst);

	s3c_mfc_openhandle_count--;
	if (s3c_mfc_openhandle_count == 0) {
		clk_disable(s3c_mfc_hclk);
		clk_disable(s3c_mfc_sclk);
		clk_disable(s3c_mfc_pclk);		
	}

	kfree(file->private_data);
	file->private_data = NULL;
	mutex_unlock(&mutex);

	return 0;
}


static ssize_t s3c_mfc_write(struct file *file, const char *buf, size_t count, loff_t *pos)
{   __D("\n");
	return 0;
}

static ssize_t s3c_mfc_read(struct file *file, char *buf, size_t count, loff_t *pos)
{   __D("\n");	
	return 0;
}

static int s3c_mfc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int buf_size;
	int nStrmLen, nHdrLen;
	int yuv_size;
	int size;
	
	void		*temp;
	unsigned int	vir_mv_addr;
	unsigned int	vir_mb_type_addr;
	unsigned int	tmp;
	unsigned int	in_usr_data, yuv_buffer, run_index, out_buf_size, databuf_vaddr, offset;
	unsigned int	 databuf_paddr;//yuv_buff_cnt
	unsigned char	*OutBuf	= NULL;
	unsigned char	*start;//, *end;
	
	s3c_mfc_inst_context_t	*pMfcInst;
	s3c_mfc_handle_t		*handle;
	s3c_mfc_codec_mode_t	codec_mode = 0;
	s3c_mfc_args_t		args;
	s3c_mfc_enc_info_t		enc_info;

	__D("\n");

	/* 
	 * Parameter Check
	 */
	handle = (s3c_mfc_handle_t *)file->private_data;
	if (handle->mfc_inst == NULL) {
		return -EFAULT;
	}

	pMfcInst = handle->mfc_inst;

	switch (cmd) {
	case IOCTL_MFC_MPEG4_ENC_INIT:
	case IOCTL_MFC_H264_ENC_INIT:
	case IOCTL_MFC_H263_ENC_INIT:
		mutex_lock(&mutex);

		mfc_debug("cmd = %d\n", cmd);

		if(copy_from_user(&args.enc_init, (s3c_mfc_enc_init_arg_t *)arg, sizeof(s3c_mfc_enc_init_arg_t))) {
			return -EFAULT;
		}

		if ( cmd == IOCTL_MFC_MPEG4_ENC_INIT )
			codec_mode = MP4_ENC;
		else if ( cmd == IOCTL_MFC_H264_ENC_INIT )
			codec_mode = AVC_ENC;
		else if ( cmd == IOCTL_MFC_H263_ENC_INIT )
			codec_mode = H263_ENC;

		/* 
		 * Initialize MFC Instance
		 */
		enc_info.width			= args.enc_init.in_width;
		enc_info.height			= args.enc_init.in_height;
		enc_info.bitrate		= args.enc_init.in_bitrate;
		enc_info.gop_number		= args.enc_init.in_gopNum;
		enc_info.frame_rate_residual	= args.enc_init.in_frameRateRes;
		enc_info.frame_rate_division	= args.enc_init.in_frameRateDiv;


		ret = s3c_mfc_instance_init_enc(pMfcInst, codec_mode, &enc_info);

		args.enc_init.ret_code = ret;
		if(copy_to_user((s3c_mfc_enc_init_arg_t *)arg, &args.enc_init, sizeof(s3c_mfc_enc_init_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_MPEG4_ENC_EXE:
	case IOCTL_MFC_H264_ENC_EXE:
	case IOCTL_MFC_H263_ENC_EXE:
		mutex_lock(&mutex);

		if(copy_from_user(&args.enc_exe, (s3c_mfc_enc_exe_arg_t *)arg, sizeof(s3c_mfc_enc_exe_arg_t))) {
			return -EFAULT; 
		}

		tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;

		start = pMfcInst->yuv_buffer;
		size = tmp * pMfcInst->yuv_buffer_count; 
		dma_cache_maint(start, size, DMA_TO_DEVICE);

		/* 
		 * Encode MFC Instance
		 */
		ret = s3c_mfc_inst_enc(pMfcInst, &nStrmLen, &nHdrLen);

		start = pMfcInst->stream_buffer;
		size = pMfcInst->stream_buffer_size;
		dma_cache_maint(start, size, DMA_FROM_DEVICE);

		args.enc_exe.ret_code	= ret;
		if (ret == S3C_MFC_INST_RET_OK) {
			args.enc_exe.out_encoded_size = nStrmLen;
			args.enc_exe.out_header_size  = nHdrLen;
		}
		
		if(copy_to_user((s3c_mfc_enc_exe_arg_t *)arg, &args.enc_exe, sizeof(s3c_mfc_enc_exe_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_MPEG4_DEC_INIT:
	case IOCTL_MFC_H264_DEC_INIT:
	case IOCTL_MFC_H263_DEC_INIT:
	case IOCTL_MFC_VC1_DEC_INIT:
		mutex_lock(&mutex);

		if(copy_from_user(&args.dec_init, (s3c_mfc_dec_init_arg_t *)arg, sizeof(s3c_mfc_dec_init_arg_t))) {
			return -EFAULT;
		}

		if ( cmd == IOCTL_MFC_MPEG4_DEC_INIT )
			codec_mode = MP4_DEC;
		else if ( cmd == IOCTL_MFC_H264_DEC_INIT )
			codec_mode = AVC_DEC;
		else if ( cmd == IOCTL_MFC_H263_DEC_INIT)
			codec_mode = H263_DEC;
		else {
			codec_mode = VC1_DEC;
		}

		/* 
		 * Initialize MFC Instance
		 */
		ret = s3c_mfc_inst_init_dec(pMfcInst, codec_mode, 
						args.dec_init.in_strmSize);

		args.dec_init.ret_code	= ret;
		if (ret == S3C_MFC_INST_RET_OK) {
			args.dec_init.out_width	     = pMfcInst->width;
			args.dec_init.out_height     = pMfcInst->height;
			args.dec_init.out_buf_width  = pMfcInst->buf_width;
			args.dec_init.out_buf_height = pMfcInst->buf_height;
		}
		if(copy_to_user((s3c_mfc_dec_init_arg_t *)arg, &args.dec_init, sizeof(s3c_mfc_dec_init_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_MPEG4_DEC_EXE:
	case IOCTL_MFC_H264_DEC_EXE:
	case IOCTL_MFC_H263_DEC_EXE:
	case IOCTL_MFC_VC1_DEC_EXE:
		mutex_lock(&mutex);

		if(copy_from_user(&args.dec_exe, (s3c_mfc_dec_exe_arg_t *)arg, sizeof(s3c_mfc_dec_exe_arg_t))) {
			return -EFAULT;
		}

		tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;

		start = pMfcInst->stream_buffer;
		size = pMfcInst->stream_buffer_size;
		dma_cache_maint(start, size, DMA_TO_DEVICE);

		ret = s3c_mfc_inst_dec(pMfcInst, args.dec_exe.in_strmSize);

		start = pMfcInst->yuv_buffer;
		size = tmp * pMfcInst->yuv_buffer_count;
		dma_cache_maint(start, size, DMA_FROM_DEVICE);	

		args.dec_exe.ret_code = ret;
		if(copy_to_user((s3c_mfc_dec_exe_arg_t *)arg, &args.dec_exe, sizeof(s3c_mfc_dec_exe_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_GET_LINE_BUF_ADDR:
		mutex_lock(&mutex);

		if(copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT;
		}

		ret = s3c_mfc_inst_get_line_buff(pMfcInst, &OutBuf, &buf_size);

		args.get_buf_addr.out_buf_size	= buf_size;
		args.get_buf_addr.out_buf_addr	= args.get_buf_addr.in_usr_data + (OutBuf - GetDataBufVirAddr());
		args.get_buf_addr.ret_code	= ret;

		if(copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT; 
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_GET_YUV_BUF_ADDR:
		mutex_lock(&mutex);

		if(copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT;
		}

		if (pMfcInst->yuv_buffer == NULL) {
			__E("mfc frame buffer is not internally allocated yet\n");
			mutex_unlock(&mutex);
			return -EFAULT;
		}

		/* FRAM_BUF address is calculated differently for Encoder and Decoder. */
		switch (pMfcInst->codec_mode) {
		case MP4_DEC:
		case AVC_DEC:
		case VC1_DEC:
		case H263_DEC:
			/* Decoder case */
			yuv_size = (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
			args.get_buf_addr.out_buf_size = yuv_size;

			in_usr_data = (unsigned int)args.get_buf_addr.in_usr_data;
			yuv_buffer = (unsigned int)pMfcInst->yuv_buffer;
			run_index = pMfcInst->run_index;
			out_buf_size = args.get_buf_addr.out_buf_size;
			databuf_vaddr = (unsigned int)GetDataBufVirAddr();
			offset = yuv_buffer + run_index * out_buf_size - databuf_vaddr;
			
#if (S3C_MFC_ROTATE_ENABLE == 1)
			if ((pMfcInst->codec_mode != VC1_DEC) && 
				(pMfcInst->post_rotation_mode & 0x0010)) {
				yuv_buff_cnt = pMfcInst->yuv_buffer_count;
				offset = yuv_buffer + yuv_buff_cnt * out_buf_size - databuf_vaddr;
			}
#endif
			args.get_buf_addr.out_buf_addr = in_usr_data + offset;
			break;

		case MP4_ENC:
		case AVC_ENC:
		case H263_ENC:
			/* Encoder case */
			yuv_size = (pMfcInst->width * pMfcInst->height * 3) >> 1;
			in_usr_data = args.get_buf_addr.in_usr_data;
			run_index = pMfcInst->run_index;
			yuv_buffer = (unsigned int)pMfcInst->yuv_buffer;
			databuf_vaddr = (unsigned int)GetDataBufVirAddr();
			offset = run_index * yuv_size + (yuv_buffer - databuf_vaddr);
			
			args.get_buf_addr.out_buf_addr = in_usr_data + offset;			
			break;
		} /* end of switch (codec_mode) */

		args.get_buf_addr.ret_code = S3C_MFC_INST_RET_OK;
		if(copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_GET_PHY_FRAM_BUF_ADDR:
		mutex_lock(&mutex);

		if(copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT;
		}

		yuv_size = (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
		args.get_buf_addr.out_buf_size = yuv_size;
		yuv_buffer = (unsigned int)pMfcInst->yuv_buffer;
		run_index = pMfcInst->run_index;
		out_buf_size = args.get_buf_addr.out_buf_size;
		databuf_vaddr = (unsigned int)GetDataBufVirAddr();
		databuf_paddr = (unsigned int)S3C6400_BASEADDR_MFC_DATA_BUF;
		offset = yuv_buffer + run_index * out_buf_size - databuf_vaddr;		
		
#if (S3C_MFC_ROTATE_ENABLE == 1)
		if ((pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->post_rotation_mode & 0x0010)) {
			yuv_buff_cnt = pMfcInst->yuv_buffer_count;
			offset = yuv_buffer + yuv_buff_cnt * out_buf_size - databuf_vaddr;
		}
#endif
		args.get_buf_addr.out_buf_addr = databuf_paddr + offset;
		args.get_buf_addr.ret_code = S3C_MFC_INST_RET_OK;

		if(copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_GET_MPEG4_ASP_PARAM:
#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))

		if(copy_from_user(&args.mpeg4_asp_param, (s3c_mfc_get_mpeg4asp_arg_t *)arg, sizeof(s3c_mfc_get_mpeg4asp_arg_t))) {
			return -EFAULT;
		}

		ret = S3C_MFC_INST_RET_OK;
		args.mpeg4_asp_param.ret_code = S3C_MFC_INST_RET_OK;
		args.mpeg4_asp_param.mp4asp_vop_time_res = pMfcInst->RET_DEC_SEQ_INIT_BAK_MP4ASP_VOP_TIME_RES;
		args.mpeg4_asp_param.byte_consumed = pMfcInst->RET_DEC_PIC_RUN_BAK_BYTE_CONSUMED;
		args.mpeg4_asp_param.mp4asp_fcode = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_FCODE;
		args.mpeg4_asp_param.mp4asp_time_base_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_TIME_BASE_LAST;
		args.mpeg4_asp_param.mp4asp_nonb_time_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_NONB_TIME_LAST;
		args.mpeg4_asp_param.mp4asp_trd = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_MP4ASP_TRD;

		args.mpeg4_asp_param.mv_addr = (args.mpeg4_asp_param.in_usr_mapped_addr + MFC_STRM_BUF_SIZE)
			+ (pMfcInst->mv_mbyte_addr - pMfcInst->phys_addr_yuv_buffer);
		args.mpeg4_asp_param.mb_type_addr = args.mpeg4_asp_param.mv_addr + S3C_MFC_MAX_MV_SIZE;	
		args.mpeg4_asp_param.mv_size = S3C_MFC_MAX_MV_SIZE;
		args.mpeg4_asp_param.mb_type_size = S3C_MFC_MAX_MBYTE_SIZE;

		vir_mv_addr = (unsigned int)((pMfcInst->stream_buffer + MFC_STRM_BUF_SIZE) + \
					(pMfcInst->mv_mbyte_addr - pMfcInst->phys_addr_yuv_buffer));
		vir_mb_type_addr = vir_mv_addr + S3C_MFC_MAX_MV_SIZE;

		if(copy_to_user((s3c_mfc_get_mpeg4asp_arg_t *)arg, &args.mpeg4_asp_param, sizeof(s3c_mfc_get_mpeg4asp_arg_t))) {
			return -EFAULT;
		}
#endif	
		break;

	case IOCTL_MFC_GET_CONFIG:
		mutex_lock(&mutex);

		if(copy_from_user(&args, (s3c_mfc_args_t *)arg, sizeof(s3c_mfc_args_t))) {
			return -EFAULT;
		}

		ret = s3c_mfc_get_config_params(pMfcInst, &args);

		if(copy_to_user((s3c_mfc_args_t *)arg, &args, sizeof(s3c_mfc_args_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_MFC_SET_CONFIG:
		mutex_lock(&mutex);

		if(copy_from_user(&args, (s3c_mfc_args_t *)arg, sizeof(s3c_mfc_args_t))) {
			return -EFAULT; 
		}

		ret = s3c_mfc_set_config_params(pMfcInst, &args);

		if(copy_to_user((s3c_mfc_args_t *)arg, &args, sizeof(s3c_mfc_args_t))) {
			return -EFAULT;
		}

		mutex_unlock(&mutex);
		break;

	case IOCTL_VIRT_TO_PHYS:
		temp = __virt_to_phys((void *)arg);
		return (int)temp;
		break;

	default:
		mutex_lock(&mutex);
		mfc_debug("requested ioctl command is not defined (ioctl cmd = 0x%x)\n", cmd);
		mutex_unlock(&mutex);
		return -ENOIOCTLCMD;
	}

	switch (ret) {
	case S3C_MFC_INST_RET_OK:
		return 0;
	default:
		return -EPERM;
	}
	return -EPERM;
}

int s3c_mfc_mmap(struct file *filp, struct vm_area_struct *vma)
{   
	unsigned long size	= vma->vm_end - vma->vm_start;
	unsigned long maxSize;
	unsigned long pageFrameNo;

	__D("\n");
	
	pageFrameNo = __phys_to_pfn(S3C6400_BASEADDR_MFC_DATA_BUF);

	maxSize = MFC_DATA_BUF_SIZE + PAGE_SIZE - (MFC_DATA_BUF_SIZE % PAGE_SIZE);

	if (size > maxSize) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED | VM_IO;

	/* nocached setup.
	 * vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	 */

	if (remap_pfn_range(vma, vma->vm_start, pageFrameNo, size, vma->vm_page_prot)) {
		__E("fail to remap\n");
		return -EAGAIN;
	}

	return 0;
}


static struct file_operations s3c_mfc_fops = {
	owner:		THIS_MODULE,
	open:		s3c_mfc_open,
	release:	s3c_mfc_release,
	ioctl:		s3c_mfc_ioctl,
	read:		s3c_mfc_read,
	write:		s3c_mfc_write,
	mmap:		s3c_mfc_mmap,
};


static struct miscdevice s3c_mfc_miscdev = {
	minor:		252, 		
	name:		"s3c-mfc",
	fops:		&s3c_mfc_fops
};


static int s3c_mfc_probe(struct platform_device *pdev)
{  
	int	size;
	int	ret;
	struct resource *res;	
	unsigned int mfc_clk;

	 __D("\n");
	 
	/* mfc clock enable  */
	s3c_mfc_hclk = clk_get(NULL, "hclk_mfc");
	if (s3c_mfc_hclk == NULL) {
		__E("failed to get mfc hclk source\n");
		return -ENOENT;
	}	
	clk_enable(s3c_mfc_hclk);

	s3c_mfc_sclk = clk_get(NULL, "sclk_mfc");
	if (s3c_mfc_sclk == NULL) {
		__E("failed to get mfc sclk source\n");
		return -ENOENT;
	}
	clk_enable(s3c_mfc_sclk);

	s3c_mfc_pclk = clk_get(NULL, "pclk_mfc");
	if (s3c_mfc_pclk == NULL) {
		__E("failed to get mfc pclk source\n");
		return -ENOENT;
	}
	clk_enable(s3c_mfc_pclk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		__E("failed to get memory region resouce\n");
		return -ENOENT;
	}

	size = (res->end-res->start)+1;
	mfc_mem = request_mem_region(res->start, size, pdev->name);
	if (mfc_mem == NULL) {
		__E("failed to get memory region\n");
		return -ENOENT;
	}

	__D("mfc_mem = 0x%X\n", (unsigned int)mfc_mem);

	mfc_base = ioremap(res->start, size);
	if (mfc_base == NULL) {
		__E("failed to ioremap() region\n");
		return -EINVAL;
	}

	__D("mfc_base = 0x%X\n", (unsigned int)mfc_base);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		__E("failed to get irq resource\n");
		return -ENOENT;
	}

	ret = request_irq(res->start, (irq_handler_t)s3c_mfc_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		__E("failed to install irq (%d)\n", ret);
		return ret;
	}

	mutex_init(&mutex);

	/* mfc clock set 133 Mhz */
	mfc_clk = readl(S3C_CLK_DIV0);
	mfc_clk |= (1<<28);
	__raw_writel(mfc_clk, S3C_CLK_DIV0);

	
	/* 2. MFC Memory Setup */
	if (MFC_MemorySetup() == FALSE) {
		__D("s3c_mfc_setup_memory() error\n");
		return -ENOMEM;
	}
	
	/* 3. MFC Hardware Initialization */
	if (MFC_HW_Init() == FALSE)
		return -ENODEV;

	ret = misc_register(&s3c_mfc_miscdev);

	clk_disable(s3c_mfc_hclk);
	clk_disable(s3c_mfc_sclk);
	clk_disable(s3c_mfc_pclk);

	return 0;
}

static int s3c_mfc_remove(struct platform_device *dev)
{   __D("\n");
	if (mfc_mem != NULL) {
		release_mem_region(mfc_mem->start,
						   mfc_mem->end - mfc_mem->start + 1);
		mfc_mem = NULL;
	}

	free_irq(IRQ_MFC, dev);

	misc_deregister(&s3c_mfc_miscdev);
	return 0;
}

static int s3c_mfc_suspend(struct platform_device *dev, pm_message_t state)
{
	__D("\n");
	
	return 0;
}

static int s3c_mfc_resume(struct platform_device *pdev)
{
	__D("\n");

	return 0;
}

static void s3c_mfc_shutdown(struct platform_device *pdev)
{
	__D("\n");
}

static struct platform_driver s3c_mfc_driver = {
	.probe		= s3c_mfc_probe,
	.remove		= s3c_mfc_remove,
	.shutdown	= s3c_mfc_shutdown,
	.suspend	= s3c_mfc_suspend,
	.resume		= s3c_mfc_resume,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "s3c-mfc",
	},
};



static int __init s3c_mfc_init(void)
{   __D("\n");
	
	if (platform_driver_register(&s3c_mfc_driver) != 0) {
		__E("fail to register platform device\n");
		return -EPERM;
	}

	__I("S3C6400 MFC driver module init OK.\n");

	return 0;
}

static void __exit s3c_mfc_exit(void)
{
	__D("\n");
	
	mutex_destroy(&mutex);

	platform_driver_unregister(&s3c_mfc_driver);
	
	__D("S3C64XX MFC driver exit.\n");
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");

