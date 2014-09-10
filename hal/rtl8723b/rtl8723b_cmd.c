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
#define _RTL8723B_CMD_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <cmd_osdep.h>
#include <mlme_osdep.h>
#include <rtw_byteorder.h>
#include <circ_buf.h>
#include <rtw_ioctl_set.h>

#include <rtl8723b_hal.h>

#define MAX_H2C_BOX_NUMS	4
#define MESSAGE_BOX_SIZE		4

#define RTL8723B_MAX_CMD_LEN	7
#define RTL8723B_EX_MESSAGE_BOX_SIZE	4

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
*| 31 - 8		|7-5	| 4 - 0	|
*| h2c_msg 	|Class	|CMD_ID	|
*| 31-0						|
*| Ext msg					|
*
******************************************/
s32 FillH2CCmd8723B(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer)
{
	u8 bcmd_down = _FALSE;
	s32 retry_cnts = 100;
	u8 h2c_box_num;
	u32	msgbox_addr;
	u32 msgbox_ex_addr;
	PHAL_DATA_TYPE pHalData;
	u32	h2c_cmd = 0;
	u32	h2c_cmd_ex = 0;
	s32 ret = _FAIL;

_func_enter_;

	padapter = GET_PRIMARY_ADAPTER(padapter);
	pHalData = GET_HAL_DATA(padapter);

	_enter_critical_mutex(&(adapter_to_dvobj(padapter)->h2c_fwcmd_mutex), NULL);

	if (!pCmdBuffer) {
		goto exit;
	}
	if(CmdLen > RTL8723B_MAX_CMD_LEN) {
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
			_rtw_memcpy((u8*)(&h2c_cmd)+1, pCmdBuffer, 3);
			_rtw_memcpy((u8*)(&h2c_cmd_ex), pCmdBuffer+3, CmdLen-3);
		}

		*(u8*)(&h2c_cmd) |= ElementID;

#ifdef CONFIG_LPS_LCLK
		//if (ElementID != H2C_8723A_LOWPWR_LPS) {
			if (rtw_register_h2c_alive(padapter) != _SUCCESS) {
				DBG_871X("%s ERROR rtw_register_h2c_alive fail\n", __func__);
			}
		//}
#endif
		if(CmdLen>3){
			msgbox_ex_addr = REG_HMEBOX_EXT0_8723B + (h2c_box_num *RTL8723B_EX_MESSAGE_BOX_SIZE);
			h2c_cmd_ex = le32_to_cpu( h2c_cmd_ex );
			rtw_write32(padapter, msgbox_ex_addr, h2c_cmd_ex);
		}
		msgbox_addr =REG_HMEBOX_0 + (h2c_box_num *MESSAGE_BOX_SIZE);
		h2c_cmd = le32_to_cpu( h2c_cmd );
		rtw_write32(padapter,msgbox_addr, h2c_cmd);

		bcmd_down = _TRUE;

		DBG_8192C("MSG_BOX:%d, CmdLen(%d), CmdID(0x%x), reg:0x%x =>h2c_cmd:0x%.8x, reg:0x%x =>h2c_cmd_ex:0x%.8x\n"
		 	,pHalData->LastHMEBoxNum , CmdLen, ElementID, msgbox_addr, h2c_cmd, msgbox_ex_addr, h2c_cmd_ex);

		pHalData->LastHMEBoxNum = (h2c_box_num+1) % MAX_H2C_BOX_NUMS;

#ifdef CONFIG_LPS_LCLK
		//if (ElementID != H2C_8723A_LOWPWR_LPS) {
			rtw_unregister_h2c_alive(padapter);
		//}
#endif
	}while((!bcmd_down) && (retry_cnts--));

	ret = _SUCCESS;

exit:

	_exit_critical_mutex(&(adapter_to_dvobj(padapter)->h2c_fwcmd_mutex), NULL);

_func_exit_;

	return ret;
}

static void ConstructBeacon(_adapter *padapter, u8 *pframe, u32 *pLength)
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

static void ConstructPSPoll(_adapter *padapter, u8 *pframe, u32 *pLength)
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

static void ConstructNullFunctionData(
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
static void ConstructARPResponse(
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

static void ConstructProbeRsp(_adapter *padapter, u8 *pframe, u32 *pLength, u8 *StaAddr, BOOLEAN bHideSSID)
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

void rtl8723b_set_FwRsvdPage_cmd(PADAPTER padapter, PRSVDPAGE_LOC rsvdpageloc)
{
	u8 u1H2CRsvdPageParm[H2C_8723B_RSVDPAGE_LOC_LEN]={0};
	u8 u1H2CAoacRsvdPageParm[H2C_8723B_AOAC_RSVDPAGE_LOC_LEN]={0};

	DBG_871X("8723BRsvdPageLoc: ProbeRsp=%d PsPoll=%d Null=%d QoSNull=%d BTNull=%d\n",
						rsvdpageloc->LocProbeRsp, rsvdpageloc->LocPsPoll, rsvdpageloc->LocNullData, rsvdpageloc->LocQosNull, rsvdpageloc->LocBTQosNull);
	SET_8723B_H2CCMD_RSVDPAGE_LOC_PROBE_RSP(u1H2CRsvdPageParm, rsvdpageloc->LocProbeRsp);
	SET_8723B_H2CCMD_RSVDPAGE_LOC_PSPOLL(u1H2CRsvdPageParm, rsvdpageloc->LocPsPoll);
	SET_8723B_H2CCMD_RSVDPAGE_LOC_NULL_DATA(u1H2CRsvdPageParm, rsvdpageloc->LocNullData);
	SET_8723B_H2CCMD_RSVDPAGE_LOC_QOS_NULL_DATA(u1H2CRsvdPageParm, rsvdpageloc->LocQosNull);
	SET_8723B_H2CCMD_RSVDPAGE_LOC_BT_QOS_NULL_DATA(u1H2CRsvdPageParm, rsvdpageloc->LocBTQosNull);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CRsvdPageParm:", u1H2CRsvdPageParm, H2C_8723B_RSVDPAGE_LOC_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_RSVD_PAGE, H2C_8723B_RSVDPAGE_LOC_LEN, u1H2CRsvdPageParm);

#ifdef CONFIG_WOWLAN
	DBG_871X("8723BAOACRsvdPageLoc: RWC=%d ArpRsp=%d NbrAdv=%d GtkRsp=%d GtkInfo=%d ProbeReq=%d NetworkList=%d\n",
						rsvdpageloc->LocRemoteCtrlInfo, rsvdpageloc->LocArpRsp, rsvdpageloc->LocNbrAdv, rsvdpageloc->LocGTKRsp, rsvdpageloc->LocGTKInfo, rsvdpageloc->LocProbeReq, rsvdpageloc->LocNetList);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_REMOTE_WAKE_CTRL_INFO(u1H2CAoacRsvdPageParm, rsvdpageloc->LocRemoteCtrlInfo);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_ARP_RSP(u1H2CAoacRsvdPageParm, rsvdpageloc->LocArpRsp);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_NEIGHBOR_ADV(u1H2CAoacRsvdPageParm, rsvdpageloc->LocNbrAdv);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_RSP(u1H2CAoacRsvdPageParm, rsvdpageloc->LocGTKRsp);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_INFO(u1H2CAoacRsvdPageParm, rsvdpageloc->LocGTKInfo);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_PROBE_REQ(u1H2CAoacRsvdPageParm, rsvdpageloc->LocProbeReq);
	SET_8723B_H2CCMD_AOAC_RSVDPAGE_LOC_NETWORK_LIST(u1H2CAoacRsvdPageParm, rsvdpageloc->LocNetList);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CAoacRsvdPageParm:", u1H2CAoacRsvdPageParm, H2C_8723B_AOAC_RSVDPAGE_LOC_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_AOAC_RSVD_PAGE, H2C_8723B_AOAC_RSVDPAGE_LOC_LEN, u1H2CAoacRsvdPageParm);
#endif
}

void rtl8723b_set_FwMediaStatusRpt_cmd(PADAPTER	padapter, u8 mstatus, u8 macid)
{
	u8 u1H2CMediaStatusRptParm[H2C_8723B_MEDIA_STATUS_RPT_LEN]={0};
	u8 macid_end = 0;

	DBG_871X("%s(): mstatus = %d macid=%d\n", __func__, mstatus, macid);

	SET_8723B_H2CCMD_MSRRPT_PARM_OPMODE(u1H2CMediaStatusRptParm, mstatus);
	SET_8723B_H2CCMD_MSRRPT_PARM_MACID_IND(u1H2CMediaStatusRptParm, 0);
	SET_8723B_H2CCMD_MSRRPT_PARM_MACID(u1H2CMediaStatusRptParm, macid);
	SET_8723B_H2CCMD_MSRRPT_PARM_MACID_END(u1H2CMediaStatusRptParm, macid_end);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CMediaStatusRptParm:", u1H2CMediaStatusRptParm, H2C_8723B_MEDIA_STATUS_RPT_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_MEDIA_STATUS_RPT, H2C_8723B_MEDIA_STATUS_RPT_LEN, u1H2CMediaStatusRptParm);
}

static void rtl8723b_set_FwKeepAlive_cmd(PADAPTER padapter, u8 benable)
{
	u8 u1H2CKeepAliveParm[H2C_8723B_KEEP_ALIVE_CTRL_LEN]={0};
	u8 adopt = 1, check_period = 5;  //NullSendPeriod = check_period*10*300ms

	DBG_871X("%s(): benable = %d\n", __func__, benable);
	SET_8723B_H2CCMD_KEEPALIVE_PARM_ENABLE(u1H2CKeepAliveParm, benable);
	SET_8723B_H2CCMD_KEEPALIVE_PARM_ADOPT(u1H2CKeepAliveParm, adopt);
	SET_8723B_H2CCMD_KEEPALIVE_PARM_CHECK_PERIOD(u1H2CKeepAliveParm, check_period);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CKeepAliveParm:", u1H2CKeepAliveParm, H2C_8723B_KEEP_ALIVE_CTRL_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_KEEP_ALIVE, H2C_8723B_KEEP_ALIVE_CTRL_LEN, u1H2CKeepAliveParm);
}

static void rtl8723b_set_FwDisconDecision_cmd(PADAPTER padapter, u8 benable)
{
	u8 u1H2CDisconDecisionParm[H2C_8723B_DISCON_DECISION_LEN]={0};
	u8 adopt = 1;
	u8 check_period = 15;  //15*300ms
	u8 trypkt_num = 3;

	DBG_871X("%s(): benable = %d\n", __func__, benable);
	SET_8723B_H2CCMD_DISCONDECISION_PARM_ENABLE(u1H2CDisconDecisionParm, benable);
	SET_8723B_H2CCMD_DISCONDECISION_PARM_ADOPT(u1H2CDisconDecisionParm, adopt);
	SET_8723B_H2CCMD_DISCONDECISION_PARM_CHECK_PERIOD(u1H2CDisconDecisionParm, check_period);
	SET_8723B_H2CCMD_DISCONDECISION_PARM_TRY_PKT_NUM(u1H2CDisconDecisionParm, trypkt_num);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CDisconDecisionParm:", u1H2CDisconDecisionParm, H2C_8723B_DISCON_DECISION_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_DISCON_DECISION, H2C_8723B_DISCON_DECISION_LEN, u1H2CDisconDecisionParm);
}

void rtl8723b_set_FwMacIdConfig_cmd(_adapter* padapter, u8 mac_id, u8 raid, u8 bw, u32 mask)
{
	u8 u1H2CMacIdConfigParm[H2C_8723B_MACID_CFG_LEN]={0};

	DBG_871X("%s(): mac_id=%d raid=0x%x bw=%d mask=0x%x\n", __func__, mac_id, raid, bw, mask);

_func_enter_;

	SET_8723B_H2CCMD_MACID_CFG_MACID(u1H2CMacIdConfigParm, mac_id);
	SET_8723B_H2CCMD_MACID_CFG_RAID(u1H2CMacIdConfigParm, raid);
	SET_8723B_H2CCMD_MACID_CFG_SGI_EN(u1H2CMacIdConfigParm, (raid&0x80)?1:0);
	SET_8723B_H2CCMD_MACID_CFG_BW(u1H2CMacIdConfigParm, bw);
	SET_8723B_H2CCMD_MACID_CFG_RATE_MASK0(u1H2CMacIdConfigParm, (u8)(mask & 0x000000ff));
	SET_8723B_H2CCMD_MACID_CFG_RATE_MASK1(u1H2CMacIdConfigParm, (u8)((mask & 0x0000ff00) >>8));
	SET_8723B_H2CCMD_MACID_CFG_RATE_MASK2(u1H2CMacIdConfigParm, (u8)((mask & 0x00ff0000) >> 16));
	SET_8723B_H2CCMD_MACID_CFG_RATE_MASK3(u1H2CMacIdConfigParm, (u8)((mask & 0xff000000) >> 24));

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CMacIdConfigParm:", u1H2CMacIdConfigParm, H2C_8723B_MACID_CFG_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_MACID_CFG, H2C_8723B_MACID_CFG_LEN, u1H2CMacIdConfigParm);

_func_exit_;
}

void rtl8723b_set_FwRssiSetting_cmd(_adapter*padapter, u8 *param)
{
	u8 u1H2CRssiSettingParm[H2C_8723B_RSSI_SETTING_LEN]={0};
	u8 mac_id = *param;
	u8 rssi = *(param+2);
	u8 uldl_state = 0;

_func_enter_;
	DBG_871X("%s(): param=%.2x-%.2x-%.2x\n", __func__, *param, *(param+1), *(param+2));
	DBG_871X("%s(): mac_id=%d rssi=%d\n", __func__, mac_id, rssi);

	SET_8723B_H2CCMD_RSSI_SETTING_MACID(u1H2CRssiSettingParm, mac_id);
	SET_8723B_H2CCMD_RSSI_SETTING_RSSI(u1H2CRssiSettingParm, rssi);
	SET_8723B_H2CCMD_RSSI_SETTING_ULDL_STATE(u1H2CRssiSettingParm, uldl_state);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CRssiSettingParm:", u1H2CRssiSettingParm, H2C_8723B_RSSI_SETTING_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_RSSI_SETTING, H2C_8723B_RSSI_SETTING_LEN, u1H2CRssiSettingParm);

_func_exit_;
}

void rtl8723b_set_FwAPReqRPT_cmd(PADAPTER padapter, u32 need_ack)
{
	u8 u1H2CApReqRptParm[H2C_8723B_AP_REQ_TXRPT_LEN]={0};
	u8 macid1 = 1, macid2 = 0;

	DBG_871X("%s(): need_ack = %d\n", __func__, need_ack);

	SET_8723B_H2CCMD_APREQRPT_PARM_MACID1(u1H2CApReqRptParm, macid1);
	SET_8723B_H2CCMD_APREQRPT_PARM_MACID2(u1H2CApReqRptParm, macid2);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CApReqRptParm:", u1H2CApReqRptParm, H2C_8723B_AP_REQ_TXRPT_LEN);
	FillH2CCmd8723B(padapter, H2C_8723B_AP_REQ_TXRPT, H2C_8723B_AP_REQ_TXRPT_LEN, u1H2CApReqRptParm);
}

void rtl8723b_set_FwAntSelReverse_cmd(PADAPTER padapter)
{
	u1Byte H2C_Parameter[5] ={0};
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(padapter);

	if (pHalData->bUseExtSPDT) {
#ifdef CONFIG_ANTENNA_AT_AUX_PORT
		H2C_Parameter[0] = 0x01;
#else
		H2C_Parameter[0] = 0x00;
#endif
		H2C_Parameter[1] = 0x01;
	} else {
		H2C_Parameter[0] = 0x00;
		H2C_Parameter[1] = 0x00;
	}

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "H2C_Parameter:", H2C_Parameter, 5);

	FillH2CCmd8723B(padapter, H2C_8723B_ANT_SEL_RSV, 5, H2C_Parameter);
}

void rtl8723b_set_FwGpioCtrl_cmd(PADAPTER padapter, u8 gpio_num, u8 high_active)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 u1H2CGpioCtrlParm[5]={0};
	u8 gpio_dur = 0xff, c2h_dur = 0x00;

_func_enter_;
	DBG_871X("%s gpio_num:%d high_active:%d\n", __func__, gpio_num, high_active);

	SET_8723B_H2CCMD_GPIOCTRL_PARM_GPIOIDX(u1H2CGpioCtrlParm, gpio_num);
	SET_8723B_H2CCMD_GPIOCTRL_PARM_C2HEN(u1H2CGpioCtrlParm, 0);
	SET_8723B_H2CCMD_GPIOCTRL_PARM_PULSE(u1H2CGpioCtrlParm, 0);//1);
	SET_8723B_H2CCMD_GPIOCTRL_PARM_HIGH_ACTIVE(u1H2CGpioCtrlParm, high_active);
	SET_8723B_H2CCMD_GPIOCTRL_PARM_EN(u1H2CGpioCtrlParm, 1);

	SET_8723B_H2CCMD_GPIOCTRL_PARM_DUR(u1H2CGpioCtrlParm, gpio_dur);

	SET_8723B_H2CCMD_GPIOCTRL_PARM_C2HDUR(u1H2CGpioCtrlParm, c2h_dur);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CGpioCtrlParm:", u1H2CGpioCtrlParm, 3);

	FillH2CCmd8723B(padapter, H2C_8723B_GPIO_CTRL, 3, u1H2CGpioCtrlParm);
_func_exit_;
}

void rtl8723b_set_FwPwrMode_cmd(PADAPTER padapter, u8 psmode)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 u1H2CPwrModeParm[H2C_8723B_PWRMODE_LEN]={0};
	u8 PowerState=0, awake_intvl = 1, byte5 = pwrpriv->bcn_ant_mode, rlbm = 0;

_func_enter_;
	DBG_871X("%s(): FW LPS mode = %d\n", __func__, psmode);

	SET_8723B_H2CCMD_PWRMODE_PARM_MODE(u1H2CPwrModeParm, (psmode>0)?1:0);
	SET_8723B_H2CCMD_PWRMODE_PARM_SMART_PS(u1H2CPwrModeParm, pwrpriv->smart_ps);
#ifdef CONFIG_WOWLAN
	if(psmode == PS_MODE_SELF_DEFINED)  //For WOWLAN LPS
	{
		awake_intvl = 4;
		rlbm = 2;
	}
	else
	{
		awake_intvl = 1;
		rlbm = 0;
	}
#endif
	SET_8723B_H2CCMD_PWRMODE_PARM_RLBM(u1H2CPwrModeParm, rlbm);
	SET_8723B_H2CCMD_PWRMODE_PARM_BCN_PASS_TIME(u1H2CPwrModeParm, awake_intvl);
	SET_8723B_H2CCMD_PWRMODE_PARM_ALL_QUEUE_UAPSD(u1H2CPwrModeParm, padapter->registrypriv.uapsd_enable);

	if(psmode == PS_MODE_ACTIVE) //active
		PowerState = 0x0C;// AllON(0x0C), RFON(0x04), RFOFF(0x00)
#if 0 //TDMA + this will cause iperf Rx very very low, we will change it back after ok
	else if(psmode == PM_Radio_On)
		PowerState = 0x4;// AllON(0x0C), RFON(0x04), RFOFF(0x00)
#endif
	else
		PowerState = 0x0;// AllON(0x0C), RFON(0x04), RFOFF(0x00)
	SET_8723B_H2CCMD_PWRMODE_PARM_PWR_STATE(u1H2CPwrModeParm, PowerState);

	/* BIT6 if set 1, fw will not set ant & PAT based */
	/* on Byte5 under recv PwrMode H2C, we should always set it to 1 */
	/* BIT4 to close SmartPs issue pspoll(1) after NULL1 under TDMA */
	byte5 |= BIT(6);
#if 0
	if (psmode == PS_MODE_ACTIVE) {
		byte5 |= BIT(6);
	} else {
		if (byte5 == BIT(4))//TDMA case
			byte5 |= BIT(6);
	}
#endif

	SET_8723B_H2CCMD_PWRMODE_PARM_BYTE5(u1H2CPwrModeParm, byte5);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CPwrModeParm:", u1H2CPwrModeParm, H2C_8723B_PWRMODE_LEN);
	BT_RecordPwrMode(padapter, &u1H2CPwrModeParm[0], 6);
	FillH2CCmd8723B(padapter, H2C_8723B_SET_PWR_MODE, H2C_8723B_PWRMODE_LEN, u1H2CPwrModeParm);
_func_exit_;
}

void rtl8723b_set_FwPsTuneParam_cmd(PADAPTER padapter)
{
	u8 u1H2CPsTuneParm[H2C_8723B_PSTUNEPARAM_LEN]={0};
	u8 bcn_to_limit = 15; //15* 100ms * awakeinterval
	u8 dtim_timeout = 5; //ms //wait broadcast data timer
	u8 ps_timeout = 20;  //ms //Keep awake when tx
	u8 dtim_period = 3;

_func_enter_;
	//DBG_871X("%s(): FW LPS mode = %d\n", __func__, psmode);

	SET_8723B_H2CCMD_PSTUNE_PARM_BCN_TO_LIMIT(u1H2CPsTuneParm, bcn_to_limit);
	SET_8723B_H2CCMD_PSTUNE_PARM_DTIM_TIMEOUT(u1H2CPsTuneParm, dtim_timeout);
	SET_8723B_H2CCMD_PSTUNE_PARM_PS_TIMEOUT(u1H2CPsTuneParm, ps_timeout);
	SET_8723B_H2CCMD_PSTUNE_PARM_ADOPT(u1H2CPsTuneParm, 1);
	SET_8723B_H2CCMD_PSTUNE_PARM_DTIM_PERIOD(u1H2CPsTuneParm, dtim_period);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CPsTuneParm:", u1H2CPsTuneParm, H2C_8723B_PSTUNEPARAM_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_PS_TUNING_PARA, H2C_8723B_PSTUNEPARAM_LEN, u1H2CPsTuneParm);
_func_exit_;
}

void rtl8723b_set_FwBtMpOper_cmd(PADAPTER padapter, u8 idx, u8 ver, u8 reqnum, u8 param1)
{
	u8 u1H2CBtMpOperParm[H2C_8723B_BTMP_OPER_LEN]={0};

_func_enter_;
	DBG_871X("%s(): idx=%d ver=%d param1=%d param2=%d\n", __func__, idx, ver, reqnum, param1);

	SET_8723B_H2CCMD_BT_MPOPER_VER(u1H2CBtMpOperParm, ver);
	SET_8723B_H2CCMD_BT_MPOPER_REQNUM(u1H2CBtMpOperParm, reqnum);
	SET_8723B_H2CCMD_BT_MPOPER_IDX(u1H2CBtMpOperParm, idx);
	SET_8723B_H2CCMD_BT_MPOPER_PARAM1(u1H2CBtMpOperParm, param1);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CBtMpOperParm:", u1H2CBtMpOperParm, H2C_8723B_BTMP_OPER_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_BT_MP_OPER, H2C_8723B_BTMP_OPER_LEN, u1H2CBtMpOperParm);
_func_exit_;
}

#ifdef CONFIG_WOWLAN
static void rtl8723b_set_FwWoWlanCtrl_Cmd(PADAPTER padapter, u8 bFuncEn)
{
	u8 u1H2CWoWlanCtrlParm[H2C_8723B_WOWLAN_LEN]={0};
	u8 discont_wake = 1, gpionum = 0, gpio_dur = 0;

#ifdef PLATFORM_LEADCORE_1813
	gpionum = 12;
#endif

	DBG_871X("%s(): bFuncEn=%d\n", __func__, bFuncEn);

	SET_8723B_H2CCMD_WOWLAN_FUNC_ENABLE(u1H2CWoWlanCtrlParm, bFuncEn);

	SET_8723B_H2CCMD_WOWLAN_PATTERN_MATCH_ENABLE(u1H2CWoWlanCtrlParm, 1);
	SET_8723B_H2CCMD_WOWLAN_MAGIC_PKT_ENABLE(u1H2CWoWlanCtrlParm, 1);

	SET_8723B_H2CCMD_WOWLAN_UNICAST_PKT_ENABLE(u1H2CWoWlanCtrlParm, 0);
	SET_8723B_H2CCMD_WOWLAN_ALL_PKT_DROP(u1H2CWoWlanCtrlParm, 0);
	SET_8723B_H2CCMD_WOWLAN_GPIO_ACTIVE(u1H2CWoWlanCtrlParm, 0);
	SET_8723B_H2CCMD_WOWLAN_DISCONNECT_WAKE_UP(u1H2CWoWlanCtrlParm, discont_wake);
	SET_8723B_H2CCMD_WOWLAN_GPIONUM(u1H2CWoWlanCtrlParm, gpionum);
#if (defined CONFIG_GPIO_WAKEUP)
	SET_8723B_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(u1H2CWoWlanCtrlParm, 0);
#else
	SET_8723B_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(u1H2CWoWlanCtrlParm, 1);
#endif
	SET_8723B_H2CCMD_WOWLAN_GPIO_DURATION(u1H2CWoWlanCtrlParm, gpio_dur);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CWoWlanCtrlParm:", u1H2CWoWlanCtrlParm, H2C_8723B_WOWLAN_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_WOWLAN, H2C_8723B_WOWLAN_LEN, u1H2CWoWlanCtrlParm);
}

static void rtl8723b_set_FwRemoteWakeCtrl_Cmd(PADAPTER padapter, u8 benable)
{
	u8 u1H2CRemoteWakeCtrlParm[H2C_8723B_REMOTE_WAKE_CTRL_LEN]={0};

	DBG_871X("%s(): Enable=%d\n", __func__, benable);

	SET_8723B_H2CCMD_REMOTE_WAKECTRL_ENABLE(u1H2CRemoteWakeCtrlParm, benable);
	SET_8723B_H2CCMD_REMOTE_WAKE_CTRL_ARP_OFFLOAD_EN(u1H2CRemoteWakeCtrlParm, 1);
	SET_8723B_H2CCMD_REMOTE_WAKE_CTRL_GTK_OFFLOAD_EN(u1H2CRemoteWakeCtrlParm, 0);
	SET_8723B_H2CCMD_REMOTE_WAKE_CTRL_MATCH_QQ_EN(u1H2CRemoteWakeCtrlParm, 1);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CRemoteWakeCtrlParm:", u1H2CRemoteWakeCtrlParm, H2C_8723B_REMOTE_WAKE_CTRL_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_REMOTE_WAKE_CTRL, H2C_8723B_REMOTE_WAKE_CTRL_LEN, u1H2CRemoteWakeCtrlParm);
}

static void rtl8723b_set_FwAOACGlobalInfo_Cmd(PADAPTER padapter,  u8 group_alg, u8 pairwise_alg)
{
	u8 u1H2CAOACGlobalInfoParm[H2C_8723B_AOAC_GLOBAL_INFO_LEN]={0};

	DBG_871X("%s(): group_alg=%d pairwise_alg=%d\n", __func__, group_alg, pairwise_alg);

	SET_8723B_H2CCMD_AOAC_GLOBAL_INFO_PAIRWISE_ENC_ALG(u1H2CAOACGlobalInfoParm, pairwise_alg);
	SET_8723B_H2CCMD_AOAC_GLOBAL_INFO_GROUP_ENC_ALG(u1H2CAOACGlobalInfoParm, group_alg);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_always_, "u1H2CAOACGlobalInfoParm:", u1H2CAOACGlobalInfoParm, H2C_8723B_AOAC_GLOBAL_INFO_LEN);

	FillH2CCmd8723B(padapter, H2C_8723B_AOAC_GLOBAL_INFO, H2C_8723B_AOAC_GLOBAL_INFO_LEN, u1H2CAOACGlobalInfoParm);
}

static void rtl8723b_set_FwWoWlanRelated_cmd(_adapter* padapter, u8 enable)
{
	struct security_priv *psecpriv = &padapter->securitypriv;
	struct pwrctrl_priv *ppwrpriv=adapter_to_pwrctl(padapter);

	DBG_871X_LEVEL(_drv_always_, "+%s()+: enable=%d\n", __func__, enable);
_func_enter_;
	if(enable)
	{
		rtl8723b_set_FwAOACGlobalInfo_Cmd(padapter, psecpriv->dot118021XGrpPrivacy, psecpriv->dot11PrivacyAlgrthm);
		if(!(ppwrpriv->wowlan_wake_reason & FWDecisionDisconnect))
		{
			rtl8723b_download_rsvd_page(padapter, RT_MEDIA_CONNECT);
			rtl8723b_set_FwMediaStatusRpt_cmd(padapter, RT_MEDIA_CONNECT, 0);
		}
		else
			DBG_871X("%s(): Disconnected, no FwJoinBssReport\n",__FUNCTION__);
		rtw_msleep_os(2);

		rtl8723b_set_FwWoWlanCtrl_Cmd(padapter, enable);
		rtw_msleep_os(2);

		rtl8723b_set_FwDisconDecision_cmd(padapter, enable);

		rtl8723b_set_FwKeepAlive_cmd(padapter, enable);

		rtl8723b_set_FwRemoteWakeCtrl_Cmd(padapter, enable);
	}
	else
	{
		rtl8723b_set_FwRemoteWakeCtrl_Cmd(padapter, enable);
		rtw_msleep_os(2);
		rtl8723b_set_FwWoWlanCtrl_Cmd(padapter, enable);
	}

_func_exit_;
	DBG_871X_LEVEL(_drv_always_, "-%s()-\n", __func__);
	return ;
}

void rtl8723b_set_wowlan_cmd(_adapter* padapter, u8 enable)
{
	rtl8723b_set_FwWoWlanRelated_cmd(padapter, enable);
}

#endif

static s32 rtl8723b_set_FwLowPwrLps_cmd(PADAPTER padapter, u8 enable)
{
	//TODO
	return _FALSE;
}

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
static void rtl8723b_set_FwRsvdPagePkt(PADAPTER padapter, BOOLEAN bDLFinished)
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
	u8	PageNum, PageNeed, TxDescLen = TXDESC_SIZE, TxDescOffset = TXDESC_OFFSET;
	u16	BufIndex;
	u32	TotalPacketLen;
	RSVDPAGE_LOC	RsvdPageLoc;
#ifdef CONFIG_WOWLAN
	u32	ARPLegnth = 0;
	struct security_priv *psecuritypriv = &padapter->securitypriv; //added by xx
	u8 currentip[4];
	u8 cur_dot11txpn[8];
#endif

	DBG_871X("%s---->\n", __FUNCTION__);

	_rtw_memset(&RsvdPageLoc, 0, sizeof(RSVDPAGE_LOC));

	ReservedPagePacket = (u8*)rtw_zmalloc(1500);
	if (ReservedPagePacket == NULL) {
		DBG_871X("%s: alloc ReservedPagePacket fail!\n", __FUNCTION__);
		return;
	}

	pHalData = GET_HAL_DATA(padapter);
	pxmitpriv = &padapter->xmitpriv;
	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	PageNum = 0;

	//3 (1) beacon
	BufIndex = TxDescOffset;
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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], PSPollLength, _TRUE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], NullDataLength, 2/*_FALSE*/, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ProbeRspLength, _FALSE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], QosNullLength, _FALSE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], BTQosNullLength, _FALSE, _TRUE);

#ifdef CONFIG_WOWLAN
	PageNeed = (u8)PageNum_128(TxDescLen + BTQosNullLength);
	PageNum += PageNeed;
	BufIndex += PageNeed*128;

	//3(7) ARP RSP
	rtw_get_current_ip_address(padapter, currentip);
	RsvdPageLoc.LocArpRsp= PageNum;
	ConstructARPResponse(
		padapter,
		&ReservedPagePacket[BufIndex],
		&ARPLegnth,
		currentip
		);
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ARPLegnth, _FALSE, _FALSE);

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
			break;
	}

	PageNeed = (u8)PageNum_128(TxDescLen + ARPLegnth);
	PageNum += PageNeed;

	BufIndex += PageNeed*128;

	//3(8) SEC IV
	rtw_get_sec_iv(padapter, cur_dot11txpn, get_my_bssid(&pmlmeinfo->network));
	RsvdPageLoc.LocRemoteCtrlInfo = PageNum;
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
	pattrib->pktlen = pattrib->last_txcmdsz = TotalPacketLen - TxDescOffset;
	_rtw_memcpy(pmgntframe->buf_addr, ReservedPagePacket, TotalPacketLen);

	rtw_hal_mgnt_xmit(padapter, pmgntframe);

	DBG_871X("%s: Set RSVD page location to Fw\n", __FUNCTION__);
	rtl8723b_set_FwRsvdPage_cmd(padapter, &RsvdPageLoc);

exit:
	rtw_mfree(ReservedPagePacket, 1500);
}

void rtl8723b_download_rsvd_page(PADAPTER padapter, u8 mstatus)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

_func_enter_;

	DBG_871X("%s mstatus(%x)\n", __FUNCTION__,mstatus);

	if(mstatus == RT_MEDIA_CONNECT)
	{
		BOOLEAN bRecover = _FALSE;
		u8 v8;

		// We should set AID, correct TSF, HW seq enable before set JoinBssReport to Fw in 88/92C.
		// Suggested by filen. Added by tynli.
		rtw_write16(padapter, REG_BCN_PSR_RPT, (0xC000|pmlmeinfo->aid));

		// set REG_CR bit 8
		v8 = rtw_read8(padapter, REG_CR+1);
		v8 |= BIT(0); // ENSWBCN
		rtw_write8(padapter,  REG_CR+1, v8);

		// Disable Hw protection for a time which revserd for Hw sending beacon.
		// Fix download reserved page packet fail that access collision with the protection time.
		// 2010.05.11. Added by tynli.
		SetBcnCtrlReg(padapter, BIT(4), BIT(3));

		// Set FWHW_TXQ_CTRL 0x422[6]=0 to tell Hw the packet is not a real beacon frame.
		if (pHalData->RegFwHwTxQCtrl & BIT(6))
			bRecover = _TRUE;

		// To tell Hw the packet is not a real beacon frame.
		rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl & ~BIT(6));
		pHalData->RegFwHwTxQCtrl &= ~BIT(6);

		rtl8723b_set_FwRsvdPagePkt(padapter, 0);

		// 2010.05.11. Added by tynli.
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

_func_exit_;
}

void rtl8723b_set_rssi_cmd(_adapter*padapter, u8 *param)
{
	rtl8723b_set_FwRssiSetting_cmd(padapter, param);
}

void rtl8723b_set_FwJoinBssRpt_cmd(PADAPTER padapter, u8 mstatus)
{
	rtl8723b_download_rsvd_page(padapter, RT_MEDIA_CONNECT);
	rtl8723b_set_FwMediaStatusRpt_cmd(padapter, mstatus, 0);
}

s32 rtl8723b_set_lowpwr_lps_cmd(PADAPTER padapter, u8 enable)
{
	struct pwrctrl_priv *ppwrpriv;
	u8 bFlag;
	s32 ret = _FALSE;

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

	ret = rtl8723b_set_FwLowPwrLps_cmd(padapter, enable);
	if (ret == _FALSE) {
		ppwrpriv->bFwLowPwrLPS = bFlag; // restore
	}

_func_exit_;
	return ret;
}

//bitmap[0:27] = tx_rate_bitmap
//bitmap[28:31]= Rate Adaptive id
//arg[0:4] = macid
//arg[5] = Short GI
void rtl8723b_Add_RateATid(PADAPTER pAdapter, u32 bitmap, u8 arg)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u8 mac_id = arg&0x1F;
	u8 shortGI = (arg>>5)&0x01;
	u8 raid = (u8)((bitmap>>28)|(shortGI?BIT7:0));
	u8 bw = pAdapter->mlmeextpriv.cur_bwmode;
	u32 mask = bitmap&0x0FFFFFFF;

	DBG_871X("%s(): bitmap=0x%x arg=0x%x\n", __func__, bitmap, arg);
	DBG_871X("%s(): mac_id=%d raid=0x%x bw=%d mask=0x%x\n", __func__, mac_id, raid, bw, mask);
	rtl8723b_set_FwMacIdConfig_cmd(pAdapter, mac_id, raid, bw, mask);
}

void rtl8723b_fw_try_ap_cmd(PADAPTER padapter, u32 need_ack)
{
	rtl8723b_set_FwAPReqRPT_cmd(padapter, need_ack);
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
	u8	PageNum, PageNeed, TxDescLen = TXDESC_SIZE, TxDescOffset = TXDESC_OFFSET;
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

	PageNum = 0;

	//3 (1) beacon
	BufIndex = TxDescOffset;
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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], PSPollLength, _TRUE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], NullDataLength, _FALSE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], ProbeRspLength, _FALSE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], QosNullLength, _FALSE, _FALSE);

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
	rtl8723b_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex-TxDescLen], BTQosNullLength, _FALSE, _TRUE);

	TotalPacketLen = BufIndex + BTQosNullLength;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	// update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->qsel = 0x10;
	pattrib->pktlen = pattrib->last_txcmdsz = TotalPacketLen - TxDescOffset;
	_rtw_memcpy(pmgntframe->buf_addr, ReservedPagePacket, TotalPacketLen);

	rtw_hal_mgnt_xmit(padapter, pmgntframe);

	DBG_8192C("%s: Set RSVD page location to Fw\n", __FUNCTION__);
	FillH2CCmd8723B(padapter, H2C_8723B_RSVD_PAGE, sizeof(RsvdPageLoc), (u8*)&RsvdPageLoc);

exit:
	rtw_mfree(ReservedPagePacket, 1024);
}

void rtl8723b_set_BTCoex_AP_mode_FwRsvdPkt_cmd(PADAPTER padapter)
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
void rtl8723b_set_p2p_ps_offload_cmd(_adapter* padapter, u8 p2p_ps_state)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
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

	FillH2CCmd8723B(padapter, H2C_8723B_P2P_PS_OFFLOAD, 1, (u8 *)p2p_ps_offload);

_func_exit_;

}
#endif //CONFIG_P2P

#ifdef CONFIG_TSF_RESET_OFFLOAD
/*
	ask FW to Reset sync register at Beacon early interrupt
*/
u8 rtl8723b_reset_tsf(_adapter *padapter, u8 reset_port )
{
	u8	buf[2];
	u8	res=_SUCCESS;

_func_enter_;
	if (IFACE_PORT0==reset_port) {
		buf[0] = 0x1; buf[1] = 0;

	} else{
		buf[0] = 0x0; buf[1] = 0x1;
	}
	FillH2CCmd8723B(padapter, H2C_8723B_RESET_TSF, 2, buf);
_func_exit_;

	return res;
}
#endif	// CONFIG_TSF_RESET_OFFLOAD
