/*
 * Copyright (C) 1999-2013, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 */
/*****************************************************************************
*                                                                            *
*                    Copyright (c) 2010 Broadcom, Inc.                       *
*                             All Rights Reserved                            *
*                                                                            *
* Confidential and Proprietary.                                              *
* The copyright notice above does not evidence any actual or intended        *
* publication. Possession, use, or copying is authorized only pursuant to a  *
* valid written license from Broadcom.                                       *
*                                                                            *
*                                                                            *
******************************************************************************
*  $Id:: ddr40_variant.h 1758 2012-08-17 15:14:03Z gennady        $:
*  $Rev::file =  : Global SVN Revision = 1780                     $:
*
*  c header version of: ddr40_variant.vh 2003 2011-05-12 15:47:21Z djackson 
*
*  Generated by make_h_from_vh from ddr40_variant.vh
*
*****************************************************************************/
/*
** Description: This module defines global parameters for this type of ddr phy
** 
*/
// Author: R. Schumann

#ifndef DDR40_TOP_MODULE_NAME   // skip if already defined

#define  DDR40_TOP_MODULE_NAME ddr40_phy_top_32_fc_40lp

#define  DDR40_PACKAGE_TYPE fc
// `define  DDR40_PACKAGE_TYPE wb

#define  DDR40_PROCESS_TYPE 40lp
// `define  DDR40_PROCESS_TYPE 40g

#define  DDR40_PACKAGE_TYPE_IS_FC
//`define  DDR40_PACKAGE_TYPE_IS_WB

#ifndef DDR40_PROCESS_TYPE_IS_40LP
 #define DDR40_PROCESS_TYPE_IS_40LP
#endif
//`ifndef DDR40_PROCESS_TYPE_IS_40G
// `define DDR40_PROCESS_TYPE_IS_40G
//`endif

#define  DDR40_DQ_WIDTH 32
// `define  DDR40_DQ_WIDTH 32
#define  DDR40_DQ_BYTES (DDR40_DQ_WIDTH/8)

#ifndef DDR40_WIDTH_IS_32
 #define  DDR40_WIDTH_IS_32 
#endif
// `define  DDR40_WIDTH_IS_16

// undefine width parameter so DDR will be 16-bit
#undef DDR40_WIDTH_IS_32

#define  DDR40_PROCESS_DETAIL tsmc40lp_M1_4Mx_1Mz_1UTRDL

// feature list
#define DDR40_INCLUDE_AUX_PINS
//`define DDR40_INCLUDE_KEY_STORAGE // no key storage in 40lp
//`define DDR40_ISO_CELLS_IN_KEY_FOB // no key storage in 40lp
//`define DDR40_INCLUDE_DCAP_BLOCK // included in *any* D0 40lp variant
#define DDR40_FIX_RD_DATA_DLY_MINMAX
//`define DDR40_USE_XXXXXXXXX_IO_CELLS
#define DDR40_CALIB_40LP_POST_DIV_FIX
#define DDR40_USE_MEMC_DDR_SCAN_CLK
#define DDR40_MAX_MIN_MIN_VDL_SCAN_CAPTURE
//`define DDR40_INCLUDE_PHYB_VDL_ADJ // not needed for the speeds 40lp supports
#define DDR40_PHYBIST_AUX_MASK_FIX
#define DDR40_DDR3_CALIB_RESET_FIX
#define DDR40_INCLUDE_NO_DQS_RD
#define DDR40_USE_STANDBY_EXIT_PIN
//`define DDR40_DATAPATH_LOOPBACK
#define DDR40_CLEAN_RST_N_FIX
#define DDR40_USE_WIDE_DYN_VDL_EN
#define DDR40_FIX_WR_ADDR_BUG
#define DDR40_FIX_S2_EXIT_BUGS
#define DDR40_INCLUDE_VDDO_CK
#define DDR40_FIX_FREEZE_CKE_OFF
#define DDR40_INCLUDE_MPR_MODE
#define DDR40_INCLUDE_SINGLE_CYC
#define DDR40_VTT_HANG_QUICK_FIX
#define DDR40_EXPANDED_REVID
#define DDR40_CC_AUTO_INIT_2ND_RESYNC_FIX
// `define DDR40_CL_127_FIX // do not include - possible critical path issues in 40LP
#define DDR40_CAL_RD_DATA_DLY_FIX

#endif // ifndef DDR40_TOP_MODULE_NAME
