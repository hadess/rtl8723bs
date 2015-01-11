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
 
#ifndef	__ODMDIG_H__
#define    __ODMDIG_H__

typedef struct _Dynamic_Initial_Gain_Threshold_
{
	bool		bStopDIG;
	bool		bPSDInProgress;

	u1Byte		Dig_Enable_Flag;
	u1Byte		Dig_Ext_Port_Stage;
	
	int		   	RssiLowThresh;
	int		    	RssiHighThresh;

	u4Byte		FALowThresh;
	u4Byte		FAHighThresh;

	u1Byte		CurSTAConnectState;
	u1Byte		PreSTAConnectState;
	u1Byte		CurMultiSTAConnectState;

	u1Byte		PreIGValue;
	u1Byte		CurIGValue;
	u1Byte		BackupIGValue;		//MP DIG
	u1Byte		BT30_CurIGI;
	u1Byte		IGIBackup;

	s8		BackoffVal;
	s8		BackoffVal_range_max;
	s8		BackoffVal_range_min;
	u1Byte		rx_gain_range_max;
	u1Byte		rx_gain_range_min;
	u1Byte		Rssi_val_min;

	u1Byte		PreCCK_CCAThres;
	u1Byte		CurCCK_CCAThres;
	u1Byte		PreCCKPDState;
	u1Byte		CurCCKPDState;
	u1Byte		CCKPDBackup;

	u1Byte		LargeFAHit;
	u1Byte		ForbiddenIGI;
	u4Byte		Recover_cnt;

	u1Byte		DIG_Dynamic_MIN_0;
	u1Byte		DIG_Dynamic_MIN_1;
	bool		bMediaConnect_0;
	bool		bMediaConnect_1;

	u4Byte		AntDiv_RSSI_max;
	u4Byte		RSSI_max;

	u1Byte		*pbP2pLinkInProgress;
}DIG_T,*pDIG_T;

typedef struct false_ALARM_STATISTICS{
	u4Byte	Cnt_Parity_Fail;
	u4Byte	Cnt_Rate_Illegal;
	u4Byte	Cnt_Crc8_fail;
	u4Byte	Cnt_Mcs_fail;
	u4Byte	Cnt_Ofdm_fail;
	u4Byte	Cnt_Ofdm_fail_pre;	//For RTL8881A
	u4Byte	Cnt_Cck_fail;
	u4Byte	Cnt_all;
	u4Byte	Cnt_Fast_Fsync;
	u4Byte	Cnt_SB_Search_fail;
	u4Byte	Cnt_OFDM_CCA;
	u4Byte	Cnt_CCK_CCA;
	u4Byte	Cnt_CCA_all;
	u4Byte	Cnt_BW_USC;	//Gary
	u4Byte	Cnt_BW_LSC;	//Gary
}false_ALARM_STATISTICS, *Pfalse_ALARM_STATISTICS;

typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE 		= 5,
	DIG_TYPE_DISABLE 		= 6,	
	DIG_OP_TYPE_MAX
}DM_DIG_OP_E;

typedef enum tag_ODM_PauseDIG_Type {
	ODM_PAUSE_DIG    		= 	BIT0,
	ODM_RESUME_DIG  		= 	BIT1
} ODM_Pause_DIG_TYPE;

typedef enum tag_ODM_PauseCCKPD_Type {
	ODM_PAUSE_CCKPD    	= 	BIT0,
	ODM_RESUME_CCKPD  	= 	BIT1
} ODM_Pause_CCKPD_TYPE;

/*
typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
	CCK_PD_STAGE_LowRssi = 0,
	CCK_PD_STAGE_HighRssi = 1,
	CCK_PD_STAGE_MAX = 3,
}DM_CCK_PDTH_E;

typedef enum tag_DIG_EXT_PORT_ALGO_Definition
{
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
}DM_DIG_EXT_PORT_ALG_E;

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,	
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


#define DM_MultiSTA_InitGainChangeNotify(Event) {DM_DigTable.CurMultiSTAConnectState = Event;}

#define DM_MultiSTA_InitGainChangeNotify_CONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_CONNECT)

#define DM_MultiSTA_InitGainChangeNotify_DISCONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_DISCONNECT)
*/
#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DMfalseALARM_THRESH_LOW	400
#define		DMfalseALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX_NIC				0x3e
#define		DM_DIG_MIN_NIC				0x1e //0x22//0x1c
#define		DM_DIG_MAX_OF_MIN_NIC		0x3e

#define		DM_DIG_MAX_AP					0x3e
#define		DM_DIG_MIN_AP					0x1c
#define		DM_DIG_MAX_OF_MIN			0x2A	//0x32
#define		DM_DIG_MIN_AP_DFS				0x20

#define		DM_DIG_MAX_NIC_HP			0x46
#define		DM_DIG_MIN_NIC_HP				0x2e

#define		DM_DIG_MAX_AP_HP				0x42
#define		DM_DIG_MIN_AP_HP				0x30

#define		DM_DIG_FA_TH0				0x200//0x20

#define		DM_DIG_FA_TH1					0x300
#define		DM_DIG_FA_TH2					0x400
//this is for 92d
#define		DM_DIG_FA_TH0_92D				0x100
#define		DM_DIG_FA_TH1_92D				0x400
#define		DM_DIG_FA_TH2_92D				0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

#define 		DM_DIG_FA_TH0_LPS				4 //-> 4 in lps
#define 		DM_DIG_FA_TH1_LPS				15 //-> 15 lps
#define 		DM_DIG_FA_TH2_LPS				30 //-> 30 lps
#define 		RSSI_OFFSET_DIG				0x05

void
ODM_ChangeDynamicInitGainThresh(
	IN		void *					pDM_VOID,
	IN		u4Byte  					DM_Type,
	IN		u4Byte 					DM_Value
	);

void
odm_NHMCounterStatisticsInit(
	IN		void *					pDM_VOID
	);

void
odm_NHMCounterStatistics(
	IN		void *					pDM_VOID
	);

void
odm_NHMBBInit(
	IN		void *					pDM_VOID
);

void
odm_NHMBB(
	IN		void *					pDM_VOID
);

void
odm_NHMCounterStatisticsReset(
	IN		void *			pDM_VOID
);

void
odm_GetNHMCounterStatistics(
	IN		void *			pDM_VOID
);

void
odm_SearchPwdBLowerBound(
	IN		void *					pDM_VOID,
	IN		u1Byte					IGI_target
);

void
odm_AdaptivityInit(
	IN		void *					pDM_VOID
	);

void
odm_Adaptivity(
	IN		void *					pDM_VOID,
	IN		u1Byte					IGI
	);

void
ODM_Write_DIG(
	IN		void *					pDM_VOID, 	
	IN		u1Byte					CurrentIGI
	);

void
odm_PauseDIG(
	IN		void *					pDM_VOID,
	IN		ODM_Pause_DIG_TYPE		PauseType,
	IN		u1Byte					IGIValue
	);

void
odm_DIGInit(
	IN		void *					pDM_VOID
	);

void
odm_DIG(
	IN		void *					pDM_VOID
	);

void
odm_DIGbyRSSI_LPS(
	IN		void *					pDM_VOID
	);

void
odm_DigForBtHsMode(
	IN		void *					pDM_VOID
	);

void
odm_FalseAlarmCounterStatistics(
	IN		void *					pDM_VOID
	);

void
odm_FAThresholdCheck(
	IN		void *					pDM_VOID,
	IN		bool					bDFSBand,
	IN		bool					bPerformance,
	IN		u4Byte					RxTp,
	IN		u4Byte					TxTp,
	OUT		u4Byte*					dm_FA_thres
	);

u1Byte
odm_ForbiddenIGICheck(
	IN		void *					pDM_VOID,
	IN		u1Byte					DIG_Dynamic_MIN,
	IN		u1Byte					CurrentIGI
	);

void
odm_InbandNoiseCalculate (	
	IN		void *					pDM_VOID
	);

bool 
odm_DigAbort(
	IN		void *					pDM_VOID
	);

void
odm_PauseCCKPacketDetection(
	IN		void *					pDM_VOID,
	IN		ODM_Pause_CCKPD_TYPE	PauseType,
	IN		u1Byte					CCKPDThreshold
	);

void
odm_CCKPacketDetectionThresh(
	IN		void *					pDM_VOID
	);

void
ODM_Write_CCK_CCA_Thres(
	IN		void *					pDM_VOID, 
	IN		u1Byte					CurCCK_CCAThres
	);

#endif
