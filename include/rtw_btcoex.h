#ifndef __INC_BT_H
#define __INC_BT_H

/* put it here before new BT coex */
/* for 8723as complete */
#ifdef CONFIG_RTL8723B

//#include <drv_conf.h>
//#include <osdep_service.h>
//#include <drv_types.h>


#define	BT_TMP_BUF_SIZE		100

#define	BT_LOGO_TEST_CLEAR		0
#define	BT_LOGO_TEST_SUPV		BIT0
#define	BT_LOGO_TEST_QOS_DIS		BIT1

#define	BT_COEX_ANT_TYPE_PG			0
#define	BT_COEX_ANT_TYPE_ANTDIV		1

#if(MP_DRIVER == 1)
// This is for MP test only
VOID
MPTBT_Test(
	IN	PADAPTER	Adapter,
	IN	u1Byte		opCode,
	IN	u1Byte		byte1,
	IN	u1Byte		byte2,
	IN	u1Byte		byte3
	);
#endif
//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST

#define	BT_CTRL_BTINFO_BUF				20
#define	BT_DBG_CONTROL_LEN_MAX			20

#define GET_BT_INFO(padapter)	(&GET_HAL_DATA(padapter)->BtInfo)

typedef enum _BT_NOTIFY_TYPE{
	NOTIFY_WIFI_IPS								= 0x0,
	NOTIFY_WIFI_LPS								= 0x1,
	NOTIFY_WIFI_SCAN								= 0x2,
	NOTIFY_WIFI_CONNECTION						= 0x3,
	NOTIFY_WIFI_MEDIA_STATUS					= 0x4,
	NOTIFY_WIFI_SPECIAL_PACKET					= 0x5,
	NOTIFY_BT_INFO								= 0x6,
	NOTIFY_WIFI_RF_STATUS						= 0x7,
	NOTIFY_BT_STACK_OPERATION					= 0x8,
	NOTIFY_WIFI_HALT							= 0x9,
	NOTIFY_WIFI_PNP								= 0xa,
	NOTIFY_DBG_CONTROL							= 0xb,
	NOTIFY_MAX
}BT_NOTIFY_TYPE,*PBT_NOTIFY_TYPE;

typedef enum _BT_MEDIA_PORT{
	MEDIA_PORT_DEFAULT							= 0x0,
	MEDIA_PORT_VWIFI							= 0x1,
	MEDIA_PORT_BT								= 0x2,
	MEDIA_PORT_MAX
}BT_MEDIA_PORT,*PBT_MEDIA_PORT;

#if 0
typedef struct _BT_COEXIST_STR{
	u1Byte					bBtExist;
	u1Byte					btTotalAntNum;
	u1Byte					btChipType;
	BOOLEAN					bInitlized;
}BT_COEXIST_STR, *PBT_COEXIST_STR;
#endif

typedef struct _BT_NOTIFY_DATA{
	u1Byte				scanType;	// scan notify
	u1Byte				ipsType;		// IPS notify
	u1Byte				lpsType;		// LPS notify
	u1Byte				actionType;	// associate notify
	u1Byte				pktType;		// special packet notify
	u1Byte				btInfo[BT_CTRL_BTINFO_BUF];	// btInfo notify
	u1Byte				btInfoLen;					// btInfo notify
	u4Byte				preHwSwRfOffReason;			// rf status notify
	u4Byte				rfStateChangeSource;				// rf status notify
	u1Byte				newRfState;					// rf status notify
	u1Byte				mediaPort;					// media status notify
	u1Byte				mediaStatus;					// media status notify
	u1Byte				stackOpType;					// stack operation notify
	u1Byte				pnpState;						// pnp notify
	u1Byte				dbgCtrlOpCode;				// dbg control notify
	u1Byte				dbgCtrlInfo[BT_DBG_CONTROL_LEN_MAX];	// dbg control notify
	u1Byte				dbgCtrlInfoLen;					// dbg control notify
}BT_NOTIFY_DATA,*PBT_NOTIFY_DATA;

typedef struct _BT_MGNT
{
	u8			bManualControl;
} BT_MGNT, *PBT_MGNT;

typedef struct _BT30Info
{
	PADAPTER			padapter;
	BT_MGNT				BtMgnt;
#if 0
	BT_ASOC_ENTRY		BtAsocEntry[MAX_BT_ASOC_ENTRY_NUM];
	BT_MGNT				BtMgnt;
	BT_DBG				BtDbg;
	BT_HCI_INFO			BtHciInfo;
	BT_TRAFFIC			BtTraffic;
	BT_SECURITY			BtSec;

#if(BT_THREAD == 0)
	RT_WORK_ITEM		HCICmdWorkItem;
	RT_TIMER				BTHCICmdTimer;
#endif
#if (SENDTXMEHTOD==0)
	RT_WORK_ITEM		HCISendACLDataWorkItem;
	RT_TIMER				BTHCISendAclDataTimer;
#elif(SENDTXMEHTOD==2)
	RT_THREAD			BTTxThread;
#endif
	RT_WORK_ITEM		BTPsDisableWorkItem;
	RT_WORK_ITEM		BTConnectWorkItem;
	RT_TIMER				BTHCIDiscardAclDataTimer;
	RT_TIMER				BTHCIJoinTimeoutTimer;
	RT_TIMER				BTTestSendPacketTimer;
	RT_TIMER				BTSupervisionPktTimer;
	RT_TIMER				BTDisconnectPhyLinkTimer;
	RT_TIMER				BTBeaconTimer;
	u8				BTBeaconTmrOn;

	RT_TIMER				BTPsDisableTimer;
	RT_TIMER				BTAuthTimeoutTimer;
	RT_TIMER				BTAsocTimeoutTimer;

	PVOID				pBtChnlList;
#endif
}BT30Info, *PBT30Info;

typedef struct _BT_COEXIST_STR
{
	u8					BluetoothCoexist;
	u8					BT_Ant_Num;
	u8					BT_CoexistType;
	//u8					BT_Ant_isolation;	//0:good, 1:bad
	//u8					BT_RadioSharedType;
	//u32					Ratio_Tx;
	//u32					Ratio_PRI;
	u8					bInitlized;
#if 0
	u32					BtRfRegOrigin1E;
	u32					BtRfRegOrigin1F;
	u8					bBTBusyTraffic;
	u8					bBTTrafficModeSet;
	u8					bBTNonTrafficModeSet;
	//BT_TRAFFIC_STATISTICS		BT21TrafficStatistics;
	u64					CurrentState;
	u64					PreviousState;
	u8					preRssiState;
	u8					preRssiState1;
	u8					preRssiStateBeacon;
	u8					bFWCoexistAllOff;
	u8					bSWCoexistAllOff;
	u8					bHWCoexistAllOff;
	u8					bBalanceOn;
	u8					bSingleAntOn;
	u8					bInterruptOn;
	u8					bMultiNAVOn;
	u8					PreWLANActH;
	u8					PreWLANActL;
	u8					WLANActH;
	u8					WLANActL;
	u8					A2DPState;
	u8					AntennaState;
	u32					lastBtEdca;
	u16					last_aggr_num;
	u8					bEDCAInitialized;
	u8					exec_cnt;
	u8					b8723aAgcTableOn;
	u8					b92DAgcTableOn;
	BT_COEXIST_8723A			halCoex8723;
	u8					btActiveZeroCnt;
	u8					bCurBtDisabled;
	u8					bPreBtDisabled;
	u8					bNeedToRoamForBtDisableEnable;
	u8					fw3aVal[5];
#endif
}BT_COEXIST_STR, *PBT_COEXIST_STR;
#if 0
typedef struct _BT_CTRL_INFO{
	PVOID				Adapter;

	RT_WORK_ITEM		notifyWorkItem[NOTIFY_MAX];
	BT_NOTIFY_DATA		notifyData;
}BT_CTRL_INFO, *PBT_CTRL_INFO;
#endif
BOOLEAN
BT_Operation(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_IsHsBusy(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_AnyClientConnectToAp(
	IN	PADAPTER	Adapter
	);
#if 0
BOOLEAN
BT_IsBtScan(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_ConnectionProcess(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_JoinerScanProgress(
	IN	PADAPTER	Adapter
	);
VOID
BT_JoinerStartToConnect(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_GetEncryptInfo(
	IN	PADAPTER			Adapter,
	IN	PSTA_ENC_INFO_T		pEncInfo
	);
BOOLEAN
BT_NeedEncrypt(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_HwEncrypt(
	IN	PADAPTER	Adapter
	);
pu1Byte
BT_GetAESKeyBuf(
	IN	PADAPTER	Adapter,
	IN	u1Byte		EntryNum
	);
u1Byte
BT_OperateChnl(
	IN	PADAPTER	Adapter
	);
VOID
BT_SetEncryptFlag(
	IN	PADAPTER	Adapter,
	IN	u1Byte		Flag
	);
#endif
VOID
BT_SetManualControl(
	IN	PADAPTER	Adapter,
	IN	BOOLEAN		Flag
	);

VOID
BT_InitializeVariables(
	IN	PADAPTER	Adapter
	);
#if 0
VOID
BT_FreeMemory(
	PADAPTER	Adapter
	);

RT_STATUS
BT_AllocateMemory(
	PADAPTER	Adapter
	);

VOID
BT_InitThreads(
	IN	PADAPTER	Adapter
	);

VOID
BT_DeInitThreads(
	IN	PADAPTER	Adapter
	);

VOID
BT_IncreaseProbeDbgTxCnt(
	IN	PADAPTER	Adapter
	);

VOID
BT_ResetBtDbgInfo(
	IN	PADAPTER	Adapter
	);
VOID
BT_SetDbgTest(
	IN	PADAPTER	Adapter,
	IN	u4Byte		argNum,
	IN	pu4Byte		pData
	);
VOID
BT_DisplayIrpInfo(
	IN	PADAPTER	Adapter
	);

VOID
BT_DisplayBtPacketInfo(
	IN	PADAPTER	Adapter
	);

VOID
BT_DisplayHciInfo(
	IN	PADAPTER	Adapter
	);

VOID
BT_DisplayHSLinkState(
	IN	PADAPTER	Adapter
	);

u1Byte
BT_TargetWirelessModeToSwitch(
	u1Byte	targetChnl,
	u1Byte	currWirelessmode
	);

BOOLEAN
BT_IsLegalChannel(
	IN	PADAPTER	Adapter,
	IN	u1Byte		channel
	);

VOID
BT_SetLogoTest(
	IN	PADAPTER	Adapter,
	IN	u4Byte		type
	);
#endif
VOID
BT_DisplayBtCoexInfo(
	IN	PADAPTER	Adapter
	);

VOID
BT_IpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	);
VOID
BT_LpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	);
VOID
BT_ScanNotify(
	IN	PADAPTER 	Adapter,
	IN	u1Byte		scanType
	);
VOID
BT_WiFiConnectNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		actionType
	);
VOID
BT_WifiMediaStatusNotify(
	IN	PADAPTER			Adapter,
	IN	RT_MEDIA_STATUS		mstatus
	);
VOID
BT_SpecialPacketNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		pktType
	);
VOID
BT_BtInfoNotify(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		tmpBuf,
	IN	u1Byte		length
	);
VOID
BT_RfStatusNotify(
	IN	PADAPTER				Adapter,
	IN	rt_rf_power_state		StateToSet,
	IN	u4Byte	ChangeSource
	);
VOID
BT_StackOperationNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		opType
	);
VOID
BT_HaltNotify(
	IN	PADAPTER	Adapter
	);
VOID
BT_PnpNotify(
	IN	PADAPTER			Adapter,
	IN	u1Byte				pnpState
	);
VOID
BT_StatusWatchdog(
	IN	PADAPTER	Adapter
	);

//=======================================
//	BTHCI module
//=======================================
u1Byte
BT_GetCurrentEntryNumByMAC(
	IN	PADAPTER 	Adapter,
	IN	pu1Byte		SA
	);
VOID
BT_IndicateAMPStatus(
	IN	PADAPTER			Adapter,
	IN	u1Byte				JoinAction,
	IN	u1Byte				channel
	);
VOID
BT_EventParse(
	PADAPTER 					Adapter,
	PVOID						pEvntData,
	ULONG						dataLen
	);
u2Byte
BT_GetPhysicalLinkHandle(
	PADAPTER	Adapter,
	u1Byte		EntryNum
	);
VOID
BT_InitializeAllTimer(
	IN	PADAPTER	Adapter
	);
VOID
BT_CancelAllTimer(
	IN	PADAPTER	Adapter
	);
VOID
BT_ReleaseAllTimer(
	IN	PADAPTER	Adapter
	);
VOID
BT_InitializeAllWorkItem(
	IN	PADAPTER	Adapter
	);
VOID
BT_FreeAllWorkItem(
	IN	PADAPTER	Adapter
	);
VOID
BT_HciReset(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_HsConnectionEstablished(
	IN	PADAPTER	Adapter
	);
VOID
BT_EventAMPStatusChange(
	IN PADAPTER 					Adapter,
	u1Byte						AMP_Status
	);
VOID
BT_DisconnectAll(
	IN	PADAPTER		Adapter
	);
VOID
BT_SetLinkStatusNotify(
	PADAPTER 					Adapter,
	PVOID						pHciCmd
	);
VOID
BT_SendEventExtBtInfoControl(
	IN	PADAPTER					Adapter,
	IN	u1Byte						dataLen,
	IN	PVOID						pData
	);
VOID
BT_SendEventExtBtCoexControl(
	IN	PADAPTER					Adapter,
	IN	u1Byte						dataLen,
	IN	PVOID						pData
	);
//=======================================
//	BTPKT module
//=======================================
#if 0
u1Byte
BT_IsBtTxPacket(
	PADAPTER			Adapter,
	PRT_TCB				pTcb
	);
BOOLEAN
BT_IsBtRxPacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	);
u4Byte
BT_PreParsePacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	);
u4Byte
BT_ParsePacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	);
u4Byte
BT_IsMgntPacket(
	IN	PADAPTER		pAdapter,
	IN	PRT_RFD			pRfd
	);
VOID
BT_ReturnDataBuffer(
	PADAPTER			pAdapter,
	PRT_TX_LOCAL_BUFFER	pLocalBuffer
	);
#endif
//=======================================
//	HALBT module
//=======================================
VOID
BT_InitHalVars(
	IN	PADAPTER			Adapter
	);
BOOLEAN
BT_IsBtExist(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_BtChipType(
	IN	PADAPTER	Adapter
	);
VOID
BT_InitHwConfig(
	IN	PADAPTER	Adapter
	);

BOOLEAN
BT_OnlySupport1T(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_GetPgAntNum(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_GetMaxBtHsMacId(
	IN	PADAPTER		Adapter
	);
VOID
BT_RemoveKey(
	IN	PADAPTER	Adapter,
	IN	u1Byte		EntryNum
	);
BOOLEAN
BT_SupportRfStatusNotify(
	IN	PADAPTER	Adapter
	);
VOID
BT_SetRtsCtsNoLenLimit(
	IN	PADAPTER	Adapter
	);
VOID
BT_ActUpdateRaMask(
	IN	PADAPTER	Adapter,
	IN	u1Byte		btMacId
	);
//=======================================
//	BTDM module
//=======================================
BOOLEAN
BT_IsBtBusy(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_IsLimitedDig(
	IN	PADAPTER	Adapter
	);
VOID
BT_CoexistMechanism(
	IN	PADAPTER	Adapter
	);
s4Byte
BT_GetHsWifiRssi(
	IN		PADAPTER	Adapter
	);
BOOLEAN
BT_IsDisableEdcaTurbo(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_AdjustRssiForCoex(
	IN	PADAPTER	Adapter
	);
VOID
BT_SetBtCoexAntNum(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type,
	IN	u1Byte		antNum
	);
BOOLEAN
BT_ForceEnterLps(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_ForceExecPwrCmd(
	IN	PADAPTER	Adapter
	);
VOID
BT_DecExecPwrCmdCnt(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_IsBtDisabled(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_IsBtCoexManualControl(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_1Ant(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BT_1AntPowerSaveMode(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_1AntLpsVal(
	IN	PADAPTER	Adapter
	);
u1Byte
BT_1AntRpwmVal(
	IN	PADAPTER	Adapter
	);
VOID
BT_RecordPwrMode(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		pCmdBuf,
	IN	u1Byte		cmdLen
	);
VOID
BT_EnableBtFwCounterPolling(
	IN	PADAPTER	Adapter
	);
//=======================================
//	BTCoex module interface
//=======================================
VOID
BT_UpdateProfileInfo(VOID);

VOID
BT_UpdateMinBtRssi(
	IN	s1Byte	btRssi
	);
VOID
BT_SetHciVersion(
	IN	u2Byte	hciVersion
	);
VOID
BT_SetBtPatchVersion(
	IN	u2Byte	btHciVersion,
	IN	u2Byte	btPatchVersion
	);
VOID
BT_DbgControl(
	IN	PADAPTER			Adapter,
	IN	u1Byte				opCode,
	IN	u1Byte				opLen,
	IN	pu1Byte				pData
	);

BOOLEAN
BT_BtCtrlAggregateBufSize(
	IN	PADAPTER	Adapter
	);

u1Byte
BT_GetAdjustAggregateBufSize(
	IN	PADAPTER	Adapter
	);

//Added for Linux android driver!
void BT_SignalCompensation(PADAPTER padapter, u8 *rssi_wifi, u8 *rssi_bt);

void BT_LpsLeave(PADAPTER padapter);

void BT_LowWiFiTrafficNotify(PADAPTER padapter);
u4Byte BT_GetRaMask(PADAPTER Adapter);
#else //CONFIG_BT_COEXIST
extern u1Byte testbuf[];

#define	BT_Operation(Adapter)						FALSE
#define	BT_IsHsBusy(Adapter)						FALSE
#define	BT_IsBtScan(Adapter)						FALSE
#define	BT_ConnectionProcess(Adapter)				FALSE
#define	BT_JoinerScanProgress(Adapter)				FALSE
#define	BT_GetEncryptInfo(Adapter, pEncInfo)			FALSE
#define	BT_NeedEncrypt(Adapter)					FALSE
#define	BT_HwEncrypt(Adapter)					FALSE
#define	BT_JoinerStartToConnect(Adapter)
#define	BT_GetAESKeyBuf(Adapter, EntryNum)			testbuf
#define	BT_OperateChnl(Adapter)					0
#define	BT_SetEncryptFlag(Adapter, Flag)
#define	BT_SetManualControl(Adapter, Flag)
#define	BT_InitializeVariables(Adapter)
#define	BT_AllocateMemory(Adapter)				RT_STATUS_SUCCESS;
#define	BT_FreeMemory(Adapter)
#define	BT_InitThreads(Adapter)
#define	BT_DeInitThreads(Adapter)
#define	BT_IncreaseProbeDbgTxCnt(Adapter)
#define	BT_ResetBtDbgInfo(Adapter)
#define	BT_SetDbgTest(Adapter, argNum, pData);
#define	BT_DisplayIrpInfo(Adapter)
#define	BT_DisplayBtPacketInfo(Adapter)
#define	BT_DisplayHciInfo(Adapter)
#define	BT_DisplayHSLinkState(Adapter)
#define	BT_DisplayBtCoexInfo(Adapter)
#define	BT_SetLogoTest(Adapter, type)
#define	BT_IpsNotify(Adapter, type)
#define	BT_LpsNotify(Adapter, type)
#define	BT_ScanNotify(Adapter, scanType)
#define	BT_ConnectNotify(Adapter, actionType)
#define	BT_WifiMediaStatusNotify(Adapter, mstatus)
#define	BT_SpecialPacketNotify(Adapter, pktType)
#define	BT_BtInfoNotify(Adapter, tmpBuf, length)
#define	BT_RfStatusNotify(Adapter, StateToSet, ChangeSource)
#define	BT_StackOperationNotify(Adapter, opType)
#define	BT_HaltNotify(Adapter)
#define	BT_PnpNotify(Adapter, pnpState)
#define	BT_StatusWatchdog(Adapter)
//=======================================
//	BTHCI module interface
//=======================================
#define	BT_GetCurrentEntryNumByMAC(Adapter, SA)				0xff
#define	BT_IndicateAMPStatus(Adapter, JoinAction, channel)
#define	BT_EventParse(Adapter, pEvntData, dataLen)
#define	BT_GetPhysicalLinkHandle(Adapter, EntryNum)			0
#define	BT_InitializeAllTimer(Adapter)
#define	BT_CancelAllTimer(Adapter)
#define	BT_ReleaseAllTimer(Adapter)
#define	BT_InitializeAllWorkItem(Adapter)
#define	BT_FreeAllWorkItem(Adapter)
#define	BT_HciReset(Adapter)
#define	BT_HsConnectionEstablished(Adapter)					FALSE
#define	BT_EventAMPStatusChange(Adapter, AMP_Status)
#define	BT_DisconnectAll(Adapter)
#define	BT_SetLinkStatusNotify(Adapter, pHciCmd)
#define	BT_SendEventExtBtInfoControl(Adapter, dataLen, pData)
#define	BT_SendEventExtBtCoexControl(Adapter, dataLen, pData)
//=======================================
//	BTPKT module interface
//=======================================
#define	BT_IsBtTxPacket(Adapter, pTcb)				(FALSE)
#define	BT_IsBtRxPacket(pAdapter, pRfd)				(FALSE)
#define	BT_PreParsePacket(pAdapter, pRfd)			(FALSE)
#define	BT_ParsePacket(pAdapter, pRfd)				(FALSE)
#define	BT_IsMgntPacket(pAdapter, pRfd)			FALSE)
#define	BT_ReturnDataBuffer(pAdapter, pLocalBuffer)

//=======================================
//	HALBT module interface
//=======================================
#define	BT_InitHalVars(Adapter)
#define	BT_IsBtExist(Adapter)			FALSE
#define	BT_BtChipType(Adapter)		0
#define	BT_InitHwConfig(Adapter)
#define	BT_OnlySupport1T(Adapter)		FALSE
#define	BT_GetPgAntNum(Adapter)		2
#define	BT_GetMaxBtHsMacId(Adapter)	0
#define	BT_RemoveKey(Adapter, EntryNum)
#define	BT_SupportRfStatusNotify(Adapter)
#define	BT_SetRtsCtsNoLenLimit(Adapter)
//=======================================
//	BTDM module interface
//=======================================
#define	BT_IsBtBusy(Adapter)					FALSE
#define	BT_IsLimitedDig(Adapter)				FALSE
#define	BT_CoexistMechanism(Adapter)
#define	BT_IsDisableEdcaTurbo(Adapter)				FALSE
#define	BT_AdjustRssiForCoex(Adapter)				0
#define	BT_SetBtCoexAntNum(Adapter, type, antNum)
#define	BT_ForceEnterLps(Adapter)							FALSE
#define	BT_ForceExecPwrCmd(Adapter)						FALSE
#define	BT_DecExecPwrCmdCnt(Adapter)
#define	BT_IsBtDisabled(Adapter)							FALSE
#define	BT_1Ant(Adapter)									FALSE
#define	BT_1AntPowerSaveMode(Adapter)					FALSE
#define	BT_1AntLpsVal(Adapter)					0
#define	BT_1AntRpwmVal(Adapter)					0
#define	BT_RecordPwrMode(Adapter, pCmdBuf, cmdLen);
#define	BT_EnableBtFwCounterPolling(Adapter)
#define	BT_IsBtCoexManualControl(Adapter)				FALSE
//=======================================
//	BTCoex module interface
//=======================================
#define	BT_UpdateProfileInfo()
#define	BT_UpdateMinBtRssi(btRssi)
#define	BT_SetHciVersion(hciVersion)
#define	BT_SetBtPatchVersion(btHciVersion, btPatchVersion)
#define	BT_DbgControl(Adapter, opCode, opLen, pData)

//Other from linux android platform
#define BT_BtCtrlAggregateBufSize(Adapter)					FALSE
#define BT_GetAdjustAggregateBufSize(Adapter)				8
#define BT_SignalCompensation(padapter,rssi_wifi,rssi_bt)
#define BT_LpsLeave(padapter)
#define BT_LowWiFiTrafficNotify(padapter)
#define BT_GetRaMask	0
#endif //!CONFIG_BT_COEXIST

#endif //CONFIG_RTL8723B
#endif
