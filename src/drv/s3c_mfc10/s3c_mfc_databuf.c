#define LOG_TAG "s3c_mfc_databuf.c"
#include "osal.h"

/* linux/driver/media/video/mfc/s3c_mfc_databuf.c
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
#include "s3c_mfc_types.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_config.h"
#include "s3c_mfc.h"

static volatile unsigned char     *vir_pDATA_BUF = NULL;
static unsigned int                phyDATA_BUF = 0;


BOOL MfcDataBufMemMapping()
{
	__D("\n");

	/* Physical register address mapping */
	phyDATA_BUF = S3C6400_BASEADDR_MFC_DATA_BUF;

	// STREAM BUFFER, FRAME BUFFER  <-- virtual data buffer address mapping
	vir_pDATA_BUF = (typeof(vir_pDATA_BUF))ioremap_cached(phyDATA_BUF, MFC_DATA_BUF_SIZE);
	if (vir_pDATA_BUF == NULL) {
		__E("For DATA_BUF : fail to mapping data buffer\n");
		goto err1;
	}

	__D("VIRTUAL ADDR DATA BUF : vir_pDATA_BUF = 0x%X\n",
		(unsigned int)vir_pDATA_BUF);
	
	return TRUE;

  err1:
	phyDATA_BUF = 0;
	return FALSE;
}

volatile unsigned char *GetDataBufVirAddr()
{   
	__D("\n");
	return vir_pDATA_BUF;
}

volatile unsigned char *s3c_mfc_get_yuvbuff_virt_addr()
{
	__D("\n");
	return (unsigned char *)vir_pDATA_BUF + MFC_STRM_BUF_SIZE;
}

unsigned int s3c_mfc_get_databuf_phys_addr()
{
	__D("\n");
	return phyDATA_BUF;
}

unsigned int s3c_mfc_get_yuvbuff_phys_addr()
{
	__D("\n");
	return (unsigned int)phyDATA_BUF + MFC_STRM_BUF_SIZE;
}
