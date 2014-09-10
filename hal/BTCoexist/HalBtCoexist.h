#ifndef	__HALBT_COEXIST_H__
#define __HALBT_COEXIST_H__

//#include <drv_conf.h>
//#include <osdep_service.h>
#include <drv_types.h>
#include "halbt_precomp.h"
#ifdef CONFIG_DEBUG
#define BT_DEBUG
#endif
#ifdef BT_DEBUG

#ifdef PLATFORM_LINUX
#define RTPRINT(a,b,c) printk c
#define RTPRINT_ADDR(dbgtype, dbgflag, printstr, _Ptr)\
{\
	u32 __i;						\
	u8 *ptr = (u8*)_Ptr;	\
	printk printstr;				\
	printk(" ");					\
	for( __i=0; __i<6; __i++ )		\
		printk("%02X%s", ptr[__i], (__i==5)?"":"-");		\
	printk("\n");							\
}
#define RTPRINT_DATA(dbgtype, dbgflag, _TitleString, _HexData, _HexDataLen)\
{\
	u32 __i;									\
	u8 *ptr = (u8*)_HexData;			\
	printk(_TitleString);					\
	for( __i=0; __i<(u32)_HexDataLen; __i++ )	\
	{										\
		printk("%02X%s", ptr[__i], (((__i + 1) % 4) == 0)?"  ":" ");\
		if (((__i + 1) % 16) == 0)	printk("\n");\
	}										\
	printk("\n");							\
}
// Added by Annie, 2005-11-22.
#define MAX_STR_LEN	64
#define PRINTABLE(_ch)	(_ch>=' ' &&_ch<='~')	// I want to see ASCII 33 to 126 only. Otherwise, I print '?'. Annie, 2005-11-22.
#define RT_PRINT_STR(_Comp, _Level, _TitleString, _Ptr, _Len)					\
{\
/*	if (((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	*/\
	{									\
		u32 __i;						\
		u8 buffer[MAX_STR_LEN];					\
		u32	length = (_Len<MAX_STR_LEN)? _Len : (MAX_STR_LEN-1) ;	\
		_rtw_memset(buffer, 0, MAX_STR_LEN);			\
		_rtw_memcpy(buffer, (u8*)_Ptr, length);		\
		for (__i=0; __i<length; __i++)					\
		{								\
			if (!PRINTABLE(buffer[__i]))	buffer[__i] = '?';	\
		}								\
		buffer[length] = '\0';						\
/*		printk("Rtl819x: ");*/						\
		printk(_TitleString);						\
		printk(": %d, <%s>\n", _Len, buffer);				\
	}\
}
#endif // PLATFORM_LINUX

#else // !BT_DEBUG

#define RTPRINT(...)
#define RTPRINT_ADDR(...)
#define RTPRINT_DATA(...)
#define RT_PRINT_STR(...)

#endif // !BT_DEBUG

#define GET_UNDECORATED_AVERAGE_RSSI(padapter)	\
			(GET_HAL_DATA(padapter)->dmpriv.EntryMinUndecoratedSmoothedPWDB)

VOID
BTDM_InitlizeVariables(
	IN	PADAPTER	Adapter
	);

s4Byte
BTDM_GetWifiRssi(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BTDM_IsHt40(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BTDM_Legacy(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BTDM_IsLimitedDig(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BTDM_IsBtBusy(
	IN	PADAPTER	Adapter
	);
BOOLEAN
BTDM_IsWifiBusy(
	IN	PADAPTER	Adapter
	);

BOOLEAN
BTDM_IsWifiUplink(
	IN	PADAPTER	Adapter
	);
//======================================
//	The following define the extern function called by
//	those no-bt module.
//======================================

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST

//#define GET_BT_INFO(padapter)	(&GET_HAL_DATA(padapter)->BtInfo)

VOID
BTDM_DisplayBtCoexInfo(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_SetBtCoexCurrAntNum(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type,
	IN	u1Byte		antNum
	);

void BTDM_InitHalVars(PADAPTER	padapter);

VOID
BTDM_SetBtExist(
	IN	PADAPTER			Adapter
	);
VOID
BTDM_SetBtChipType(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_SetBtCoexAntNum(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type,
	IN	u1Byte		antNum
	);

VOID
BTDM_InitHwConfig(
	IN	PADAPTER	Adapter
	);

BOOLEAN
BTDM_IsDisableEdcaTurbo(
	IN	PADAPTER	Adapter
	);
s4Byte
BTDM_GetHsWifiRssi(
	IN		PADAPTER	Adapter
	);

u1Byte
BTDM_AdjustRssiForCoex(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_Coexist(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_IpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	);
VOID
BTDM_LpsNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		type
	);
VOID
BTDM_ScanNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		scanType
	);
VOID
BTDM_ConnectNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		action
	);
VOID
BTDM_MediaStatusNotify(
	IN	PADAPTER	Adapter,
	IN	RT_MEDIA_STATUS		mstatus
	);
VOID
BTDM_SpecialPacketNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		pktType
	);
VOID
BTDM_BtInfoNotify(
	IN	PADAPTER	Adapter,
	IN	pu1Byte		tmpBuf,
	IN	u1Byte		length
	);
VOID
BTDM_StackOperationNotify(
	IN	PADAPTER	Adapter,
	IN	u1Byte		stackOpType
	);
BOOLEAN
BTDM_IsBtDisabled(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_ApStatusWatchdog(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_InitializeAllWorkItem(
	IN	PADAPTER	Adapter
	);
VOID
BTDM_FreeAllWorkItem(
	IN	PADAPTER	Adapter
	);

VOID
BTDM_RejectApAggregatedPacket(
	IN	PADAPTER	Adapter,
	IN	BOOLEAN		bReject
	);
VOID
BTDM_SetManualControl(
	IN	BOOLEAN		bManual
	);

VOID
BTDM_SignalCompensation(PADAPTER padapter, u8 *rssi_wifi, u8 *rssi_bt);

VOID
BTDM_LpsLeave(PADAPTER padapter);

VOID
BTDM_LowWiFiTrafficNotify(PADAPTER padapter);
#endif	// #if  (BT_30_SUPPORT == 1)
#endif	// #ifndef	__HALBT_COEXIST_H__
