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
#ifndef __RTL8723B_PG_H__
#define __RTL8723B_PG_H__

//====================================================
//			EEPROM/Efuse PG Offset for 8723BE/8723BU/8723BS
//====================================================
// 0x10 ~ 0x63 = TX power area.
#define	EEPROM_TX_PWR_INX_8723B				0x10

#define	EEPROM_ChannelPlan_8723B				0xB8
#define	EEPROM_XTAL_8723B						0xB9
#define	EEPROM_THERMAL_METER_8723B			0xBA
#define	EEPROM_IQK_LCK_8723B					0xBB
#define	EEPROM_2G_5G_PA_TYPE_8723B			0xBC
#define	EEPROM_2G_LNA_TYPE_GAIN_SEL_8723B	0xBD
#define	EEPROM_5G_LNA_TYPE_GAIN_SEL_8723B	0xBF

#define	EEPROM_RF_BOARD_OPTION_8723B		0xC1
#define	EEPROM_FEATURE_OPTION_8723B			0xC2
#define	EEPROM_RF_BT_SETTING_8723B			0xC3
#define	EEPROM_VERSION_8723B					0xC4
#define	EEPROM_CustomID_8723B				0xC5
#define	EEPROM_TX_BBSWING_2G_8723B			0xC6
#define	EEPROM_TX_PWR_CALIBRATE_RATE_8723B	0xC8
#define	EEPROM_RF_ANTENNA_OPT_8723B		0xC9
#define	EEPROM_RFE_OPTION_8723B				0xCA

//RTL8723BE
#define	EEPROM_MAC_ADDR_8723BE				0xD0
#define	EEPROM_VID_8723BE						0xD6
#define	EEPROM_DID_8723BE						0xD8
#define	EEPROM_SVID_8723BE						0xDA
#define	EEPROM_SMID_8723BE						0xDC

//RTL8723BU
#define	EEPROM_MAC_ADDR_8723BU				0xD7
#define	EEPROM_VID_8723BU						0xD0
#define	EEPROM_PID_8723BU						0xD2
#define   EEPROM_PA_TYPE_8723BU                               0xBC
#define   EEPROM_LNA_TYPE_2G_8723BU                               0xBD

//RTL8723BS
#define	EEPROM_MAC_ADDR_8723BS				0x11A

//RTL8723BS
#define	EEPROM_PACKAGE_METHOD_8723BS		0x1FB

//====================================================
//			EEPROM/Efuse Value Type
//====================================================
#define	EETYPE_TX_PWR_8723B								0x0
#define	EEPROM_Default_ThermalMeter_8723B		0x18
#define	EEPROM_Default_CrystalCap_8723B		0x20

//
// Default Value for EEPROM or EFUSE!!!
//
#define EEPROM_Default_TSSI					0x0
#define EEPROM_Default_TxPowerDiff			0x0
#define EEPROM_Default_CrystalCap			0x5
#define EEPROM_Default_BoardType			0x02 // Default: 2X2, RTL8192CE(QFPN68)
#define EEPROM_Default_TxPower				0x1010
#define EEPROM_Default_HT2T_TxPwr			0x10

#define EEPROM_Default_LegacyHTTxPowerDiff	0x3
#define EEPROM_Default_ThermalMeter			0x12

#define EEPROM_Default_AntTxPowerDiff		0x0
#define EEPROM_Default_TxPwDiff_CrystalCap	0x5
#define EEPROM_Default_TxPowerLevel			0x25

#define EEPROM_Default_HT40_2SDiff			0x0
#define EEPROM_Default_HT20_Diff			2	// HT20<->40 default Tx Power Index Difference
#define EEPROM_Default_LegacyHTTxPowerDiff	0x3
#define EEPROM_Default_HT40_PwrMaxOffset	0
#define EEPROM_Default_HT20_PwrMaxOffset	0

#define EEPROM_Default_CrystalCap_88E 		0x20
#define	EEPROM_Default_ThermalMeter_88E		0x18

#ifdef CONFIG_RF_GAIN_OFFSET
#define EEPROM_Default_RFGainOffset			0xff
#endif //CONFIG_RF_GAIN_OFFSET

//New EFUSE deafult value
#define 	EEPROM_DEFAULT_24G_INDEX		0x2D
#define 	EEPROM_DEFAULT_24G_HT20_DIFF	0X02
#define 	EEPROM_DEFAULT_24G_OFDM_DIFF	0X04

#define 	EEPROM_DEFAULT_5G_INDEX			0X2A
#define 	EEPROM_DEFAULT_5G_HT20_DIFF		0X00
#define 	EEPROM_DEFAULT_5G_OFDM_DIFF		0X04

#define 	EEPROM_DEFAULT_DIFF				0XFE
#define	EEPROM_DEFAULT_CHANNEL_PLAN	0x7F
#define	EEPROM_DEFAULT_BOARD_OPTION	0x00
#define	EEPROM_DEFAULT_FEATURE_OPTION	0x00
#define	EEPROM_DEFAULT_BT_OPTION		0x10


// For debug
#define EEPROM_Default_PID					0x1234
#define EEPROM_Default_VID					0x5678
#define EEPROM_Default_CustomerID			0xAB
#define	EEPROM_Default_CustomerID_8188E		0x00
#define EEPROM_Default_SubCustomerID		0xCD
#define EEPROM_Default_Version				0

#define EEPROM_CHANNEL_PLAN_FCC				0x0
#define EEPROM_CHANNEL_PLAN_IC				0x1
#define EEPROM_CHANNEL_PLAN_ETSI				0x2
#define EEPROM_CHANNEL_PLAN_SPAIN			0x3
#define EEPROM_CHANNEL_PLAN_FRANCE			0x4
#define EEPROM_CHANNEL_PLAN_MKK				0x5
#define EEPROM_CHANNEL_PLAN_MKK1				0x6
#define EEPROM_CHANNEL_PLAN_ISRAEL			0x7
#define EEPROM_CHANNEL_PLAN_TELEC			0x8
#define EEPROM_CHANNEL_PLAN_GLOBAL_DOMAIN	0x9
#define EEPROM_CHANNEL_PLAN_WORLD_WIDE_13	0xA
#define EEPROM_CHANNEL_PLAN_NCC				0xB
#define EEPROM_USB_OPTIONAL1					0xE
#define EEPROM_CHANNEL_PLAN_BY_HW_MASK		0x80

//3b'111: default (8723BS default is TFBGA-80 for SPRD)
//3b'110: for 8723BS QFN68
//3b'101: for 8723BS TFBGA-90
//3b'100: for 8723BS TFBGA-79
#define EEPROM_PACKAGE_METHOD_DEFAULT		0x07
#define EEPROM_PACKAGE_METHOD_QFN68			0x06
#define EEPROM_PACKAGE_METHOD_TFBGA90		0x05
#define EEPROM_PACKAGE_METHOD_TFBGA79		0x04

#define EEPROM_CID_DEFAULT				0x0
#define EEPROM_CID_TOSHIBA					0x4
#define EEPROM_CID_CCX						0x10 // CCX test. By Bruce, 2009-02-25.
#define EEPROM_CID_QMI						0x0D
#define EEPROM_CID_WHQL 					0xFE // added by chiyoko for dtm, 20090108

#define	RTL_EEPROM_ID						0x8129

//----------------------------------------------------------------------------
//       EEPROM/EFUSE data structure definition.
//----------------------------------------------------------------------------
#define	MAX_RF_PATH		4
#define	MAX_CHNL_GROUP_24G		6
#define 	MAX_TX_COUNT				4
#define 	HWSET_MAX_SIZE				512

typedef struct _TxPowerInfo24G{
	u1Byte IndexCCK_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
	u1Byte IndexBW40_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G-1];
	//If only one tx, only BW20 and OFDM are used.
	s1Byte CCK_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s1Byte OFDM_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s1Byte BW20_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s1Byte BW40_Diff[MAX_RF_PATH][MAX_TX_COUNT];
}TxPowerInfo24G, *PTxPowerInfo24G;


typedef	enum _BT_Ant_NUM
{
	Ant_x2	= 0,
	Ant_x1	= 1
} BT_Ant_NUM, *PBT_Ant_NUM;

#if 0
typedef	enum _BT_CoType
{
	BT_2Wire		= 0,
	BT_ISSC_3Wire	= 1,
	BT_Accel		= 2,
	BT_CSR_BC4		= 3,
	BT_CSR_BC8		= 4,
	BT_RTL8756		= 5,
	BT_RTL8723A	= 6,
	BT_RTL8723B 	= 7,
} BT_CoType, *PBT_CoType;
#endif

typedef	enum _BT_BOARD_TYPE{
	BT_2WIRE		= 0,
	BT_ISSC_3WIRE	= 1,
	BT_ACCEL		= 2,
	BT_CSR_BC4		= 3,
	BT_CSR_BC8		= 4,
	BT_RTL8756		= 5,
	BT_RTL8723A		= 6,
	BT_RTL8821		= 7,
	BT_RTL8723B		= 8,
	BT_RTL8192E		= 9,
	BT_RTL8813A		= 10,
	BT_RTL8812A		= 11
} BT_BOARD_TYPE, *PBT_BOARD_TYPE;

typedef	enum _BT_RadioShared
{
	BT_Radio_Shared 	= 0,
	BT_Radio_Individual	= 1,
} BT_RadioShared, *PBT_RadioShared;
#endif

