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
#ifndef __RTW_IOCTL_SET_H_
#define __RTW_IOCTL_SET_H_


typedef u8 NDIS_802_11_PMKID_VALUE[16];

typedef struct _BSSIDInfo {
	NDIS_802_11_MAC_ADDRESS  BSSID;
	NDIS_802_11_PMKID_VALUE  PMKID;
} BSSIDInfo, *PBSSIDInfo;


u8 rtw_set_802_11_authentication_mode(_adapter *pdapter, NDIS_802_11_AUTHENTICATION_MODE authmode);
u8 rtw_set_802_11_bssid(_adapter* padapter, u8 *bssid);
u8 rtw_set_802_11_add_wep(_adapter * padapter, NDIS_802_11_WEP * wep);
u8 rtw_set_802_11_disassociate(_adapter * padapter);
u8 rtw_set_802_11_bssid_list_scan(_adapter* padapter, NDIS_802_11_SSID *pssid, int ssid_max_num);
u8 rtw_set_802_11_infrastructure_mode(_adapter * padapter, NDIS_802_11_NETWORK_INFRASTRUCTURE networktype);
u8 rtw_set_802_11_ssid(_adapter * padapter, NDIS_802_11_SSID * ssid);
u8 rtw_set_802_11_connect(_adapter* padapter, u8 *bssid, NDIS_802_11_SSID *ssid);

u8 rtw_validate_bssid(u8 *bssid);
u8 rtw_validate_ssid(NDIS_802_11_SSID *ssid);

u16 rtw_get_cur_max_rate(_adapter *adapter);

#endif
