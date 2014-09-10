/******************************************************************************
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
#define _RTL8723A_CMD_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <cmd_osdep.h>
#include <mlme_osdep.h>
#include <rtw_byteorder.h>
#include <circ_buf.h>
#include <rtw_ioctl_set.h>

#include <rtl8723a_hal.h>


#define RTL92C_MAX_H2C_BOX_NUMS	4
#define RTL92C_MAX_CMD_LEN	5
#define MESSAGE_BOX_SIZE		4
#define EX_MESSAGE_BOX_SIZE	2


static u8 _is_fw_read_cmd_down(_adapter* padapter, u8 msgbox_num)
{
	u8	read_down = _FALSE;
	int 	retry_cnts = 100;

	u8 valid;

	//DBG_8192C(" _is_fw_read_cmd_down ,reg_1cc(%x),msg_box(%d)...\n",rtw_read8(padapter,REG_HMETFR),msgbox_num);

	do{
		valid = rtw_read8(padapter,REG_HMETFR) & BIT(msgbox_num);
		if(0 == valid ){
			read_down = _TRUE;
		}
#ifdef CONFIG_WOWLAN
		else
			rtw_msleep_os(1);
#endif
	}while( (!read_down) && (retry_cnts--));

	return read_down;

}


/*****************************************
* H2C Msg format :
*| 31 - 8		|7		| 6 - 0	|
*| h2c_msg 	|Ext_bit	|CMD_ID	|
*
******************************************/
s32 FillH2CCmd(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer)
{
	u8 bcmd_down = _FALSE;
	s32 retry_cnts = 100;
	u8 h2c_box_num;
	u32	msgbox_addr;
	u32 msgbox_ex_addr;
	PHAL_DATA_TYPE pHalData;
	u32	h2c_cmd = 0;
	u16	h2c_cmd_ex = 0;
	s32 ret = _FAIL;

_func_enter_;

	padapter = GET_PRIMARY_ADAPTER(padapter);
	pHalData = GET_HAL_DATA(padapter);

	_enter_critical_mutex(&(adapter_to_dvobj(padapter)->h2c_fwcmd_mutex), NULL);

	if (!pCmdBuffer) {
		goto exit;
	}
	if (CmdLen > RTL92C_MAX_CMD_LEN) {
		goto exit;
	}
	if (padapter->bSurpriseRemoved == _TRUE)
		goto exit;

	//pay attention to if  race condition happened in  H2C cmd setting.
	do{
		h2c_box_num = pHalData->LastHMEBoxNum;

		if(!_is_fw_read_cmd_down(padapter, h2c_box_num)){
			DBG_8192C(" fw read cmd failed...\n");
			goto exit;
		}

		if(CmdLen<=3)
		{
			_rtw_memcpy((u8*)(&h2c_cmd)+1, pCmdBuffer, CmdLen );
		}
		else{
			_rtw_memcpy((u8*)(&h2c_cmd_ex), pCmdBuffer, EX_MESSAGE_BOX_SIZE);
			_rtw_memcpy((u8*)(&h2c_cmd)+1, pCmdBuffer+2,( CmdLen-EX_MESSAGE_BOX_SIZE));
			*(u8*)(&h2c_cmd) |= BIT(7);
		}

		*(u8*)(&h2c_cmd) |= ElementID;

#ifdef CONFIG_LPS_LCLK
		if (ElementID != LOWPWR_LPS_EID) {
			if (rtw_register_h2c_alive(padapter) != _SUCCESS) {
				DBG_871X("%s ERROR rtw_register_h2c_alive fail\n", __func__);
			}
		}
#endif
		if(h2c_cmd & BIT(7)){
			msgbox_ex_addr = REG_HMEBOX_EXT_0 + (h2c_box_num *EX_MESSAGE_BOX_SIZE);
			h2c_cmd_ex = le16_to_cpu( h2c_cmd_ex );
			rtw_write16(padapter, msgbox_ex_addr, h2c_cmd_ex);
		}
		msgbox_addr =REG_HMEBOX_0 + (h2c_box_num *MESSAGE_BOX_SIZE);
		h2c_cmd = le32_to_cpu( h2c_cmd );
		rtw_write32(padapter,msgbox_addr, h2c_cmd);

		bcmd_down = _TRUE;

		DBG_8192C("MSG_BOX:%d, CmdLen(%d), CmdID(%d), reg:0x%x =>h2c_cmd:0x%x, reg:0x%x =>h2c_cmd_ex:0x%x ..\n"
		 	,pHalData->LastHMEBoxNum , CmdLen, ElementID, msgbox_addr, h2c_cmd, msgbox_ex_addr, h2c_cmd_ex);

		pHalData->LastHMEBoxNum = (h2c_box_num+1) % RTL92C_MAX_H2C_BOX_NUMS;

#ifdef CONFIG_LPS_LCLK
		if (ElementID != LOWPWR_LPS_EID) {
			rtw_unregister_h2c_alive(padapter);
		}
#endif
	}while((!bcmd_down) && (retry_cnts--));

	ret = _SUCCESS;

exit:

	_exit_critical_mutex(&(adapter_to_dvobj(padapter)->h2c_fwcmd_mutex), NULL);

_func_exit_;

	return ret;
}

u8 rtl8192c_h2c_msg_hdl(_adapter *padapter, unsigned char *pbuf)
{
	u8 ElementID, CmdLen;
	u8 *pCmdBuffer;
	struct cmd_msg_parm  *pcmdmsg;

	if(!pbuf)
		return H2C_PARAMETERS_ERROR;

	pcmdmsg = (struct cmd_msg_parm*)pbuf;
	ElementID = pcmdmsg->eid;
	CmdLen = pcmdmsg->sz;
	pCmdBuffer = pcmdmsg->buf;

	FillH2CCmd(padapter, ElementID, CmdLen, pCmdBuffer);

	return H2C_SUCCESS;
}

#if defined(CONFIG_AUTOSUSPEND) && defined(SUPPORT_HW_RFOFF_DETECTED)
u8 rtl8192c_set_FwSelectSuspend_cmd(_adapter *padapter ,u8 bfwpoll, u16 period)
{
	u8	res=_SUCCESS;
	struct H2C_SS_RFOFF_PARAM param;
	DBG_8192C("==>%s bfwpoll(%x)\n",__FUNCTION__,bfwpoll);
	param.gpio_period = period;//Polling GPIO_11 period time
	param.ROFOn = (_TRUE == bfwpoll)?1:0;
	FillH2CCmd(padapter, SELECTIVE_SUSPEND_ROF_CMD, sizeof(param), (u8*)(&param));
	return res;
}
#endif //CONFIG_AUTOSUSPEND && SUPPORT_HW_RFOFF_DETECTED

u8 rtl8192c_set_rssi_cmd(_adapter*padapter, u8 *param)
{
	u8	res=_SUCCESS;

_func_enter_;

	*((u32*) param ) = cpu_to_le32( *((u32*) param ) );

	FillH2CCmd(padapter, RSSI_SETTING_EID, 3, param);

_func_exit_;

	return res;
}

u8 rtl8192c_set_raid_cmd(_adapter*padapter, u32 mask, u8 arg)
{
	u8	buf[5];
	u8	res=_SUCCESS;

_func_enter_;

	_rtw_memset(buf, 0, 5);
	mask = cpu_to_le32( mask );
	_rtw_memcpy(buf, &mask, 4);
	buf[4]  = arg;

	FillH2CCmd(padapter, MACID_CONFIG_EID, 5, buf);

_func_exit_;

	return res;

}

//bitmap[0:27] = tx_rate_bitmap
//bitmap[28:31]= Rate Adaptive id
//arg[0:4] = macid
//arg[5] = Short GI
void rtl8192c_Add_RateATid(PADAPTER pAdapter, u32 bitmap, u8 arg)
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if(pHalData->fw_ractrl == _TRUE)
	{
		rtl8192c_set_raid_cmd(pAdapter, bitmap, arg);
	}
	else
	{
		u8 macid, init_rate, shortGIrate=_FALSE;

		init_rate = get_highest_rate_idx(bitmap&0x0fffffff)&0x3f;

		macid = arg&0x1f;

		shortGIrate = (arg&BIT(5)) ? _TRUE:_FALSE;

		if (shortGIrate==_TRUE)
			init_rate |= BIT(6);

		rtw_write8(pAdapter, (REG_INIDATA_RATE_SEL+macid), (u8)init_rate);
	}

}

static s32 _rtl8723a_set_lowpwr_lps_cmd(PADAPTER padapter, u8 enable)
{
	LOWPWR_LPS_PARM lowpwr_lps;
#ifdef CONFIG_WOWLAN
	PWR_LPS_TUNE_PARAM pwr_param;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
#endif
	u8 BcnRcvTime = 0;
	u32 BcnLen;
	s32 ret;

_func_enter_;

	DBG_871X("%s: Enable=%d\n", __FUNCTION__, enable);

	_rtw_memset(&lowpwr_lps, 0, sizeof(LOWPWR_LPS_PARM));
	if (enable == _TRUE)
	{
#ifdef CONFIG_WOWLAN
		if(pwrpriv->bInSuspend){
			lowpwr_lps.enable = 1;
			lowpwr_lps.bcn_count= 3;  // max 15
			lowpwr_lps.tb_bcn_threshold = 1; // max 7
			lowpwr_lps.bcn_interval = 0x86; //About 5ms
			lowpwr_lps.drop_threshold = 11;
			lowpwr_lps.max_early_period = 13;
			lowpwr_lps.max_bcn_timeout_period = 25;
		}else
#endif
		{
			// rcv time(micro%32) use CCK-1M
			// fcs is 4
			BcnLen = padapter->mlmepriv.cur_network.network.BcnLength;
			BcnLen += 500; //add 5ms
			BcnRcvTime = ((BcnLen<<3)+31) >> 5;
			//BcnRcvTime += 6400 >> 5; //add 4ms
			DBG_871X("%s: BcnLength=%d BcnRcvTime=%d (32us)\n", __FUNCTION__, BcnLen, BcnRcvTime);

			lowpwr_lps.enable = 1;
			lowpwr_lps.bcn_count= 2; // max 15
			lowpwr_lps.tb_bcn_threshold = 2; // max 7
			lowpwr_lps.bcn_interval = BcnRcvTime;
			lowpwr_lps.drop_threshold = 0;
			lowpwr_lps.max_early_period = 13;
			lowpwr_lps.max_bcn_timeout_period = 25;
		}
	}

	ret = FillH2CCmd(padapter, LOWPWR_LPS_EID, 5, (u8*)&lowpwr_lps);

#ifdef CONFIG_WOWLAN
	if(pwrpriv->bInSuspend){
		pwr_param.bcn_timeout = 2;
		pwr_param.dtim_timeout = 15;  //5
		pwr_param.adopt = 1;
		pwr_param.ps_timeout = 20;
		pwr_param.ps_dtim_period = 3;
		ret |= FillH2CCmd(padapter, H2C_COM_PS_TUNE_PARAM, 4, (u8*)&pwr_param);
	}
#endif

_func_exit_;

	return ret;
}

s32 rtl8723a_set_lowpwr_lps_cmd(PADAPTER padapter, u8 enable)
{
	struct pwrctrl_priv *ppwrpriv;
	u8 bFlag;
	s32 ret;

_func_enter_;
	#ifndef CONFIG_LOW_PWR_LPS
	return _TRUE;
	#endif
	ppwrpriv = adapter_to_pwrctl(padapter);
	if (ppwrpriv->bFwLowPwrLPS == enable)
		return _TRUE;

	if (enable == _TRUE) {
		if (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _FALSE) {
			return _FALSE;
		}
	}

	bFlag = ppwrpriv->bFwLowPwrLPS;
	ppwrpriv->bFwLowPwrLPS = enable;

	ret = _rtl8723a_set_lowpwr_lps_cmd(padapter, enable);
	if (ret == _FALSE) {
		ppwrpriv->bFwLowPwrLPS = bFlag; // restore
	}

_func_exit_;

	return ret;
}

void rtl8723a_set_FwPwrMode_cmd(PADAPTER padapter, u8 Mode)
{
	SETPWRMODE_PARM H2CSetPwrMode;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

_func_enter_;

	DBG_871X("%s: Mode=%d SmartPS=%d UAPSD=%d BcnMode=0x%02x\n", __FUNCTION__,
			Mode, pwrpriv->smart_ps, padapter->registrypriv.uapsd_enable, pwrpriv->bcn_ant_mode);

	H2CSetPwrMode.Mode = Mode;
	H2CSetPwrMode.SmartPS = pwrpriv->smart_ps;
#ifdef CONFIG_WOWLAN
	if(Mode == PS_MODE_DTIM)  //For WOWLAN LPS
		H2CSetPwrMode.AwakeInterval = 3;
	else
		H2CSetPwrMode.AwakeInterval = 1;
#else
	H2CSetPwrMode.AwakeInterval = 1;
#endif
	H2CSetPwrMode.bAllQueueUAPSD = padapter->registrypriv.uapsd_enable;
	/* BIT6 if set 1, fw will not set ant & PAT based */
	/* on Byte5 under recv PwrMode H2C, we should always set it to 1 */
	/* BIT4 to close SmartPs issue pspoll(1) after NULL1 under TDMA */
	H2CSetPwrMode.BcnAntMode = pwrpriv->bcn_ant_mode | BIT(6);

	FillH2CCmd(padapter, SET_PWRMODE_EID, sizeof(H2CSetPwrMode), (u8 *)&H2CSetPwrMode);

_func_exit_;
}

void ConstructBeacon(_adapter *padapter, u8 *pframe, u32 *pLength)
{
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16					*fctrl;
	u32					rate_len, pktlen;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


	//DBG_871X("%s\n", __FUNCTION__);

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_rtw_memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(cur_network), ETH_ALEN);

	SetSeqNum(pwlanhdr, 0/*pmlmeext->mgnt_seq*/);
	//pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_BEACON);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pktlen = sizeof (struct rtw_ieee80211_hdr_3addr);

	//timestamp will be inserted by hardware
	pframe += 8;
	pktlen += 8;

	// beacon interval: 2 bytes
	_rtw_memcpy(pframe, (unsigned char *)(rtw_get_beacon_interval_from_ie(cur_network->IEs)), 2);

	pframe += 2;
	pktlen += 2;

	// capability info: 2 bytes
	_rtw_memcpy(pframe, (unsigned char *)(rtw_get_capability_from_ie(cur_network->IEs)), 2);

	pframe += 2;
	pktlen += 2;

	if( (pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)
	{
		//DBG_871X("ie len=%d\n", cur_network->IELength);
		pktlen += cur_network->IELength - sizeof(NDIS_802_11_FIXED_IEs);
		_rtw_memcpy(pframe, cur_network->IEs+sizeof(NDIS_802_11_FIXED_IEs), pktlen);

		goto _ConstructBeacon;
	}

	//below for ad-hoc mode

	// SSID
	pframe = rtw_set_ie(pframe, _SSID_IE_, cur_network->Ssid.SsidLength, cur_network->Ssid.Ssid, &pktlen);

	// supported rates...
	rate_len = rtw_get_rateset_len(cur_network->SupportedRates);
	pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, ((rate_len > 8)? 8: rate_len), cur_network->SupportedRates, &pktlen);

	// DS parameter set
	pframe = rtw_set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&(cur_network->Configuration.DSConfig), &pktlen);

	if( (pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
	{
		u32 ATIMWindow;
		// IBSS Parameter Set...
		//ATIMWindow = cur->Configuration.ATIMWindow;
		ATIMWindow = 0;
		pframe = rtw_set_ie(pframe, _IBSS_PARA_IE_, 2, (unsigned char *)(&ATIMWindow), &pktlen);
	}


	//todo: ERP IE


	// EXTERNDED SUPPORTED RATE
	if (rate_len > 8)
	{
		pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (rate_len - 8), (cur_network->SupportedRates + 8), &pktlen);
	}


	//todo:HT for adhoc

_ConstructBeacon:

	if ((pktlen + TXDESC_SIZE) > 512)
	{
		DBG_871X("beacon frame too large\n");
		return;
	}

	*pLength = pktlen;

	//DBG_871X("%s bcn_sz=%d\n", __FUNCTION__, pktlen);

}

void ConstructPSPoll(_adapter *padapter, u8 *pframe, u32 *pLength)
{
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16					*fctrl;
	u32					pktlen;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//DBG_871X("%s\n", __FUNCTION__);

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	// Frame control.
	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	SetPwrMgt(fctrl);
	SetFrameSubType(pframe, WIFI_PSPOLL);

	// AID.
	SetDuration(pframe, (pmlmeinfo->aid | 0xc000));

	// BSSID.
	_rtw_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	// TA.
	_rtw_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);

	*pLength = 16;
}

void ConstructNullFunctionData(
	PADAPTER padapter,
	u8		*pframe,
	u32		*pLength,
	u8		*StaAddr,
	u8		bQoS,
	u8		AC,
	u8		bEosp,
	u8		bForcePowerSave)
{
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16						*fctrl;
	u32						pktlen;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct wlan_network		*cur_network = &pmlmepriv->cur_network;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	//DBG_871X("%s:%d\n", __FUNCTION__, bForcePowerSave);

	pwlanhdr = (struct rtw_ieee80211_hdr*)pframe;

	fctrl = &pwlanhdr->frame_ctl;
	*(fctrl) = 0;
	if (bForcePowerSave)
	{
		SetPwrMgt(fctrl);
	}

	switch(cur_network->network.InfrastructureMode)
	{
		case Ndis802_11Infrastructure:
			SetToDs(fctrl);
			_rtw_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr3, StaAddr, ETH_ALEN);
			break;
		case Ndis802_11APMode:
			SetFrDs(fctrl);
			_rtw_memcpy(pwlanhdr->addr1, StaAddr, ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr2, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr3, myid(&(padapter->eeprompriv)), ETH_ALEN);
			break;
		case Ndis802_11IBSS:
		default:
			_rtw_memcpy(pwlanhdr->addr1, StaAddr, ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
			_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
			break;
	}

	SetSeqNum(pwlanhdr, 0);

	if (bQoS == _TRUE) {
		struct rtw_ieee80211_hdr_3addr_qos *pwlanqoshdr;

		SetFrameSubType(pframe, WIFI_QOS_DATA_NULL);

		pwlanqoshdr = (struct rtw_ieee80211_hdr_3addr_qos*)pframe;
		SetPriority(&pwlanqoshdr->qc, AC);
		SetEOSP(&pwlanqoshdr->qc, bEosp);

		pktlen = sizeof(struct rtw_ieee80211_hdr_3addr_qos);
	} else {
		SetFrameSubType(pframe, WIFI_DATA_NULL);

		pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	}

	*pLength = pktlen;
}


#ifdef CONFIG_WOWLAN
//
// Description:
//	Construct the ARP response packet to support ARP offload.
//
void
ConstructARPResponse(
	PADAPTER padapter,
	u8			*pframe,
	u32			*pLength,
	u8			*pIPAddress
	)
{
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16						*fctrl;
	u32						pktlen;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct wlan_network		*cur_network = &pmlmepriv->cur_network;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	static u8			ARPLLCHeader[8] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06};
	u8				*pARPRspPkt = pframe;
	u8			EncryptionHeadOverhead = 0;
	//DBG_871X("%s:%d\n", __FUNCTION__, bForcePowerSave);

	pwlanhdr = (struct rtw_ieee80211_hdr*)pframe;

	fctrl = &pwlanhdr->frame_ctl;
	*(fctrl) = 0;

	//-------------------------------------------------------------------------
	// MAC Header.
	//-------------------------------------------------------------------------
	SetFrameType(fctrl, WIFI_DATA);
	//SetFrameSubType(fctrl, 0);
	SetToDs(fctrl);
	_rtw_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, 0);
	SetDuration(pwlanhdr, 0);
	//SET_80211_HDR_FRAME_CONTROL(pARPRspPkt, 0);
	//SET_80211_HDR_TYPE_AND_SUBTYPE(pARPRspPkt, Type_Data);
	//SET_80211_HDR_TO_DS(pARPRspPkt, 1);
	//SET_80211_HDR_ADDRESS1(pARPRspPkt, pMgntInfo->Bssid);
	//SET_80211_HDR_ADDRESS2(pARPRspPkt, Adapter->CurrentAddress);
	//SET_80211_HDR_ADDRESS3(pARPRspPkt, pMgntInfo->Bssid);

	//SET_80211_HDR_DURATION(pARPRspPkt, 0);
	//SET_80211_HDR_FRAGMENT_SEQUENCE(pARPRspPkt, 0);
	*pLength = sMacHdrLng;

//YJ,del,120503
#if 0
	//-------------------------------------------------------------------------
	// Qos Header: leave space for it if necessary.
	//-------------------------------------------------------------------------
	if(pStaQos->CurrentQosMode > QOS_DISABLE)
	{
		SET_80211_HDR_QOS_EN(pARPRspPkt, 1);
		PlatformZeroMemory(&(Buffer[*pLength]), sQoSCtlLng);
		*pLength += sQoSCtlLng;
	}
#endif
	//-------------------------------------------------------------------------
	// Security Header: leave space for it if necessary.
	//-------------------------------------------------------------------------

#if 1
	switch (psecuritypriv->dot11PrivacyAlgrthm)
	{
		case _WEP40_:
		case _WEP104_:
			EncryptionHeadOverhead = 4;
			break;
		case _TKIP_:
			EncryptionHeadOverhead = 8;
			break;
		case _AES_:
			EncryptionHeadOverhead = 8;
			break;
#ifdef CONFIG_WAPI_SUPPORT
		case _SMS4_:
			EncryptionHeadOverhead = 18;
			break;
#endif
		default:
			EncryptionHeadOverhead = 0;
	}

	if(EncryptionHeadOverhead > 0)
	{
		_rtw_memset(&(pframe[*pLength]), 0,EncryptionHeadOverhead);
	       	*pLength += EncryptionHeadOverhead;
		//SET_80211_HDR_WEP(pARPRspPkt, 1);  //Suggested by CCW.
		SetPrivacy(fctrl);
	}
#endif
	//-------------------------------------------------------------------------
	// Frame Body.
	//-------------------------------------------------------------------------
	pARPRspPkt =  (u8*)(pframe+ *pLength);
	// LLC header
	_rtw_memcpy(pARPRspPkt, ARPLLCHeader, 8);
	*pLength += 8;

	// ARP element
	pARPRspPkt += 8;
	SET_ARP_PKT_HW(pARPRspPkt, 0x0100);
	SET_ARP_PKT_PROTOCOL(pARPRspPkt, 0x0008);	// IP protocol
	SET_ARP_PKT_HW_ADDR_LEN(pARPRspPkt, 6);
	SET_ARP_PKT_PROTOCOL_ADDR_LEN(pARPRspPkt, 4);
	SET_ARP_PKT_OPERATION(pARPRspPkt, 0x0200); // ARP response
	SET_ARP_PKT_SENDER_MAC_ADDR(pARPRspPkt, myid(&(padapter->eeprompriv)));
	SET_ARP_PKT_SENDER_IP_ADDR(pARPRspPkt, pIPAddress);
	SET_ARP_PKT_TARGET_MAC_ADDR(pARPRspPkt, get_my_bssid(&(pmlmeinfo->network)));
	SET_ARP_PKT_TARGET_IP_ADDR(pARPRspPkt, pIPAddress);
	*pLength += 28;

}
#endif


void ConstructProbeRsp(_adapter *padapter, u8 *pframe, u32 *pLength, u8 *StaAddr, BOOLEAN bHideSSID)
{
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16					*fctrl;
	u8					*mac, *bssid;
	u32					pktlen;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);


	//DBG_871X("%s\n", __FUNCTION__);

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = myid(&(padapter->eeprompriv));
	bssid = cur_network->MacAddress;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_rtw_memcpy(pwlanhdr->addr1, StaAddr, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, bssid, ETH_ALEN);

	SetSeqNum(pwlanhdr, 0);
	SetFrameSubType(fctrl, WIFI_PROBERSP);

	pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	pframe += pktlen;

	if(cur_network->IELength>MAX_IE_SZ)
		return;

	_rtw_memcpy(pframe, cur_network->IEs, cur_network->IELength);
	pframe += cur_network->IELength;
	pktlen += cur_network->IELength;

	*pLength = pktlen;
}

// To check if reserved page content is destroyed by beacon beacuse beacon is too large.
// 2010.06.23. Added by tynli.
VOID
CheckFwRsvdPageContent(
	IN	PADAPTER		Adapter
)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(Adapter);
	u32	MaxBcnPageNum;

 	if(pHalData->FwRsvdPageStartOffset != 0)
 	{
 		/*MaxBcnPageNum = PageNum_128(pMgntInfo->MaxBeaconSize);
		RT_ASSERT((MaxBcnPageNum <= pHalData->FwRsvdPageStartOffset),
			("CheckFwRsvdPageContent(): The reserved page content has been"\
			"destroyed by beacon!!! MaxBcnPageNum(%d) FwRsvdPageStartOffset(%d)\n!",
			MaxBcnPageNum, pHalData->FwRsvdPageStartOffset));*/
 	}
}

#ifdef CONFIG_WOWLAN
void rtw_get_current_ip_address(PADAPTER padapter, u8 *pcurrentip);
void rtw_get_sec_iv(PADAPTER padapter, u8*pcur_dot11txpn, u8 *StaAddr);
#endif
//
// Description: Fill the reserved packets that FW will use to RSVD page.
//			Now we just send 4 types packet to rsvd page.
//			(1)Beacon, (2)Ps-poll, (3)Null data, (4)ProbeRsp.
//	Input:
//	    bDLFinished - FALSE: At the first time we will send all the packets as a large packet to Hw,
//				 		so we need to set the packet length to total lengh.
//			      TRUE: At the second time, we should send the first packet (default:beacon)
//						to Hw again and set the lengh in descriptor to the real beacon lengh.
// 2009.10.15 by tynli.
static void SetFwRsvdPagePkt(PADAPTER padapter, BOOLEAN bDLFinished)
{
	PHAL_DATA_TYPE pHalData;
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	struct xmit_priv	*pxmitpriv;
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;
	u32	BeaconLength, ProbeRspLength, PSPollLength;
	u32	NullDataLength, QosNullLength, BTQosNullLength;
	u8	*ReservedPagePacket;
	u8	PageNum, PageNeed, TxDescLen;
	u16	BufIndex;
	u32	TotalPacketLen;
	RSVDPAGE_LOC	RsvdPageLoc;
#ifdef CONFIG_WOWLAN
	u8	bEnableRsvdPage = 1;
	u32	ARPLegnth = 0;
	RSVDPAGE_LOC_EXT	RsvdPageLoc_ext;
	struct security_priv *psecuritypriv = &padapter->securitypriv; //added by xx
	u8 currentip[4];
	u8 cur_dot11txpn[8];
#endif

	DBG_871X("%s---->\n", __FUNCTION__);

	ReservedPagePacket = (u8*)rtw_zmalloc(1500);
	if (ReservedPagePacket == NULL) {
		DBG_871X("%s: alloc ReservedPagePacket fail!\n", __FUNCTION__);
		return;
	}

	pHalData = GET_HAL_DATA(padapter);
	pxmitpriv = &padapter->xmitpriv;
	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	TxDescLen = TXDESC_SIZE;
	PageNum = 0;

	//3 (1) beacon
	BufIndex = TXDESC_OFFSET;
	ConstructBeacon(padapter, &ReservedPagePacket[BufIndex], &BeaconLength);

	// When we count the first page size, we need to reserve description size for the RSVD
	// packet, it will be filled in front of the packet in TXPKTBUF.
	PageNeed = (u8)PageNum_128(TxDescLen + BeaconLength);
	// To reserved 2 pages for beacon buffer. 2010.06.24.
	if (PageNeed == 1)
		PageNeed += 1;
	PageNum += PageNeed;
	pHalData->FwRsvdPageStartOffset = PageNum;

	BufIndex += PageNeed*128;

	//3 (2) ps-poll
	RsvdPageLoc.LocPsPoll = PageNum;
	ConstructPSPoll(padapter, &ReservedPagePacket[BufIndex], &PSPollLength);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], PSPollLength, _TRUE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + PSPollLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3 (3) null data
	RsvdPageLoc.LocNullData = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&NullDataLength,
		get_my_bssid(&pmlmeinfo->network),
		_FALSE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], NullDataLength, 2/*_FALSE*/, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + NullDataLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3 (4) probe response
	RsvdPageLoc.LocProbeRsp = PageNum;
	ConstructProbeRsp(
		padapter,
		&ReservedPagePacket[BufIndex],
		&ProbeRspLength,
		get_my_bssid(&pmlmeinfo->network),
		_FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ProbeRspLength, _FALSE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + ProbeRspLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3 (5) Qos null data
	RsvdPageLoc.LocQosNull = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&QosNullLength,
		get_my_bssid(&pmlmeinfo->network),
		_TRUE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], QosNullLength, _FALSE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + QosNullLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3 (6) BT Qos null data
	RsvdPageLoc.LocBTQosNull = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&BTQosNullLength,
		get_my_bssid(&pmlmeinfo->network),
		_TRUE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], BTQosNullLength, _FALSE, _TRUE);

#ifdef CONFIG_WOWLAN
	PageNeed = (u8)PageNum_128(TxDescLen + BTQosNullLength);
	PageNum += PageNeed;
	BufIndex += PageNeed*128;

	//3(7) ARP
	rtw_get_current_ip_address(padapter, currentip);
	RsvdPageLoc_ext.LocARP = PageNum;
	ConstructARPResponse(
		padapter,
		&ReservedPagePacket[BufIndex],
		&ARPLegnth,
		currentip
		);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ARPLegnth, _FALSE, _FALSE);

	//added by xx
 	switch (psecuritypriv->dot11PrivacyAlgrthm)
	{
		case _WEP40_:
		case _WEP104_:
		case _TKIP_:
			ReservedPagePacket[BufIndex-TxDescLen+6]  |= BIT(6);
			break;
		case _AES_:
			ReservedPagePacket[BufIndex-TxDescLen+6]  |= BIT(6)|BIT(7);
			break;
#ifdef CONFIG_WAPI_SUPPORT
		case _SMS4_:
			ReservedPagePacket[BufIndex-TxDescLen+6]  |= BIT(7);
			break;
#endif
		default:
			;
	}

	PageNeed = (u8)PageNum_128(TxDescLen + ARPLegnth);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

        // added by xx
	//3(8) sec IV
	rtw_get_sec_iv(padapter, cur_dot11txpn, get_my_bssid(&pmlmeinfo->network));
	RsvdPageLoc_ext.remoteCtrlInfo= PageNum;
	_rtw_memcpy(ReservedPagePacket+BufIndex-TxDescLen, cur_dot11txpn, 8);

	TotalPacketLen = BufIndex-TxDescLen + sizeof (union pn48); //IV len
#else
	TotalPacketLen = BufIndex + BTQosNullLength;
#endif

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	// update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->qsel = 0x10;
	pattrib->pktlen = pattrib->last_txcmdsz = TotalPacketLen - TXDESC_OFFSET;
	_rtw_memcpy(pmgntframe->buf_addr, ReservedPagePacket, TotalPacketLen);

	rtw_hal_mgnt_xmit(padapter, pmgntframe);

	DBG_871X("%s: Set RSVD page location to Fw\n", __FUNCTION__);
	FillH2CCmd(padapter, RSVD_PAGE_EID, sizeof(RsvdPageLoc), (u8*)&RsvdPageLoc);
#ifdef CONFIG_WOWLAN
	FillH2CCmd(padapter, RSVD_PAGE_EXT_EID, sizeof(RsvdPageLoc_ext), (u8*)&RsvdPageLoc_ext);
	FillH2CCmd(padapter, RSVD_PAGE_ENABLE_EID, 1, (u8*)&bEnableRsvdPage);

	RT_PRINT_DATA(_module_rtl871x_recv_c_, _drv_always_,"xx: ReservedPagePacket:\n", ReservedPagePacket, TotalPacketLen);
#endif
exit:
	rtw_mfree(ReservedPagePacket, 1500);
}

void rtl8723a_set_FwJoinBssReport_cmd(PADAPTER padapter, u8 mstatus)
{
	JOINBSSRPT_PARM	JoinBssRptParm;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

_func_enter_;

	DBG_871X("%s mstatus(%x)\n", __FUNCTION__,mstatus);

	if(mstatus == 1)
	{
		BOOLEAN bRecover = _FALSE;
		u8 v8;

		// We should set AID, correct TSF, HW seq enable before set JoinBssReport to Fw in 88/92C.
		// Suggested by filen. Added by tynli.
		rtw_write16(padapter, REG_BCN_PSR_RPT, (0xC000|pmlmeinfo->aid));
		// Do not set TSF again here or vWiFi beacon DMA INT will not work.
		//correct_TSF(padapter, pmlmeext);
		// Hw sequende enable by dedault. 2010.06.23. by tynli.
		//rtw_write16(padapter, REG_NQOS_SEQ, ((pmlmeext->mgnt_seq+100)&0xFFF));
		//rtw_write8(padapter, REG_HWSEQ_CTRL, 0xFF);

		// set REG_CR bit 8
		v8 = rtw_read8(padapter, REG_CR+1);
		v8 |= BIT(0); // ENSWBCN
		rtw_write8(padapter,  REG_CR+1, v8);

		// Disable Hw protection for a time which revserd for Hw sending beacon.
		// Fix download reserved page packet fail that access collision with the protection time.
		// 2010.05.11. Added by tynli.
//			SetBcnCtrlReg(padapter, 0, BIT(3));
//			SetBcnCtrlReg(padapter, BIT(4), 0);
		SetBcnCtrlReg(padapter, BIT(4), BIT(3));

		// Set FWHW_TXQ_CTRL 0x422[6]=0 to tell Hw the packet is not a real beacon frame.
		if (pHalData->RegFwHwTxQCtrl & BIT(6))
			bRecover = _TRUE;

		// To tell Hw the packet is not a real beacon frame.
		//U1bTmp = rtw_read8(padapter, REG_FWHW_TXQ_CTRL+2);
		rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl & ~BIT(6));
		pHalData->RegFwHwTxQCtrl &= ~BIT(6);
		SetFwRsvdPagePkt(padapter, 0);

		// 2010.05.11. Added by tynli.
//			SetBcnCtrlReg(padapter, BIT3, 0);
//			SetBcnCtrlReg(padapter, 0, BIT4);
		SetBcnCtrlReg(padapter, BIT(3), BIT(4));

		// To make sure that if there exists an adapter which would like to send beacon.
		// If exists, the origianl value of 0x422[6] will be 1, we should check this to
		// prevent from setting 0x422[6] to 0 after download reserved page, or it will cause
		// the beacon cannot be sent by HW.
		// 2010.06.23. Added by tynli.
		if(bRecover)
		{
			rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl | BIT(6));
			pHalData->RegFwHwTxQCtrl |= BIT(6);
		}

		// Clear CR[8] or beacon packet will not be send to TxBuf anymore.
		v8 = rtw_read8(padapter, REG_CR+1);
		v8 &= ~BIT(0); // ~ENSWBCN
		rtw_write8(padapter, REG_CR+1, v8);
	}

	JoinBssRptParm.OpMode = mstatus;
#ifdef CONFIG_WOWLAN
	JoinBssRptParm.MacID = 0;
#endif
	FillH2CCmd(padapter, JOINBSS_RPT_EID, sizeof(JoinBssRptParm), (u8 *)&JoinBssRptParm);

_func_exit_;
}

#ifdef CONFIG_BT_COEXIST
static void SetFwRsvdPagePkt_BTCoex(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	struct xmit_priv	*pxmitpriv;
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;
	u8	fakemac[6]={0x00,0xe0,0x4c,0x00,0x00,0x00};
	u32	BeaconLength, ProbeRspLength, PSPollLength;
	u32	NullDataLength, QosNullLength, BTQosNullLength;
	u8	*ReservedPagePacket;
	u8	PageNum, PageNeed, TxDescLen;
	u16	BufIndex;
	u32	TotalPacketLen;
	RSVDPAGE_LOC	RsvdPageLoc;


	DBG_871X("+%s\n", __FUNCTION__);

	ReservedPagePacket = (u8*)rtw_zmalloc(1024);
	if (ReservedPagePacket == NULL) {
		DBG_871X("%s: alloc ReservedPagePacket fail!\n", __FUNCTION__);
		return;
	}

	pHalData = GET_HAL_DATA(padapter);
	pxmitpriv = &padapter->xmitpriv;
	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	TxDescLen = TXDESC_SIZE;
	PageNum = 0;

	//3 (1) beacon
	BufIndex = TXDESC_OFFSET;
#if 0
	ConstructBeacon(padapter, &ReservedPagePacket[BufIndex], &BeaconLength);

	// When we count the first page size, we need to reserve description size for the RSVD
	// packet, it will be filled in front of the packet in TXPKTBUF.
	PageNeed = (u8)PageNum_128(TxDescLen + BeaconLength);
	// To reserved 2 pages for beacon buffer. 2010.06.24.
	if (PageNeed == 1)
		PageNeed += 1;
#else
	// skip Beacon Packet
	PageNeed = 3;
#endif

	PageNum += PageNeed;
	pHalData->FwRsvdPageStartOffset = PageNum;

	BufIndex += PageNeed*128;

	//3 (2) ps-poll
#if 0 // skip
	RsvdPageLoc.LocPsPoll = PageNum;
	ConstructPSPoll(padapter, &ReservedPagePacket[BufIndex], &PSPollLength);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], PSPollLength, _TRUE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + PSPollLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;
#endif

	//3 (3) null data
	RsvdPageLoc.LocNullData = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&NullDataLength,
		fakemac,
		_FALSE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], NullDataLength, _FALSE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + NullDataLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3 (4) probe response
#if 0 // skip
	RsvdPageLoc.LocProbeRsp = PageNum;
	ConstructProbeRsp(
		padapter,
		&ReservedPagePacket[BufIndex],
		&ProbeRspLength,
		get_my_bssid(&pmlmeinfo->network),
		_FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ProbeRspLength, _FALSE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + ProbeRspLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;
#endif

	//3 (5) Qos null data
#if 0 // skip
	RsvdPageLoc.LocQosNull = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&QosNullLength,
		get_my_bssid(&pmlmeinfo->network),
		_TRUE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], QosNullLength, _FALSE, _FALSE);

	PageNeed = (u8)PageNum_128(TxDescLen + QosNullLength);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;
#endif

	//3 (6) BT Qos null data
	RsvdPageLoc.LocBTQosNull = PageNum;
	ConstructNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&BTQosNullLength,
		fakemac,
		_TRUE, 0, 0, _FALSE);
	rtl8723a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], BTQosNullLength, _FALSE, _TRUE);

	TotalPacketLen = BufIndex + BTQosNullLength;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	// update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->qsel = 0x10;
	pattrib->pktlen = pattrib->last_txcmdsz = TotalPacketLen - TXDESC_OFFSET;
	_rtw_memcpy(pmgntframe->buf_addr, ReservedPagePacket, TotalPacketLen);

	rtw_hal_mgnt_xmit(padapter, pmgntframe);

	DBG_8192C("%s: Set RSVD page location to Fw\n", __FUNCTION__);
	FillH2CCmd(padapter, RSVD_PAGE_EID, sizeof(RsvdPageLoc), (u8*)&RsvdPageLoc);

exit:
	rtw_mfree(ReservedPagePacket, 1024);
}

void rtl8723a_set_BTCoex_AP_mode_FwRsvdPkt_cmd(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 bRecover = _FALSE;


	DBG_8192C("+%s\n", __FUNCTION__);

	pHalData = GET_HAL_DATA(padapter);

	// Set FWHW_TXQ_CTRL 0x422[6]=0 to tell Hw the packet is not a real beacon frame.
	if (pHalData->RegFwHwTxQCtrl & BIT(6))
		bRecover = _TRUE;

	// To tell Hw the packet is not a real beacon frame.
	pHalData->RegFwHwTxQCtrl &= ~BIT(6);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl);
	SetFwRsvdPagePkt_BTCoex(padapter);

	// To make sure that if there exists an adapter which would like to send beacon.
	// If exists, the origianl value of 0x422[6] will be 1, we should check this to
	// prevent from setting 0x422[6] to 0 after download reserved page, or it will cause
	// the beacon cannot be sent by HW.
	// 2010.06.23. Added by tynli.
	if (bRecover)
	{
		pHalData->RegFwHwTxQCtrl |= BIT(6);
		rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl);
	}
}
#endif

#ifdef CONFIG_P2P
void rtl8192c_set_p2p_ctw_period_cmd(_adapter* padapter, u8 ctwindow)
{
	struct P2P_PS_CTWPeriod_t p2p_ps_ctw;

	p2p_ps_ctw.CTWPeriod = ctwindow;

	FillH2CCmd(padapter, P2P_PS_CTW_CMD_EID, 1, (u8 *)(&p2p_ps_ctw));

}

void rtl8192c_set_p2p_ps_offload_cmd(_adapter* padapter, u8 p2p_ps_state)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct pwrctrl_priv		*pwrpriv = adapter_to_pwrctl(padapter);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	struct P2P_PS_Offload_t	*p2p_ps_offload = &pHalData->p2p_ps_offload;
	u8	i;
	u16	ctwindow;
	u32	start_time, tsf_low;

_func_enter_;

	switch(p2p_ps_state)
	{
		case P2P_PS_DISABLE:
			DBG_8192C("P2P_PS_DISABLE \n");
			_rtw_memset(p2p_ps_offload, 0 ,1);
			break;
		case P2P_PS_ENABLE:
			DBG_8192C("P2P_PS_ENABLE \n");
			// update CTWindow value.
			if( pwdinfo->ctwindow > 0 )
			{
				p2p_ps_offload->CTWindow_En = 1;
				ctwindow = pwdinfo->ctwindow;
				if(IS_HARDWARE_TYPE_8723A(padapter))
				{
					//rtw_write16(padapter, REG_ATIMWND, ctwindow);
				}
				else
				{
					rtl8192c_set_p2p_ctw_period_cmd(padapter, ctwindow);
				}
			}

			// hw only support 2 set of NoA
			for( i=0 ; i<pwdinfo->noa_num ; i++)
			{
				// To control the register setting for which NOA
				rtw_write8(padapter, 0x5CF, (i << 4));
				if(i == 0)
					p2p_ps_offload->NoA0_En = 1;
				else
					p2p_ps_offload->NoA1_En = 1;

				// config P2P NoA Descriptor Register
				rtw_write32(padapter, 0x5E0, pwdinfo->noa_duration[i]);

				rtw_write32(padapter, 0x5E4, pwdinfo->noa_interval[i]);

				//Get Current TSF value
				tsf_low = rtw_read32(padapter, REG_TSFTR);

				start_time = pwdinfo->noa_start_time[i];
				if(pwdinfo->noa_count[i] != 1)
				{
					while( start_time <= (tsf_low+(50*1024) ) )
					{
						start_time += pwdinfo->noa_interval[i];
						if(pwdinfo->noa_count[i] != 255)
							pwdinfo->noa_count[i]--;
					}
				}
				//DBG_8192C("%s(): start_time = %x\n",__FUNCTION__,start_time);
				rtw_write32(padapter, 0x5E8, start_time);

				rtw_write8(padapter, 0x5EC, pwdinfo->noa_count[i]);
			}

			if( (pwdinfo->opp_ps == 1) || (pwdinfo->noa_num > 0) )
			{
				// rst p2p circuit
				rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(4));

				p2p_ps_offload->Offload_En = 1;

				if(rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO))
				{
					p2p_ps_offload->role= 1;
					p2p_ps_offload->AllStaSleep = 0;
				}
				else
				{
					p2p_ps_offload->role= 0;
				}

				p2p_ps_offload->discovery = 0;
			}
			break;
		case P2P_PS_SCAN:
			DBG_8192C("P2P_PS_SCAN \n");
			p2p_ps_offload->discovery = 1;
			break;
		case P2P_PS_SCAN_DONE:
			DBG_8192C("P2P_PS_SCAN_DONE \n");
			p2p_ps_offload->discovery = 0;
			pwdinfo->p2p_ps = P2P_PS_ENABLE;
			break;
		default:
			break;
	}

	FillH2CCmd(padapter, P2P_PS_OFFLOAD_EID, 1, (u8 *)p2p_ps_offload);

_func_exit_;

}
#endif //CONFIG_P2P

#ifdef CONFIG_IOL
#include <rtw_iol.h>
int rtl8192c_IOL_exec_cmds_sync(ADAPTER *adapter, struct xmit_frame *xmit_frame, u32 max_wating_ms)
{
	IO_OFFLOAD_LOC	IoOffloadLoc;
	u32 start_time = rtw_get_current_time();
	u32 passing_time_ms;
	u8 polling_ret;
	int ret = _FAIL;

	if (rtw_IOL_append_END_cmd(xmit_frame) != _SUCCESS)
		goto exit;

	dump_mgntframe_and_wait(adapter, xmit_frame, max_wating_ms);

	IoOffloadLoc.LocCmd = 0;
	if(_SUCCESS != FillH2CCmd(adapter, H2C_92C_IO_OFFLOAD, sizeof(IO_OFFLOAD_LOC), (u8 *)&IoOffloadLoc))
		goto exit;

	//polling if the IO offloading is done
	while( (passing_time_ms=rtw_get_passing_time_ms(start_time)) <= max_wating_ms) {
		#if 0 //C2H
		if(0xff == rtw_read8(adapter, REG_C2HEVT_CLEAR))
			break;
		#else// 0x1c3
		if(0x00 != (polling_ret=rtw_read8(adapter, 0x1c3)))
			break;
		#endif
		rtw_msleep_os(5);
	}
	#if 0 //debug
	DBG_871X("IOL %s, polling_ret:0x%02x, 0x1c0=0x%08x, 0x1c4=0x%08x, 0x1cc=0x%08x, 0x1e8=0x%08x, 0x130=0x%08x, 0x134=0x%08x\n"
			, polling_ret==0xff?"success":"error"
			, polling_ret
			, rtw_read32(adapter, 0x1c0)
			, rtw_read32(adapter, 0x1c4)
			, rtw_read32(adapter, 0x1cc)
			, rtw_read32(adapter, 0x1e8)
			, rtw_read32(adapter, 0x130)
			, rtw_read32(adapter, 0x134)
	);
	rtw_write32(adapter, 0x1c0, 0x0);
	#endif

	if(polling_ret == 0xff)
		ret =_SUCCESS;
	else {
		DBG_871X("IOL %s, polling_ret:0x%02x\n"
			//", 0x1c0=0x%08x, 0x1c4=0x%08x, 0x1cc=0x%08x, 0x1e8=0x%08x, 0x130=0x%08x, 0x134=0x%08x\n"
			, polling_ret==0xff?"success":"error"
			, polling_ret
			//, rtw_read32(adapter, 0x1c0)
			//, rtw_read32(adapter, 0x1c4)
			//, rtw_read32(adapter, 0x1cc)
			//, rtw_read32(adapter, 0x1e8)
			//, rtw_read32(adapter, 0x130)
			//, rtw_read32(adapter, 0x134)
		);
		#if 0 //debug
		rtw_write16(adapter, 0x1c4, 0x0000);
		rtw_msleep_os(10);
		DBG_871X("after reset, 0x1c4=0x%08x\n", rtw_read32(adapter, 0x1c4));
		#endif

	}

	{
		#if 0 //C2H
		u32 c2h_evt;
		int i;
		c2h_evt = rtw_read32(adapter, REG_C2HEVT_MSG_NORMAL);
		DBG_871X("%s io-offloading complete, in %ums: 0x%08x\n", __FUNCTION__, passing_time_ms, c2h_evt);
		rtw_write8(adapter, REG_C2HEVT_CLEAR, 0x0);
		#else// 0x1c3
		//DBG_871X("%s IOF complete in %ums\n", __FUNCTION__, passing_time_ms);
		rtw_write8(adapter, 0x1c3, 0x0);
		#endif
	}

exit:
	return ret;

}
#endif //CONFIG_IOL

#ifdef CONFIG_TSF_RESET_OFFLOAD
/*
	ask FW to Reset sync register at Beacon early interrupt
*/
u8 rtl8723c_reset_tsf(_adapter *padapter, u8 reset_port )
{
	u8	buf[2];
	u8	res=_SUCCESS;

_func_enter_;
	if (IFACE_PORT0==reset_port) {
		buf[0] = 0x1; buf[1] = 0;

	} else{
		buf[0] = 0x0; buf[1] = 0x1;
	}
	FillH2CCmd(padapter, H2C_RESET_TSF, 2, buf);
_func_exit_;

	return res;
}
#endif	// CONFIG_TSF_RESET_OFFLOAD
#ifdef CONFIG_WOWLAN
void rtl8723as_set_wowlan_cmd(_adapter* padapter, u8 enable)
{
	u8		res=_SUCCESS;
	u32		test=0;
	u16 		encry_algo = 0;
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	SETWOWLAN_PARM		pwowlan_parm;
	struct pwrctrl_priv	*pwrpriv=adapter_to_pwrctl(padapter);
	struct security_priv	*psecuritypriv = &padapter->securitypriv;

_func_enter_;
		DBG_871X_LEVEL(_drv_always_, "+%s+\n", __func__);

		pwowlan_parm.mode =0;
		pwowlan_parm.gpio_index=0;
		pwowlan_parm.gpio_duration=0;
		pwowlan_parm.second_mode =0;
		pwowlan_parm.reserve=0;

		if(enable){

			pwowlan_parm.mode |=FW_WOWLAN_FUN_EN;
			pwrpriv->wowlan_magic =_TRUE;
			//pwrpriv->wowlan_unicast =_TRUE;
			pwrpriv->wowlan_pattern =_TRUE;

			pwowlan_parm.second_mode |= BIT4; //IPv4 Unicast pkt wakeup
			pwowlan_parm.second_mode |= BIT3; //pairwisekey wakeup

			if(pwrpriv->wowlan_pattern ==_TRUE){
				pwowlan_parm.mode |= FW_WOWLAN_PATTERN_MATCH;
				DBG_871X_LEVEL(_drv_info_, "%s 2.pwowlan_parm.mode=0x%x \n",__FUNCTION__,pwowlan_parm.mode );
			}
			if(pwrpriv->wowlan_magic ==_TRUE){
				pwowlan_parm.mode |=FW_WOWLAN_MAGIC_PKT;
				DBG_871X_LEVEL(_drv_info_, "%s 3.pwowlan_parm.mode=0x%x \n",__FUNCTION__,pwowlan_parm.mode );
			}
			if(pwrpriv->wowlan_unicast ==_TRUE){
				pwowlan_parm.mode |=FW_WOWLAN_UNICAST;
				DBG_871X_LEVEL(_drv_info_, "%s 4.pwowlan_parm.mode=0x%x \n",__FUNCTION__,pwowlan_parm.mode );
			}

			//SEC Type:
			encry_algo = (u16)(psecuritypriv->dot11PrivacyAlgrthm |(psecuritypriv->dot118021XGrpPrivacy << 8));
			DBG_871X_LEVEL(_drv_info_, "%s psecuritypriv->dot11PrivacyAlgrthm = 0x%x ,psecuritypriv->dot118021XGrpPrivacy:%d\n",__FUNCTION__,psecuritypriv->dot11PrivacyAlgrthm,psecuritypriv->dot118021XGrpPrivacy);
			res = FillH2CCmd(padapter, SEC_TYPE_EID, 2, (u8 *)&encry_algo);

			DBG_871X_LEVEL(_drv_info_, "%s pwrpriv->wowlan_wake_reason=0x%x \n",__FUNCTION__,pwrpriv->wowlan_wake_reason);
			if(!(pwrpriv->wowlan_wake_reason & FWDecisionDisconnect))
				rtl8723a_set_FwJoinBssReport_cmd(padapter, 1);
			else
				DBG_871X_LEVEL(_drv_always_, "%s, disconnected, no FwJoinBssReport\n",__FUNCTION__);
			rtw_msleep_os(2);



			//WOWLAN_GPIO_ACTIVE means GPIO high active
			//pwowlan_parm.mode |=FW_WOWLAN_GPIO_ACTIVE;
			//pwowlan_parm.mode |=FW_WOWLAN_REKEY_WAKEUP;
			pwowlan_parm.mode |=FW_WOWLAN_DEAUTH_WAKEUP;
			//pwowlan_parm.mode |=FW_WOWLAN_ALL_PKT_DROP;

			//DataPinWakeUp
			//pwowlan_parm.gpio_index=0x80;

			DBG_871X_LEVEL(_drv_info_, "%s 5.pwowlan_parm.mode=0x%x \n",__FUNCTION__,pwowlan_parm.mode);
			DBG_871X_LEVEL(_drv_info_, "%s 6.pwowlan_parm.index=0x%x \n",__FUNCTION__,pwowlan_parm.gpio_index);

			res = FillH2CCmd(padapter, H2C_COM_WWLAN , 5, (u8 *)&pwowlan_parm);

			rtw_msleep_os(2);

			//disconnect decision
			pwowlan_parm.mode =3;
			pwowlan_parm.gpio_index=15;
			pwowlan_parm.gpio_duration=3;
			FillH2CCmd(padapter, H2C_COM_DISCNT_DECISION, 3, (u8 *)&pwowlan_parm);

			//keep alive period = 10 * 10 BCN interval
			pwowlan_parm.mode =1;
			pwowlan_parm.gpio_index=10;

			res = FillH2CCmd(padapter, H2C_COM_KEEP_ALIVE, 2, (u8 *)&pwowlan_parm);

			rtw_msleep_os(2);

			//GTK offload
			pwowlan_parm.mode =0;

			res = FillH2CCmd(padapter, H2C_COM_GTK_OFFLOAD, 1, (u8 *)&pwowlan_parm);

			rtw_msleep_os(2);

			//enable Remote wake ctrl
			pwowlan_parm.mode = 1;
			pwowlan_parm.gpio_index=0;
			pwowlan_parm.gpio_duration=0;

			res = FillH2CCmd(padapter, H2C_COM_REMOTE_WAKE_CTRL, 3, (u8 *)&pwowlan_parm);
		} else {
			pwrpriv->wowlan_magic =_FALSE;
			res = FillH2CCmd(padapter, H2C_COM_REMOTE_WAKE_CTRL, 3, (u8 *)&pwowlan_parm);
			rtw_msleep_os(2);
			res = FillH2CCmd(padapter, H2C_COM_WWLAN, 2, (u8 *)&pwowlan_parm);
		}
_func_exit_;
		DBG_871X_LEVEL(_drv_always_, "-%s res:%d-\n", __func__, res);
		return ;
}
#endif  //CONFIG_WOWLAN

