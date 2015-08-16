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

#ifndef __HAL_PG_H__
#define __HAL_PG_H__

//====================================================
//			EEPROM/Efuse PG Offset for 8723BE/8723BU/8723BS
//====================================================
// 0x10 ~ 0x63 = TX power area.
#define	EEPROM_TX_PWR_INX_8723B				0x10

#define	EEPROM_ChannelPlan_8723B				0xB8
#define	EEPROM_XTAL_8723B						0xB9
#define	EEPROM_THERMAL_METER_8723B			0xBA

#define	EEPROM_RF_BOARD_OPTION_8723B		0xC1
#define	EEPROM_RF_BT_SETTING_8723B			0xC3
#define	EEPROM_VERSION_8723B					0xC4
#define	EEPROM_CustomID_8723B				0xC5

//RTL8723BS
#define	EEPROM_MAC_ADDR_8723BS				0x11A
#define EEPROM_Voltage_ADDR_8723B			0x8

//====================================================
//			EEPROM/Efuse Default Value
//====================================================

#define EEPROM_CHANNEL_PLAN_BY_HW_MASK		0x80

#define RTL_EEPROM_ID							0x8129
#define EEPROM_Default_ThermalMeter			0x12
#define	EEPROM_Default_ThermalMeter_8723B		0x18


#define EEPROM_Default_CrystalCap				0x0
#define EEPROM_Default_CrystalCap_8723B			0x20

//New EFUSE deafult value
#define EEPROM_DEFAULT_24G_INDEX			0x2D
#define EEPROM_DEFAULT_24G_HT20_DIFF		0X02
#define EEPROM_DEFAULT_24G_OFDM_DIFF	0X04

#define EEPROM_DEFAULT_DIFF				0XFE
#define EEPROM_DEFAULT_BOARD_OPTION		0x00
#define EEPROM_DEFAULT_RFE_OPTION		0x04

//
// For VHT series TX power by rate table.
// VHT TX power by rate off setArray =
// Band:-2G&5G = 0 / 1
// RF: at most 4*4 = ABCD=0/1/2/3
// CCK=0 OFDM=1/2 HT-MCS 0-15=3/4/56 VHT=7/8/9/10/11
//
#define TX_PWR_BY_RATE_NUM_BAND			2
#define TX_PWR_BY_RATE_NUM_RF			4
#define TX_PWR_BY_RATE_NUM_RATE			84

//----------------------------------------------------------------------------
//       EEPROM/EFUSE data structure definition.
//----------------------------------------------------------------------------
#define MAX_RF_PATH_NUM	2
#define MAX_CHNL_GROUP		3+9
typedef struct _TxPowerInfo{
	u8 CCKIndex[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 HT40_1SIndex[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 HT40_2SIndexDiff[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	s8 HT20IndexDiff[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 OFDMIndexDiff[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 HT40MaxOffset[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 HT20MaxOffset[MAX_RF_PATH_NUM][MAX_CHNL_GROUP];
	u8 TSSI_A[3];
	u8 TSSI_B[3];
	u8 TSSI_A_5G[3];		//5GL/5GM/5GH
	u8 TSSI_B_5G[3];
}TxPowerInfo, *PTxPowerInfo;

#define	MAX_RF_PATH				4
#define		RF_PATH_MAX				MAX_RF_PATH
#define	MAX_CHNL_GROUP_24G		6

//It must always set to 4, otherwise read efuse table secquence will be wrong.
#define		MAX_TX_COUNT				4

typedef struct _TxPowerInfo24G{
	u8 IndexCCK_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
	u8 IndexBW40_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
	//If only one tx, only BW20 and OFDM are used.
	s8 CCK_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8 OFDM_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8 BW20_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8 BW40_Diff[MAX_RF_PATH][MAX_TX_COUNT];
}TxPowerInfo24G, *PTxPowerInfo24G;

typedef	enum _BT_Ant_NUM{
	Ant_x2	= 0,
	Ant_x1	= 1
} BT_Ant_NUM, *PBT_Ant_NUM;

typedef	enum _BT_CoType{
	BT_RTL8723B		= 8,
} BT_CoType, *PBT_CoType;

typedef	enum _BT_RadioShared{
	BT_Radio_Shared		= 0,
	BT_Radio_Individual	= 1,
} BT_RadioShared, *PBT_RadioShared;


#endif
