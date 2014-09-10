//==================================================
//
//	This module is for BT related Hal code.
//
//		04/13/2011 created by Cosa
//
//==================================================

#include <drv_types.h>
#ifdef CONFIG_RTL8723A
#include <rtl8723a_hal.h>
#endif

#ifdef CONFIG_RTL8723B
#include <rtl8723b_hal.h>
#endif

#include <halbt.h>

#ifdef CONFIG_RTL8723B

//#if(BT_30_SUPPORT == 1)
#ifdef CONFIG_BT_COEXIST
//==================================================
//	local function
//==================================================


//==================================================
//	extern function
//==================================================
#if 0
VOID
HALBT_DownloadRsvdPageForBt(
	IN	PADAPTER	Adapter
	)
{
	if(BT_1Ant(Adapter))
	{
		HalDownloadRsvdPage(Adapter, FALSE);
	}
}
#endif

BOOLEAN
HALBT_SupportRfStatusNotify(
	IN	PADAPTER	Adapter
	)
{
	BOOLEAN		bSupport=FALSE;

	if(IS_HARDWARE_TYPE_8723AU(Adapter))
		bSupport = TRUE;

	// Disable RF status notification when customer is Lenovo China
	//if(Adapter->MgntInfo.CustomerID == RT_CID_LENOVO_CHINA)
	//	bSupport = FALSE;

	return bSupport;
}

u1Byte
HALBT_GetPgAntNum(
	IN	PADAPTER	Adapter
	)
{
	u1Byte			antNum=2;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(pHalData->bt_coexist.BT_Ant_Num == Ant_x2)
		antNum = 2;
	else if(pHalData->bt_coexist.BT_Ant_Num == Ant_x1)
		antNum = 1;

	return antNum;
}

BOOLEAN
HALBT_IsBtExist(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if( IS_HARDWARE_TYPE_8192C(Adapter) ||
		IS_HARDWARE_TYPE_8192D(Adapter) ||
		IS_HARDWARE_TYPE_8723A(Adapter) ||
		IS_HARDWARE_TYPE_8723B(Adapter))
	{
		if(pHalData->bt_coexist.BluetoothCoexist)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

u1Byte
HALBT_BTChipType(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	return pHalData->bt_coexist.BT_CoexistType;
}


VOID
HALBT_SetRtsCtsNoLenLimit(
	IN	PADAPTER	Adapter
	)
{
#if( RTS_CTS_NO_LEN_LIMIT == 1)
	PlatformEFIOWrite4Byte(Adapter, 0x4c8, 0xc140402);
#endif
}

VOID
HALBT_SwitchWirelessMode(
	IN	PADAPTER			Adapter,
	IN	u1Byte				targetWirelessMode
	)
{

}

VOID
HALBT_InitHalVars(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

#if (MP_DRIVER == 1)
	//pHalData->EEPROMBluetoothCoexist = 0;
#endif
	//pHalData->bt_coexist.bBtExist = pHalData->EEPROMBluetoothCoexist;
	//pHalData->bt_coexist.btTotalAntNum = pHalData->EEPROMBluetoothAntNum;
	//pHalData->bt_coexist.btChipType = pHalData->EEPROMBluetoothType;

	pHalData->bt_coexist.BluetoothCoexist = pHalData->EEPROMBluetoothCoexist;
	pHalData->bt_coexist.BT_Ant_Num = pHalData->EEPROMBluetoothAntNum;
	pHalData->bt_coexist.BT_CoexistType = pHalData->EEPROMBluetoothType;
	//pHalData->bt_coexist.BT_Ant_isolation = pHalData->EEPROMBluetoothAntIsolation;
	//pHalData->bt_coexist.BT_RadioSharedType = pHalData->EEPROMBluetoothRadioShared;
}

extern s32 FillH2CCmd(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer);
VOID
HALBT_SendBtDbgH2c(
	IN	PADAPTER	Adapter,
	IN	u4Byte		CmdLen,
	IN	pu1Byte 		pCmdBuffer
	)
{
	if(IS_HARDWARE_TYPE_8723A(Adapter))
	{
		FillH2CCmd(Adapter, 0x46, CmdLen, pCmdBuffer);
	}
	else
	{
		FillH2CCmd(Adapter, 0x67, CmdLen, pCmdBuffer);
	}
}
#endif
#endif
