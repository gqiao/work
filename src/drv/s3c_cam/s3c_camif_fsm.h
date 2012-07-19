/****************************************************************
 * $ID: s3c_camif_fsm.h 二, 07  4月 2009 14:16:56 +0800  root $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  范美辉(Meihui Fan)  <mhfan@hhcn.com>            *
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 * Last modified: 二, 07  4月 2009 14:43:19 +0800       by root #
 ****************************************************************/
#ifndef S3C_CAMIF_FSM_H
#define S3C_CAMIF_FSM_H

#include "s3c_camif.h"

//const char *fsm_version = "$Id: s3c_camif_fsm.c,v 1.2 2007/09/07 01:10:59 yreom Exp $";

extern ssize_t camif_start_2fsm(camif_cfg_t *cfg);
extern int camif_enter_2fsm(camif_cfg_t *cfg);
extern ssize_t camif_start_preview(camif_cfg_t *cfg);
extern ssize_t camif_pc_stop(camif_cfg_t *cfg);
extern ssize_t camif_stop_preview(camif_cfg_t *cfg);
extern ssize_t camif_start_preview(camif_cfg_t *cfg);
extern ssize_t camif_stop_codec(camif_cfg_t *cfg);
//extern void camif_start_p_with_c(camif_cfg_t *cfg);
extern int camif_enter_c_4fsm(camif_cfg_t *cfg);
#endif//S3C_CAMIF_FSM_H
/***************** End Of File: s3c_camif_fsm.h *****************/
// vim:sts=4:ts=8: 
