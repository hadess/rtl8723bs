/******************************************************************************
 * Customer code to add GPIO control during WLAN start/stop
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#ifndef __CUSTOM_GPIO_H__
#define __CUSTOM_GPIO_H___

#include <drv_conf.h>
#include <osdep_service.h>

#ifdef PLATFORM_OS_XP
#include <drv_types_xp.h>
#endif

#ifdef PLATFORM_OS_CE
#include <drv_types_ce.h>
#endif

#ifdef PLATFORM_LINUX
#include <drv_types_linux.h>
#endif

typedef enum cust_gpio_modes {
	WLAN_PWDN_ON,
	WLAN_PWDN_OFF,
	WLAN_POWER_ON,
	WLAN_POWER_OFF,
	WLAN_BT_PWDN_ON,
	WLAN_BT_PWDN_OFF
} cust_gpio_modes_t;

extern int rtw_wifi_gpio_init(void);
extern int rtw_wifi_gpio_deinit(void);
extern void rtw_wifi_gpio_wlan_ctrl(int onoff);
extern int rtw_wifi_mp_gpio_init(void);
#endif
