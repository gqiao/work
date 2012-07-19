#define LOG_TAG "s3c_mfc_inst_pool.c"
#include "osal.h"

/* linux/driver/media/video/mfc/s3c_mfc_inst_pool.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 * This file stores information about different instancesof MFC.
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "s3c_mfc_config.h"
#include "s3c_mfc_inst_pool.h"

#if !defined(MFC_NUM_INSTANCES_MAX)
#error "MFC_NUM_INSTANCES_MAX should be defined."
#endif
#if ((MFC_NUM_INSTANCES_MAX <= 0) || (MFC_NUM_INSTANCES_MAX > 8))
#error "MFC_NUM_INSTANCES_MAX should be in the range of 1 ~ 8."
#endif

static int s3c_mfc_inst_no = 0;
static int s3c_mfc_inst_status[MFC_NUM_INSTANCES_MAX] = {0, };
static int s3c_mfc_num_inst_avail = MFC_NUM_INSTANCES_MAX;

int s3c_mfc_get_avail_inst_pool_num(void)
{   __D("\n");
	return s3c_mfc_num_inst_avail;
}

int s3c_mfc_occupy_inst_pool(void)
{   __D("\n");
	int  i;

	if (s3c_mfc_num_inst_avail == 0)
		return -1;

	for (i = 0; i < ARRAY_SIZE(s3c_mfc_inst_status); i++) {
		if (s3c_mfc_inst_status[s3c_mfc_inst_no] == 0) {
			s3c_mfc_num_inst_avail--;
			s3c_mfc_inst_status[s3c_mfc_inst_no] = 1;
			return s3c_mfc_inst_no;
		}

		s3c_mfc_inst_no = (s3c_mfc_inst_no + 1) % ARRAY_SIZE(s3c_mfc_inst_status);
	}

	return -1;
}

int s3c_mfc_release_inst_pool(int instance_no)
{   __D("\n");
	if (instance_no >= ARRAY_SIZE(s3c_mfc_inst_status) ||
		instance_no < 0) {
		return -1;
	}

	if (s3c_mfc_inst_status[instance_no] == 0)
		return -1;

	s3c_mfc_num_inst_avail++;
	s3c_mfc_inst_status[instance_no] = 0;

	return instance_no;
}


void s3c_mfc_occupy_all_inst_pool(void)
{   __D("\n");
	int  i;

	if (s3c_mfc_num_inst_avail == 0)
		return;

	for (i = 0; i < MFC_NUM_INSTANCES_MAX; i++) {
		if (s3c_mfc_inst_status[i] == 0) {
			s3c_mfc_num_inst_avail--;
			s3c_mfc_inst_status[i] = 1;
		}
	}
}

void s3c_mfc_release_all_inst_pool(void)
{   __D("\n");
	int  i;

	if (s3c_mfc_num_inst_avail == MFC_NUM_INSTANCES_MAX)
		return;

	for (i = 0; i < MFC_NUM_INSTANCES_MAX; i++) {
		if (s3c_mfc_inst_status[i] == 1) {
			s3c_mfc_num_inst_avail++;
			s3c_mfc_inst_status[i] = 0;
		}
	}
}
