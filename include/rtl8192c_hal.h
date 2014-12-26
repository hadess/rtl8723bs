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
#ifndef __RTL8192C_HAL_H__
#define __RTL8192C_HAL_H__

//#include "hal_com.h"

#if 1
#include "hal_data.h"
#else
#include "../hal/OUTSRC/odm_precomp.h"
#endif


#include "drv_types.h"
#include "rtl8192c_spec.h"
#include "Hal8192CPhyReg.h"
#include "Hal8192CPhyCfg.h"
#include "rtl8192c_rf.h"
#include "rtl8192c_dm.h"
#include "rtl8192c_recv.h"
#include "rtl8192c_xmit.h"
#include "rtl8192c_cmd.h"
#include "rtl8192c_led.h"

#define FW_8192C_SIZE					16384+32//16k
#define FW_8192C_START_ADDRESS		0x1000
//#define FW_8192C_END_ADDRESS		0x3FFF //Filen said this is for test chip
#define FW_8192C_END_ADDRESS		0x1FFF

#define IS_FW_HEADER_EXIST_92C(_pFwHdr)	((le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x92C0 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x88C0 ||\
									(le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x2300)


typedef struct _RT_FIRMWARE_8192C{
	FIRMWARE_SOURCE	eFWSource;
	u8*			szFwBuffer;
	u32			ulFwLength;
} RT_FIRMWARE_8192C, *PRT_FIRMWARE_8192C;

//
// This structure must be cared byte-ordering
//
// Added by tynli. 2009.12.04.
typedef struct _RT_8192C_FIRMWARE_HDR {//8-byte alinment required

	//--- LONG WORD 0 ----
	u16		Signature;	// 92C0: test chip; 92C, 88C0: test chip; 88C1: MP A-cut; 92C1: MP A-cut
	u8		Category;	// AP/NIC and USB/PCI
	u8		Function;	// Reserved for different FW function indcation, for further use when driver needs to download different FW in different conditions
	u16		Version;		// FW Version
	u8		Subversion;	// FW Subversion, default 0x00
	u16		Rsvd1;


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

}RT_8192C_FIRMWARE_HDR, *PRT_8192C_FIRMWARE_HDR;

#define DRIVER_EARLY_INT_TIME_8192C		0x05
#define BCN_DMA_ATIME_INT_TIME_8192C		0x02



// Note: We will divide number of page equally for each queue other than public queue!

#define TX_TOTAL_PAGE_NUMBER_8192C		0xF8
#define TX_PAGE_BOUNDARY		(TX_TOTAL_PAGE_NUMBER_8192C + 1)

// For Normal Chip Setting
// (HPQ + LPQ + NPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER_8192C
#define NORMAL_PAGE_NUM_PUBQ			0xE7
#define NORMAL_PAGE_NUM_HPQ			0x0C
#define NORMAL_PAGE_NUM_LPQ			0x02
#define NORMAL_PAGE_NUM_NPQ			0x02


// For Test Chip Setting
// (HPQ + LPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER_8192C
#define TEST_PAGE_NUM_PUBQ		0x7E


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

#define		EFUSE_REAL_CONTENT_LEN		512
#define		EFUSE_MAP_LEN					128
#define		EFUSE_MAX_SECTION			16
#define		EFUSE_IC_ID_OFFSET			506	//For some inferiority IC purpose. added by Roger, 2009.09.02.
#define 		AVAILABLE_EFUSE_ADDR(addr) 	(addr < EFUSE_REAL_CONTENT_LEN)
//
// <Roger_Notes> To prevent out of boundary programming case, leave 1byte and program full section
// 9bytes + 1byt + 5bytes and pre 1byte.
// For worst case:
// | 1byte|----8bytes----|1byte|--5bytes--| 
// |         |            Reserved(14bytes)	      |
//
#define		EFUSE_OOB_PROTECT_BYTES 		15	// PG data exclude header, dummy 6 bytes frome CP test and reserved 1byte.


#define		EFUSE_MAP_LEN_8723			256
#define		EFUSE_MAX_SECTION_8723		32

//========================================================
//			EFUSE for BT definition
//========================================================
#define		EFUSE_BT_REAL_CONTENT_LEN		1536	// 512*3
#define		EFUSE_BT_MAP_LEN					1024	// 1k bytes
#define		EFUSE_BT_MAX_SECTION				128		// 1024/8

#define		EFUSE_PROTECT_BYTES_BANK			16
enum c2h_id_8192c {
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

#define INCLUDE_MULTI_FUNC_BT(_Adapter)	(GET_HAL_DATA(_Adapter)->MultiFunc & RT_MULTI_FUNC_BT)
#define INCLUDE_MULTI_FUNC_GPS(_Adapter)	(GET_HAL_DATA(_Adapter)->MultiFunc & RT_MULTI_FUNC_GPS)

VOID rtl8192c_FirmwareSelfReset(IN PADAPTER Adapter);
int FirmwareDownload92C(IN PADAPTER Adapter);
VOID InitializeFirmwareVars92C(PADAPTER Adapter);
u8 GetEEPROMSize8192C(PADAPTER Adapter);
void rtl8192c_EfuseParseChnlPlan(PADAPTER padapter, u8 *hwinfo, BOOLEAN AutoLoadFail);

HAL_VERSION rtl8192c_ReadChipVersion(IN PADAPTER Adapter);
void rtl8192c_ReadBluetoothCoexistInfo(PADAPTER Adapter, u8 *PROMContent, BOOLEAN AutoloadFail);

VOID rtl8192c_EfuseParseIDCode(PADAPTER pAdapter, u8 *hwinfo);
void rtl8192c_set_hal_ops(struct hal_ops *pHalFunc);

s32 c2h_id_filter_ccx_8192c(u8 *buf);

void SetHwReg8192C(PADAPTER padapter, u8 variable, u8 *val);
void GetHwReg8192C(PADAPTER padapter, u8 variable, u8 *val);

#endif

