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
#ifndef __RTL8192D_HAL_H__
#define __RTL8192D_HAL_H__

//#include "hal_com.h"

#if 1
#include "hal_data.h"
#else
#include "../hal/OUTSRC/odm_precomp.h"
#endif

#include "rtl8192d_spec.h"
#include "Hal8192DPhyReg.h"
#include "Hal8192DPhyCfg.h"
#include "rtl8192d_rf.h"
#include "rtl8192d_dm.h"
#include "rtl8192d_recv.h"
#include "rtl8192d_xmit.h"
#include "rtl8192d_cmd.h"
#include "rtl8192d_led.h"

//
// Check if FW header exists. We do not consider the lower 4 bits in this case. 
// By tynli. 2009.12.04.
//
#define IS_FW_HEADER_EXIST_92D(_pFwHdr)	((le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x92C0 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x88C0 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFFF) == 0x92D0 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFFF) == 0x92D1 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFFF) == 0x92D2 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFFF) == 0x92D3 )

#define FW_8192D_SIZE				0x8020 // Max FW len = 32k + 32(FW header length).
#define FW_8192D_START_ADDRESS	0x1000
#define FW_8192D_END_ADDRESS		0x1FFF



typedef struct _RT_FIRMWARE_8192D{
	FIRMWARE_SOURCE	eFWSource;
	u8*			szFwBuffer;
	u32			ulFwLength;
} RT_FIRMWARE_8192D, *PRT_FIRMWARE_8192D;

//
// This structure must be cared byte-ordering
//
// Added by tynli. 2009.12.04.
typedef struct _RT_8192D_FIRMWARE_HDR {//8-byte alinment required

	//--- LONG WORD 0 ----
	u16		Signature;	// 92C0: test chip; 92C, 88C0: test chip; 88C1: MP A-cut; 92C1: MP A-cut
	u8		Category;	// AP/NIC and USB/PCI
	u8		Function;	// Reserved for different FW function indcation, for further use when driver needs to download different FW in different conditions
	u16		Version;		// FW Version
	u8		Subversion;	// FW Subversion, default 0x00
	u8		Rsvd1;


	//--- LONG WORD 1 ----
	u8		Month;	// Release time Month field
	u8		Date;	// Release time Date field
	u8		Hour;	// Release time Hour field
	u8		Minute;	// Release time Minute field
	u16		RamCodeSize;	// The size of RAM code
	u16		Rsvd2;

	//--- LONG WORD 2 ----
	u32		SvnIdx;	// The SVN entry index
	u32		Rsvd3;

	//--- LONG WORD 3 ----
	u32		Rsvd4;
	u32		Rsvd5;

}RT_8192D_FIRMWARE_HDR, *PRT_8192D_FIRMWARE_HDR;

#define DRIVER_EARLY_INT_TIME_8192D		0x05
#define BCN_DMA_ATIME_INT_TIME_8192D		0x02

typedef	enum _BT_CurState{
	BT_OFF		= 0,	
	BT_ON		= 1,
} BT_CurState, *PBT_CurState;

typedef	enum _BT_ServiceType{
	BT_SCO			= 0,	
	BT_A2DP			= 1,
	BT_HID			= 2,
	BT_HID_Idle		= 3,
	BT_Scan			= 4,
	BT_Idle			= 5,
	BT_OtherAction	= 6,
	BT_Busy			= 7,
	BT_OtherBusy		= 8,
} BT_ServiceType, *PBT_ServiceType;

typedef struct _BT_COEXIST_STR{
	u8					BluetoothCoexist;
	u8					BT_Ant_Num;
	u8					BT_CoexistType;
	u8					BT_State;
	u8					BT_CUR_State;		//0:on, 1:off
	u8					BT_Ant_isolation;	//0:good, 1:bad
	u8					BT_PapeCtrl;		//0:SW, 1:SW/HW dynamic
	u8					BT_Service;			
	u8					BT_RadioSharedType;
	u8					Ratio_Tx;
	u8					Ratio_PRI;
}BT_COEXIST_STR, *PBT_COEXIST_STR;





// Note: We will divide number of page equally for each queue other than public queue!

#define TX_TOTAL_PAGE_NUMBER_8192D		0xF8
#define TX_PAGE_BOUNDARY			(TX_TOTAL_PAGE_NUMBER_8192D + 1)

// For Normal Chip Setting
// (HPQ + LPQ + NPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER_8192D
#define NORMAL_PAGE_NUM_PUBQ		0x56


// For Test Chip Setting
// (HPQ + LPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER_8192D
#define TEST_PAGE_NUM_PUBQ			0x89
#define TX_TOTAL_PAGE_NUMBER_92D_DUAL_MAC		0x7A
#define NORMAL_PAGE_NUM_PUBQ_92D_DUAL_MAC			0x5A
#define NORMAL_PAGE_NUM_HPQ_92D_DUAL_MAC			0x10
#define NORMAL_PAGE_NUM_LPQ_92D_DUAL_MAC			0x10
#define NORMAL_PAGE_NUM_NORMALQ_92D_DUAL_MAC		0

#define TX_PAGE_BOUNDARY_DUAL_MAC			(TX_TOTAL_PAGE_NUMBER_92D_DUAL_MAC + 1)

// For Test Chip Setting
#define WMM_TEST_TX_TOTAL_PAGE_NUMBER	0xF5
#define WMM_TEST_TX_PAGE_BOUNDARY	(WMM_TEST_TX_TOTAL_PAGE_NUMBER + 1) //F6

#define WMM_TEST_PAGE_NUM_PUBQ		0xA3
#define WMM_TEST_PAGE_NUM_HPQ		0x29
#define WMM_TEST_PAGE_NUM_LPQ		0x29


//Note: For Normal Chip Setting ,modify later
#define WMM_NORMAL_TX_TOTAL_PAGE_NUMBER	0xF5
#define WMM_NORMAL_TX_PAGE_BOUNDARY	(WMM_TEST_TX_TOTAL_PAGE_NUMBER + 1) //F6

#define WMM_NORMAL_PAGE_NUM_PUBQ		0xB0
#define WMM_NORMAL_PAGE_NUM_HPQ		0x29
#define WMM_NORMAL_PAGE_NUM_LPQ			0x1C
#define WMM_NORMAL_PAGE_NUM_NPQ		0x1C

#define WMM_NORMAL_PAGE_NUM_PUBQ_92D		0X65//0x82
#define WMM_NORMAL_PAGE_NUM_HPQ_92D		0X30//0x29
#define WMM_NORMAL_PAGE_NUM_LPQ_92D		0X30
#define WMM_NORMAL_PAGE_NUM_NPQ_92D		0X30

//-------------------------------------------------------------------------
//	Chip specific
//-------------------------------------------------------------------------

#define CHIP_BONDING_IDENTIFIER(_value)	(((_value)>>22)&0x3)
#define CHIP_BONDING_92C_1T2R	0x1
#define CHIP_BONDING_88C_USB_MCARD	0x2
#define CHIP_BONDING_88C_USB_HP	0x1

//-------------------------------------------------------------------------
//	Channel Plan
//-------------------------------------------------------------------------


#define EFUSE_REAL_CONTENT_LEN	1024
#define EFUSE_MAP_LEN				256
#define EFUSE_MAX_SECTION			32
#define EFUSE_MAX_SECTION_BASE	16
// <Roger_Notes> To prevent out of boundary programming case, leave 1byte and program full section
// 9bytes + 1byt + 5bytes and pre 1byte.
// For worst case:
// | 2byte|----8bytes----|1byte|--7bytes--| //92D
#define EFUSE_OOB_PROTECT_BYTES 	18 // PG data exclude header, dummy 7 bytes frome CP test and reserved 1byte.

typedef enum _PA_MODE {
	PA_MODE_EXTERNAL = 0x00,
	PA_MODE_INTERNAL_SP3T = 0x01,
	PA_MODE_INTERNAL_SPDT = 0x02	
} PA_MODE;

/* Copy from rtl8192c */
enum c2h_id_8192d {
	C2H_DBG = 0,
	C2H_TSF = 1,
	C2H_AP_RPT_RSP = 2,
	C2H_CCX_TX_RPT = 3,
	C2H_BT_RSSI = 4,
	C2H_BT_OP_MODE = 5,
	C2H_EXT_RA_RPT = 6,
	C2H_HW_INFO_EXCH = 10,
	C2H_C2H_H2C_TEST = 11,
	C2H_BT_INFO = 12,
	C2H_BT_MP_INFO = 15,
	MAX_C2HEVENT
};

int FirmwareDownload92D(IN PADAPTER Adapter);
void rtl8192d_FirmwareSelfReset(IN PADAPTER Adapter);
void rtl8192d_ReadChipVersion(IN PADAPTER Adapter);
void rtl8192d_EfuseParseChnlPlan(PADAPTER Adapter, u8 *hwinfo, bool AutoLoadFail);
void rtl8192d_ReadTxPowerInfo(PADAPTER Adapter, u8* PROMContent, bool AutoLoadFail);
void rtl8192d_ResetDualMacSwitchVariables(IN PADAPTER Adapter);
u8 GetEEPROMSize8192D(PADAPTER Adapter);
bool PHY_CheckPowerOffFor8192D(PADAPTER Adapter);
void PHY_SetPowerOnFor8192D(PADAPTER Adapter);
//void PHY_ConfigMacPhyMode92D(PADAPTER Adapter);
void rtl8192d_free_hal_data(_adapter * padapter);
void rtl8192d_set_hal_ops(struct hal_ops *pHalFunc);

void SetHwReg8192D(_adapter *adapter, u8 variable, u8 *val);
void GetHwReg8192D(_adapter *adapter, u8 variable, u8 *val);
#endif

