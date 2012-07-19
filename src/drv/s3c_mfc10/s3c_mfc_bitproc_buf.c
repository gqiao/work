#define LOG_TAG "s3c_mfc_bitproc_buf.c"
#include "osal.h"

/* linux/driver/media/video/mfc/s3c_mfc_bitproc_buf.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/io.h>
#include <linux/kernel.h>

#include "s3c_mfc_base.h"
#include "s3c_mfc_bitproc_buf.h"
#include "s3c_mfc_config.h"
#include "prism_s.h"
#include "s3c_mfc.h"

static volatile unsigned char     *vir_pBITPROC_BUF  = NULL;
static unsigned int                phyBITPROC_BUF  = 0;


BOOL MfcBitProcBufMemMapping()
{   
	__D("\n");

	/* Physical register address mapping */
	phyBITPROC_BUF = S3C6400_BASEADDR_MFC_BITPROC_BUF;
	
	// FIRWARE/WORKING/PARAMETER BUFFER  <-- virtual bitprocessor buffer address mapping
	vir_pBITPROC_BUF = (typeof(vir_pBITPROC_BUF))ioremap_nocache(S3C6400_BASEADDR_MFC_BITPROC_BUF, MFC_BITPROC_BUF_SIZE);
	if (vir_pBITPROC_BUF == NULL) {
		__E("For BITPROC_BUF : fail to mapping bitprocessor buffer\n");
		goto err1;
	}	

	return TRUE;
	
  err1:
	phyBITPROC_BUF = 0;
	return FALSE;
}

volatile unsigned char *s3c_mfc_get_bitproc_buff_virt_addr()
{
	__D("\n");
	return vir_pBITPROC_BUF;
}

unsigned char *s3c_mfc_get_param_buff_virt_addr()
{
	__D("\n");
	return (unsigned char *)(vir_pBITPROC_BUF + MFC_CODE_BUF_SIZE + MFC_WORK_BUF_SIZE);
}

void MfcFirmwareIntoCodeBuf()
{
	unsigned int  i, j;
	unsigned int  data;

	unsigned int *uAddrFirmwareCode;

	__D("\n");

	uAddrFirmwareCode = (unsigned int *)vir_pBITPROC_BUF;

	/* 
	 * Putting the Boot & Firmware code into SDRAM 
	 * Boot code(1KB) + Codec Firmware (79KB)
	 *
	 */
	for (i = j = 0 ; i < sizeof(bit_code) / sizeof(bit_code[0]); i += 2, j++) {
		data = (bit_code[i] << 16) | bit_code[i + 1];

		*(uAddrFirmwareCode + j) = data;
	}
}
