#define LOG_TAG "s3c_mfc_sfr.c"
#include "osal.h"
#include "s3c.h"

/* linux/driver/media/video/mfc/s3c_mfc_sfr.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 * This source file is for setting the MFC's registers.
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include "regs-mfc.h"

#include "s3c_mfc_sfr.h"
#include "s3c_mfc_config.h"
#include "prism_s.h"
#include "s3c_mfc_intr_noti.h"
#include "s3c_mfc_base.h"

extern wait_queue_head_t	             s3c_mfc_wait_queue;
extern unsigned int		                 s3c_mfc_intr_type;

static volatile S3C6400_MFC_SFR __iomem *vir_pMFC_SFR = NULL;
static volatile unsigned int __iomem    *vir_pSW_RESET;

static unsigned int phyMFC_SFR  = 0;
static unsigned int phySW_RESET = 0;

BOOL MfcSfrMemMapping()
{
	__D("\n");

	/* Physical register address mapping */
	phyMFC_SFR  = S3C6400_BASEADDR_MFC_SFR;
	phySW_RESET = S3C6400_BASEADDR_MFC_SFR + S3C6400_MFC_SFR_SIZE;

	// virtual address mapping
	vir_pMFC_SFR = (typeof(vir_pMFC_SFR))ioremap_nocache(phyMFC_SFR, S3C6400_MFC_SFR_SIZE);
	if (vir_pMFC_SFR == NULL)
	{
		__E("fail to mapping sfr MFC Host I/F Registers\n");
		goto err1;
	}

	__D("VIRTUAL ADDR MFC SFR : vir_pMFC_SFR = 0x%X\n", (unsigned int)vir_pMFC_SFR);
	
	vir_pSW_RESET = (typeof(vir_pSW_RESET))((int)vir_pMFC_SFR + S3C6400_MFC_SFR_SIZE);
	
	__D("vir_pMFC_SFR = 0x%X\n", (unsigned int)vir_pSW_RESET);
	__D("phyMFC_SFR = 0x%X\n",   (unsigned int)phyMFC_SFR);
	__D("phySW_RESET = 0x%X\n",  (unsigned int)phySW_RESET);
	
	return TRUE;

  err1:
	phyMFC_SFR  = 0;
	phySW_RESET = 0;
	return FALSE;
}

volatile void* s3c_mfc_get_sfr_virt_addr()
{
	__D("\n");
	return vir_pMFC_SFR;
}

int s3c_mfc_sleep()
{
	/* Wait until finish executing command. */
	while (readl(vir_pMFC_SFR + S3C_MFC_BUSY_FLAG) != 0)
		udelay(1);

	/* Issue Sleep Command. */
	writel(0x01, vir_pMFC_SFR + S3C_MFC_BUSY_FLAG);
	writel(0x0A, vir_pMFC_SFR + S3C_MFC_RUN_CMD);

	while (readl(vir_pMFC_SFR + S3C_MFC_BUSY_FLAG) != 0)
		udelay(1);

	return 1;
}

int s3c_mfc_wakeup()
{
	/* Bit processor gets started. */
	writel(0x01, vir_pMFC_SFR + S3C_MFC_BUSY_FLAG);
	writel(0x01, vir_pMFC_SFR + S3C_MFC_CODE_RUN);

	while (readl(vir_pMFC_SFR + S3C_MFC_BUSY_FLAG) != 0)
		udelay(1);

	/* Bit processor wakes up. */
	writel(0x01, vir_pMFC_SFR + S3C_MFC_BUSY_FLAG);
	writel(0x0B, vir_pMFC_SFR + S3C_MFC_RUN_CMD);

	while (readl(vir_pMFC_SFR + S3C_MFC_BUSY_FLAG) != 0)
		udelay(1);

	return 1;
}


static char *s3c_mfc_get_cmd_string(MFC_COMMAND mfc_cmd)
{
	switch ((int)mfc_cmd) {
	case SEQ_INIT:
		return "SEQ_INIT";

	case SEQ_END:
		return "SEQ_END";

	case PIC_RUN:
		return "PIC_RUN";

	case SET_FRAME_BUF:
		return "SET_FRAME_BUF";

	case ENC_HEADER:
		return "ENC_HEADER";

	case ENC_PARA_SET:
		return "ENC_PARA_SET";

	case DEC_PARA_SET:
		return "DEC_PARA_SET";

	case GET_FW_VER:
		return "GET_FW_VER";

	}

	return "UNDEF CMD";
}

static int WaitForReady(void)
{
	int   i;

	__D("\n");

#ifdef _WIN32_WCE
	for (i=0; i<15; i++) {
		if (vir_pMFC_SFR->BusyFlag == 0) {
			return TRUE;
		}
		Sleep(2);
	}
#else
	for (i = 0; i < 1000; i++) {
		if (vir_pMFC_SFR->BusyFlag == 0) {
			return TRUE;
		}
		//udelay(100);	/* 1/1000 second */
		mdelay(1);
	}
#endif
	__D("timeout in waiting for the bitprocessor available\n");

	return FALSE;
}


int GetFirmwareVersion(void)
{
	unsigned int prd_no, ver_no;

	__D("\n");
	
	WaitForReady();

	vir_pMFC_SFR->RunCommand = GET_FW_VER;

	__D("GET_FW_VER command was issued\n");

	WaitForReady();

	prd_no = vir_pMFC_SFR->param.dec_seq_init.RET_DEC_SEQ_SUCCESS >> 16;

	ver_no = vir_pMFC_SFR->param.dec_seq_init.RET_DEC_SEQ_SUCCESS & 0x00FFFF;

	__D("GET_FW_VER => 0x%x, 0x%x\n", prd_no, ver_no);
	__D("BUSY_FLAG => %d\n", 					\
		readl(vir_pMFC_SFR + S3C_MFC_BUSY_FLAG));

	return vir_pMFC_SFR->param.dec_seq_init.RET_DEC_SEQ_SUCCESS;
}


BOOL s3c_mfc_issue_command(int inst_no, s3c_mfc_codec_mode_t codec_mode, MFC_COMMAND mfc_cmd)
{
	unsigned int intr_reason;

	writel(inst_no, vir_pMFC_SFR + S3C_MFC_RUN_INDEX);

	if (codec_mode == H263_DEC) {
		writel(MP4_DEC, vir_pMFC_SFR + S3C_MFC_RUN_COD_STD);
	} else if (codec_mode == H263_ENC) {
		writel(MP4_ENC, vir_pMFC_SFR + S3C_MFC_RUN_COD_STD);
	} else {
		writel(codec_mode, vir_pMFC_SFR + S3C_MFC_RUN_COD_STD);
	}

	switch (mfc_cmd) {
	case PIC_RUN:
	case SEQ_INIT:
	case SEQ_END:
		writel(mfc_cmd, vir_pMFC_SFR + S3C_MFC_RUN_CMD);

		if(interruptible_sleep_on_timeout(&s3c_mfc_wait_queue, 500) == 0) {
			s3c_mfc_stream_end();
			return FALSE; 
		}

		intr_reason = s3c_mfc_intr_type;

		if (intr_reason == S3C_MFC_INTR_REASON_INTRNOTI_TIMEOUT) {
			__E("command = %s, WaitInterruptNotification returns TIMEOUT\n", \
								s3c_mfc_get_cmd_string(mfc_cmd));
			return FALSE;
		}
		if (intr_reason & S3C_MFC_INTR_REASON_BUFFER_EMPTY) {
			__E("command = %s, BUFFER EMPTY interrupt was raised\n", s3c_mfc_get_cmd_string(mfc_cmd));
			return FALSE;
		}
		break;

	default:
		if (WaitForReady() == FALSE) {
			__E("command = %s, bitprocessor is busy before issuing the command\n",	\
									s3c_mfc_get_cmd_string(mfc_cmd));
			return FALSE;
		}

		writel(mfc_cmd, vir_pMFC_SFR + S3C_MFC_RUN_CMD);
		WaitForReady();

	} 

	return TRUE;
}

/* Perform the SW_RESET */
void MfcReset(void)
{
	*vir_pSW_RESET = 0x00;
	*vir_pSW_RESET = 0x01;

	/* Interrupt is enabled for PIC_RUN command and empty/full STRM_BUF status. */
	vir_pMFC_SFR->IntEnable   = 0xC00E;
	vir_pMFC_SFR->IntReason   = 0x0;
	vir_pMFC_SFR->BitIntClear = 0x1;
}

/* 
 * Clear the MFC Interrupt
 * After catching the MFC Interrupt,
 * it is required to call this functions for clearing the interrupt-related register.
 */
void s3c_mfc_clear_intr(void)
{
	writel(0x1, vir_pMFC_SFR + S3C_MFC_BITS_INT_CLEAR);
	writel(S3C_MFC_INTR_REASON_NULL, vir_pMFC_SFR + S3C_MFC_INT_REASON);
}

/* Check INT_REASON register of MFC (the interrupt reason register) */
unsigned int s3c_mfc_intr_reason(void)
{
	return readl(vir_pMFC_SFR + S3C_MFC_INT_REASON);
}

/* 
 * Set the MFC's SFR of DEC_FUNC_CTRL to 1.
 * It means that the data will not be added more to the STRM_BUF.
 * It is required in RING_BUF mode (VC-1 DEC).
 */
void s3c_mfc_set_eos(int buffer_mode)
{
	if (buffer_mode == 1) {
		writel(1 << 1, vir_pMFC_SFR + S3C_MFC_DEC_FUNC_CTRL);
	} else {
		writel(1, vir_pMFC_SFR + S3C_MFC_DEC_FUNC_CTRL);
	}
}

void s3c_mfc_stream_end()
{
	writel(0, vir_pMFC_SFR + S3C_MFC_DEC_FUNC_CTRL);
}

void MfcFirmwareIntoCodeDownReg(void)
{
	unsigned int  i;
	unsigned int  data;

	__D("\n");
	
	/* Download the Boot code into MFC's internal memory */
	for (i = 0; i < 512; i++) {
		data = bit_code[i];
		//vir_pMFC_SFR->CodeDownLoad = ((i<<16) | data);
		vir_pMFC_SFR->CodeDownLoad.CodeAddr = i;
		vir_pMFC_SFR->CodeDownLoad.CodeData = data;
	}
}

void MfcStartBitProcessor()
{
	__D("\n");
	vir_pMFC_SFR->CodeRun = 0x01;
}

void s3c_mfc_stop_bit_processor(void)
{
	writel(0x00, vir_pMFC_SFR + S3C_MFC_CODE_RUN);
}

void MfcConfigSFR_BITPROC_BUF(void)
{
	__D("\n");
	
	/* 
	 * CODE BUFFER ADDRESS (BASE + 0x100)
	 * 	: Located from the Base address of the BIT PROCESSOR'S Firmware code segment
	 */
	vir_pMFC_SFR->CodeBufAddr = S3C6400_BASEADDR_MFC_BITPROC_BUF;

	/* 
	 * WORKING BUFFER ADDRESS (BASE + 0x104)
	 * 	: Located from the next to the BIT PROCESSOR'S Firmware code segment
	 */
	vir_pMFC_SFR->WorkBufAddr = vir_pMFC_SFR->CodeBufAddr + MFC_CODE_BUF_SIZE;


	/* PARAMETER BUFFER ADDRESS (BASE + 0x108)
	 * 	: Located from the next to the WORKING BUFFER
	 */
	vir_pMFC_SFR->ParaBufAddr = vir_pMFC_SFR->WorkBufAddr + MFC_WORK_BUF_SIZE;
}

void MfcConfigSFR_CTRL_OPTS()
{
	/* BIT STREAM BUFFER CONTROL (BASE + 0x10C) */
	vir_pMFC_SFR->BitStreamCtrl.SelBigEndian   = 0;
	vir_pMFC_SFR->BitStreamCtrl.BufStsCheckDis = 0;

	
	/* FRAME MEMORY CONTROL  (BASE + 0x110) */
	vir_pMFC_SFR->FrameMemCtrl = 0;

	/* DECODER FUNCTION CONTROL (BASE + 0x114) */
	vir_pMFC_SFR->DecFuncCtrl = 0;  // 0: Whole stream is not in buffer

	/* WORK BUFFER CONTROL (BASE + 0x11C) */
	vir_pMFC_SFR->BitWorkBufCtrl = 0;  // 0: Work buffer control is disabled
}

EXPORT_SYMBOL(vir_pMFC_SFR);
