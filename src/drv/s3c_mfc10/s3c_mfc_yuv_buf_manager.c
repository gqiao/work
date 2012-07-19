#define LOG_TAG "s3c_mfc_yuv_buf_manager.c"
#include "osal.h"

/* linux/driver/media/video/mfc/s3c_mfc_yuv_buf_manager.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 * This source file is for managing the YUV buffer.
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <linux/kernel.h>

#include "s3c_mfc_yuv_buf_manager.h"
#include "s3c_mfc_types.h"
#include "s3c_mfc.h"

/* 
 * The size in bytes of the BUF_SEGMENT. 
 * The buffers are fragemented into the segment unit of this size.
 */
#define BUF_SEGMENT_SIZE	1024


typedef struct {
	unsigned char *pBaseAddr;
	int            idx_commit;
} s3c_mfc_segment_info_t;


typedef struct {
	int index_base_seg;
	int num_segs;
} s3c_mfc_commit_info_t;


static s3c_mfc_segment_info_t  *_p_segment_info = NULL;
static s3c_mfc_commit_info_t   *_p_commit_info  = NULL;

static unsigned char *_pBufferBase  = NULL;
static int            _nBufferSize  = 0;
static int            _nNumSegs		= 0;


/* 
 * int FramBufMgrInit(unsigned char *pBufBase, int nBufSize) 
 *
 * Description 
 * 		This function initializes the MfcFramBufMgr(Buffer Segment Manager)
 * Parameters
 * 		pBufBase [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions. 
 * 		nBufSize [IN]: buffer size in bytes
 * Return Value 
 * 		1 : Success
 * 		0 : Fail
 */
BOOL FramBufMgrInit(unsigned char *pBufBase, int nBufSize)
{   
	int i;

	__D("\n");
	
	if (pBufBase == NULL || nBufSize == 0)
		return FALSE;

	if ((_pBufferBase != NULL) && (_nBufferSize != 0)) {
		if ((pBufBase == _pBufferBase) && (nBufSize == _nBufferSize))
			return TRUE;

		FramBufMgrFinal();
	}

	_pBufferBase = pBufBase;
	_nBufferSize = nBufSize;
	_nNumSegs = nBufSize / BUF_SEGMENT_SIZE;

	_p_segment_info = (typeof(_p_segment_info))kzalloc(_nNumSegs * sizeof(*_p_segment_info), GFP_KERNEL);
	for (i = 0; i < _nNumSegs; i++) {
		_p_segment_info[i].pBaseAddr = pBufBase  +  (i * BUF_SEGMENT_SIZE);
		_p_segment_info[i].idx_commit = 0;
	}

	_p_commit_info  = (typeof(_p_commit_info))kzalloc(_nNumSegs * sizeof(*_p_commit_info), GFP_KERNEL);
	for (i = 0; i < _nNumSegs; i++) {
		_p_commit_info[i].index_base_seg  = -1;
		_p_commit_info[i].num_segs        = 0;
	}

	return TRUE;
}


void FramBufMgrFinal()
{   __D("\n");
	if (_p_segment_info != NULL) {
		kfree(_p_segment_info);
		_p_segment_info = NULL;
	}

	if (_p_commit_info != NULL) {
		kfree(_p_commit_info);
		_p_commit_info = NULL;
	}

	_pBufferBase  = NULL;
	_nBufferSize  = 0;
	_nNumSegs = 0;
}

/* 
 * unsigned char *s3c_mfc_commit_yuv_buffer_mgr(int idx_commit, int commit_size)
 *
 * Description
 * 	This function requests the commit for commit_size buffer to be reserved.
 * Parameters
 * 	idx_commit  [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions.
 * 	commit_size [IN]: commit size in bytes
 * Return Value
 * 	NULL : Failed to commit (Wrong parameters, commit_size too big, and so on.)
 * 	Otherwise it returns the pointer which was committed.
 */
unsigned char *s3c_mfc_commit_yuv_buffer_mgr(int idx_commit, int commit_size)
{   
	int  i, j;
	int  num_yuv_buf_seg;

	__D("\n");
	
	if (_p_segment_info == NULL || _p_commit_info == NULL) {
		return NULL;
	}

	/* check parameters */
	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return NULL;
	if (commit_size <= 0 || commit_size > _nBufferSize)
		return NULL;

	if (_p_commit_info[idx_commit].index_base_seg != -1)
		return NULL;


	if ((commit_size % BUF_SEGMENT_SIZE) == 0)
		num_yuv_buf_seg = commit_size / BUF_SEGMENT_SIZE;
	else
		num_yuv_buf_seg = (commit_size / BUF_SEGMENT_SIZE)  +  1;

	for (i=0; i<(_nNumSegs - num_yuv_buf_seg); i++) {
		if (_p_segment_info[i].idx_commit != 0)
			continue;

		for (j=0; j<num_yuv_buf_seg; j++) {
			if (_p_segment_info[i+j].idx_commit != 0)
				break;
		}

		if (j == num_yuv_buf_seg) {

			for (j=0; j<num_yuv_buf_seg; j++) {
				_p_segment_info[i+j].idx_commit = 1;
			}

			_p_commit_info[idx_commit].index_base_seg  = i;
			_p_commit_info[idx_commit].num_segs        = num_yuv_buf_seg;

			return _p_segment_info[i].pBaseAddr;
		} else {
			i = i + j - 1;
		}
	}

	return NULL;
}


/*
 * void s3c_mfc_free_yuv_buffer_mgr(int idx_commit)
 *
 * Description
 * 	This function frees the committed region of buffer.
 * Parameters
 * 	idx_commit  [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions.
 * Return Value
 * 	None
 */
void s3c_mfc_free_yuv_buffer_mgr(int idx_commit)
{   
	int  i;
	int  index_base_seg;
	int  num_yuv_buf_seg;

	__D("\n");
	
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return;

	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return;

	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return;


	index_base_seg =  _p_commit_info[idx_commit].index_base_seg;
	num_yuv_buf_seg=  _p_commit_info[idx_commit].num_segs;

	for (i = 0; i < num_yuv_buf_seg; i++) {
		_p_segment_info[index_base_seg + i].idx_commit = 0;
	}


	_p_commit_info[idx_commit].index_base_seg  =  -1;
	_p_commit_info[idx_commit].num_segs        =  0;

}

/* 
 * unsigned char *s3c_mfc_get_yuv_buffer(int idx_commit)
 *
 * Description
 * 	This function obtains the committed buffer of 'idx_commit'.
 * Parameters
 * 	idx_commit  [IN]: commit index of the buffer which will be obtained
 * Return Value
 * 	NULL : Failed to get the indicated buffer (Wrong parameters, not committed, and so on.)
 * 	Otherwise it returns the pointer which was committed.
 */
unsigned char *s3c_mfc_get_yuv_buffer(int idx_commit)
{   
	int index_base_seg;

	__D("\n");
	
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return NULL;

	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return NULL;

	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return NULL;

	index_base_seg  =  _p_commit_info[idx_commit].index_base_seg;

	return _p_segment_info[index_base_seg].pBaseAddr;
}

/* 
 * int s3c_mfc_get_yuv_buffer_size(int idx_commit)
 *
 * Description
 * 	This function obtains the size of the committed buffer of 'idx_commit'.
 * Parameters
 * 	idx_commit  [IN]: commit index of the buffer which will be obtained
 * Return Value
 * 	0 : Failed to get the size of indicated buffer (Wrong parameters, not committed, and so on.)
 * 	Otherwise it returns the size of the buffer.
 * 	Note that the size is multiples of the BUF_SEGMENT_SIZE.
 */
int s3c_mfc_get_yuv_buffer_size(int idx_commit)
{   __D("\n");
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return 0;

	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return 0;

	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return 0;

	return (_p_commit_info[idx_commit].num_segs * BUF_SEGMENT_SIZE);
}

/*
 * void s3c_mfc_print_commit_yuv_buffer_info()
 *
 * Description
 * 	This function prints the commited information on the console screen.
 * Parameters
 * 	None
 * Return Value
 * 	None
 */
void s3c_mfc_print_commit_yuv_buffer_info()
{   
	int  i;

	__D("\n");
	
	if (_p_segment_info == NULL || _p_commit_info == NULL) {
		mfc_err("fram buffer manager is not initialized\n");
		return;
	}


	for (i = 0; i < _nNumSegs; i++) {
		if (_p_commit_info[i].index_base_seg != -1)  {
			mfc_debug("commit index = %03d, base segment index = %d\n",	\
						i, _p_commit_info[i].index_base_seg);
			mfc_debug("commit index = %03d, number of segment = %d\n", \
						i, _p_commit_info[i].num_segs);
		}
	}
}
