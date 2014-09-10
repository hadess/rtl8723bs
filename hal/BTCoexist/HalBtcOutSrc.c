/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
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

#include "halbt_precomp.h"

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST
//====================================
//		Global variables
//====================================
const char *const BtProfileString[]={
	"NONE",
	"A2DP",
	"PAN",
	"HID",
	"SCO",
};
const char *const BtSpecString[]={
	"1.0b",
	"1.1",
	"1.2",
	"2.0+EDR",
	"2.1+EDR",
	"3.0+HS",
	"4.0",
};
const char *const BtLinkRoleString[]={
	"Master",
	"Slave",
};
const char *const h2cStaString[]={
	"successful",
	"h2c busy",
	"rf off",
	"fw not read",
};

const char *const ioStaString[]={
	"IO_STATUS_SUCCESS",
	"IO_STATUS_FAIL_CANNOT_IO",
	"IO_STATUS_FAIL_RF_OFF",
	"IO_STATUS_FAIL_FW_READ_CLEAR_TIMEOUT",
	"IO_STATUS_FAIL_WAIT_IO_EVENT_TIMEOUT",
	"IO_STATUS_INVALID_LEN",
	"IO_STATUS_IO_IDLE_QUEUE_EMPTY",
	"IO_STATUS_IO_INSERT_WAIT_QUEUE_FAIL",
	"IO_STATUS_UNKNOWN_FAIL",
	"IO_STATUS_WRONG_LEVEL",
	"IO_STATUS_H2C_STOPPED",
};

BTC_COEXIST				GLBtCoexist;

u4Byte					GLBtcDbgType[BTC_MSG_MAX];
u1Byte					GLBtcDbgBuf[BT_TMP_BUF_SIZE];

//====================================
//		Debug related function
//====================================
BOOLEAN
halbtcoutsrc_IsBtCoexistAvailable(
	IN 	PBTC_COEXIST		pBtCoexist
	)
{
	if(!pBtCoexist->bBinded ||
		NULL == pBtCoexist->Adapter)
	{
		return FALSE;
	}
	return TRUE;
}

VOID
halbtcoutsrc_DbgInit(VOID)
{
	u1Byte	i;

	for(i=0; i<BTC_MSG_MAX; i++)
		GLBtcDbgType[i] = 0;

	GLBtcDbgType[BTC_MSG_INTERFACE]			= 	\
			INTF_INIT								|
			INTF_NOTIFY							|
			0;

	GLBtcDbgType[BTC_MSG_ALGORITHM]			= 	\
			ALGO_BT_RSSI_STATE					|
			ALGO_WIFI_RSSI_STATE					|
			ALGO_BT_MONITOR						|
			ALGO_TRACE							|
			ALGO_TRACE_FW						|
			ALGO_TRACE_FW_DETAIL				|
			ALGO_TRACE_FW_EXEC					|
			ALGO_TRACE_SW						|
			ALGO_TRACE_SW_DETAIL				|
			ALGO_TRACE_SW_EXEC					|
			0;
}

BOOLEAN
halbtcoutsrc_IsHwMailboxExist(
	IN	PBTC_COEXIST			pBtCoexist
	)
{
	if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		return FALSE;
	}
	else
		return TRUE;
}

u4Byte
halbtcoutsrc_GetWifiBw(
	IN	PBTC_COEXIST			pBtCoexist
	)
{
	PADAPTER		Adapter=(PADAPTER)pBtCoexist->Adapter;
	u4Byte			wifiBw=BTC_WIFI_BW_HT20;
#if 0
	PADAPTER		pExtAdapter=NULL;
	PMGNT_INFO		pExtMgntInfo;
	PMGNT_INFO		pMgntInfo=&Adapter->MgntInfo;
	BOOLEAN			bPureWifiConnected=FALSE;

	// default port
	if(	pMgntInfo->bMediaConnect ||
		(AsocEntry_AnyStationAssociated(pMgntInfo)) )
	{
		bPureWifiConnected = TRUE;
		if(BTDM_IsHt40(Adapter))
		{
			wifiBw = BTC_WIFI_BW_HT40;
		}
		else
		{
			if(BTDM_Legacy(Adapter))
				wifiBw = BTC_WIFI_BW_LEGACY;
			else
				wifiBw = BTC_WIFI_BW_HT20;
		}
	}
	else
	{
		// ext port, check vWifi.
		if(Adapter->MgntInfo.NdisVersion >= RT_NDIS_VERSION_6_20)
		{
			if(IsAPModeExist(Adapter))
			{
				pExtAdapter = GetFirstExtAdapter(Adapter);
				if(pExtAdapter)
				{
					pExtMgntInfo = &pExtAdapter->MgntInfo;
					if(AsocEntry_AnyStationAssociated(pExtMgntInfo))
					{
						bPureWifiConnected = TRUE;
						if(BTDM_IsHt40(pExtAdapter))
						{
							wifiBw = BTC_WIFI_BW_HT40;
						}
						else
						{
							if(BTDM_Legacy(pExtAdapter))
								wifiBw = BTC_WIFI_BW_LEGACY;
							else
								wifiBw = BTC_WIFI_BW_HT20;
						}
					}
				}
			}
		}

		// if default port && vwifi not connected to any,
		// check HS link.
		if(!bPureWifiConnected)
		{
			if(BT_Operation(Adapter))// check if HS link is exists
			{
				wifiBw = BTC_WIFI_BW_LEGACY;
			}
		}
	}
#endif

	if(BTDM_IsHt40(Adapter))
	{
		wifiBw = BTC_WIFI_BW_HT40;
	}
	else
	{
		if(BTDM_Legacy(Adapter))
			wifiBw = BTC_WIFI_BW_LEGACY;
		else
			wifiBw = BTC_WIFI_BW_HT20;
	}
	return wifiBw;
}

u1Byte
halbtcoutsrc_GetWifiCentralChnl(
	IN	PBTC_COEXIST			pBtCoexist
	)
{
	PADAPTER		Adapter=(PADAPTER)pBtCoexist->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
	u1Byte			chnl = 1;

	if(pHalData->CurrentChannel)
		chnl = pHalData->CurrentChannel;
#if 0
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);
	PADAPTER		pExtAdapter=NULL;
	PMGNT_INFO		pExtMgntInfo;
	PMGNT_INFO		pMgntInfo=&Adapter->MgntInfo;
	PBT30Info		pBTInfo=&Adapter->MgntInfo.BtInfo;
	PBT_MGNT		pBtMgnt=&pBTInfo->BtMgnt;
	u1Byte			chnl=pMgntInfo->pChannelInfo->CurrentChannelCenterFrequency;

	// default port
	if(	pMgntInfo->bMediaConnect ||
		(AsocEntry_AnyStationAssociated(pMgntInfo)) )
	{
		chnl = pMgntInfo->pChannelInfo->CurrentChannelCenterFrequency;
	}
	else
	{
		// ext port, check vWifi.
		if(Adapter->MgntInfo.NdisVersion >= RT_NDIS_VERSION_6_20)
		{
			if(IsAPModeExist(Adapter))
			{
				pExtAdapter = GetFirstExtAdapter(Adapter);
				if(pExtAdapter)
				{
					pExtMgntInfo = &pExtAdapter->MgntInfo;
					if(AsocEntry_AnyStationAssociated(pExtMgntInfo))
					{
						chnl = pExtMgntInfo->pChannelInfo->CurrentChannelCenterFrequency;
					}
				}
			}
		}

		// if default port && vwifi not connected to any,
		// check HS link.
		if(chnl == 0)
		{
			if(BT_Operation(Adapter))// check if HS link is exists
			{
				chnl = pBtMgnt->BTChannel;
			}
			else
			{
				chnl = pMgntInfo->pChannelInfo->CurrentChannelCenterFrequency;
			}
		}
	}
#endif
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("halbtcoutsrc_GetWifiCentralChnl:%d\n",chnl));
	return chnl;
}

VOID
halbtcoutsrc_LeaveLps(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER		Adapter=pBtCoexist->Adapter;
	//PMGNT_INFO		pMgntInfo=&Adapter->MgntInfo;
	//PRT_POWER_SAVE_CONTROL	pPSC = GET_POWER_SAVE_CONTROL(pMgntInfo);
	struct pwrctrl_priv *pwrctrl = adapter_to_pwrctl(Adapter);
	BOOLEAN bApEnable=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_AP_MODE_ENABLE, &bApEnable);
	if (bApEnable) {
		DBG_871X("halbtcoutsrc_LeaveLps()<--dont leave lps under AP mode\n");
		return;
	}

	DBG_871X("halbtcoutsrc_LeaveLps()-->Leave LPS 32K!pwrctrl->pwr_mode:%d\n",pwrctrl->pwr_mode);
	if(pBtCoexist->btInfo.bForceLps)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Leave LPS!!!\n"));
		pBtCoexist->btInfo.bForceLps = FALSE;
		pBtCoexist->btInfo.bBtPwrSaveMode = FALSE;
		pBtCoexist->btInfo.forceExecPwrCmdCnt++;
	}
	//pPSC->bLeisurePs = TRUE;
	//LeisurePSLeave(Adapter, LPS_DISABLE_BT_COEX);

	if (pwrctrl->pwr_mode != PS_MODE_ACTIVE) {
		rtw_set_ps_mode(Adapter, PS_MODE_ACTIVE, 0, 0);
		LPS_RF_ON_check(Adapter, 100);
	}
}

VOID
halbtcoutsrc_EnterLps(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER		Adapter=pBtCoexist->Adapter;
	u1Byte			LpsVal = 0;
	BOOLEAN bApEnable=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_AP_MODE_ENABLE, &bApEnable);
	if (bApEnable) {
		DBG_871X("halbtcoutsrc_EnterLps()<--dont enter lps under AP mode\n");
		return;
	}
	if(pBtCoexist->btInfo.bForceLps)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Normal LPS behavior!!!\n"));
		// reset bt coex related power save flag.
		pBtCoexist->btInfo.bForceLps = FALSE;
		pBtCoexist->btInfo.bBtPwrSaveMode = FALSE;
#if 0
		// recover the LPS state to the original
		Adapter->HalFunc.UpdateLPSStatusHandler(
				Adapter,
				pPSC->RegLeisurePsMode,
				pPSC->RegPowerSaveMode);
		//pBtCoexist->btInfo.bForceToRoam = TRUE;
#endif
	}

	/* this AP should not close pspoll */
	if (!rtw_check_iot_peer_ps_should_pspoll(Adapter))
		LpsVal |= BIT4; //BIT4 is close pspoll

	rtw_set_ps_mode(Adapter, PM_Radio_On, 0, LpsVal); //RF ON;
}

VOID
halbtcoutsrc_NormalLps(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER		Adapter=pBtCoexist->Adapter;
	BOOLEAN bApEnable=FALSE;
	u8	smart_ps = Adapter->registrypriv.smart_ps;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_AP_MODE_ENABLE, &bApEnable);
	if (bApEnable) {
		DBG_871X("halbtcoutsrc_NormalLps<--dont enter lps under AP mode\n");
		return;
	}

	if(!pBtCoexist->btInfo.bForceLps)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], enter LPS!!!\n"));
		pBtCoexist->btInfo.bForceLps = TRUE;
		pBtCoexist->btInfo.bBtPwrSaveMode = TRUE;
		pBtCoexist->btInfo.forceExecPwrCmdCnt++;
	}

	if (rtw_check_iot_peer_ps_should_pspoll(Adapter))
		smart_ps = 0;
	rtw_set_ps_mode(Adapter, PS_MODE_MIN, smart_ps, pBtCoexist->btInfo.lps1Ant);
}

VOID
halbtcoutsrc_AggregationCheck(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER Adapter=pBtCoexist->Adapter;
	BOOLEAN bNeedToAct=FALSE;

	if(pBtCoexist->btInfo.bRejectAggPkt)
		BTDM_RejectApAggregatedPacket(Adapter, TRUE);
	else
	{
		if( pBtCoexist->btInfo.bPreBtCtrlAggBufSize !=
			pBtCoexist->btInfo.bBtCtrlAggBufSize)
		{
			bNeedToAct = TRUE;
			pBtCoexist->btInfo.bPreBtCtrlAggBufSize = pBtCoexist->btInfo.bBtCtrlAggBufSize;
		}

		if(pBtCoexist->btInfo.bBtCtrlAggBufSize)
		{
			if( pBtCoexist->btInfo.preAggBufSize !=
				pBtCoexist->btInfo.aggBufSize )
			{
				bNeedToAct = TRUE;
			}
			pBtCoexist->btInfo.preAggBufSize = pBtCoexist->btInfo.aggBufSize;
		}

		if(bNeedToAct && (Adapter->mlmeextpriv.mlmext_info.CurRxBufSize != pBtCoexist->btInfo.aggBufSize))
		{
			BTDM_RejectApAggregatedPacket(Adapter, TRUE);
			BTDM_RejectApAggregatedPacket(Adapter, FALSE);
		}
	}

}

BOOLEAN
halbtcoutsrc_Get(
	IN 	PVOID		pVoidBtCoexist,
	IN	u1Byte			getType,
	OUT	PVOID			pOutBuf
	)
{
	PBTC_COEXIST		pBtCoexist = (PBTC_COEXIST)pVoidBtCoexist;
	PADAPTER			Adapter=pBtCoexist->Adapter;
	PHAL_DATA_TYPE		pHalData=GET_HAL_DATA(Adapter);
	struct pwrctrl_priv 	*pwrctrlpriv = adapter_to_pwrctl(Adapter);
	//PADAPTER			pExtAdapter=GetFirstExtAdapter(Adapter);
	//PMGNT_INFO			pDefMgntInfo=&Adapter->MgntInfo;
	//PMGNT_INFO			pExtMgntInfo=NULL;
	BOOLEAN				bSoftApExist=FALSE, bVwifiExist=FALSE;
	PBOOLEAN			pBoolean=(PBOOLEAN)pOutBuf;
	ps4Byte				pS4Tmp=(ps4Byte)pOutBuf;
	pu4Byte				pU4Tmp=(pu4Byte)pOutBuf;
	pu1Byte				pU1Tmp=(pu1Byte)pOutBuf;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return FALSE;

	//if(pExtAdapter)
	//{
	//	pExtMgntInfo = &pExtAdapter->MgntInfo;
	//}

	switch(getType)
	{
		case BTC_GET_BL_HS_OPERATION:
			//if(BT_Operation(Adapter)) //not support HCI
			//	*pBoolean = TRUE;
			//else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_HS_CONNECTING:
			//if(BT_ConnectionProcess(Adapter))
			//	*pBoolean = TRUE;
			//else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_CONNECTED:
			{
				BOOLEAN		bTmp=FALSE;
				if (check_fwstate(&Adapter->mlmepriv, WIFI_ASOC_STATE) == _TRUE)
					bTmp = TRUE;
				if(BT_AnyClientConnectToAp(Adapter))
					bTmp = TRUE;
				//if(BT_HsConnectionEstablished(Adapter))
				//	bTmp = TRUE;
				*pBoolean = bTmp;
			}
			break;
		case BTC_GET_BL_WIFI_BUSY:
			if(BTDM_IsWifiBusy(Adapter))
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_SCAN:
			if (check_fwstate(&Adapter->mlmepriv, WIFI_SITE_MONITOR) == _TRUE)
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_LINK:
			if (check_fwstate(&Adapter->mlmepriv, _FW_UNDER_LINKING) == _TRUE)
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_ROAM:
			//if(MgntRoamingInProgress(pDefMgntInfo))
			if (check_fwstate(&Adapter->mlmepriv, _FW_UNDER_LINKING) == _TRUE)
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_4_WAY_PROGRESS:
			if(check_fwstate(&Adapter->mlmepriv, WIFI_STATION_STATE) &&
				(Adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X ||
				Adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI ||
				check_fwstate(&Adapter->mlmepriv, WIFI_UNDER_WPS) == _TRUE) &&
				(Adapter->securitypriv.bStaInstallPairwiseKey == _FALSE)){
				*pBoolean = TRUE;
			}
			else
				*pBoolean = FALSE;

			break;
		case BTC_GET_BL_WIFI_DHCP:
			*pBoolean = Adapter->mlmepriv.android_btcex_bdhcp;
			break;
		case BTC_GET_BL_WIFI_SOFTAP_IDLE:
			*pBoolean = !BT_AnyClientConnectToAp( Adapter);
			break;
		case BTC_GET_BL_WIFI_SOFTAP_LINKING:
			{
				u32	curr_time = 0, delta_time = 0;
				curr_time = rtw_get_current_time();
				delta_time = curr_time - Adapter->mlmeextpriv.onauth_time;
				delta_time = rtw_systime_to_ms(delta_time);
				if (delta_time < 5000)
					*pBoolean = TRUE;
				else
					*pBoolean = FALSE;
			}
			break;
		case BTC_GET_BL_WIFI_IN_EARLY_SUSPEND:
			*pBoolean = pwrctrlpriv->in_early_suspend;
			break;
		case BTC_GET_BL_WIFI_UNDER_5G:
			//if(IS_WIRELESS_MODE_5G(Adapter))
			//	*pBoolean = TRUE;
			//else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_AP_MODE_ENABLE:
			if (check_fwstate(&Adapter->mlmepriv, WIFI_AP_STATE) == _TRUE)
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_ENABLE_ENCRYPTION:
			if(_NO_PRIVACY_ == Adapter->securitypriv.dot11PrivacyAlgrthm)
				*pBoolean = FALSE;
			else
				*pBoolean = TRUE;
			break;
		case BTC_GET_BL_WIFI_UNDER_B_MODE:
			if (pHalData->CurrentWirelessMode == WIRELESS_MODE_B)
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_BL_WIFI_CHANGE_PSPOLL:
			if (rtw_check_iot_peer_ps_should_pspoll(Adapter))
				*pBoolean = TRUE;
			else
				*pBoolean = FALSE;
			break;
		case BTC_GET_S4_WIFI_RSSI:
			*pS4Tmp = BTDM_GetWifiRssi(Adapter);
			break;
		case BTC_GET_S4_HS_RSSI:
			*pS4Tmp = pHalData->dmpriv.BT_EntryMinUndecoratedSmoothedPWDB;
			break;
		case BTC_GET_U4_WIFI_BW:
			*pU4Tmp = halbtcoutsrc_GetWifiBw(pBtCoexist);
			break;
		case BTC_GET_U4_WIFI_TRAFFIC_DIRECTION:
			if(BTDM_IsWifiUplink(Adapter))
				*pU4Tmp = BTC_WIFI_TRAFFIC_TX;
			else
				*pU4Tmp = BTC_WIFI_TRAFFIC_RX;
			break;
		case BTC_GET_U4_WIFI_FW_VER:
			*pU4Tmp = (pHalData->FirmwareVersion << 8 |pHalData->FirmwareSubVersion);//Adapter->MgntInfo.FirmwareVersion;
			break;
		case BTC_GET_U4_WIFI_OUT_DPDT:
			if (pHalData->bUseExtSPDT)
				*pU1Tmp = TRUE;
			else
				*pU1Tmp = FALSE;
		case BTC_GET_U4_BT_PATCH_VER:
			//COEX_TODO
			//*pU4Tmp = halbtcoutsrc_GetBtPatchVer(pBtCoexist);
			break;
		case BTC_GET_U1_WIFI_DOT11_CHNL:
			*pU1Tmp = Adapter->mlmeextpriv.cur_channel;//pDefMgntInfo->dot11CurrentChannelNumber;
			break;
		case BTC_GET_U1_WIFI_CENTRAL_CHNL:
			*pU1Tmp = halbtcoutsrc_GetWifiCentralChnl(pBtCoexist);
			break;
		case BTC_GET_U1_WIFI_HS_CHNL:
			*pU1Tmp = 1;//BT_OperateChnl(Adapter);
			break;
		case BTC_GET_U1_MAC_PHY_MODE:
			#if 0
			if(SINGLEMAC_SINGLEPHY == pHalData->MacPhyMode92D)
				*pU1Tmp = BTC_SMSP;
			else if(DUALMAC_SINGLEPHY == pHalData->MacPhyMode92D)
				*pU1Tmp = BTC_DMSP;
			else if(DUALMAC_DUALPHY == pHalData->MacPhyMode92D)
				*pU1Tmp = BTC_DMDP;
			else
			#endif
				*pU1Tmp = BTC_MP_UNKNOWN;
			break;

		//=======1Ant===========
		case BTC_GET_U1_LPS_MODE:
			*pU1Tmp = pBtCoexist->pwrModeVal[0];
			break;
#if 0
		case BTC_GET_U1_LPS:
			*pU1Tmp = pBtCoexist->pwrModeVal[5];
			break;
		case BTC_GET_U1_RPWM:
			*pU1Tmp = pBtCoexist->pwrModeVal[4];
			break;
#endif
		//===1:CTA 2:CMCC RF 3: CMCC IPERF
		case BTC_GET_DRIVER_TEST_CFG:
			*pU1Tmp = Adapter->drv_in_test;
			break;

		default:
			break;
	}

	return TRUE;
}

BOOLEAN
halbtcoutsrc_Set(
	IN 	PVOID		pVoidBtCoexist,
	IN	u1Byte			setType,
	IN	PVOID			pInBuf
	)
{
	PBTC_COEXIST	pBtCoexist = (PBTC_COEXIST)pVoidBtCoexist;
	PADAPTER		Adapter=pBtCoexist->Adapter;
	PHAL_DATA_TYPE	pHalData=GET_HAL_DATA(Adapter);
	PBOOLEAN		pBoolean=(PBOOLEAN)pInBuf;
	pu1Byte			pU1Tmp=(pu1Byte)pInBuf;
	pu4Byte			pU4Tmp=(pu4Byte)pInBuf;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return FALSE;

	switch(setType)
	{
		// set some BOOLEAN type variables.
		case BTC_SET_BL_BT_DISABLE:
			pBtCoexist->btInfo.bBtDisabled = *pBoolean;
			break;
		case BTC_SET_BL_BT_TRAFFIC_BUSY:
			pBtCoexist->btInfo.bBtBusy = *pBoolean;
			break;
		case BTC_SET_BL_BT_LIMITED_DIG:
			pBtCoexist->btInfo.bLimitedDig = *pBoolean;
			break;
		case BTC_SET_BL_FORCE_TO_ROAM:
			pBtCoexist->btInfo.bForceToRoam = *pBoolean;
			break;
		case BTC_SET_BL_TO_REJ_AP_AGG_PKT:
			pBtCoexist->btInfo.bRejectAggPkt = *pBoolean;
			break;
		case BTC_SET_BL_BT_CTRL_AGG_SIZE:
			pBtCoexist->btInfo.bBtCtrlAggBufSize = *pBoolean;
			break;

		// set some u1Byte type variables.
		case BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON:
			pBtCoexist->btInfo.rssiAdjustForAgcTableOn = *pU1Tmp;
			break;
		case BTC_SET_U1_AGG_BUF_SIZE:
			pBtCoexist->btInfo.aggBufSize = *pU1Tmp;
			break;

		// the following are some action which will be triggered
		case BTC_SET_ACT_GET_BT_RSSI:
			//BTHCI_SendGetBtRssiEvent(Adapter);
			break;
		case BTC_SET_ACT_AGGREGATE_CTRL:
			halbtcoutsrc_AggregationCheck(pBtCoexist);
			break;


		//=======1Ant===========
		// set some BOOLEAN type variables.

		// set some u1Byte type variables.
		case BTC_SET_U1_RSSI_ADJ_VAL_FOR_1ANT_COEX_TYPE:
			pBtCoexist->btInfo.rssiAdjustFor1AntCoexType = *pU1Tmp;
			break;
		case BTC_SET_UI_SCAN_SIG_COMPENSATION:
			Adapter->mlmepriv.scan_compensation = *pU1Tmp;
			break;
		case BTC_SET_U1_1ANT_LPS:
			pBtCoexist->btInfo.lps1Ant = *pU1Tmp;
			break;
		case BTC_SET_U1_1ANT_RPWM:
			pBtCoexist->btInfo.rpwm1Ant = *pU1Tmp;
			break;
		// the following are some action which will be triggered
		case BTC_SET_ACT_LEAVE_LPS:
			halbtcoutsrc_LeaveLps(pBtCoexist);
			break;
		case BTC_SET_ACT_ENTER_LPS:
			halbtcoutsrc_EnterLps(pBtCoexist);
			break;
		case BTC_SET_ACT_NORMAL_LPS:
			halbtcoutsrc_NormalLps(pBtCoexist);
			break;
		case BTC_SET_ACT_UPDATE_RAMASK:
			pBtCoexist->btInfo.raMask = *pU4Tmp;
			if (check_fwstate(&Adapter->mlmepriv, WIFI_ASOC_STATE) == _TRUE)
			{
				BT_ActUpdateRaMask(Adapter, 0);
			}
			break;
		case BTC_SET_ACT_INC_FORCE_EXEC_PWR_CMD_CNT:
			pBtCoexist->btInfo.forceExecPwrCmdCnt++;
			break;
		case BTC_SET_ACT_CTRL_BT_INFO:
#if 0 //disble for HCI
			{
				u1Byte	dataLen=*pU1Tmp;
				u1Byte	tmpBuf[20];
				if(dataLen)
				{
					_rtw_memcpy(&tmpBuf[0], pU1Tmp+1, dataLen);
				}
				BT_SendEventExtBtInfoControl(Adapter, dataLen, &tmpBuf[0]);
			}
#endif
			break;
		case BTC_SET_ACT_CTRL_BT_COEX:
#if 0 //disble for HCI
			{
				u1Byte	dataLen=*pU1Tmp;
				u1Byte	tmpBuf[20];
				if(dataLen)
				{
					PlatformMoveMemory(&tmpBuf[0], pU1Tmp+1, dataLen);
				}
				BT_SendEventExtBtCoexControl(Adapter, dataLen, &tmpBuf[0]);
			}
#endif
			break;
		//=====================
		default:
			break;
	}

	return TRUE;
}

VOID
halbtcoutsrc_DisplayCoexStatistics(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER		Adapter=(PADAPTER)pBtCoexist->Adapter;
	//PBT_MGNT		pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;
	PHAL_DATA_TYPE	pHalData=GET_HAL_DATA(Adapter);
	pu1Byte			cliBuf=pBtCoexist->cliBuf;
	u1Byte			i;

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Statistics]============");
	CL_PRINTF(cliBuf);
#if 0
#if(IO_THREAD_SUPPORT != 1)
	for(i=0; i<H2C_STATUS_MAX; i++)
	{
		if(pHalData->h2cStatistics[i])
		{
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = [%s] = %d", "H2C statistics", \
				h2cStaString[i], pHalData->h2cStatistics[i]);
			CL_PRINTF(cliBuf);
		}
	}

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "lastHMEBoxNum", \
		pHalData->LastHMEBoxNum);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x / 0x%x", "LastSuccessFwEid/FirstfailedFwEid", \
		pHalData->lastSuccessH2cEid, pHalData->firstFailedH2cEid);
	CL_PRINTF(cliBuf);
#endif

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d/ %d", "c2hIsr/c2hIntr/clr1AF/noRdy/noBuf", \
		pHalData->InterruptLog.nIMR_C2HCMD, DBG_Var.c2hInterruptCnt, DBG_Var.c2hClrReadC2hCnt,
		DBG_Var.c2hNotReadyCnt, DBG_Var.c2hBufAlloFailCnt);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d", "c2hPacket", \
		DBG_Var.c2hPacketCnt);
	CL_PRINTF(cliBuf);
#endif
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "Periodical/ DbgCtrl", \
		pBtCoexist->statistics.cntPeriodical, pBtCoexist->statistics.cntDbgCtrl);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "InitHw/InitCoexDm/", \
		pBtCoexist->statistics.cntInitHwConfig, pBtCoexist->statistics.cntInitCoexDm);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d/ %d", "Ips/Lps/Scan/Connect/Mstatus", \
		pBtCoexist->statistics.cntIpsNotify, pBtCoexist->statistics.cntLpsNotify,
		pBtCoexist->statistics.cntScanNotify, pBtCoexist->statistics.cntConnectNotify,
		pBtCoexist->statistics.cntMediaStatusNotify);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "Special pkt/Bt info", \
		pBtCoexist->statistics.cntSpecialPacketNotify, pBtCoexist->statistics.cntBtInfoNotify);
	CL_PRINTF(cliBuf);
}

VOID
halbtcoutsrc_DisplayBtLinkInfo(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
#if 0
	PADAPTER		Adapter=(PADAPTER)pBtCoexist->Adapter;
	PBT_MGNT		pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;
	pu1Byte			cliBuf=pBtCoexist->cliBuf;
	u1Byte			i;

	for(i=0; i<pBtMgnt->ExtConfig.NumberOfACL; i++)
	{
		if(pBtMgnt->ExtConfig.HCIExtensionVer >= 1)
		{
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s/ %s/ %s", "Bt link type/spec/role", \
				BtProfileString[pBtMgnt->ExtConfig.aclLink[i].BTProfile],
				BtSpecString[pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec],
				BtLinkRoleString[pBtMgnt->ExtConfig.aclLink[i].linkRole]);
			CL_PRINTF(cliBuf);				}
		else
		{
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s/ %s", "Bt link type/spec", \
				BtProfileString[pBtMgnt->ExtConfig.aclLink[i].BTProfile],
				BtSpecString[pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec]);
			CL_PRINTF(cliBuf);
		}
	}
#endif
}

VOID
halbtcoutsrc_DisplayBtFwInfo(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
#if 0
	pu1Byte 		cliBuf=pBtCoexist->cliBuf;
	u2Byte		btRealFwVer=0x0;
	u1Byte		btFwVer=0x0;
	static u1Byte	cnt=0;

	if(!pBtCoexist->btInfo.btRealFwVer && cnt<=5)
	{
		if(halbtcoutsrc_IsHwMailboxExist(pBtCoexist))
		{	// mailbox exists, through mailbox
			if(NDBG_GetBtFwVersion(pBtCoexist->Adapter, &btRealFwVer, &btFwVer))
			{
				pBtCoexist->btInfo.btRealFwVer = btRealFwVer;
				pBtCoexist->btInfo.btFwVer = btFwVer;
			}
			else
			{
				pBtCoexist->btInfo.btRealFwVer = 0x0;
				pBtCoexist->btInfo.btFwVer = 0x0;
			}
		}
		else	// no mailbox, query bt patch version through stack.
		{
			u1Byte	dataLen=2;
			u1Byte	buf[4] = {0};
			buf[0] = 0x0;	// OP_Code
			buf[1] = 0x0;	// OP_Code_Length
			BT_SendEventExtBtCoexControl(pBtCoexist->Adapter, dataLen, &buf[0]);
		}
		cnt++;
	}
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x(%d) (try cnt=%d)", "Bt Ver", \
		pBtCoexist->btInfo.btRealFwVer, pBtCoexist->btInfo.btRealFwVer, cnt);
	CL_PRINTF(cliBuf);
#endif
}

VOID
halbtcoutsrc_DisplayFwPwrModeCmd(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	pu1Byte 		cliBuf=pBtCoexist->cliBuf;

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x %02x ", "Power mode cmd ", \
		pBtCoexist->pwrModeVal[0], pBtCoexist->pwrModeVal[1],
		pBtCoexist->pwrModeVal[2], pBtCoexist->pwrModeVal[3],
		pBtCoexist->pwrModeVal[4], pBtCoexist->pwrModeVal[5]);
	CL_PRINTF(cliBuf);
}

//====================================
//		IO related function
//====================================
u1Byte
halbtcoutsrc_Read1Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return	rtw_read8(Adapter, RegAddr);
}


u2Byte
halbtcoutsrc_Read2Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return	rtw_read16(Adapter, RegAddr);
}


u4Byte
halbtcoutsrc_Read4Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return	rtw_read32(Adapter, RegAddr);
}


VOID
halbtcoutsrc_Write1Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u1Byte			Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	rtw_write8(Adapter, RegAddr, Data);
}

VOID
halbtcoutsrc_BitMaskWrite1Byte(
	IN 	PVOID		pBtcContext,
	IN	u4Byte		regAddr,
	IN	u1Byte		bitMask,
	IN	u1Byte		data1b
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;
	u1Byte			originalValue, bitShift=0;
	u1Byte			i;

	if(bitMask!= bMaskDWord)
	{//if not "double word" write
		originalValue = rtw_read8(Adapter, regAddr);
		for(i=0; i<=7; i++)
		{
			if((bitMask>>i)&0x1)
				break;
		}
		bitShift = i;
		data1b = ((originalValue) & (~bitMask)) |( ((data1b << bitShift)) & bitMask);
	}
	rtw_write8(Adapter, regAddr, data1b);
}


VOID
halbtcoutsrc_Write2Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u2Byte			Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	rtw_write16(Adapter, RegAddr, Data);
}


VOID
halbtcoutsrc_Write4Byte(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	rtw_write32(Adapter, RegAddr, Data);
}


VOID
halbtcoutsrc_SetMacReg(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask,
	IN	u4Byte			Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}


u4Byte
halbtcoutsrc_GetMacReg(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}


VOID
halbtcoutsrc_SetBbReg(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask,
	IN	u4Byte			Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}


u4Byte
halbtcoutsrc_GetBbReg(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}


VOID
halbtcoutsrc_SetRfReg(
	IN 	PVOID				pBtcContext,
	IN	u1Byte				eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask,
	IN	u4Byte				Data
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data);
}


u4Byte
halbtcoutsrc_GetRfReg(
	IN 	PVOID				pBtcContext,
	IN	u1Byte				eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;

	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
}
#ifdef CONFIG_RTL8723A
extern s32 FillH2CCmd(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer);
#endif
#ifdef CONFIG_RTL8723B
extern s32 FillH2CCmd8723B(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer);
#endif
VOID
halbtcoutsrc_FillH2cCmd(
	IN 	PVOID				pBtcContext,
	IN	u1Byte 				elementId,
	IN	u4Byte 				cmdLen,
	IN	pu1Byte				pCmdBuffer
	)
{
	PBTC_COEXIST		pBtCoexist=(PBTC_COEXIST)pBtcContext;
	PADAPTER		Adapter=pBtCoexist->Adapter;
#ifdef CONFIG_RTL8723A
	FillH2CCmd(Adapter, elementId, cmdLen, pCmdBuffer);
#endif
#ifdef CONFIG_RTL8723B
	//if(elementId != 0x66 && elementId != 0x61)
	FillH2CCmd8723B(Adapter, elementId, cmdLen, pCmdBuffer);
#endif
}

VOID
halbtcoutsrc_DisplayDbgMsg(
	IN	PVOID		pVoidBtCoexist,
	IN	u1Byte			dispType
	)
{
	PBTC_COEXIST pBtCoexist = (PBTC_COEXIST)pVoidBtCoexist;
	switch(dispType)
	{
		case BTC_DBG_DISP_COEX_STATISTICS:
			halbtcoutsrc_DisplayCoexStatistics(pBtCoexist);
			break;
		case BTC_DBG_DISP_BT_LINK_INFO:
			halbtcoutsrc_DisplayBtLinkInfo(pBtCoexist);
			break;
		case BTC_DBG_DISP_BT_FW_VER:
			halbtcoutsrc_DisplayBtFwInfo(pBtCoexist);
			break;
		case BTC_DBG_DISP_FW_PWR_MODE_CMD:
			halbtcoutsrc_DisplayFwPwrModeCmd(pBtCoexist);
			break;
		default:
			break;
	}
}

BOOLEAN
halbtcoutsrc_UnderIps(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PADAPTER			Adapter=pBtCoexist->Adapter;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(Adapter);
	u1Byte	ipsmode = IPS_NONE;
	rt_rf_power_state	rfState;//cur power state

	ipsmode = rtw_get_ips_mode_req(pwrctrlpriv);
	if((rfState == rf_off) && (ipsmode & (IPS_NORMAL|IPS_LEVEL_2)))
	{
		return TRUE;
	}

	return FALSE;
}

//====================================
//		Extern functions called by other module
//====================================
BOOLEAN
EXhalbtcoutsrc_InitlizeVariables(
	IN	PVOID		Adapter
	)
{
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;

	pBtCoexist->statistics.cntBind++;

	halbtcoutsrc_DbgInit();

	if(pBtCoexist->bBinded)
		return FALSE;
	else
		pBtCoexist->bBinded = TRUE;

#if( defined(CONFIG_PCI_HCI))
	pBtCoexist->chipInterface = BTC_INTF_PCI;
#elif( defined(CONFIG_USB_HCI))
	pBtCoexist->chipInterface = BTC_INTF_USB;
#elif( defined(CONFIG_SDIO_HCI))
	pBtCoexist->chipInterface = BTC_INTF_SDIO;
#elif( defined(CONFIG_GSPI_HCI))
	pBtCoexist->chipInterface = BTC_INTF_GSPI;
#else
	pBtCoexist->chipInterface = BTC_INTF_UNKNOWN;
#endif

	if(NULL == pBtCoexist->Adapter)
	{
		pBtCoexist->Adapter = Adapter;
	}

	pBtCoexist->stackInfo.bProfileNotified = FALSE;

	pBtCoexist->fBtcRead1Byte = halbtcoutsrc_Read1Byte;
	pBtCoexist->fBtcWrite1Byte = halbtcoutsrc_Write1Byte;
	pBtCoexist->fBtcWrite1ByteBitMask = halbtcoutsrc_BitMaskWrite1Byte;
	pBtCoexist->fBtcRead2Byte = halbtcoutsrc_Read2Byte;
	pBtCoexist->fBtcWrite2Byte = halbtcoutsrc_Write2Byte;
	pBtCoexist->fBtcRead4Byte = halbtcoutsrc_Read4Byte;
	pBtCoexist->fBtcWrite4Byte = halbtcoutsrc_Write4Byte;

	pBtCoexist->fBtcSetBbReg = halbtcoutsrc_SetBbReg;
	pBtCoexist->fBtcGetBbReg = halbtcoutsrc_GetBbReg;

	pBtCoexist->fBtcSetRfReg = halbtcoutsrc_SetRfReg;
	pBtCoexist->fBtcGetRfReg = halbtcoutsrc_GetRfReg;

	pBtCoexist->fBtcFillH2c = halbtcoutsrc_FillH2cCmd;
	pBtCoexist->fBtcDispDbgMsg = halbtcoutsrc_DisplayDbgMsg;

	pBtCoexist->fBtcGet = halbtcoutsrc_Get;
	pBtCoexist->fBtcSet = halbtcoutsrc_Set;

	pBtCoexist->cliBuf = &GLBtcDbgBuf[0];

	pBtCoexist->btInfo.bBtCtrlAggBufSize = FALSE;
	pBtCoexist->btInfo.aggBufSize = 8;

	return TRUE;
}

VOID
EXhalbtcoutsrc_InitHwConfig(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cntInitHwConfig++;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_InitHwConfig(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_InitHwConfig(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_InitHwConfig(pBtCoexist);
	}
}

VOID
EXhalbtcoutsrc_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cntInitCoexDm++;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_InitCoexDm(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_InitCoexDm(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_InitCoexDm(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		//EXhalbtc8812a1ant_InitCoexDm(pBtCoexist);
	}

	pBtCoexist->bInitilized = TRUE;
}

VOID
EXhalbtcoutsrc_IpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	u1Byte	ipsType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntIpsNotify++;
	if(pBtCoexist->bManualControl)
		return;

	if(rf_off == type)
		ipsType = BTC_IPS_ENTER;
	else
		ipsType = BTC_IPS_LEAVE;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_IpsNotify(pBtCoexist, ipsType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_IpsNotify(pBtCoexist, ipsType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_IpsNotify(pBtCoexist, ipsType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_IpsNotify(pBtCoexist, ipsType);
	}
}

VOID
EXhalbtcoutsrc_LpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	u1Byte	lpsType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntLpsNotify++;
	if(pBtCoexist->bManualControl)
		return;

	if(PS_STATE_S0 == type ||PS_STATE_S2 == type)
		lpsType = BTC_LPS_ENABLE;
	else
		lpsType = BTC_LPS_DISABLE;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_LpsNotify(pBtCoexist, lpsType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_LpsNotify(pBtCoexist, lpsType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_LpsNotify(pBtCoexist, lpsType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_LpsNotify(pBtCoexist, lpsType);
	}
}

VOID
EXhalbtcoutsrc_ScanNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	u1Byte	scanType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntScanNotify++;
	if(pBtCoexist->bManualControl)
		return;

	if(type)
		scanType = BTC_SCAN_START;
	else
		scanType = BTC_SCAN_FINISH;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_ScanNotify(pBtCoexist, scanType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_ScanNotify(pBtCoexist, scanType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_ScanNotify(pBtCoexist, scanType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_ScanNotify(pBtCoexist, scanType);
	}
}

VOID
EXhalbtcoutsrc_ConnectNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			action
	)
{
	u1Byte	assoType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntConnectNotify++;
	if(pBtCoexist->bManualControl)
		return;

	if(action)
		assoType = BTC_ASSOCIATE_START;
	else
		assoType = BTC_ASSOCIATE_FINISH;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_ConnectNotify(pBtCoexist, assoType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_ConnectNotify(pBtCoexist, assoType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_ConnectNotify(pBtCoexist, assoType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_ConnectNotify(pBtCoexist, assoType);
	}
}

VOID
EXhalbtcoutsrc_MediaStatusNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	RT_MEDIA_STATUS	mediaStatus
	)
{
	u1Byte	mStatus;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntMediaStatusNotify++;
	if(pBtCoexist->bManualControl)
		return;

	if(RT_MEDIA_CONNECT == mediaStatus)
		mStatus = BTC_MEDIA_CONNECT;
	else
		mStatus = BTC_MEDIA_DISCONNECT;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_MediaStatusNotify(pBtCoexist, mStatus);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_MediaStatusNotify(pBtCoexist, mStatus);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_MediaStatusNotify(pBtCoexist, mStatus);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_MediaStatusNotify(pBtCoexist, mStatus);
	}
}

VOID
EXhalbtcoutsrc_SpecialPacketNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			pktType
	)
{
	u1Byte	packetType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntSpecialPacketNotify++;
	if(pBtCoexist->bManualControl)
		return;

	//if(PACKET_DHCP == pktType)
		packetType = BTC_PACKET_DHCP;
	//else if(PACKET_EAPOL == pktType)
	//	packetType = BTC_PACKET_EAPOL;
	//else
	//	packetType = BTC_PACKET_UNKNOWN;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_SpecialPacketNotify(pBtCoexist, packetType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_SpecialPacketNotify(pBtCoexist, packetType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_SpecialPacketNotify(pBtCoexist, packetType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_SpecialPacketNotify(pBtCoexist, packetType);
	}
}

VOID
EXhalbtcoutsrc_BtInfoNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	pu1Byte			tmpBuf,
	IN	u1Byte			length
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntBtInfoNotify++;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_BtInfoNotify(pBtCoexist, tmpBuf, length);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_BtInfoNotify(pBtCoexist, tmpBuf, length);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_BtInfoNotify(pBtCoexist, tmpBuf, length);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_BtInfoNotify(pBtCoexist, tmpBuf, length);
	}
}

VOID
EXhalbtcoutsrc_StackOperationNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	u1Byte	stackOpType;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntStackOperationNotify++;
	if(pBtCoexist->bManualControl)
		return;
#if 0
	if( (HCI_BT_OP_INQUIRY_START == type) ||
		(HCI_BT_OP_PAGING_START == type) ||
		(HCI_BT_OP_PAIRING_START == type))
	{
		stackOpType = BTC_STACK_OP_INQ_PAGE_PAIR_START;
	}
	else if( (HCI_BT_OP_INQUIRY_FINISH == type) ||
		(HCI_BT_OP_PAGING_SUCCESS == type) ||
		(HCI_BT_OP_PAGING_UNSUCCESS == type) ||
		(HCI_BT_OP_PAIRING_FINISH == type) )
	{
		stackOpType = BTC_STACK_OP_INQ_PAGE_PAIR_FINISH;
	}
	else
	{
		stackOpType = BTC_STACK_OP_NONE;
	}
#else
	stackOpType = BTC_STACK_OP_NONE;
#endif
	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_StackOperationNotify(pBtCoexist, stackOpType);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_StackOperationNotify(pBtCoexist, stackOpType);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_StackOperationNotify(pBtCoexist, stackOpType);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_StackOperationNotify(pBtCoexist, stackOpType);
	}
}

VOID
EXhalbtcoutsrc_HaltNotify(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_HaltNotify(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_HaltNotify(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_HaltNotify(pBtCoexist);
		if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723a1ant_HaltNotify(pBtCoexist);
	}
}

VOID
EXhalbtcoutsrc_PnpNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			pnpState
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_PnpNotify(pBtCoexist, pnpState);
	}
}

VOID
EXhalbtcoutsrc_Periodical(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntPeriodical++;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_Periodical(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_Periodical(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_Periodical(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
		{
			if(!halbtcoutsrc_UnderIps(pBtCoexist))
				EXhalbtc8723a1ant_Periodical(pBtCoexist);
		}
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_Periodical(pBtCoexist);
	}
}

VOID
EXhalbtcoutsrc_DbgControl(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				opCode,
	IN	u1Byte				opLen,
	IN	pu1Byte				pData
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cntDbgCtrl++;
#if 0
	if(IS_HARDWARE_TYPE_8821(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8821a2ant_DbgControl(pBtCoexist, opCode, opLen, pData);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
		{
			if(!halbtcoutsrc_UnderIps(pBtCoexist))
				EXhalbtc8821a1ant_DbgControl(pBtCoexist, opCode, opLen, pData);
		}
	}
	else if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_DbgControl(pBtCoexist, opCode, opLen, pData);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_DbgControl(pBtCoexist, opCode, opLen, pData);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_DbgControl(pBtCoexist, opCode, opLen, pData);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
		{
			if(!halbtcoutsrc_UnderIps(pBtCoexist))
				EXhalbtc8723a1ant_DbgControl(pBtCoexist, opCode, opLen, pData);
		}
	}
	else if(IS_HARDWARE_TYPE_8192C(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8188c2ant_DbgControl(pBtCoexist, opCode, opLen, pData);
	}
	else if(IS_HARDWARE_TYPE_8192D(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8192d2ant_DbgControl(pBtCoexist, opCode, opLen, pData);
	}
	else

	if(IS_HARDWARE_TYPE_8192E(pBtCoexist->Adapter))
	{
		EXhalbtc8192e1ant_DbgControl(pBtCoexist, opCode, opLen, pData);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_DbgControl(pBtCoexist, opCode, opLen, pData);
	}
#endif
}

VOID
EXhalbtcoutsrc_StackUpdateProfileInfo(
	)
{
#if 0
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;
	PADAPTER		Adapter=(PADAPTER)GLBtCoexist.Adapter;
	PBT_MGNT		pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;
	u1Byte			i;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stackInfo.bProfileNotified = TRUE;

	pBtCoexist->stackInfo.numOfLink =
		pBtMgnt->ExtConfig.NumberOfACL+pBtMgnt->ExtConfig.NumberOfSCO;

	// reset first
	pBtCoexist->stackInfo.bBtLinkExist = FALSE;
	pBtCoexist->stackInfo.bScoExist = FALSE;
	pBtCoexist->stackInfo.bAclExist = FALSE;
	pBtCoexist->stackInfo.bA2dpExist = FALSE;
	pBtCoexist->stackInfo.bHidExist = FALSE;
	pBtCoexist->stackInfo.bPanExist = FALSE;

	if(!pBtMgnt->ExtConfig.NumberOfACL)
		pBtCoexist->stackInfo.minBtRssi = 0;

	if(pBtCoexist->stackInfo.numOfLink)
	{
		pBtCoexist->stackInfo.bBtLinkExist = TRUE;
		if(pBtMgnt->ExtConfig.NumberOfSCO)
			pBtCoexist->stackInfo.bScoExist = TRUE;
		if(pBtMgnt->ExtConfig.NumberOfACL)
			pBtCoexist->stackInfo.bAclExist = TRUE;
	}

	for(i=0; i<pBtMgnt->ExtConfig.NumberOfACL; i++)
	{
		if(BT_PROFILE_A2DP == pBtMgnt->ExtConfig.aclLink[i].BTProfile)
		{
			pBtCoexist->stackInfo.bA2dpExist = TRUE;
		}
		else if(BT_PROFILE_PAN == pBtMgnt->ExtConfig.aclLink[i].BTProfile)
		{
			pBtCoexist->stackInfo.bPanExist = TRUE;
		}
		else if(BT_PROFILE_HID == pBtMgnt->ExtConfig.aclLink[i].BTProfile)
		{
			pBtCoexist->stackInfo.bHidExist = TRUE;
		}
		else
		{
			pBtCoexist->stackInfo.bUnknownAclExist = TRUE;
		}
	}
#endif
}

VOID
EXhalbtcoutsrc_UpdateMinBtRssi(
	IN	s1Byte	btRssi
	)
{
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stackInfo.minBtRssi = btRssi;
}


VOID
EXhalbtcoutsrc_SetHciVersion(
	IN	u2Byte	hciVersion
	)
{
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stackInfo.hciVersion = hciVersion;
}

VOID
EXhalbtcoutsrc_SetBtPatchVersion(
	IN	u2Byte	btHciVersion,
	IN	u2Byte	btPatchVersion
	)
{
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;

	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->btInfo.btRealFwVer = btPatchVersion;
	pBtCoexist->btInfo.btHciVer = btHciVersion;
}

VOID
EXhalbtcoutsrc_SetBtExist(
	IN	BOOLEAN		bBtExist
	)
{
	GLBtCoexist.boardInfo.bBtExist = bBtExist;
}

VOID
EXhalbtcoutsrc_SetChipType(
	IN	u1Byte		chipType
	)
{
	switch(chipType)
	{
		default:
		case BT_2WIRE:
		case BT_ISSC_3WIRE:
		case BT_ACCEL:
		case BT_RTL8756:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_UNDEF;
			break;
		case BT_CSR_BC4:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_CSR_BC4;
			break;
		case BT_CSR_BC8:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_CSR_BC8;
			break;
		case BT_RTL8723A:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_RTL8723A;
			break;
		case BT_RTL8821:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_RTL8821;
			break;
		case BT_RTL8723B:
			GLBtCoexist.boardInfo.btChipType = BTC_CHIP_RTL8723B;
			break;
	}
}

VOID
EXhalbtcoutsrc_SetAntNum(
	IN	u1Byte		type,
	IN	u1Byte		antNum
	)
{
	if(BT_COEX_ANT_TYPE_PG == type)
	{
		GLBtCoexist.boardInfo.pgAntNum = antNum;
		GLBtCoexist.boardInfo.btdmAntNum = antNum;
	}
	else if(BT_COEX_ANT_TYPE_ANTDIV == type)
	{
		GLBtCoexist.boardInfo.btdmAntNum = antNum;
	}

	GLBtCoexist.boardInfo.btdmAntPos = BTC_ANTENNA_AT_MAIN_PORT;
#ifdef CONFIG_ANTENNA_AT_AUX_PORT
	GLBtCoexist.boardInfo.btdmAntPos = BTC_ANTENNA_AT_AUX_PORT;
#endif
	DBG_871X("Set Ant Pos = %d\n",GLBtCoexist.boardInfo.btdmAntPos);
}

VOID
EXhalbtcoutsrc_DisplayBtCoexInfo(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723b2ant_DisplayCoexInfo(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_DisplayCoexInfo(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_DisplayCoexInfo(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723a1ant_DisplayCoexInfo(pBtCoexist);
	}
	else if(IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
	{
		EXhalbtc8812a1ant_DisplayCoexInfo(pBtCoexist);
	}
}


VOID
EXhalbtcoutsrc_SignalCompensation(PBTC_COEXIST pBtCoexist, u8 *rssi_wifi, u8 *rssi_bt)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_SignalCompensation(pBtCoexist,rssi_wifi,rssi_bt);
	}
	else if(IS_HARDWARE_TYPE_8723A(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 2)
			EXhalbtc8723a2ant_DisplayCoexInfo(pBtCoexist);
		else if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723a1ant_DisplayCoexInfo(pBtCoexist);
	}
}

VOID
EXhalbtcoutsrc_LpsLeave(PBTC_COEXIST pBtCoexist)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_LpsLeave(pBtCoexist);
	}
}
VOID
EXhalbtcoutsrc_LowWiFiTrafficNotify(PBTC_COEXIST pBtCoexist)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	if(IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
	{
		if(pBtCoexist->boardInfo.btdmAntNum == 1)
			EXhalbtc8723b1ant_LowWiFiTrafficNotify(pBtCoexist);
	}
}
#endif

