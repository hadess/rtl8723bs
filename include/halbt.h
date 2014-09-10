#ifndef __INC_HAL_BT_H
#define __INC_HAL_BT_H

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST
#define RTS_CTS_NO_LEN_LIMIT	0


//==================================================
//	extern function
//==================================================
u1Byte
HALBT_GetMaxBtHsMacId(
	IN	PADAPTER		Adapter
	);

VOID
HALBT_DownloadRsvdPageForBt(
	IN	PADAPTER	Adapter
	);

BOOLEAN
HALBT_SupportRfStatusNotify(
	IN	PADAPTER	Adapter
	);

u1Byte
HALBT_GetPgAntNum(
	IN	PADAPTER	Adapter
	);
VOID
HALBT_SetKey(
	PADAPTER            	Adapter,
	u1Byte			EntryNum);

VOID
HALBT_RemoveKey(
	PADAPTER     		Adapter,
	u1Byte			EntryNum);
BOOLEAN
HALBT_IsBtExist(
	IN	PADAPTER	Adapter
	);
u1Byte
HALBT_BTChipType(
	IN	PADAPTER	Adapter
	);
VOID
HALBT_InitHwConfig(
	IN	PADAPTER	Adapter
	);
VOID
HALBT_SetRtsCtsNoLenLimit(
	IN	PADAPTER	Adapter
	);
BOOLEAN
HALBT_OnlySupport1T(
	IN	PADAPTER	Adapter
	);
VOID
HALBT_SwitchWirelessMode(
	IN	PADAPTER			Adapter,
	IN	u1Byte				targetWirelessMode
	);
VOID
HALBT_InitHalVars(
	IN	PADAPTER			Adapter
	);
VOID
HALBT_SendBtDbgH2c(
	IN	PADAPTER	Adapter,
	IN	u4Byte		CmdLen,
	IN	pu1Byte 		pCmdBuffer
	);
VOID
WA_HALBT_EnableBtFwCounterPolling(
	IN	PADAPTER	Adapter
	);

#else
#define	BT_COEXIST_STR		u1Byte

#endif


#endif
