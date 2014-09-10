//==================================================
//
//	This module is for BT related common code.
//
//		04/13/2011 created by Cosa
//
//==================================================

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>

#ifdef CONFIG_RTL8723B

//#include "../hal/BTCoexist/halbt_precomp.h"
#include "../hal/BTCoexist/HalBtCoexist.h"
#include "../hal/BTCoexist/HalBtcOutSrc.h"

u1Byte	testbuf[10];	// this is only for compile when BT_30_SUPPORT = 0

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST

static u1Byte	btDbgBuf[BT_TMP_BUF_SIZE];

#if 0
VOID
bt_IpsNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_LpsNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_ScanNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_ConnectNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_MediaStatusNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_SpecialPacketNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_BtInfoNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_RfStatusNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_StackOperationNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_HaltNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_PnpNotifyWorkItemCallback(
	IN PVOID		pContext
	);
VOID
bt_DbgControlWorkItemCallback(
	IN PVOID		pContext
	);

VOID
(*bt_NotifyWorkItemCallback[]) (IN	PVOID	pContext)={
	bt_IpsNotifyWorkItemCallback,
	bt_LpsNotifyWorkItemCallback,
	bt_ScanNotifyWorkItemCallback,
	bt_ConnectNotifyWorkItemCallback,
	bt_MediaStatusNotifyWorkItemCallback,
	bt_SpecialPacketNotifyWorkItemCallback,
	bt_BtInfoNotifyWorkItemCallback,
	bt_RfStatusNotifyWorkItemCallback,
	bt_StackOperationNotifyWorkItemCallback,
	bt_HaltNotifyWorkItemCallback,
	bt_PnpNotifyWorkItemCallback,
	bt_DbgControlWorkItemCallback
};

//==================================================
//	local function
//==================================================
VOID
bt_IpsNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], IPS Notify workitem callback for Adapter=0x%x, ipsType=%d\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.ipsType));

	BTDM_IpsNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.ipsType);
}
VOID
bt_LpsNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], LPS Notify workitem callback for Adapter=0x%x, lpsType=%d\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.lpsType));

	BTDM_LpsNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.lpsType);
}
VOID
bt_ScanNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], SCAN Notify workitem callback for Adapter=0x%x, scanType=%d\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.scanType));

	BTHCI_ScanNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.scanType);
	BTDM_ScanNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.scanType);
}
VOID
bt_ConnectNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Connect Notify workitem callback for Adapter=0x%x, actionType=%d\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.actionType));

	BTDM_ConnectNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.actionType);
}

VOID
bt_MediaStatusNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Media status Notify workitem callback for Adapter=0x%x, mediaStatus=0x%x\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.mediaStatus));

	BTDM_MediaStatusNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.mediaStatus);
}
VOID
bt_SpecialPacketNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Special packet Notify workitem callback for Adapter=0x%x, pktType=%d\n",
		pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.pktType));

	//if(pBtCtrlInfo->notifyData.pktType == PACKET_DHCP)
	{
		BTDM_SpecialPacketNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.pktType);
	}
}
VOID
bt_BtInfoNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], BT Info Notify workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	BTDM_BtInfoNotify(pBtCtrlInfo->Adapter, &pBtCtrlInfo->notifyData.btInfo[0], pBtCtrlInfo->notifyData.btInfoLen);
}
VOID
bt_RfStatusNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], RF Status Notify workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], preHwSwRfOffReason=0x%x, newRfState=0x%x, rfStateChangeSource=0x%x\n",
		pBtCtrlInfo->notifyData.preHwSwRfOffReason,
		pBtCtrlInfo->notifyData.newRfState,
		pBtCtrlInfo->notifyData.rfStateChangeSource));

	BTHCI_RfStatusNotify(pBtCtrlInfo->Adapter,
		(RT_RF_CHANGE_SOURCE)pBtCtrlInfo->notifyData.preHwSwRfOffReason,
		(RT_RF_POWER_STATE)pBtCtrlInfo->notifyData.newRfState,
		(RT_RF_CHANGE_SOURCE)pBtCtrlInfo->notifyData.rfStateChangeSource);
}

VOID
bt_StackOperationNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Stack Operation Notify workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], stackOpType=0x%x\n",
		pBtCtrlInfo->notifyData.stackOpType));

	BTDM_StackOperationNotify(pBtCtrlInfo->Adapter, pBtCtrlInfo->notifyData.stackOpType);
}

VOID
bt_HaltNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Halt Notify workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	EXhalbtcoutsrc_HaltNotify(&GLBtCoexist);
}

VOID
bt_PnpNotifyWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Pnp Notify workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	EXhalbtcoutsrc_PnpNotify(&GLBtCoexist, pBtCtrlInfo->notifyData.pnpState);
}

VOID
bt_DbgControlWorkItemCallback(
	IN PVOID		pContext
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=(PBT_CTRL_INFO)pContext;

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], Dbg Control workitem callback for Adapter=0x%x\n",
		pBtCtrlInfo->Adapter));

	EXhalbtcoutsrc_DbgControl(&GLBtCoexist,
		pBtCtrlInfo->notifyData.dbgCtrlOpCode,
		pBtCtrlInfo->notifyData.dbgCtrlInfoLen,
		&pBtCtrlInfo->notifyData.dbgCtrlInfo[0]);
}

VOID
bt_InitializeBtCtrlWorkItem(
	IN	PADAPTER	Adapter
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	u1Byte	i=0;

	for(i=0; i<NOTIFY_MAX; i++)
	{
		PlatformInitializeWorkItem(
			Adapter,
			&(pBtCtrlInfo->notifyWorkItem[i]),
			(RT_WORKITEM_CALL_BACK)bt_NotifyWorkItemCallback[i],
			(PVOID)pBtCtrlInfo,
			"notifyWorkItem");
	}
}

VOID
bt_FreeBtCtrlWorkItem(
	IN	PADAPTER	Adapter
	)
{
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	u1Byte	i;

	for(i=0; i<NOTIFY_MAX; i++)
	{
		PlatformFreeWorkItem(&(pBtCtrlInfo->notifyWorkItem[i]));
	}
}


RT_STATUS
bt_DataListInitialize(
	IN	PADAPTER	Adapter
	)
{
	RT_STATUS 				rtStatus = RT_STATUS_SUCCESS;
	PRT_TX_LOCAL_BUFFER		pLocalBuffer;
	u4Byte			i;

	do{
#if (SENDTXMEHTOD==0 || SENDTXMEHTOD==2)
		Adapter->BTDataListBuf.Length = Adapter->NumBTTotalDataBlock*sizeof(RT_TX_LOCAL_BUFFER);
		rtStatus = PlatformAllocateMemory(Adapter, &Adapter->BTDataListBuf.Ptr,Adapter->BTDataListBuf.Length);
		if(rtStatus != RT_STATUS_SUCCESS)
		{
			RT_DISP(FIOCTL, IOCTL_STATE, ("bt_DataListInitialize allocate BTDataListBuf fail!!\n"));
			break;
		}
		PlatformZeroMemory(Adapter->BTDataListBuf.Ptr, Adapter->BTDataListBuf.Length);

		pLocalBuffer = (PRT_TX_LOCAL_BUFFER)Adapter->BTDataListBuf.Ptr;
		for(i=0; i<Adapter->NumBTTotalDataBlock; i++)
		{
			rtStatus = PlatformAllocateSharedMemory(Adapter, &pLocalBuffer[i].Buffer, BTMaxDataBlockLen);
			if(rtStatus!=RT_STATUS_SUCCESS)
				break;

			RTInsertTailListWithCnt(&Adapter->BTDataIdleQueue, &pLocalBuffer[i].List, &Adapter->NumIdleBTDataBlock);
		}
		RT_DISP(FIOCTL, IOCTL_STATE, ("Insert %d  into BT data idle queue.", Adapter->NumIdleBTDataBlock));
#endif

		//2 Allocate BT local buffers
		Adapter->BTDataLocalBuf.Length=Adapter->NumBTDataLocalBuf*sizeof(RT_TX_LOCAL_BUFFER);
		rtStatus=PlatformAllocateMemory(Adapter, &Adapter->BTDataLocalBuf.Ptr,Adapter->BTDataLocalBuf.Length);
		if(rtStatus!=RT_STATUS_SUCCESS)
			break;
		PlatformZeroMemory(Adapter->BTDataLocalBuf.Ptr,Adapter->BTDataLocalBuf.Length);
		Adapter->irp_dbg.bBTLocalBufPtr = _TRUE;

		pLocalBuffer=(PRT_TX_LOCAL_BUFFER)Adapter->BTDataLocalBuf.Ptr;
		for(i=0;i<Adapter->NumBTDataLocalBuf;i++)
		{
			rtStatus = PlatformAllocateSharedMemory(Adapter, &pLocalBuffer[i].Buffer, BTMaxDataBlockLen);
			if(rtStatus!=RT_STATUS_SUCCESS)
				break;
			else
				Adapter->irp_dbg.BTLocalBufAllocCnt++;

			RTInsertTailListWithCnt(&Adapter->BTLocalBufQueue, &pLocalBuffer[i].List, &Adapter->NumBTLocalBufIdle);
		}

	}while(_FALSE);

	return rtStatus;
}

VOID
bt_FreeBTData(
	IN	PADAPTER	Adapter
	)
{
	PRT_TX_LOCAL_BUFFER		pLocalBuffer;

#if (SENDTXMEHTOD==0 ||SENDTXMEHTOD==2)
	u4Byte	nFreed;

	if(!RTIsListEmpty(&Adapter->BTDataTxQueue))
	{
		while(!RTIsListEmpty(&Adapter->BTDataTxQueue))
		{
			pLocalBuffer = (PRT_TX_LOCAL_BUFFER)RTRemoveHeadListWithCnt(&Adapter->BTDataTxQueue, &Adapter->NumTxBTDataBlock);
			RTInsertTailListWithCnt(&Adapter->BTDataIdleQueue, &pLocalBuffer->List, &Adapter->NumIdleBTDataBlock);
		}
	}
	if(!RTIsListEmpty(&Adapter->BTDataTxWaitQueue))
	{
		while(!RTIsListEmpty(&Adapter->BTDataTxWaitQueue))
		{
			pLocalBuffer = (PRT_TX_LOCAL_BUFFER)RTRemoveHeadListWithCnt(&Adapter->BTDataTxWaitQueue, &Adapter->NumTxBTDataWaitBlock);
			RTInsertTailListWithCnt(&Adapter->BTDataIdleQueue, &pLocalBuffer->List, &Adapter->NumIdleBTDataBlock);
		}
	}

	if(!RTIsListEmpty(&Adapter->BTDataIdleQueue))
	{
		nFreed=0;
		while(!RTIsListEmpty(&Adapter->BTDataIdleQueue))
		{
			pLocalBuffer=(PRT_TX_LOCAL_BUFFER)RTRemoveHeadListWithCnt(&Adapter->BTDataIdleQueue, &Adapter->NumIdleBTDataBlock);

			PlatformFreeSharedMemory(Adapter, &pLocalBuffer->Buffer);
			nFreed++;
		}
		if((nFreed != Adapter->NumBTTotalDataBlock) ||
			Adapter->NumIdleBTDataBlock != 0)
		{
			RT_DISP(FIOCTL, IOCTL_STATE,("Error!! NumIdleBTDataBlock = %d, free(%d) less than allocated (%d)\n", Adapter->NumIdleBTDataBlock, nFreed, Adapter->NumBTTotalDataBlock));
		}
	}

	if(Adapter->BTDataListBuf.Ptr != NULL)
	{
		PlatformFreeMemory(Adapter->BTDataListBuf.Ptr,Adapter->BTDataListBuf.Length);
	}
#endif
	//Free BT local buffers
	if(!RTIsListEmpty(&Adapter->BTLocalBufQueue))
	{
		//nFreed=0;
		Adapter->irp_dbg.BTLocalBufFreeCnt = 0;
		while(!RTIsListEmpty(&Adapter->BTLocalBufQueue))
		{
			pLocalBuffer=(PRT_TX_LOCAL_BUFFER)RTRemoveHeadListWithCnt(&Adapter->BTLocalBufQueue, &Adapter->NumBTLocalBufIdle);

			PlatformFreeSharedMemory(Adapter, &pLocalBuffer->Buffer);
			Adapter->irp_dbg.BTLocalBufFreeCnt++;
		}
		//RT_ASSERT(nFreed==Adapter->NumBTDataLocalBuf,("Freed BT local buffer(%ld) less than allocated(%ld)!!\n", nFreed, Adapter->NumBTDataLocalBuf));
	}
	if(Adapter->BTDataLocalBuf.Ptr != NULL)
	{
		PlatformFreeMemory(Adapter->BTDataLocalBuf.Ptr,Adapter->BTDataLocalBuf.Length);
	}
}
#endif

//==================================================
//	extern function
//==================================================
//
// Make sure this function is called before enter IPS (rf not turn off yet) and
// after leave IPS(rf already turned on)
//
VOID
BT_IpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	//pBtCtrlInfo->notifyData.ipsType = type;
	BTDM_IpsNotify(Adapter,type);
	//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_IPS]));
}
VOID
BT_LpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	//pBtCtrlInfo->notifyData.lpsType = type;
	//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_LPS]));
	BTDM_LpsNotify(Adapter, type);
}

VOID
BT_ScanNotify(
	IN	PADAPTER 	Adapter,
	IN	u1Byte		scanType
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	//pBtCtrlInfo->notifyData.scanType = scanType;
	//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_SCAN]));
	BTDM_ScanNotify(Adapter,scanType);
}
VOID
BT_WiFiConnectNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		actionType
	)
{
	// action :
	// _TRUE = wifi connection start
	// _FALSE = wifi connection finished
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;
#if 0
	if(pBtCtrlInfo->notifyData.actionType != actionType)
	{
		pBtCtrlInfo->notifyData.actionType = actionType;
		PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_CONNECTION]));
	}
#endif
	BTDM_ConnectNotify(Adapter,actionType);
}
VOID
BT_WifiMediaStatusNotify(
	IN	PADAPTER			Adapter,
	IN	RT_MEDIA_STATUS		mstatus
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	//pBtCtrlInfo->notifyData.mediaStatus = (u1Byte)mstatus;
	//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_MEDIA_STATUS]));
	BTDM_MediaStatusNotify(Adapter, mstatus);
}
VOID
BT_SpecialPacketNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		pktType
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	BTDM_SpecialPacketNotify(Adapter,pktType);
	//pBtCtrlInfo->notifyData.pktType = pktType;
	//if(pBtCtrlInfo->notifyData.pktType == PACKET_DHCP)
	//{
		//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_SPECIAL_PACKET]));
	//}
}

VOID
BT_BtInfoNotify(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		tmpBuf,
	IN	u1Byte		length
	)
{
	//PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	if(length <= BT_CTRL_BTINFO_BUF)
	{
		//pBtCtrlInfo->notifyData.btInfoLen = length;
		//PlatformMoveMemory(&pBtCtrlInfo->notifyData.btInfo[0], tmpBuf, length);
		//PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_BT_INFO]));
		BTDM_BtInfoNotify(Adapter, tmpBuf, length);
	}
}

VOID
BT_RfStatusNotify(
	IN	PADAPTER				Adapter,
	IN	rt_rf_power_state		StateToSet,
	IN	u4Byte	ChangeSource
	)
{
#if 0
	PBT_CTRL_INFO		pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PMGNT_INFO			pMgntInfo=&(Adapter->MgntInfo);
	RT_RF_CHANGE_SOURCE	RfOffReason=pMgntInfo->RfOffReason;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	// We only have to check RF On/Off by HW/SW,
	// if rf is off by IPS or LPS, don't consider here
	if(ChangeSource < RF_CHANGE_BY_HW)
	{
		return;
	}
	RfOffReason &= (RF_CHANGE_BY_HW|RF_CHANGE_BY_SW);

	RT_DISP(FBT, BT_NOTIFY_EVENT, ("[BT], RF Status Notify for Adapter=0x%x\n",
		Adapter));

	pBtCtrlInfo->notifyData.preHwSwRfOffReason = RfOffReason;
	pBtCtrlInfo->notifyData.newRfState = StateToSet;
	pBtCtrlInfo->notifyData.rfStateChangeSource = ChangeSource;
	PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_RF_STATUS]));
#endif
}

VOID
BT_StackOperationNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		opType
	)
{
#if 0
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	pBtCtrlInfo->notifyData.stackOpType = opType;
	PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_BT_STACK_OPERATION]));
#endif
}

VOID
BT_HaltNotify(
	IN	PADAPTER	Adapter
	)
{
	EXhalbtcoutsrc_HaltNotify(&GLBtCoexist);
#if 0
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_HALT]));
#endif
}

BOOLEAN
BT_AnyClientConnectToAp(
	IN	PADAPTER	Adapter
	)
{
	//PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	//PADAPTER		pExtAdapter=NULL, pDefaultAdapter=Adapter;
	//PMGNT_INFO		pExtMgntInfo;
	BOOLEAN			bCurStaAssociated = _FALSE;
	struct sta_priv *pstapriv = &Adapter->stapriv;

	// client is connected to AP mode(vwifi or softAp)
	if((pstapriv->asoc_sta_count > 2))
		bCurStaAssociated = _TRUE;
	return bCurStaAssociated;
}

BOOLEAN
BT_Operation(
	IN	PADAPTER	Adapter
	)
{
#if 0
	PBT30Info	pBTInfo = GET_BT_INFO(Adapter);//&Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;

	if(pBtMgnt->BtOperationOn)
		return _TRUE;
	else
#endif
		return _FALSE;
}

//#ifdef BT_HCI
#if 0
VOID
BT_PnpNotify(
	IN	PADAPTER			Adapter,
	IN	u1Byte				pnpState
	)
{
#if 0
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	pBtCtrlInfo->notifyData.pnpState = pnpState;
	PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_WIFI_PNP]));
#endif
}

VOID
BT_DbgControl(
	IN	PADAPTER			Adapter,
	IN	u1Byte				opCode,
	IN	u1Byte				opLen,
	IN	pu1Byte				pData
	)
{
#if 0
	PBT_CTRL_INFO	pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;
	PADAPTER		pDefaultAdapter=GetDefaultAdapter(Adapter);

	if(pDefaultAdapter != Adapter)
		return;

	if(!HALBT_IsBtExist(Adapter))
		return;

	if(opLen <= BT_DBG_CONTROL_LEN_MAX)
	{
		pBtCtrlInfo->notifyData.dbgCtrlOpCode = opCode;
		pBtCtrlInfo->notifyData.dbgCtrlInfoLen = opLen;
		PlatformMoveMemory(&pBtCtrlInfo->notifyData.dbgCtrlInfo[0], pData, opLen);
		PlatformScheduleWorkItem(&(pBtCtrlInfo->notifyWorkItem[NOTIFY_DBG_CONTROL]));
	}
#endif
}

BOOLEAN
BT_AnyClientConnectToAp(
	IN	PADAPTER	Adapter
	)
{
	//PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	//PADAPTER		pExtAdapter=NULL, pDefaultAdapter=Adapter;
	//PMGNT_INFO		pExtMgntInfo;
	BOOLEAN			bCurStaAssociated = _FALSE;
	struct sta_priv *pstapriv = &Adapter->stapriv;

	// client is connected to AP mode(vwifi or softAp)
	if((pstapriv->asoc_sta_count > 2))
		bCurStaAssociated = _TRUE;
	return bCurStaAssociated;
}

BOOLEAN
BT_Operation(
	IN	PADAPTER	Adapter
	)
{
#if 0
	PBT30Info	pBTInfo = GET_BT_INFO(Adapter);//&Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;

	if(pBtMgnt->BtOperationOn)
		return _TRUE;
	else
#endif
		return _FALSE;
}

BOOLEAN
BT_IsHsBusy(
	IN	PADAPTER	Adapter
	)
{
#if 0
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_TRAFFIC 	pBtTraffic=&pBTInfo->BtTraffic;

	if(BT_Operation(Adapter))
	{
		if( pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic ||
			pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic )
			return _TRUE;
	}
#endif
	return _FALSE;
}

BOOLEAN
BT_IsBtScan(
	IN	PADAPTER	Adapter
	)
{
	return _FALSE;
#if 0
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;

	if(pBtMgnt->bBtScan)
		return _TRUE;
	else
		return _FALSE;
#endif
}

BOOLEAN
BT_ConnectionProcess(
	IN	PADAPTER	Adapter
	)
{
	return _FALSE;
#if 0
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;

	if(pBtMgnt->bBTConnectInProgress)
		return _TRUE;
	else
		return _FALSE;
#endif
}

BOOLEAN
BT_JoinerScanProgress(
	IN	PADAPTER	Adapter
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;
	u1Byte		EntryNum=MAX_BT_ASOC_ENTRY_NUM;

	EntryNum = pBtMgnt->CurrentConnectEntryNum;

	if( (pBtMgnt->bBTConnectInProgress) &&
		(EntryNum < MAX_BT_ASOC_ENTRY_NUM) &&
		(pBTInfo->BtAsocEntry[EntryNum].AMPRole == AMP_BTAP_JOINER) )
	{
		RT_DISP(FIOCTL, IOCTL_STATE, ("[BT], Joiner scan is under progress!!\n"));
		return _TRUE;
	}
	else
		return _FALSE;
}

VOID
BT_JoinerStartToConnect(
	IN	PADAPTER	Adapter
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt = &pBTInfo->BtMgnt;

	if(pBtMgnt->bBtScan)
	{
		RT_DISP(FIOCTL, IOCTL_STATE, ("[Bt scan], customized scan ended!!\n"));
		RT_DISP(FIOCTL, IOCTL_STATE, ("[BT], Joiner start the connection progress!!\n"));
		pBtMgnt->bBtScan = _FALSE;
		PlatformScheduleWorkItem(&(pBTInfo->BTConnectWorkItem));
	}
}

BOOLEAN
BT_GetEncryptInfo(
	IN	PADAPTER			Adapter,
	IN	PSTA_ENC_INFO_T		pEncInfo
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_SECURITY	pBtSec=&pBTInfo->BtSec;


	// For EAPOL Key don't encrypt !!
	if(IsSecProtEapol(pEncInfo->SecProtInfo) )
	{
		pEncInfo->IsEncByHW = _FALSE;
		return _FALSE;
	}

	if(!pBtSec->bRegNoEncrypt)
	{
		// NOW we need to encrypt BT data
		if(pBtSec->bUsedHwEncrypt)
		{	// HW encrypt
			pEncInfo->IsEncByHW = _TRUE;
		}
		else
		{
			pEncInfo->IsEncByHW = _FALSE;
		}

	 	RT_TRACE(COMP_TEST , DBG_LOUD , ("====>pEncInfo->IsEncByHW = %d \n",pEncInfo->IsEncByHW));
		return _TRUE;
	}
	else
	{
		pEncInfo->IsEncByHW = _FALSE;
		return _FALSE;
	}
}

BOOLEAN
BT_NeedEncrypt(
	IN	PADAPTER	Adapter
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_SECURITY	pBtSec=&pBTInfo->BtSec;

	if(pBtSec->bRegNoEncrypt)
		return _FALSE;
	else
		return _TRUE;
}

BOOLEAN
BT_HwEncrypt(
	IN	PADAPTER	Adapter
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_SECURITY	pBtSec=&pBTInfo->BtSec;

	if(pBtSec->bUsedHwEncrypt)
		return _TRUE;
	else
		return _FALSE;
}

pu1Byte
BT_GetAESKeyBuf(
	IN	PADAPTER	Adapter,
	IN	u1Byte		EntryNum
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;

	if(EntryNum >= MAX_BT_ASOC_ENTRY_NUM)
	{
		RT_ASSERT(_FALSE, ("GetAESKeyBuf, invalid Entry Num!!\n"));
		return 0;
	}
	else
	{
		return pBTInfo->BtAsocEntry[EntryNum].AESKeyBuf;
	}
}

u1Byte
BT_OperateChnl(
	IN	PADAPTER	Adapter
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_MGNT	pBtMgnt=&pBTInfo->BtMgnt;

	return pBtMgnt->BTChannel;
}

VOID
BT_SetEncryptFlag(
	IN	PADAPTER	Adapter,
	IN	u1Byte		Flag
	)
{
	PBT30Info	pBTInfo = &Adapter->MgntInfo.BtInfo;
	PBT_SECURITY	pBtSec=&pBTInfo->BtSec;

	pBtSec->bRegNoEncrypt = Flag;
}
#endif //end for BT_HCI

VOID
BT_SetManualControl(
	IN	PADAPTER	Adapter,
	IN	BOOLEAN		Flag
	)
{
	BTDM_SetManualControl(Flag);
}

VOID
BT_InitializeVariables(
	IN	PADAPTER	Adapter
	)
{
#if 0
	RT_DISP(FIOCTL, IOCTL_STATE, ("\nBT_InitializeVariables() for Adapter=0x%x\n", Adapter));

	PlatformZeroMemory(&Adapter->irp_dbg, sizeof(IRP_DBG_INFO));

	// Initialize all BT related List
	RT_DISP(FIOCTL, IOCTL_STATE, ("BT_InitializeVariables(), initialize BT List!!\n"));
#if (SENDTXMEHTOD==0 ||SENDTXMEHTOD==2)
	RTInitializeListHead(&Adapter->BTDataIdleQueue);
	RTInitializeListHead(&Adapter->BTDataTxQueue);
	RTInitializeListHead(&Adapter->BTDataTxWaitQueue);
#endif
	RTInitializeListHead(&Adapter->BTLocalBufQueue);

#if (SENDTXMEHTOD==0 || SENDTXMEHTOD==2)
		Adapter->NumBTTotalDataBlock = BTTotalDataBlockNum;
		Adapter->NumIdleBTDataBlock = 0;
#endif
	Adapter->NumBTDataLocalBuf = BTLocalBufNum;

	Adapter->MgntInfo.btCtrlInfo.Adapter = (PVOID)Adapter;
#endif
	BTDM_InitlizeVariables(Adapter);
}

//===============================================================
//===============================================================
#if 0
RT_STATUS
BT_AllocateMemory(
	PADAPTER Adapter)
{
	RT_STATUS rtStatus = RT_STATUS_SUCCESS;
	u1Byte	i;

	if(!HALBT_IsBtExist(Adapter))
		return rtStatus;

	RT_DISP(FIOCTL, IOCTL_STATE, ("BT_AllocateMemory()\n"));
	Adapter->irp_dbg.bListInit= _TRUE;

	for(i=0;i<MAX_BT_ASOC_ENTRY_NUM;i++)
	{
		rtStatus = PlatformAllocateMemory(
		Adapter,
		(PVOID *)&(Adapter->MgntInfo.BtInfo.BtAsocEntry[i].AmpAsocCmdData.AMPAssocfragment),
		TOTAL_ALLOCIATE_ASSOC_LEN);

		if(rtStatus!=RT_STATUS_SUCCESS)
		{
			RT_DISP(FIOCTL, IOCTL_STATE, ("Allocate Memory AMPAssocfragment fail!!\n"));
			return rtStatus;
		}
		else
		{
			PlatformZeroMemory(Adapter->MgntInfo.BtInfo.BtAsocEntry[i].AmpAsocCmdData.AMPAssocfragment,TOTAL_ALLOCIATE_ASSOC_LEN);
		}
	}
	Adapter->irp_dbg.bAmpAsocBuf = _TRUE;

	rtStatus = bt_DataListInitialize(Adapter);
	if(rtStatus != RT_STATUS_SUCCESS)
		return rtStatus;

	return rtStatus;
}

VOID
BT_FreeMemory(
	PADAPTER	Adapter)
{
	u1Byte		i;
	u4Byte	QueueId = 0;

	RT_DISP(FIOCTL, IOCTL_STATE, ("BT_FreeMemory() ==>\n"));

	if(!HALBT_IsBtExist(Adapter))
		return;

	for(i=0;i<MAX_BT_ASOC_ENTRY_NUM;i++)
	{
		if(Adapter->MgntInfo.BtInfo.BtAsocEntry[i].AmpAsocCmdData.AMPAssocfragment !=NULL)
		{
			PlatformFreeMemory(Adapter->MgntInfo.BtInfo.BtAsocEntry[i].AmpAsocCmdData.AMPAssocfragment,TOTAL_ALLOCIATE_ASSOC_LEN);
		}
	}

	bt_FreeBTData(Adapter);

	RT_DISP(FIOCTL, IOCTL_STATE, ("[BT]irp_dbg.bListInit = %d\n", Adapter->irp_dbg.bListInit));
	RT_DISP(FIOCTL, IOCTL_STATE, ("[BT]irp_dbg.bAmpAsocBuf = %d\n", Adapter->irp_dbg.bAmpAsocBuf));
	RT_DISP(FIOCTL, IOCTL_STATE, ("[BT]irp_dbg.bBTLocalBufPtr = %d\n", Adapter->irp_dbg.bBTLocalBufPtr));
	RT_DISP(FIOCTL, IOCTL_STATE, ("[BT]irp_dbg.BTLocalBufAllocCnt = %d\n", Adapter->irp_dbg.BTLocalBufAllocCnt));
	RT_DISP(FIOCTL, IOCTL_STATE, ("[BT]irp_dbg.BTLocalBufFreeCnt = %d\n", Adapter->irp_dbg.BTLocalBufFreeCnt));

	RT_DISP(FIOCTL, IOCTL_STATE, ("BT_FreeMemory() <==\n"));
}

VOID
BT_InitThreads(
	IN	PADAPTER	Adapter
	)
{
#if (SENDTXMEHTOD==2)
	PlatformInitializeThread(
		Adapter,
		&Adapter->MgntInfo.BtInfo.BTTxThread,
		(RT_THREAD_CALL_BACK)IOCTL_TxBTThread,
		"BTTxThread",
		_TRUE,
		0,
		NULL);
#endif
}
VOID
BT_DeInitThreads(
	IN	PADAPTER	Adapter
	)
{
#if (SENDTXMEHTOD==2)
	RT_TRACE(COMP_INIT, DBG_LOUD, ("N6CDeInitThread 2()\n"));
	PlatformWaitThreadEnd(Adapter, &(Adapter->MgntInfo.BtInfo.BTTxThread));
	PlatformCancelThread(Adapter, &(Adapter->MgntInfo.BtInfo.BTTxThread));
	PlatformReleaseThread(Adapter, &(Adapter->MgntInfo.BtInfo.BTTxThread));
#endif
}
VOID
BT_IncreaseProbeDbgTxCnt(
	IN	PADAPTER	Adapter
	)
{
	PBT_DBG 	pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	pBtDbg->dbgBtPkt.btPktTxProbReq++;
}

VOID
BT_ResetBtDbgInfo(
	IN	PADAPTER	Adapter
	)
{
	PBT_DBG 	pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	pBtDbg->dbgCtrl = _FALSE;
	pBtDbg->dbgProfile = 0;

	//BT_HCI_DBG_INFO
	PlatformZeroMemory((PVOID)&pBtDbg->dbgHciInfo, sizeof(BT_HCI_DBG_INFO));

	//BT_IRP_DBG_INFO
	//NOTE: don't reset irp MJCreate
	//pBtDbg->dbgIrpInfo.irpMJCreate
	pBtDbg->dbgIrpInfo.irpIoControl = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlHciCmd = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlHciEvent = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlHciRxData = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlHciTxData = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlHciTxData1s = 0;
	pBtDbg->dbgIrpInfo.irpIoCtrlUnknown = 0;

	//BT_PACKET_DBG_INFO
	PlatformZeroMemory((PVOID)&pBtDbg->dbgBtPkt, sizeof(BT_PACKET_DBG_INFO));
}
VOID
BT_SetDbgTest(
	IN	PADAPTER	Adapter,
	IN	u4Byte		argNum,
	IN	pu4Byte		pData
	)
{
	PBT_MGNT	pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;
	PBT_DBG		pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	u1Byte	i;
	u1Byte	test[10];

	for(i=0; i<argNum; i++)
	{
		test[i] = (u1Byte)pData[i];
		//DbgPrint("pData[%d]=0x%x\n", i, test);
	}

	//BT_SpecialPacketNotify(Adapter, PACKET_DHCP);
	#if 0
	if(pData[0])
		BTDM_RejectApAggregatedPacket(Adapter, _TRUE);
	else
		BTDM_RejectApAggregatedPacket(Adapter, _FALSE);
	#endif
	//BTDM_1AntSetWifiRssiThresh(Adapter, (u1Byte)pData[0]);
	//BTDM_ForceBtCoexMechanism(Adapter, (u1Byte)pData[0]);
	//BTDM_ForceA2dpSink(Adapter, (u1Byte)pData[1]);
	#if 0
	{
		u1Byte	dataLen=3;
		u1Byte	buf[4] = {0};
		buf[0] = 0x1;	// polling enable
		buf[1] = 0x2;	// polling time
		buf[2] = 0x1;	// auto report enable
		BT_SendEventExtBtInfoControl(Adapter, dataLen, &buf[0]);
	}
	#endif
	BT_PnpNotify(Adapter, BTC_WIFI_PNP_SLEEP);
}

VOID
BT_DisplayIrpInfo(
	IN	PADAPTER	Adapter
	)
{
	PBT_DBG		pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n ");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n [IRP info]");
		DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "MJ Create", \
			pBtDbg->dbgIrpInfo.irpMJCreate);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control", \
			pBtDbg->dbgIrpInfo.irpIoControl);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control- Unknown", \
		pBtDbg->dbgIrpInfo.irpIoCtrlUnknown);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control-Hci commands", \
			pBtDbg->dbgIrpInfo.irpIoCtrlHciCmd);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control-Hci events", \
			pBtDbg->dbgIrpInfo.irpIoCtrlHciEvent);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control-Hci tx acl data", \
			pBtDbg->dbgIrpInfo.irpIoCtrlHciTxData);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control-Hci rx acl data", \
		pBtDbg->dbgIrpInfo.irpIoCtrlHciRxData);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IO Control-BT Tx data(1 sec) ", \
		pBtDbg->dbgIrpInfo.irpIoCtrlHciTxData1s);
	DCMD_Printf(btDbgBuf);

}

VOID
BT_DisplayBtPacketInfo(
	IN	PADAPTER	Adapter
	)
{
	PBT_DBG		pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n ");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n [BT packet info]");
		DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15s %-15s %-15s ", \
		"packet type", "Tx", "Rx", "Drop");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15s %-15s %-15s ", \
		"===========", "==", "==", "====");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"probe req", pBtDbg->dbgBtPkt.btPktTxProbReq,
		pBtDbg->dbgBtPkt.btPktRxProbReq, pBtDbg->dbgBtPkt.btPktRxProbReqFail);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"probe rsp", pBtDbg->dbgBtPkt.btPktTxProbRsp,
		pBtDbg->dbgBtPkt.btPktRxProbRsp, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"auth", pBtDbg->dbgBtPkt.btPktTxAuth,
		pBtDbg->dbgBtPkt.btPktRxAuth, pBtDbg->dbgBtPkt.btPktRxAuthButDrop);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"assoc req", pBtDbg->dbgBtPkt.btPktTxAssocReq,
		pBtDbg->dbgBtPkt.btPktRxAssocReq, pBtDbg->dbgBtPkt.btPktRxAssocReqButDrop);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"assoc rsp", pBtDbg->dbgBtPkt.btPktTxAssocRsp,
		pBtDbg->dbgBtPkt.btPktRxAssocRsp, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"4way 1st", pBtDbg->dbgBtPkt.btPktTx4way1st,
		pBtDbg->dbgBtPkt.btPktRx4way1st, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"4way 2nd", pBtDbg->dbgBtPkt.btPktTx4way2nd,
		pBtDbg->dbgBtPkt.btPktRx4way2nd, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"4way 3rd", pBtDbg->dbgBtPkt.btPktTx4way3rd,
		pBtDbg->dbgBtPkt.btPktRx4way3rd, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"4way 4th", pBtDbg->dbgBtPkt.btPktTx4way4th,
		pBtDbg->dbgBtPkt.btPktRx4way4th, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"link super req", pBtDbg->dbgBtPkt.btPktTxLinkSuperReq,
		pBtDbg->dbgBtPkt.btPktRxLinkSuperReq, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"link super rsp", pBtDbg->dbgBtPkt.btPktTxLinkSuperRsp,
		pBtDbg->dbgBtPkt.btPktRxLinkSuperRsp, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"bt data", pBtDbg->dbgBtPkt.btPktTxData,
		pBtDbg->dbgBtPkt.btPktRxData, 0);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-15s %-15d %-15d %-15d ", \
		"disassoc", pBtDbg->dbgBtPkt.btPktTxDisassoc,
		pBtDbg->dbgBtPkt.btPktRxDisassoc, 0);
	DCMD_Printf(btDbgBuf);

}

VOID
BT_DisplayHciInfo(
	IN	PADAPTER	Adapter
	)
{
	PBT_DBG		pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n ");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n [HCI info]");
		DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "Total HCI command received", \
			pBtDbg->dbgHciInfo.hciCmdCnt);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "Unknown HCI command received", \
			pBtDbg->dbgHciInfo.hciCmdCntUnknown);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_CREATE_PHYSICAL_LINK", \
		pBtDbg->dbgHciInfo.hciCmdCntCreatePhyLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_ACCEPT_PHYSICAL_LINK", \
		pBtDbg->dbgHciInfo.hciCmdCntAcceptPhyLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_DISCONNECT_PHYSICAL_LINK",  \
		pBtDbg->dbgHciInfo.hciCmdCntDisconnectPhyLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_CREATE_LOGICAL_LINK", \
		pBtDbg->dbgHciInfo.hciCmdCntCreateLogLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_ACCEPT_LOGICAL_LINK", \
		pBtDbg->dbgHciInfo.hciCmdCntAcceptLogLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_DISCONNECT_LOGICAL_LINK", \
		pBtDbg->dbgHciInfo.hciCmdCntDisconnectLogLink);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_READ_LOCAL_AMP_ASSOC", \
		pBtDbg->dbgHciInfo.hciCmdCntReadLocalAmpAssoc);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_WRITE_REMOTE_AMP_ASSOC", \
		pBtDbg->dbgHciInfo.hciCmdCntWriteRemoteAmpAssoc);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_SET_ACL_LINK_STATUS", \
		pBtDbg->dbgHciInfo.hciCmdCntSetAclLinkStatus);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_SET_SCO_LINK_STATUS", \
		pBtDbg->dbgHciInfo.hciCmdCntSetScoLinkStatus);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_EXTENSION_VERSION_NOTIFY", \
		pBtDbg->dbgHciInfo.hciCmdCntExtensionVersionNotify);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "HCI_LINK_STATUS_NOTIFY", \
		pBtDbg->dbgHciInfo.hciCmdCntLinkStatusNotify);
	DCMD_Printf(btDbgBuf);
}

VOID
BT_DisplayHSLinkState(
	IN	PADAPTER	Adapter
	)
{
	PBT_MGNT	pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;
	PBT_DBG		pBtDbg=&Adapter->MgntInfo.BtInfo.BtDbg;
	PMGNT_INFO	pMgntInfo=&Adapter->MgntInfo;
	BOOLEAN		bRoam=_FALSE, bScan=_FALSE, bLink=_FALSE;
	u4Byte		u4Tmp[4]={0};

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n ");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n [BT3.0 high speed]");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "Dot11 channel", \
		pMgntInfo->dot11CurrentChannelNumber);
	DCMD_Printf(btDbgBuf);
	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "BT channel", \
		pBtMgnt->BTChannel);
	DCMD_Printf(btDbgBuf);

	bRoam = MgntRoamingInProgress(pMgntInfo);
	bScan = MgntScanInProgress(pMgntInfo);
	bLink = MgntIsLinkInProgress(pMgntInfo);
	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d ", "bLink/ bRoam/ bScan", \
		bLink, bRoam, bScan);
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "BT Tx packets (1 sec)", \
		pBtDbg->dbgIrpInfo.irpIoCtrlHciTxData1s);
	DCMD_Printf(btDbgBuf);

	if(pBtMgnt->BtOperationOn)
	{
		rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "BT High speed mode state", \
			"Operation ON!!");
		DCMD_Printf(btDbgBuf);
		rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "BT HS Remote Peer", \
			((BTPKT_RemoteHsPeer(Adapter) == BT_PEER_REALTEK)? "REALTEK":"OTHER VENDOR"));
		DCMD_Printf(btDbgBuf);
		rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Wifi Traffic Direction", \
			((BTDM_IsWifiUplink(Adapter))? "uplink":"downlink"));
		DCMD_Printf(btDbgBuf);

		if(pBtMgnt->bBTConnectInProgress)
		{
			rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Wifi physical link state", \
				"It is trying to establish the link!!");
		}
		else
		{
			rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Wifi physical link state", \
				"Establishment finished!!");
		}
		DCMD_Printf(btDbgBuf);

		if(pBtMgnt->bPhyLinkInProgress)
		{
			rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state", \
				"Under progress");
			DCMD_Printf(btDbgBuf);
			switch(pBtMgnt->BTCurrentConnectType)
			{
				case BT_CONNECT_AUTH_REQ:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"BT_CONNECT_AUTH_REQ");
					break;
				case BT_CONNECT_AUTH_RSP:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"BT_CONNECT_AUTH_RSP");
					break;
				case BT_CONNECT_ASOC_REQ:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"BT_CONNECT_ASOC_REQ");
					break;
				case BT_CONNECT_ASOC_RSP:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"BT_CONNECT_ASOC_RSP");
					break;
				case BT_DISCONNECT:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"BT_DISCONNECT");
					break;
				default:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment tx state ", \
						"ERROR !!!");
					break;
			}
			DCMD_Printf(btDbgBuf);
			switch(pBtMgnt->BTReceiveConnectPkt)
			{
				case BT_CONNECT_AUTH_REQ:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"BT_CONNECT_AUTH_REQ");
					break;
				case BT_CONNECT_AUTH_RSP:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"BT_CONNECT_AUTH_RSP");
					break;
				case BT_CONNECT_ASOC_REQ:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"BT_CONNECT_ASOC_REQ");
					break;
				case BT_CONNECT_ASOC_RSP:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"BT_CONNECT_ASOC_RSP");
					break;
				case BT_DISCONNECT:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"BT_DISCONNECT");
					break;
				default:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical Link establishment rx state ", \
						"ERROR !!!");
					break;
			}
			DCMD_Printf(btDbgBuf);
		}
		else
		{
			switch(pBtDbg->dbgHciInfo.hciCmdPhyLinkStatus)
			{
				case HCI_STATUS_CONNECT_RJT_LIMIT_RESOURCE:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : no enough resource!!!");
					break;
				case HCI_STATUS_CONNECT_REJ_NOT_SUIT_CHNL_FOUND:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : non suitable channel!!!");
					break;
				case HCI_STATUS_SUCCESS:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Success!!!");
					break;
				case HCI_STATUS_CONTROLLER_BUSY:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : controller busy!!!");
					break;
				case HCI_STATUS_UNKNOW_CONNECT_ID:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : unknown connect id!!!");
					break;
				case HCI_STATUS_UNSPECIFIC_ERROR:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : unspecific error!!!");
					break;
				case HCI_STATUS_CONNECT_ACCEPT_TIMEOUT:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : connect accept timeout!!!");
					break;
				case HCI_STATUS_AUTH_FAIL:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"Fail : 4 way fail!!!");
					break;
				default:
					rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "Physical link progress state ", \
						"???? : not defined status, need to do advanced check!!!");
					break;
			}
			DCMD_Printf(btDbgBuf);
		}
	}
	else
	{
		rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s ", "BT High speed mode state ", \
			"Operation OFF!!");
		DCMD_Printf(btDbgBuf);
	}

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n ");
	DCMD_Printf(btDbgBuf);

	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n [BT3.0 HS register setting]");
	DCMD_Printf(btDbgBuf);
	// dig related
	u4Tmp[0] = PlatformEFIORead4Byte(Adapter, 0xc50);
	u4Tmp[1]  = PlatformEFIORead1Byte(Adapter, 0xa0a);
	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0xc50 / 0xa0a (dig/cck cca thres)", \
		u4Tmp[0], u4Tmp[1]);
	DCMD_Printf(btDbgBuf);
	// edca
	u4Tmp[0]  = PlatformEFIORead4Byte(Adapter, 0x504);
	u4Tmp[1]  = PlatformEFIORead4Byte(Adapter, 0x508);
	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x504/ 0x508(edca VI_Q/BE_Q)", \
		u4Tmp[0], u4Tmp[1]);
	DCMD_Printf(btDbgBuf);

	// beacon control (TBTT[3])
	u4Tmp[0]  = PlatformEFIORead4Byte(Adapter, 0x550);
	rsprintf(btDbgBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "0x550 (beacon control)", \
		u4Tmp[0]);
	DCMD_Printf(btDbgBuf);
}

u1Byte
BT_TargetWirelessModeToSwitch(
	u1Byte	targetChnl,
	u1Byte	currWirelessmode
	)
{
	u1Byte	targetWirelessMode=currWirelessmode;

	if(targetChnl <= 14)
	{
		if( (currWirelessmode & WIRELESS_MODE_B) ||
			(currWirelessmode & WIRELESS_MODE_G) ||
			(currWirelessmode & WIRELESS_MODE_N_24G) )
		{
			targetWirelessMode = currWirelessmode;
		}
		else
		{
			// if current currWirelessmode is A or N5G, then we
			// set default wireless mode for N 2.4G.
			targetWirelessMode = WIRELESS_MODE_N_24G;
		}
	}
	else
	{
		if( (currWirelessmode & WIRELESS_MODE_A) ||
			(currWirelessmode & WIRELESS_MODE_N_5G) ||
			(currWirelessmode & WIRELESS_MODE_AC_5G) )
		{
			targetWirelessMode = currWirelessmode;
		}
		else
		{
			targetWirelessMode = WIRELESS_MODE_N_5G;
		}
		targetWirelessMode = WIRELESS_MODE_N_5G;
	}

	RT_DISP(FIOCTL, IOCTL_STATE, ("Target wirelessMode(0x%x) for BT connection\n", targetWirelessMode));
	return targetWirelessMode;
}

BOOLEAN
BT_IsLegalChannel(
	IN	PADAPTER	Adapter,
	IN	u1Byte		channel
	)
{
	PRT_CHANNEL_LIST		pChannelList = NULL;
	u2Byte				i;

	RtActChannelList(Adapter, RT_CHNL_LIST_ACTION_GET_CHANNEL_LIST_FOR_BT, NULL, &pChannelList);

	RT_DISP(FIOCTL, IOCTL_STATE, ("pChannelList->ChannelPlan=%d, pChannelList->ChannelLen=%d\n",
		pChannelList->ChannelPlan, pChannelList->ChannelLen));
	RT_DISP(FIOCTL, IOCTL_STATE, ("channel list=["));
	for(i=0; i<pChannelList->ChannelLen; i++)
	{
		if(i == (pChannelList->ChannelLen-1))
		{
			RT_DISP(FIOCTL, IOCTL_STATE, ("%d]\n", pChannelList->ChnlListEntry[i].ChannelNum));
		}
		else
		{
			RT_DISP(FIOCTL, IOCTL_STATE, ("%d, ", pChannelList->ChnlListEntry[i].ChannelNum));
		}
	}

	for(i = 0; i < pChannelList->ChannelLen; i ++)
	{
		RT_DISP(FIOCTL, IOCTL_STATE, ("Check if chnl(%d) in channel plan contains bt target chnl(%d) for BT connection\n", pChannelList->ChnlListEntry[i].ChannelNum, channel));
		if((channel == pChannelList->ChnlListEntry[i].ChannelNum)||
			(channel == pChannelList->ChnlListEntry[i].ChannelNum + 2))
		{
			return channel;
		}
	}
	return 0;
}

VOID
BT_SetLogoTest(
	IN	PADAPTER	Adapter,
	IN	u4Byte		type
	)
{
	PBT_MGNT	pBtMgnt=&Adapter->MgntInfo.BtInfo.BtMgnt;

	switch(type)
	{
		case 0:
			RT_DISP(FIOCTL, IOCTL_STATE, ("BT_LOGO_TEST_CLEAR\n"));
			pBtMgnt->btLogoTest = 0;
			break;
		case 1:
			RT_DISP(FIOCTL, IOCTL_STATE, ("BT_LOGO_TEST_SUPV\n"));
			pBtMgnt->btLogoTest |= BT_LOGO_TEST_SUPV;
			break;
		case 2:
			RT_DISP(FIOCTL, IOCTL_STATE, ("BT_LOGO_TEST_QOS_DIS\n"));
			pBtMgnt->btLogoTest |= BT_LOGO_TEST_QOS_DIS;
			break;
		default:
			RT_DISP(FIOCTL, IOCTL_STATE, ("BT_LOGO_TEST_UNKNOWN\n"));
			break;
	}
	RT_DISP(FIOCTL, IOCTL_STATE, ("btLogoTest setting bitmap = 0x%x\n",
		pBtMgnt->btLogoTest));
}

VOID
BT_StatusWatchdog(
	IN	PADAPTER		Adapter
	)
{
#if 0
	PMGNT_INFO			pMgntInfo = &(Adapter->MgntInfo);
	PBT30Info			pBTInfo = &pMgntInfo->BtInfo;
	PBT_MGNT			pBtMgnt = &pBTInfo->BtMgnt;
	PBT_TRAFFIC			pBtTraffic=&pBTInfo->BtTraffic;
	BOOLEAN				bRfOff=_FALSE, bTxBusy=_FALSE, bRxBusy=_FALSE;
	PADAPTER 			pDefaultAdapter = GetDefaultAdapter(Adapter);
	PBT_CTRL_INFO		pBtCtrlInfo=&Adapter->MgntInfo.btCtrlInfo;

	if(pDefaultAdapter != Adapter)
	{
		return;
	}
	BTDM_ApStatusWatchdog(Adapter);

	if((pMgntInfo->RfOffReason & RF_CHANGE_BY_HW) ||
		(pMgntInfo->RfOffReason & RF_CHANGE_BY_SW))
		bRfOff = _TRUE;

	if(!pBtMgnt->bRfStatusNotified)
	{
		BTHCI_SendEventExtWifiRfStatusNotify(Adapter, pMgntInfo->RfOffReason);
		pBtMgnt->bRfStatusNotified = _TRUE;
	}
	if(!pBtMgnt->bBtRsvedPageDownload)
	{
		// download BT related RSVD page. BT qos-null
		HALBT_DownloadRsvdPageForBt(Adapter);
		pBtMgnt->bBtRsvedPageDownload = _TRUE;
	}

	// Here we indicate AMP status and some other states.
	if(!MgntRoamingInProgress(pMgntInfo) &&
		!MgntIsLinkInProgress(pMgntInfo) &&
		!MgntScanInProgress(pMgntInfo) &&
		!bRfOff)
	{
		static u1Byte BTwaitcnt=0, wifiAssoFinishCnt=0;
		if(pBtMgnt->BTNeedAMPStatusChg)
		{
			BTwaitcnt++;
			if(BTwaitcnt >= 2)
			{
				BTHCI_EventAMPStatusChange(Adapter, AMP_STATUS_FULL_CAPACITY_FOR_BT);
				BTwaitcnt = 0;
			}
		}

		if(pBtCtrlInfo->notifyData.actionType)
		{
			wifiAssoFinishCnt++;
			if(wifiAssoFinishCnt >= 2)
			{
				BT_WiFiConnectNotify(Adapter, _FALSE);
				wifiAssoFinishCnt = 0;
			}
		}
	}

	if(GLBtCoexist.btInfo.bForceToRoam)
	{
		GLBtCoexist.btInfo.bForceToRoam = _FALSE;
		MgntForceRoaming(Adapter);
	}

	RT_DISP(FIOCTL, IOCTL_BT_TP, ("[BT traffic], TxPktCntInPeriod=%d, TxPktLenInPeriod=%"i64fmt"d\n",
		pBtTraffic->Bt30TrafficStatistics.TxPktCntInPeriod,
		pBtTraffic->Bt30TrafficStatistics.TxPktLenInPeriod));
	RT_DISP(FIOCTL, IOCTL_BT_TP, ("[BT traffic], RxPktCntInPeriod=%d, RxPktLenInPeriod=%"i64fmt"d\n",
		pBtTraffic->Bt30TrafficStatistics.RxPktCntInPeriod,
		pBtTraffic->Bt30TrafficStatistics.RxPktLenInPeriod));
	if( pBtTraffic->Bt30TrafficStatistics.TxPktCntInPeriod > 100 ||
		pBtTraffic->Bt30TrafficStatistics.RxPktCntInPeriod > 100 )
	{
		if(pBtTraffic->Bt30TrafficStatistics.RxPktLenInPeriod > pBtTraffic->Bt30TrafficStatistics.TxPktLenInPeriod)
			bRxBusy = _TRUE;
		else if(pBtTraffic->Bt30TrafficStatistics.TxPktLenInPeriod > pBtTraffic->Bt30TrafficStatistics.RxPktLenInPeriod)
			bTxBusy = _TRUE;
	}

	pBtTraffic->Bt30TrafficStatistics.TxPktCntInPeriod = 0;
	pBtTraffic->Bt30TrafficStatistics.RxPktCntInPeriod = 0;
	pBtTraffic->Bt30TrafficStatistics.TxPktLenInPeriod = 0;
	pBtTraffic->Bt30TrafficStatistics.RxPktLenInPeriod = 0;
	pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic = bTxBusy;
	pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic = bRxBusy;
	RT_DISP(FIOCTL, IOCTL_BT_TP, ("[BT traffic], bTxBusyTraffic=%d, bRxBusyTraffic=%d\n",
		pBtTraffic->Bt30TrafficStatistics.bTxBusyTraffic,
		pBtTraffic->Bt30TrafficStatistics.bRxBusyTraffic));
#endif
}

//=======================================
//	BTHCI module interface
//=======================================
u1Byte
BT_GetCurrentEntryNumByMAC(
	IN	PADAPTER 	Adapter,
	IN	pu1Byte		SA
	)
{
	return BTHCI_GetCurrentEntryNumByMAC(Adapter, SA);
}

VOID
BT_IndicateAMPStatus(
	IN	PADAPTER			Adapter,
	IN	u1Byte				JoinAction,
	IN	u1Byte				channel
	)
{
	BTHCI_IndicateAMPStatus(Adapter, JoinAction, channel);
}
VOID
BT_EventParse(
	PADAPTER 					Adapter,
	PVOID						pEvntData,
	ULONG						dataLen
	)
{
	BTHCI_EventParse(Adapter, pEvntData, dataLen);
}
u2Byte
BT_GetPhysicalLinkHandle(
	PADAPTER	Adapter,
	u1Byte		EntryNum
	)
{
	return BTHCI_GetPhysicalLinkHandle(Adapter, EntryNum);
}
VOID
BT_InitializeAllTimer(
	IN	PADAPTER	Adapter
	)
{
	BTHCI_InitializeAllTimer(Adapter);
}
VOID
BT_CancelAllTimer(
	IN	PADAPTER	Adapter
	)
{
	BTHCI_CancelAllTimer(Adapter);
}
VOID
BT_ReleaseAllTimer(
	IN	PADAPTER	Adapter
	)
{
	BTHCI_ReleaseAllTimer(Adapter);
}
VOID
BT_InitializeAllWorkItem(
	IN	PADAPTER	Adapter
	)
{
	bt_InitializeBtCtrlWorkItem(Adapter);
	BTHCI_InitializeAllWorkItem(Adapter);
	BTDM_InitializeAllWorkItem(Adapter);
}
VOID
BT_FreeAllWorkItem(
	IN	PADAPTER	Adapter
	)
{
	bt_FreeBtCtrlWorkItem(Adapter);
	BTHCI_FreeAllWorkItem(Adapter);
	BTDM_FreeAllWorkItem(Adapter);
}
VOID
BT_HciReset(
	IN	PADAPTER	Adapter
	)
{
	BTHCI_Reset(Adapter);
}
BOOLEAN
BT_HsConnectionEstablished(
	IN	PADAPTER	Adapter
	)
{
	return BTHCI_HsConnectionEstablished(Adapter);
}
VOID
BT_EventAMPStatusChange(
	IN PADAPTER 					Adapter,
	u1Byte						AMP_Status
	)
{
	BTHCI_EventAMPStatusChange(Adapter, AMP_Status);
}
VOID
BT_DisconnectAll(
	IN	PADAPTER		Adapter
	)
{
	BTHCI_DisconnectAll(Adapter);
}
VOID
BT_SetLinkStatusNotify(
	PADAPTER 					Adapter,
	PPACKET_IRP_HCICMD_DATA	pHciCmd
	)
{
	BTHCI_SetLinkStatusNotify(Adapter, pHciCmd);
}

VOID
BT_SendEventExtBtInfoControl(
	IN	PADAPTER					Adapter,
	IN	u1Byte						dataLen,
	IN	PVOID						pData
	)
{
	RT_DISP_DATA(FIOCTL, IOCTL_BT_HCICMD_EXT, "Send BT Info event, Hex Data :\n",
			pData, dataLen);

	BTHCI_SendEventExtBtInfoControl(Adapter, dataLen, pData);
}

VOID
BT_SendEventExtBtCoexControl(
	IN	PADAPTER					Adapter,
	IN	u1Byte						dataLen,
	IN	PVOID						pData
	)
{
	RT_DISP_DATA(FIOCTL, IOCTL_BT_HCICMD_EXT, "Send BT Coex Control event, Hex Data :\n",
		pData, dataLen);

	BTHCI_SendEventExtBtCoexControl(Adapter, dataLen, pData);
}
//=======================================
//	BTPKT module interface
//=======================================
u1Byte
BT_IsBtTxPacket(
	PADAPTER			Adapter,
	PRT_TCB				pTcb
	)
{
	return BTPKT_IsBTPacketTx(Adapter, pTcb);
}
BOOLEAN
BT_IsBtRxPacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	)
{
	return BTPKT_IsBTPacketRx(pAdapter, pRfd);
}

u4Byte
BT_PreParsePacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	)
{
	return BTPKT_PreParsePacket(pAdapter, pRfd);
}

u4Byte
BT_ParsePacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	)
{
	return BTPKT_ParsePacket(pAdapter, pRfd);
}
u4Byte
BT_IsMgntPacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	)
{
	return BTPKT_IsMgntPacket(pAdapter, pRfd);
}

VOID
BT_ReturnDataBuffer(
	PADAPTER			pAdapter,
	PRT_TX_LOCAL_BUFFER	pLocalBuffer
	)
{
	BTPKT_ReturnDataBuffer(pAdapter, pLocalBuffer);
}
#endif //end for BT_HCI

VOID
BT_DisplayBtCoexInfo(
	IN	PADAPTER	Adapter
	)
{
	BTDM_DisplayBtCoexInfo(Adapter);
}

//===============================================================
//===============================================================
//=======================================
//	HALBT module
//=======================================
VOID
BT_InitHalVars(
	IN	PADAPTER			Adapter
	)
{
	u1Byte	antNum = 2;

	// HALBT_InitHalVars() must be called first
	BTDM_InitHalVars(Adapter);

	// called after HALBT_InitHalVars()
	BTDM_SetBtExist(Adapter);
	BTDM_SetBtChipType(Adapter);
	antNum = HALBT_GetPgAntNum(Adapter);
	BTDM_SetBtCoexAntNum(Adapter, BT_COEX_ANT_TYPE_PG, antNum);
}

BOOLEAN
BT_IsBtExist(
	IN	PADAPTER	Adapter
	)
{
	return HALBT_IsBtExist(Adapter);
}
u1Byte
BT_BtChipType(
	IN	PADAPTER	Adapter
	)
{
	return HALBT_BTChipType(Adapter);
}
VOID
BT_InitHwConfig(
	IN	PADAPTER	Adapter
	)
{
	if(!BT_IsBtExist(Adapter))
		return;
	//HALBT_InitHwConfig(Adapter);
	EXhalbtcoutsrc_InitHwConfig(&GLBtCoexist);
	EXhalbtcoutsrc_InitCoexDm(&GLBtCoexist);
}

u1Byte
BT_GetPgAntNum(
	IN	PADAPTER	Adapter
	)
{
	return HALBT_GetPgAntNum(Adapter);
}

BOOLEAN
BT_SupportRfStatusNotify(
	IN	PADAPTER	Adapter
	)
{
	return HALBT_SupportRfStatusNotify(Adapter);
}
VOID
BT_SetRtsCtsNoLenLimit(
	IN	PADAPTER	Adapter
	)
{
	HALBT_SetRtsCtsNoLenLimit(Adapter);
}

VOID
BT_ActUpdateRaMask(
	IN	PADAPTER	Adapter,
	IN	u1Byte		macId
	)
{
	rtw_hal_update_ra_mask(Adapter, 0, DM_RATR_STA_HIGH);
}
//=======================================
//	BTDM module
//=======================================
BOOLEAN
BT_IsBtBusy(
	IN	PADAPTER	Adapter
	)
{
	return BTDM_IsBtBusy(Adapter);
}
BOOLEAN
BT_IsLimitedDig(
	IN	PADAPTER	Adapter
	)
{
	return BTDM_IsLimitedDig(Adapter);
}
VOID
BT_CoexistMechanism(
	IN	PADAPTER	Adapter
	)
{
	BTDM_Coexist(Adapter);
}
s4Byte
BT_GetHsWifiRssi(
	IN		PADAPTER	Adapter
	)
{
	return BTDM_GetHsWifiRssi(Adapter);
}

BOOLEAN
BT_IsDisableEdcaTurbo(
	IN	PADAPTER	Adapter
	)
{
	return BTDM_IsDisableEdcaTurbo(Adapter);
}
u1Byte
BT_AdjustRssiForCoex(
	IN	PADAPTER	Adapter
	)
{
	return BTDM_AdjustRssiForCoex(Adapter);
}
VOID
BT_SetBtCoexAntNum(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type,
	IN	u1Byte		antNum
	)
{
	BTDM_SetBtCoexAntNum(Adapter, type, antNum);
}

//=======================================
//	BTCoex module
//=======================================
VOID
BT_UpdateProfileInfo(
	)
{
	EXhalbtcoutsrc_StackUpdateProfileInfo();
}
VOID
BT_UpdateMinBtRssi(
	IN	s1Byte	btRssi
	)
{
	EXhalbtcoutsrc_UpdateMinBtRssi(btRssi);
}
VOID
BT_SetHciVersion(
	IN	u2Byte	hciVersion
	)
{
	EXhalbtcoutsrc_SetHciVersion(hciVersion);
}
VOID
BT_SetBtPatchVersion(
	IN	u2Byte	btHciVersion,
	IN	u2Byte	btPatchVersion
	)
{
	EXhalbtcoutsrc_SetBtPatchVersion(btHciVersion, btPatchVersion);
}
//=======================================
//	Other module
//=======================================


BOOLEAN
BT_ForceEnterLps(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
		return _FALSE;

	if(GLBtCoexist.boardInfo.btdmAntNum == 1)
	{
		if(BT_Operation(Adapter))
			return _FALSE;
		else
		{
			if(GLBtCoexist.btInfo.bForceLps)
				return _TRUE;
		}
	}
	return _FALSE;
}
BOOLEAN
BT_1Ant(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
		return _FALSE;

	if(GLBtCoexist.boardInfo.btdmAntNum == 1)
		return _TRUE;
	else
		return _FALSE;
}

BOOLEAN
BT_1AntPowerSaveMode(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
	{
		return _FALSE;
	}

	if( (GLBtCoexist.boardInfo.btdmAntNum == 1) &&
		(GLBtCoexist.btInfo.bBtPwrSaveMode) )
	{
		return _TRUE;
	}
	else
		return _FALSE;
}

u1Byte
BT_1AntLpsVal(
	IN	PADAPTER	Adapter
	)
{
	return GLBtCoexist.btInfo.lps1Ant;
}

u1Byte
BT_1AntRpwmVal(
	IN	PADAPTER	Adapter
	)
{
	return GLBtCoexist.btInfo.rpwm1Ant;
}

VOID
BT_RecordPwrMode(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		pCmdBuf,
	IN	u1Byte		cmdLen
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write pwrModeCmd=0x%04x%08x\n",
		pCmdBuf[0]<<8|pCmdBuf[1],
		pCmdBuf[2]<<24|pCmdBuf[3]<<16|pCmdBuf[4]<<8|pCmdBuf[5]));

	_rtw_memcpy(&GLBtCoexist.pwrModeVal[0], pCmdBuf, cmdLen);
}

BOOLEAN
BT_IsBtCoexManualControl(
	IN	PADAPTER	Adapter
	)
{
	if(GLBtCoexist.bManualControl)
		return _TRUE;
	else
		return _FALSE;
}

BOOLEAN
BT_ForceExecPwrCmd(
	IN	PADAPTER	Adapter
	)
{
	if(GLBtCoexist.btInfo.forceExecPwrCmdCnt)
		return _TRUE;
	else
		return _FALSE;
}

VOID
BT_DecExecPwrCmdCnt(
	IN	PADAPTER	Adapter
	)
{
	if(GLBtCoexist.btInfo.forceExecPwrCmdCnt)
		GLBtCoexist.btInfo.forceExecPwrCmdCnt--;
}

BOOLEAN
BT_IsBtDisabled(
	IN	PADAPTER	Adapter
	)
{
	if(BTDM_IsBtDisabled(Adapter))
		return _TRUE;
	else
		return _FALSE;
}

BOOLEAN
BT_BtCtrlAggregateBufSize(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
		return FALSE;

	if(GLBtCoexist.btInfo.bBtCtrlAggBufSize)
		return TRUE;
	else
		return FALSE;
}

u1Byte
BT_GetAdjustAggregateBufSize(
	IN	PADAPTER	Adapter
	)
{
	return GLBtCoexist.btInfo.aggBufSize;
}

void BT_SignalCompensation(PADAPTER padapter, u8 *rssi_wifi, u8 *rssi_bt)
{
	if(!BT_IsBtDisabled(padapter))
		BTDM_SignalCompensation(padapter, rssi_wifi, rssi_bt);
}

void BT_LpsLeave(PADAPTER padapter)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("BT_LpsLeave()\n"));
	BTDM_LpsLeave(padapter);
}

void BT_LowWiFiTrafficNotify(PADAPTER padapter)
{
	//BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("BT_LowWiFiTrafficNotify()\n"));
	BTDM_LowWiFiTrafficNotify(padapter);
}

u4Byte
BT_GetRaMask(
	IN	PADAPTER	Adapter
	)
{
	if(!HALBT_IsBtExist(Adapter))
		return 0;
	if(GLBtCoexist.btInfo.bBtDisabled)
		return 0;
	if(GLBtCoexist.boardInfo.btdmAntNum != 1)
		return 0;

	return GLBtCoexist.btInfo.raMask;
}
#endif
#endif
