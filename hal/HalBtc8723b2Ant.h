//===========================================
// The following is for 8723B 2Ant BT Co-exist definition
//===========================================
#define	BT_INFO_8723B_2ANT_B_FTP						BIT7
#define	BT_INFO_8723B_2ANT_B_A2DP					BIT6
#define	BT_INFO_8723B_2ANT_B_HID						BIT5
#define	BT_INFO_8723B_2ANT_B_SCO_BUSY				BIT4
#define	BT_INFO_8723B_2ANT_B_ACL_BUSY				BIT3
#define	BT_INFO_8723B_2ANT_B_INQ_PAGE				BIT2
#define	BT_INFO_8723B_2ANT_B_SCO_ESCO				BIT1
#define	BT_INFO_8723B_2ANT_B_CONNECTION				BIT0

#define		BTC_RSSI_COEX_THRESH_TOL_8723B_2ANT		2

typedef enum _BT_INFO_SRC_8723B_2ANT{
	BT_INFO_SRC_8723B_2ANT_WIFI_FW			= 0x0,
	BT_INFO_SRC_8723B_2ANT_BT_RSP				= 0x1,
	BT_INFO_SRC_8723B_2ANT_BT_ACTIVE_SEND		= 0x2,
	BT_INFO_SRC_8723B_2ANT_MAX
}BT_INFO_SRC_8723B_2ANT,*PBT_INFO_SRC_8723B_2ANT;

typedef enum _BT_8723B_2ANT_BT_STATUS{
	BT_8723B_2ANT_BT_STATUS_NON_CONNECTED_IDLE	= 0x0,
	BT_8723B_2ANT_BT_STATUS_CONNECTED_IDLE		= 0x1,
	BT_8723B_2ANT_BT_STATUS_INQ_PAGE				= 0x2,
	BT_8723B_2ANT_BT_STATUS_ACL_BUSY				= 0x3,
	BT_8723B_2ANT_BT_STATUS_SCO_BUSY				= 0x4,
	BT_8723B_2ANT_BT_STATUS_ACL_SCO_BUSY			= 0x5,
	BT_8723B_2ANT_BT_STATUS_MAX
}BT_8723B_2ANT_BT_STATUS,*PBT_8723B_2ANT_BT_STATUS;

typedef enum _BT_8723B_2ANT_COEX_ALGO{
	BT_8723B_2ANT_COEX_ALGO_UNDEFINED			= 0x0,
	BT_8723B_2ANT_COEX_ALGO_SCO				= 0x1,
	BT_8723B_2ANT_COEX_ALGO_HID				= 0x2,
	BT_8723B_2ANT_COEX_ALGO_A2DP				= 0x3,
	BT_8723B_2ANT_COEX_ALGO_A2DP_PANHS		= 0x4,
	BT_8723B_2ANT_COEX_ALGO_PANEDR			= 0x5,
	BT_8723B_2ANT_COEX_ALGO_PANHS			= 0x6,
	BT_8723B_2ANT_COEX_ALGO_PANEDR_A2DP		= 0x7,
	BT_8723B_2ANT_COEX_ALGO_PANEDR_HID		= 0x8,
	BT_8723B_2ANT_COEX_ALGO_HID_A2DP_PANEDR	= 0x9,
	BT_8723B_2ANT_COEX_ALGO_HID_A2DP			= 0xa,
	BT_8723B_2ANT_COEX_ALGO_MAX				= 0xb,
}BT_8723B_2ANT_COEX_ALGO,*PBT_8723B_2ANT_COEX_ALGO;

typedef struct _COEX_DM_8723B_2ANT{
	// fw mechanism
	u1Byte		preBtDecPwrLvl;
	u1Byte		curBtDecPwrLvl;
	u1Byte		preFwDacSwingLvl;
	u1Byte		curFwDacSwingLvl;
	bool		bCurIgnoreWlanAct;
	bool		bPreIgnoreWlanAct;
	u1Byte		prePsTdma;
	u1Byte		curPsTdma;
	u1Byte		psTdmaPara[5];
	u1Byte		psTdmaDuAdjType;
	bool		bResetTdmaAdjust;
	bool		bAutoTdmaAdjust;
	bool		bPrePsTdmaOn;
	bool		bCurPsTdmaOn;
	bool		bPreBtAutoReport;
	bool		bCurBtAutoReport;

	// sw mechanism
	bool		bPreRfRxLpfShrink;
	bool		bCurRfRxLpfShrink;
	u4Byte		btRf0x1eBackup;
	bool 	bPreLowPenaltyRa;
	bool		bCurLowPenaltyRa;
	bool		bPreDacSwingOn;
	u4Byte		preDacSwingLvl;
	bool		bCurDacSwingOn;
	u4Byte		curDacSwingLvl;
	bool		bPreAdcBackOff;
	bool		bCurAdcBackOff;
	bool 	bPreAgcTableEn;
	bool		bCurAgcTableEn;
	u4Byte		preVal0x6c0;
	u4Byte		curVal0x6c0;
	u4Byte		preVal0x6c4;
	u4Byte		curVal0x6c4;
	u4Byte		preVal0x6c8;
	u4Byte		curVal0x6c8;
	u1Byte		preVal0x6cc;
	u1Byte		curVal0x6cc;
	bool		bLimitedDig;

	// algorithm related
	u1Byte		preAlgorithm;
	u1Byte		curAlgorithm;
	u1Byte		btStatus;
	u1Byte		wifiChnlInfo[3];

	bool		bNeedRecover0x948;
	u4Byte		backup0x948;
} COEX_DM_8723B_2ANT, *PCOEX_DM_8723B_2ANT;

typedef struct _COEX_STA_8723B_2ANT{	
	bool					bBtLinkExist;
	bool					bScoExist;
	bool					bA2dpExist;
	bool					bHidExist;
	bool					bPanExist;

	bool					bUnderLps;
	bool					bUnderIps;
	u4Byte					highPriorityTx;
	u4Byte					highPriorityRx;
	u4Byte					lowPriorityTx;
	u4Byte					lowPriorityRx;
	u1Byte					btRssi;
	bool				bBtTxRxMask;
	u1Byte					preBtRssiState;
	u1Byte					preWifiRssiState[4];
	bool					bC2hBtInfoReqSent;
	u1Byte					btInfoC2h[BT_INFO_SRC_8723B_2ANT_MAX][10];
	u4Byte					btInfoC2hCnt[BT_INFO_SRC_8723B_2ANT_MAX];
	bool					bC2hBtInquiryPage;
	u1Byte					btRetryCnt;
	u1Byte					btInfoExt;
}COEX_STA_8723B_2ANT, *PCOEX_STA_8723B_2ANT;

//===========================================
// The following is interface which will notify coex module.
//===========================================
void
EXhalbtc8723b2ant_PowerOnSetting(
	IN	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtc8723b2ant_InitHwConfig(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	bool				bWifiOnly
	);
void
EXhalbtc8723b2ant_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtc8723b2ant_IpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
void
EXhalbtc8723b2ant_LpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
void
EXhalbtc8723b2ant_ScanNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
void
EXhalbtc8723b2ant_ConnectNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
void
EXhalbtc8723b2ant_MediaStatusNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
	);
void
EXhalbtc8723b2ant_SpecialPacketNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
	);
void
EXhalbtc8723b2ant_BtInfoNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	pu1Byte			tmpBuf,
	IN	u1Byte			length
	);
void
EXhalbtc8723b2ant_HaltNotify(
	IN	PBTC_COEXIST			pBtCoexist
	);
void
EXhalbtc8723b2ant_PnpNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				pnpState
	);
void
EXhalbtc8723b2ant_Periodical(
	IN	PBTC_COEXIST			pBtCoexist
	);
void
EXhalbtc8723b2ant_DisplayCoexInfo(
	IN	PBTC_COEXIST		pBtCoexist
	);

