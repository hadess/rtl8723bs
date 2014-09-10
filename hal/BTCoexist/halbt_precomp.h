/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef	__HALBT_PRECOMP_H__
#define __HALBT_PRECOMP_H__
//============================================================
// include files
//============================================================
#include <drv_types.h>
#include <osdep_service.h>

#ifdef CONFIG_RTL8723B
#include <rtl8723b_hal.h>
#endif
#ifdef CONFIG_RTL8723A
#include <rtl8723a_hal.h>
#endif


#include <halbt.h>

#include "HalBtCoexist.h"
#include "HalBtcOutSrc.h"

#include "HalBtc8723a1Ant.h"
#include "HalBtc8723a2Ant.h"

#include "HalBtc8723b1Ant.h"
#include "HalBtc8723b2Ant.h"

#include "HalBtc8812a1Ant.h"


#define GetDefaultAdapter(padapter)	padapter

#endif	// __HALBT_PRECOMP_H__
