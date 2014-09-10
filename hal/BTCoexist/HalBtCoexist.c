//============================================================
// Description:
//
// This file is for 92CE/92CU dynamic mechanism
// for BT Co-exist only
//
// By cosa 08/17/2010
//
//============================================================

//============================================================
// include files
//============================================================

#include <drv_types.h>
#include "halbt_precomp.h"

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST
//============================================================
// local function
//============================================================
VOID
btdm_PwdbMonitor(
	IN	PADAPTER	Adapter
	)
{
#if 0
	//PMGNT_INFO 		pMgntInfo = &(GetDefaultAdapter(Adapter)->MgntInfo);
	PBT_MGNT		pBtMgnt = &pMgntInfo->BtInfo.BtMgnt;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte			H2C_Parameter[3] ={0};
	s4Byte			tmpBTEntryMaxPWDB=0, tmpBTEntryMinPWDB=0xff;
	u1Byte			i;

	if(pBtMgnt->BtOperationOn)
	{
		for(i=0; i<MAX_BT_ASOC_ENTRY_NUM; i++)
		{
			if(pMgntInfo->BtInfo.BtAsocEntry[i].bUsed)
			{
				if(pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB < tmpBTEntryMinPWDB)
					tmpBTEntryMinPWDB = pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB;
				if(pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB > tmpBTEntryMaxPWDB)
					tmpBTEntryMaxPWDB = pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB;

				//
				// Report every BT connection (HS mode) RSSI to FW
				//
				H2C_Parameter[2] = (u1Byte)(pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB & 0xFF);
				H2C_Parameter[0] = (HALBT_GetMaxBtHsMacId(Adapter)-1-i);
				RTPRINT(FDM, DM_BT30, ("RSSI report for BT[%d], H2C_Par = 0x%x\n", i, H2C_Parameter[0]));
				FillH2CCmd(Adapter, H2C_RSSI_REPORT, 3, H2C_Parameter);
				RTPRINT(FDM, (DM_PWDB|DM_BT30), ("BT_Entry Mac :"), pMgntInfo->BtInfo.BtAsocEntry[i].BTRemoteMACAddr)
				RTPRINT(FDM, (DM_PWDB|DM_BT30), ("BT rx pwdb[%d] = 0x%lx(%ld)\n", i, pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB,
					pMgntInfo->BtInfo.BtAsocEntry[i].UndecoratedSmoothedPWDB));
			}
		}
		if(tmpBTEntryMaxPWDB != 0)	// If associated entry is found
		{
			pHalData->BT_EntryMaxUndecoratedSmoothedPWDB = tmpBTEntryMaxPWDB;
			RTPRINT(FDM, (DM_PWDB|DM_BT30), ("BT_EntryMaxPWDB = 0x%lx(%ld)\n",
				tmpBTEntryMaxPWDB, tmpBTEntryMaxPWDB));
		}
		else
		{
			pHalData->BT_EntryMaxUndecoratedSmoothedPWDB = 0;
		}
		if(tmpBTEntryMinPWDB != 0xff) // If associated entry is found
		{
			pHalData->BT_EntryMinUndecoratedSmoothedPWDB = tmpBTEntryMinPWDB;
			RTPRINT(FDM, (DM_PWDB|DM_BT30), ("BT_EntryMinPWDB = 0x%lx(%ld)\n",
				tmpBTEntryMinPWDB, tmpBTEntryMinPWDB));
		}
		else
		{
			pHalData->BT_EntryMinUndecoratedSmoothedPWDB = 0;
		}
	}
#endif
}

//============================================================
// extern function
//============================================================
VOID
BTDM_InitlizeVariables(
	IN	PADAPTER	Adapter
	)
{
	EXhalbtcoutsrc_InitlizeVariables((PVOID)Adapter);
}

VOID
BTDM_InitializeAllWorkItem(
	IN	PADAPTER	Adapter
	)
{
}

VOID
BTDM_FreeAllWorkItem(
	IN	PADAPTER	Adapter
	)
{
}

VOID
BTDM_DisplayBtCoexInfo(
	IN	PADAPTER	Adapter
	)
{
	EXhalbtcoutsrc_DisplayBtCoexInfo(&GLBtCoexist);
}


BOOLEAN
BTDM_IsHt40(
	IN	PADAPTER	Adapter
	)
{
	u8 isHT40 = _TRUE;
	HT_CHANNEL_WIDTH bw = Adapter->mlmeextpriv.cur_bwmode;

	if (bw == HT_CHANNEL_WIDTH_20)
	{
		isHT40 = _FALSE;
	}
	else if (bw == HT_CHANNEL_WIDTH_40)
	{
		isHT40 = _TRUE;
	}

	return isHT40;
}

BOOLEAN
BTDM_Legacy(
	IN	PADAPTER	Adapter
	)
{
	struct mlme_ext_priv *pmlmeext;
	u8			isLegacy = _FALSE;

	pmlmeext = &Adapter->mlmeextpriv;
	if ((pmlmeext->cur_wireless_mode == WIRELESS_11B) ||
		(pmlmeext->cur_wireless_mode == WIRELESS_11G) ||
		(pmlmeext->cur_wireless_mode == WIRELESS_11BG))
		isLegacy = _TRUE;

	return isLegacy;
}

s4Byte
BTDM_GetWifiRssi(
	IN	PADAPTER	Adapter
	)
{
#if 1
	struct mlme_priv *pmlmepriv;
	PHAL_DATA_TYPE	pHalData;
	s32			UndecoratedSmoothedPWDB = 0;


	pmlmepriv = &Adapter->mlmepriv;
	pHalData = GET_HAL_DATA(Adapter);

//	if (pMgntInfo->bMediaConnect)	// Default port
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{
		UndecoratedSmoothedPWDB = GET_UNDECORATED_AVERAGE_RSSI(Adapter);
	}
	else // associated entry pwdb
	{
		UndecoratedSmoothedPWDB = pHalData->dmpriv.EntryMinUndecoratedSmoothedPWDB;
		//pHalData->BT_EntryMinUndecoratedSmoothedPWDB
	}
	RTPRINT(FBT, BT_TRACE, ("BTDM_GetRxSS() = %d\n", UndecoratedSmoothedPWDB));
	return UndecoratedSmoothedPWDB;
#else
	PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	s4Byte			UndecoratedSmoothedPWDB=0;
	s4Byte			UndecoratedSmoothedPWDB1=0;

	if(pMgntInfo->bMediaConnect)	// Default port
	{
		UndecoratedSmoothedPWDB = GET_UNDECORATED_AVERAGE_RSSI(Adapter);
		if(BT_Operation(Adapter))
		{
			UndecoratedSmoothedPWDB1 = pHalData->BT_EntryMinUndecoratedSmoothedPWDB;
			if(UndecoratedSmoothedPWDB1 < UndecoratedSmoothedPWDB)
				UndecoratedSmoothedPWDB = UndecoratedSmoothedPWDB1;
		}
	}
	else // associated entry pwdb
	{
		if(BT_Operation(Adapter))
		{
			UndecoratedSmoothedPWDB = pHalData->BT_EntryMinUndecoratedSmoothedPWDB;
		}
		else
		{
			UndecoratedSmoothedPWDB = pHalData->EntryMinUndecoratedSmoothedPWDB;
		}
	}
	RTPRINT(FBT, BT_TRACE, ("[BTCoex], BTDM_GetWifiRssi() = %ld\n", UndecoratedSmoothedPWDB));
	return UndecoratedSmoothedPWDB;
#endif
}

BOOLEAN
BTDM_IsDisableEdcaTurbo(
	IN	PADAPTER	Adapter
	)
{
	//PMGNT_INFO		pMgntInfo=&Adapter->MgntInfo;
	//PBT_MGNT		pBtMgnt=&pMgntInfo->BtInfo.BtMgnt;
	PHAL_DATA_TYPE	pHalData=GET_HAL_DATA(Adapter);
	BOOLEAN			bBtChangeEdca=FALSE;
	u4Byte			curEdcaVal;
	u4Byte			edcaBtHsUpLink=0x5ea42b, edcaBtHsDownLink=0x5ea42b;
	u4Byte			edcaHs;
	u4Byte			edcaAddr=REG_EDCA_VI_PARAM;

	if(!HALBT_IsBtExist(Adapter))
		return FALSE;

	//if(!pBtMgnt->bSupportProfile)
	//	return FALSE;

	if(!BT_Operation(Adapter))
		return FALSE;
#if 0
	if(BTPKT_RemoteHsPeer(Adapter) != BT_PEER_REALTEK)
	{
		edcaBtHsDownLink = 0xa422;
	}
#endif
	curEdcaVal = rtw_read32(Adapter, edcaAddr);
	if(BTDM_IsWifiUplink(Adapter))
	{
		if(curEdcaVal != edcaBtHsUpLink)
		{
			edcaHs = edcaBtHsUpLink;
			bBtChangeEdca = TRUE;
		}
	}
	else
	{
		if(curEdcaVal != edcaBtHsDownLink)
		{
			edcaHs = edcaBtHsDownLink;
			bBtChangeEdca = TRUE;
		}
	}
	if(bBtChangeEdca)
	{
		rtw_write32(Adapter, edcaAddr, edcaHs);
	}

	return TRUE;
}

VOID
BTDM_Coexist(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE* pHalData=GET_HAL_DATA(Adapter);

#if defined(CONFIG_CONCURRENT_MODE)
		if (Adapter->adapter_type != PRIMARY_ADAPTER)
		{
			RTPRINT(FBT, BT_TRACE, ("[DM][BT], CONFIG_CONCURRENT_MODE  !!\n"));
			RTPRINT(FBT, BT_TRACE, ("[DM][BT], padapter->adapter_type != PRIMARY_ADAPTER  !!\n"));
			return;
		}
#endif

	if(!HALBT_IsBtExist(Adapter))
	{
		RTPRINT(FBT, BT_TRACE, ("[BTCoex], BT not exists!!\n"));
		return;
	}
#if 0
	if(!pHalData->bt_coexist.bInitlized)
	{
		RT_DISP(FBT, BT_TRACE, ("[BTCoex], EXhalbtcoutsrc_InitCoexDm()\n"));
		EXhalbtcoutsrc_InitCoexDm(&GLBtCoexist);
		pHalData->bt_coexist.bInitlized = TRUE;
	}
#endif
	btdm_PwdbMonitor(Adapter);
	EXhalbtcoutsrc_Periodical(&GLBtCoexist);
}

s4Byte
BTDM_GetHsWifiRssi(
	IN		PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	s4Byte	UndecoratedSmoothedPWDB=0;

	UndecoratedSmoothedPWDB = pHalData->dmpriv.BT_EntryMinUndecoratedSmoothedPWDB;

	return UndecoratedSmoothedPWDB;
}

BOOLEAN
BTDM_IsBtBusy(
	IN	PADAPTER	Adapter
	)
{
	return GLBtCoexist.btInfo.bBtBusy;
}

BOOLEAN
BTDM_IsLimitedDig(
	IN	PADAPTER	Adapter
	)
{
	return GLBtCoexist.btInfo.bLimitedDig;
}

BOOLEAN
BTDM_IsWifiBusy(
	IN	PADAPTER	Adapter
	)
{

#if 0
	PMGNT_INFO 		pMgntInfo = &(GetDefaultAdapter(Adapter)->MgntInfo);
	PBT30Info		pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_TRAFFIC 	pBtTraffic=&pBTInfo->BtTraffic;

	if(BT_1Ant(Adapter) && BT_Operation(Adapter))
	{
		return TRUE;
	}

	if( (pMgntInfo->bMediaConnect) ||				// Default port, station or Ad-hoc
		(BTHCI_HsConnectionEstablished(Adapter)))	// HS mode
	{
		if(pMgntInfo->LinkDetectInfo.bBusyTraffic ||
			pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic ||
			pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic)
			return TRUE;
	}
	if(IsExtAPModeExist(Adapter))
	{
		PADAPTER		pExtAdapter = GetFirstExtAdapter(Adapter);
		PMGNT_INFO		pExtMgntInfo = NULL;

		pExtMgntInfo = &(pExtAdapter->MgntInfo);

		if( (pExtMgntInfo->LinkDetectInfo.bBusyTraffic))
			return TRUE;
	}

		return FALSE;
#endif
	struct mlme_priv *pmlmepriv = &(GetDefaultAdapter(Adapter)->mlmepriv);
	//PBT30Info		pBTInfo = GET_BT_INFO(Adapter);
	//PBT_TRAFFIC 	pBtTraffic = &pBTInfo->BtTraffic;

	//RTPRINT(FBT, BT_TRACE,("[BTCoex], IsWifiBusy, bBusyTraffic(%d) bTxBusyTraffic(%d), bRxBusyTraffic(%d)\n",	pmlmepriv->LinkDetectInfo.bBusyTraffic));
		//pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic,
		//pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic)
		//);

	if (pmlmepriv->LinkDetectInfo.bBusyTraffic
	//||	pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic ||
	//	pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic
		)
		return _TRUE;
	else
		return _FALSE;
}
BOOLEAN
BTDM_IsWifiUplink(
	IN	PADAPTER	Adapter
	)
{
	//PMGNT_INFO 		pMgntInfo = &(GetDefaultAdapter(Adapter)->MgntInfo);
	struct mlme_priv *pmlmepriv = &(GetDefaultAdapter(Adapter)->mlmepriv);
	//PBT30Info		pBTInfo = &Adapter->MgntInfo.BtInfo;
	//PBT_TRAFFIC		pBtTraffic=&pBTInfo->BtTraffic;

	if(pmlmepriv->LinkDetectInfo.bTxBusyTraffic)
		return TRUE;
	else
		return FALSE;
#if 0
	if( (pMgntInfo->bMediaConnect) ||				// Default port
		(BTHCI_HsConnectionEstablished(Adapter)))	// HS mode
	{
		if( (pMgntInfo->LinkDetectInfo.bTxBusyTraffic) ||
			(pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic))
			return TRUE;
		else
			return FALSE;
	}
	else if(IsExtAPModeExist(Adapter))
	{
		PADAPTER		pExtAdapter = GetFirstExtAdapter(Adapter);
		PMGNT_INFO		pExtMgntInfo = NULL;

		pExtMgntInfo = &(pExtAdapter->MgntInfo);

		if( (pExtMgntInfo->LinkDetectInfo.bTxBusyTraffic))
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
#endif
}

VOID
BTDM_StackOperationNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		stackOpType
	)
{
	RTPRINT(FBT, BT_TRACE, ("[BTCoex], BTDM_StackOperationNotify()\n"));

	EXhalbtcoutsrc_StackOperationNotify(&GLBtCoexist, stackOpType);
}

u1Byte
BTDM_AdjustRssiForCoex(
	IN	PADAPTER	Adapter
	)
{
	u1Byte			adjRssiVal=0;
	PBTC_COEXIST		pBtCoexist=&GLBtCoexist;

	if(!HALBT_IsBtExist(Adapter))
		return 0;

	if(pBtCoexist->Adapter != Adapter)
		return 0;

	if(GLBtCoexist.boardInfo.btdmAntNum == 1)
	{
		adjRssiVal = pBtCoexist->btInfo.rssiAdjustForAgcTableOn+
				pBtCoexist->btInfo.rssiAdjustFor1AntCoexType;
	}
	else if(GLBtCoexist.boardInfo.btdmAntNum == 2)
	{
		adjRssiVal = pBtCoexist->btInfo.rssiAdjustForAgcTableOn;
	}
	return adjRssiVal;
}

VOID
BTDM_InitHalVars(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte	antNum = 2;

#if (MP_DRIVER == 1)
	//pHalData->EEPROMBluetoothCoexist = 0;
#endif
	//pHalData->bt_coexist.bBtExist = pHalData->EEPROMBluetoothCoexist;
	//pHalData->bt_coexist.btTotalAntNum = pHalData->EEPROMBluetoothAntNum;
	//pHalData->bt_coexist.btChipType = pHalData->EEPROMBluetoothType;

	pHalData->bt_coexist.BluetoothCoexist= pHalData->EEPROMBluetoothCoexist;
	pHalData->bt_coexist.BT_Ant_Num = pHalData->EEPROMBluetoothAntNum;
	pHalData->bt_coexist.BT_CoexistType = pHalData->EEPROMBluetoothType;
	//pHalData->bt_coexist.BT_Ant_isolation = pHalData->EEPROMBluetoothAntIsolation;
	//pHalData->bt_coexist.BT_RadioSharedType = pHalData->EEPROMBluetoothRadioShared;

	BTDM_SetBtExist(Adapter);
	BTDM_SetBtChipType(Adapter);
	antNum = HALBT_GetPgAntNum(Adapter);
	BTDM_SetBtCoexAntNum(Adapter, BT_COEX_ANT_TYPE_PG, antNum);

}

VOID
BTDM_SetBtExist(
	IN	PADAPTER			Adapter
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);

	EXhalbtcoutsrc_SetBtExist((BOOLEAN)pHalData->bt_coexist.BluetoothCoexist);
}

VOID
BTDM_SetBtChipType(
	IN	PADAPTER	Adapter
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);

	EXhalbtcoutsrc_SetChipType(pHalData->bt_coexist.BT_CoexistType); //btChipType
}

VOID
BTDM_SetBtCoexAntNum(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type,
	IN	u1Byte		antNum
	)
{
	EXhalbtcoutsrc_SetAntNum(type, antNum);
}

VOID
BTDM_InitHwConfig(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
		return;
	//HALBT_InitHwConfig(Adapter);
	EXhalbtcoutsrc_InitHwConfig(&GLBtCoexist);
	EXhalbtcoutsrc_InitCoexDm(&GLBtCoexist);
}

VOID
BTDM_IpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	)
{
	EXhalbtcoutsrc_IpsNotify(&GLBtCoexist, type);
}

VOID
BTDM_LpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	)
{
	EXhalbtcoutsrc_LpsNotify(&GLBtCoexist, type);
}

VOID
BTDM_ScanNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		scanType
	)
{
	EXhalbtcoutsrc_ScanNotify(&GLBtCoexist, scanType);
}

VOID
BTDM_ConnectNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		action
	)
{
	EXhalbtcoutsrc_ConnectNotify(&GLBtCoexist, action);
}

VOID
BTDM_MediaStatusNotify(
	IN	PADAPTER			Adapter,
	IN	RT_MEDIA_STATUS		mstatus
	)
{
	EXhalbtcoutsrc_MediaStatusNotify(&GLBtCoexist, mstatus);
}

VOID
BTDM_SpecialPacketNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		pktType
	)
{
	EXhalbtcoutsrc_SpecialPacketNotify(&GLBtCoexist, pktType);
}

VOID
BTDM_BtInfoNotify(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		tmpBuf,
	IN	u1Byte		length
	)
{
	RTPRINT(FBT, BT_TRACE, ("[BTCoex], ================BTDM_BtInfoNotify==========================\n"));
	EXhalbtcoutsrc_BtInfoNotify(&GLBtCoexist, tmpBuf, length);
	RTPRINT(FBT, BT_TRACE, ("[BTCoex], ================END BTDM_BtInfoNotify======================\n"));
}

BOOLEAN
BTDM_IsBtDisabled(
	IN	PADAPTER	Adapter
	)
{
	if(GLBtCoexist.btInfo.bBtDisabled)
		return TRUE;
	else
		return FALSE;
}

VOID
BTDM_ApStatusWatchdog(
	IN	PADAPTER	Adapter
	)
{
	//PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	static BOOLEAN	bPreStaAssociated=FALSE;
	BOOLEAN			bCurStaAssociated=FALSE;

	bCurStaAssociated = BT_AnyClientConnectToAp(Adapter);

#if 0
	if(bPreStaAssociated != bCurStaAssociated)
	{
		if( (pMgntInfo->bMediaConnect) ||				// Default port, station or Ad-hoc
			(BTHCI_HsConnectionEstablished(Adapter)))	// HS mode
		{
			// default port is connected, don't inform BT
			// wifi chnl&BW info for AP mode because
			// default port already did it.
		}
		else if(IsExtAPModeExist(Adapter))
		{
			PADAPTER		pExtAdapter = GetFirstExtAdapter(Adapter);

			if(bCurStaAssociated)
			{
				BT_MediaStatusNotify(pExtAdapter, RT_MEDIA_CONNECT);
			}
			else
			{
				BT_MediaStatusNotify(pExtAdapter, RT_MEDIA_DISCONNECT);
			}
		}
		bPreStaAssociated = bCurStaAssociated;
	}
#endif
}

VOID
BTDM_RejectApAggregatedPacket(
	IN	PADAPTER	Adapter,
	IN	BOOLEAN		bReject
	)
{
	struct mlme_ext_info		*pmlmeinfo = &Adapter->mlmeextpriv.mlmext_info;

	if(bReject)
	{
		//RTPRINT(FBT, BT_TRACE, ("[BTCoex], Set to Reject Ampdu\n"));
		if (pmlmeinfo->bAcceptAddbaReq)
		{
			RTPRINT(FBT, BT_TRACE, ("[BTCoex], Reject Ampdu progress\n"));
			pmlmeinfo->bAcceptAddbaReq = FALSE;
			//if(GetTs(Adapter, (PTS_COMMON_INFO*)(&pRxTs), pMgntInfo->Bssid, 0, RX_DIR, FALSE))
			send_delba(Adapter,0, get_my_bssid(&(pmlmeinfo->network)));
		}
	}
	else
	{
		//RTPRINT(FBT, BT_TRACE, ("[BTCoex], Set to Accept Ampdu\n"));
		if (!pmlmeinfo->bAcceptAddbaReq)
		{
			RTPRINT(FBT, BT_TRACE, ("[BTCoex], Accept Ampdu progress\n"));
			pmlmeinfo->bAcceptAddbaReq = TRUE;
		}
	}
}

VOID
BTDM_SetManualControl(
	IN	BOOLEAN		bManual
	)
{
	GLBtCoexist.bManualControl = bManual;
}

VOID
BTDM_SignalCompensation(PADAPTER padapter, u8 *rssi_wifi, u8 *rssi_bt)
{
	EXhalbtcoutsrc_SignalCompensation(&GLBtCoexist,rssi_wifi,rssi_bt);
}

VOID
BTDM_LpsLeave(PADAPTER padapter)
{
	EXhalbtcoutsrc_LpsLeave(&GLBtCoexist);
}

VOID
BTDM_LowWiFiTrafficNotify(PADAPTER padapter)
{
	EXhalbtcoutsrc_LowWiFiTrafficNotify(&GLBtCoexist);
}
#endif
