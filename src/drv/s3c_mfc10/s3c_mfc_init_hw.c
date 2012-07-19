#define LOG_TAG "s3c_mfc_init_hw.c"
#include "osal.h"

/* linux/driver/media/video/mfc/s3c_mfc_init_hw.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 * This source file is for initializing the MFC's H/W setting.
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "s3c_mfc_base.h"
#include "s3c_mfc_sfr.h"
#include "s3c_mfc_bitproc_buf.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_types.h"
#include "s3c_mfc_config.h"
#include "s3c_mfc_yuv_buf_manager.h"
#include "s3c_mfc.h"

BOOL MFC_MemorySetup(void)
{
	BOOL ret_sfr, ret_bit, ret_dat;
	unsigned char *pDataBuf;

   __D("\n");

   ret_sfr = MfcSfrMemMapping();
   if (ret_sfr == FALSE) {
	   __E("fail to mapping MFC Host I/F Registers\n");
	   return FALSE;
	}
   
	__D("MfcSfrMemMapping() OK\n");
	
	ret_bit = MfcBitProcBufMemMapping();
	if (ret_bit == FALSE) {
		__E("fail to mapping bitprocessor buffer memory\n");
		return FALSE;
	}

	__D("MfcBitProcBufMemMapping() OK\n");
	
	ret_dat	= MfcDataBufMemMapping();
	if (ret_dat == FALSE) {
		mfc_err("fail to mapping data buffer memory \n");
		return FALSE;
	}

	__D("MfcDataBufMemMapping() OK\n");
	
	/* FramBufMgr Module Initialization */
	pDataBuf = (unsigned char *)GetDataBufVirAddr();
	FramBufMgrInit(pDataBuf + MFC_STRM_BUF_SIZE, MFC_FRAM_BUF_SIZE);

	return TRUE;
}

BOOL MFC_HW_Init(void)
{   __D("\n");

	// 1. Reset the MFC IP
	MfcReset();

	// 2. Download Firmware code into MFC
	MfcFirmwareIntoCodeBuf();
	MfcFirmwareIntoCodeDownReg();
	__D("downloading firmware into bitprocessor\n");

	// 3. Start Bit Processor
	MfcStartBitProcessor();

	// 4. Set the Base Address Registers for the following 3 buffers (CODE_BUF, WORKING_BUF, PARAMETER_BUF)
	MfcConfigSFR_BITPROC_BUF();

	/* 
	 * 5. Set the Control Registers
	 * 	- STRM_BUF_CTRL
	 * 	- FRME_BUF_CTRL
	 * 	- DEC_FUNC_CTRL
	 * 	- WORK_BUF_CTRL
	 */
	MfcConfigSFR_CTRL_OPTS();

	GetFirmwareVersion();

	return TRUE;
}

