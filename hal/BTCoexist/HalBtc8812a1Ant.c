//============================================================
// Description:
//
// This file is for 8812a1ant Co-exist mechanism
//
// History
// 2012/11/15 Cosa first check in.
//
//============================================================

//============================================================
// include files
//============================================================
#include "Mp_Precomp.h"
#if(BT_30_SUPPORT == 1)
//============================================================
// Global variables, these are static variables
//============================================================
static COEX_DM_8812A_1ANT		GLCoexDm8812a1Ant;
static PCOEX_DM_8812A_1ANT 	pCoexDm=&GLCoexDm8812a1Ant;
static COEX_STA_8812A_1ANT		GLCoexSta8812a1Ant;
static PCOEX_STA_8812A_1ANT	pCoexSta=&GLCoexSta8812a1Ant;

const char *const GLBtInfoSrc8812a1Ant[]={
	"BT Info[wifi fw]",
	"BT Info[bt rsp]",
	"BT Info[bt auto report]",
};

//============================================================
// local function proto type if needed
//============================================================
//============================================================
// local function start with halbtc8812a1ant_
//============================================================
#if 0
VOID
halbtc8812a1ant_Reg0x550Bit3(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bSet
	)
{
	u1Byte	u1tmp=0;

	u1tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x550);
	if(bSet)
	{
		u1tmp |= BIT3;
	}
	else
	{
		u1tmp &= ~BIT3;
	}
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x550, u1tmp);
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], set 0x550[3]=%d\n", (bSet? 1:0)));
}
#endif
u1Byte
halbtc8812a1ant_BtRssiState(
	u1Byte			levelNum,
	u1Byte			rssiThresh,
	u1Byte			rssiThresh1
	)
{
	s4Byte			btRssi=0;
	u1Byte			btRssiState;

	btRssi = pCoexSta->btRssi;

	if(levelNum == 2)
	{
		if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_LOW) ||
			(pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_LOW))
		{
			if(btRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				btRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to High\n"));
			}
			else
			{
				btRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Low\n"));
			}
		}
		else
		{
			if(btRssi < rssiThresh)
			{
				btRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Low\n"));
			}
			else
			{
				btRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at High\n"));
			}
		}
	}
	else if(levelNum == 3)
	{
		if(rssiThresh > rssiThresh1)
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi thresh error!!\n"));
			return pCoexSta->preBtRssiState;
		}

		if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_LOW) ||
			(pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_LOW))
		{
			if(btRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				btRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Medium\n"));
			}
			else
			{
				btRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Low\n"));
			}
		}
		else if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_MEDIUM) ||
			(pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_MEDIUM))
		{
			if(btRssi >= (rssiThresh1+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				btRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to High\n"));
			}
			else if(btRssi < rssiThresh)
			{
				btRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Low\n"));
			}
			else
			{
				btRssiState = BTC_RSSI_STATE_STAY_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Medium\n"));
			}
		}
		else
		{
			if(btRssi < rssiThresh1)
			{
				btRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Medium\n"));
			}
			else
			{
				btRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at High\n"));
			}
		}
	}

	pCoexSta->preBtRssiState = btRssiState;

	return btRssiState;
}

u1Byte
halbtc8812a1ant_WifiRssiState(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			index,
	IN	u1Byte			levelNum,
	IN	u1Byte			rssiThresh,
	IN	u1Byte			rssiThresh1
	)
{
	s4Byte			wifiRssi=0;
	u1Byte			wifiRssiState;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_S4_WIFI_RSSI, &wifiRssi);

	if(levelNum == 2)
	{
		if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_LOW) ||
			(pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_LOW))
		{
			if(wifiRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				wifiRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to High\n"));
			}
			else
			{
				wifiRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Low\n"));
			}
		}
		else
		{
			if(wifiRssi < rssiThresh)
			{
				wifiRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Low\n"));
			}
			else
			{
				wifiRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at High\n"));
			}
		}
	}
	else if(levelNum == 3)
	{
		if(rssiThresh > rssiThresh1)
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI thresh error!!\n"));
			return pCoexSta->preWifiRssiState[index];
		}

		if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_LOW) ||
			(pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_LOW))
		{
			if(wifiRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				wifiRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Medium\n"));
			}
			else
			{
				wifiRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Low\n"));
			}
		}
		else if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_MEDIUM) ||
			(pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_MEDIUM))
		{
			if(wifiRssi >= (rssiThresh1+BTC_RSSI_COEX_THRESH_TOL_8812A_1ANT))
			{
				wifiRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to High\n"));
			}
			else if(wifiRssi < rssiThresh)
			{
				wifiRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Low\n"));
			}
			else
			{
				wifiRssiState = BTC_RSSI_STATE_STAY_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Medium\n"));
			}
		}
		else
		{
			if(wifiRssi < rssiThresh1)
			{
				wifiRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Medium\n"));
			}
			else
			{
				wifiRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at High\n"));
			}
		}
	}

	pCoexSta->preWifiRssiState[index] = wifiRssiState;

	return wifiRssiState;
}

VOID
halbtc8812a1ant_MonitorBtEnableDisable(
	IN 	PBTC_COEXIST		pBtCoexist
	)
{
	static BOOLEAN	bPreBtDisabled=FALSE;
	static u4Byte		btDisableCnt=0;
	BOOLEAN			bBtActive=TRUE, bBtDisableBy68=FALSE, bBtDisabled=FALSE;
	u4Byte			u4Tmp=0;

	// This function check if bt is disabled

	if(	pCoexSta->highPriorityTx == 0 &&
		pCoexSta->highPriorityRx == 0 &&
		pCoexSta->lowPriorityTx == 0 &&
		pCoexSta->lowPriorityRx == 0)
	{
		bBtActive = FALSE;
	}
	if(	pCoexSta->highPriorityTx == 0xffff &&
		pCoexSta->highPriorityRx == 0xffff &&
		pCoexSta->lowPriorityTx == 0xffff &&
		pCoexSta->lowPriorityRx == 0xffff)
	{
		bBtActive = FALSE;
	}
	if(bBtActive)
	{
		btDisableCnt = 0;
		bBtDisabled = FALSE;
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_DISABLE, &bBtDisabled);
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is enabled !!\n"));
	}
	else
	{
		btDisableCnt++;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], bt all counters=0, %d times!!\n",
				btDisableCnt));
		if(btDisableCnt >= 2 ||bBtDisableBy68)
		{
			bBtDisabled = TRUE;
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_DISABLE, &bBtDisabled);
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is disabled !!\n"));
		}
	}
	if(bPreBtDisabled != bBtDisabled)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is from %s to %s!!\n",
			(bPreBtDisabled ? "disabled":"enabled"),
			(bBtDisabled ? "disabled":"enabled")));
		bPreBtDisabled = bBtDisabled;
		if(!bBtDisabled)
		{
		}
		else
		{
		}
	}
}

VOID
halbtc8812a1ant_MonitorBtCtr(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u4Byte 			regHPTxRx, regLPTxRx, u4Tmp;
	u4Byte			regHPTx=0, regHPRx=0, regLPTx=0, regLPRx=0;
	u1Byte			u1Tmp;

	regHPTxRx = 0x770;
	regLPTxRx = 0x774;

	u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, regHPTxRx);
	regHPTx = u4Tmp & bMaskLWord;
	regHPRx = (u4Tmp & bMaskHWord)>>16;

	u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, regLPTxRx);
	regLPTx = u4Tmp & bMaskLWord;
	regLPRx = (u4Tmp & bMaskHWord)>>16;

	pCoexSta->highPriorityTx = regHPTx;
	pCoexSta->highPriorityRx = regHPRx;
	pCoexSta->lowPriorityTx = regLPTx;
	pCoexSta->lowPriorityRx = regLPRx;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], High Priority Tx/Rx (reg 0x%x)=%x(%d)/%x(%d)\n",
		regHPTxRx, regHPTx, regHPTx, regHPRx, regHPRx));
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], Low Priority Tx/Rx (reg 0x%x)=%x(%d)/%x(%d)\n",
		regLPTxRx, regLPTx, regLPTx, regLPRx, regLPRx));

	// reset counter
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x76e, 0xc);
}

VOID
halbtc8812a1ant_QueryBtInfo(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte	dataLen=3;
	u1Byte	buf[5] = {0};
	static	u4Byte	btInfoCnt=0;

	if(!btInfoCnt ||
		(pCoexSta->btInfoC2hCnt[BT_INFO_SRC_8812A_1ANT_BT_RSP]-btInfoCnt)>2)
	{
		buf[0] = dataLen;
		buf[1] = 0x1;	// polling enable, 1=enable, 0=disable
		buf[2] = 0x2;	// polling time in seconds
		buf[3] = 0x1;	// auto report enable, 1=enable, 0=disable

		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_INFO, (PVOID)&buf[0]);
	}
	btInfoCnt = pCoexSta->btInfoC2hCnt[BT_INFO_SRC_8812A_1ANT_BT_RSP];
}
u1Byte
halbtc8812a1ant_ActionAlgorithm(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN				bBtHsOn=FALSE;
	u1Byte				algorithm=BT_8812A_1ANT_COEX_ALGO_UNDEFINED;
	u1Byte				numOfDiffProfile=0;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);

	if(!pStackInfo->bBtLinkExist)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], No profile exists!!!\n"));
		return algorithm;
	}

	if(pStackInfo->bScoExist)
		numOfDiffProfile++;
	if(pStackInfo->bHidExist)
		numOfDiffProfile++;
	if(pStackInfo->bPanExist)
		numOfDiffProfile++;
	if(pStackInfo->bA2dpExist)
		numOfDiffProfile++;

	if(numOfDiffProfile == 1)
	{
		if(pStackInfo->bScoExist)
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO only\n"));
			algorithm = BT_8812A_1ANT_COEX_ALGO_SCO;
		}
		else
		{
			if(pStackInfo->bHidExist)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID only\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_HID;
			}
			else if(pStackInfo->bA2dpExist)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP only\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_A2DP;
			}
			else if(pStackInfo->bPanExist)
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], PAN(HS) only\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANHS;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], PAN(EDR) only\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR;
				}
			}
		}
	}
	else if(numOfDiffProfile == 2)
	{
		if(pStackInfo->bScoExist)
		{
			if(pStackInfo->bHidExist)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_HID;
			}
			else if(pStackInfo->bA2dpExist)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP ==> SCO\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_SCO;
			}
			else if(pStackInfo->bPanExist)
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_SCO;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + PAN(EDR)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_HID;
				}
			}
		}
		else
		{
			if( pStackInfo->bHidExist &&
				pStackInfo->bA2dpExist )
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_HID_A2DP;
			}
			else if( pStackInfo->bHidExist &&
				pStackInfo->bPanExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_HID_A2DP;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + PAN(EDR)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_HID;
				}
			}
			else if( pStackInfo->bPanExist &&
				pStackInfo->bA2dpExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_A2DP_PANHS;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP + PAN(EDR)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_A2DP;
				}
			}
		}
	}
	else if(numOfDiffProfile == 3)
	{
		if(pStackInfo->bScoExist)
		{
			if( pStackInfo->bHidExist &&
				pStackInfo->bA2dpExist )
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + A2DP ==> HID\n"));
				algorithm = BT_8812A_1ANT_COEX_ALGO_HID;
			}
			else if( pStackInfo->bHidExist &&
				pStackInfo->bPanExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_HID_A2DP;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + PAN(EDR)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_HID;
				}
			}
			else if( pStackInfo->bPanExist &&
				pStackInfo->bA2dpExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_SCO;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP + PAN(EDR) ==> HID\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_HID;
				}
			}
		}
		else
		{
			if( pStackInfo->bHidExist &&
				pStackInfo->bPanExist &&
				pStackInfo->bA2dpExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP + PAN(HS)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_HID_A2DP;
				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP + PAN(EDR)\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_HID_A2DP_PANEDR;
				}
			}
		}
	}
	else if(numOfDiffProfile >= 3)
	{
		if(pStackInfo->bScoExist)
		{
			if( pStackInfo->bHidExist &&
				pStackInfo->bPanExist &&
				pStackInfo->bA2dpExist )
			{
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Error!!! SCO + HID + A2DP + PAN(HS)\n"));

				}
				else
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + A2DP + PAN(EDR)==>PAN(EDR)+HID\n"));
					algorithm = BT_8812A_1ANT_COEX_ALGO_PANEDR_HID;
				}
			}
		}
	}

	return algorithm;
}

BOOLEAN
halbtc8812a1ant_NeedToDecBtPwr(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	BOOLEAN		bRet=FALSE;
	BOOLEAN		bBtHsOn=FALSE, bWifiConnected=FALSE;
	s4Byte		btHsRssi=0;

	if(!pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn))
		return FALSE;
	if(!pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected))
		return FALSE;
	if(!pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_S4_HS_RSSI, &btHsRssi))
		return FALSE;

	if(bWifiConnected)
	{
		if(bBtHsOn)
		{
			if(btHsRssi > 37)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], Need to decrease bt power for HS mode!!\n"));
				bRet = TRUE;
			}
		}
		else
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], Need to decrease bt power for Wifi is connected!!\n"));
			bRet = TRUE;
		}
	}

	return bRet;
}

VOID
halbtc8812a1ant_SetFwDacSwingLevel(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			dacSwingLvl
	)
{
	u1Byte			H2C_Parameter[1] ={0};

	// There are several type of dacswing
	// 0x18/ 0x10/ 0xc/ 0x8/ 0x4/ 0x6
	H2C_Parameter[0] = dacSwingLvl;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], Set Dac Swing Level=0x%x\n", dacSwingLvl));
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write 0x64=0x%x\n", H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x64, 1, H2C_Parameter);
}

VOID
halbtc8812a1ant_SetFwDecBtPwr(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bDecBtPwr
	)
{
	u1Byte	dataLen=3;
	u1Byte	buf[5] = {0};

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], decrease Bt Power : %s\n",
			(bDecBtPwr? "Yes!!":"No!!")));

	buf[0] = dataLen;
	buf[1] = 0x3;		// OP_Code
	buf[2] = 0x1;		// OP_Code_Length
	if(bDecBtPwr)
		buf[3] = 0x1;	// OP_Code_Content
	else
		buf[3] = 0x0;

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_COEX, (PVOID)&buf[0]);
}

VOID
halbtc8812a1ant_DecBtPwr(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bDecBtPwr
	)
{
	return;
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s Dec BT power = %s\n",
		(bForceExec? "force to":""), ((bDecBtPwr)? "ON":"OFF")));
	pCoexDm->bCurDecBtPwr = bDecBtPwr;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreDecBtPwr=%d, bCurDecBtPwr=%d\n",
			pCoexDm->bPreDecBtPwr, pCoexDm->bCurDecBtPwr));

		if(pCoexDm->bPreDecBtPwr == pCoexDm->bCurDecBtPwr)
			return;
	}
	halbtc8812a1ant_SetFwDecBtPwr(pBtCoexist, pCoexDm->bCurDecBtPwr);

	pCoexDm->bPreDecBtPwr = pCoexDm->bCurDecBtPwr;
}

VOID
halbtc8812a1ant_SetFwBtLnaConstrain(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bBtLnaConsOn
	)
{
	u1Byte	dataLen=3;
	u1Byte	buf[5] = {0};

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], set BT LNA Constrain: %s\n",
		(bBtLnaConsOn? "ON!!":"OFF!!")));

	buf[0] = dataLen;
	buf[1] = 0x2;		// OP_Code
	buf[2] = 0x1;		// OP_Code_Length
	if(bBtLnaConsOn)
		buf[3] = 0x1;	// OP_Code_Content
	else
		buf[3] = 0x0;

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_COEX, (PVOID)&buf[0]);
}

VOID
halbtc8812a1ant_SetBtLnaConstrain(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bBtLnaConsOn
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s BT Constrain = %s\n",
		(bForceExec? "force":""), ((bBtLnaConsOn)? "ON":"OFF")));
	pCoexDm->bCurBtLnaConstrain = bBtLnaConsOn;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreBtLnaConstrain=%d, bCurBtLnaConstrain=%d\n",
			pCoexDm->bPreBtLnaConstrain, pCoexDm->bCurBtLnaConstrain));

		if(pCoexDm->bPreBtLnaConstrain == pCoexDm->bCurBtLnaConstrain)
			return;
	}
	halbtc8812a1ant_SetFwBtLnaConstrain(pBtCoexist, pCoexDm->bCurBtLnaConstrain);

	pCoexDm->bPreBtLnaConstrain = pCoexDm->bCurBtLnaConstrain;
}

VOID
halbtc8812a1ant_SetFwBtPsdMode(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			btPsdMode
	)
{
	u1Byte	dataLen=3;
	u1Byte	buf[5] = {0};

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], set BT PSD mode=0x%x\n",
		btPsdMode));

	buf[0] = dataLen;
	buf[1] = 0x4;			// OP_Code
	buf[2] = 0x1;			// OP_Code_Length
	buf[3] = btPsdMode;	// OP_Code_Content

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_COEX, (PVOID)&buf[0]);
}


VOID
halbtc8812a1ant_SetBtPsdMode(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	u1Byte			btPsdMode
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s BT PSD mode = 0x%x\n",
		(bForceExec? "force":""), btPsdMode));
	pCoexDm->bCurBtPsdMode = btPsdMode;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreBtPsdMode=0x%x, bCurBtPsdMode=0x%x\n",
			pCoexDm->bPreBtPsdMode, pCoexDm->bCurBtPsdMode));

		if(pCoexDm->bPreBtPsdMode == pCoexDm->bCurBtPsdMode)
			return;
	}
	halbtc8812a1ant_SetFwBtPsdMode(pBtCoexist, pCoexDm->bCurBtPsdMode);

	pCoexDm->bPreBtPsdMode = pCoexDm->bCurBtPsdMode;
}


VOID
halbtc8812a1ant_SetBtAutoReport(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bEnableAutoReport
	)
{
#if 0
	u1Byte			H2C_Parameter[1] ={0};

	H2C_Parameter[0] = 0;

	if(bEnableAutoReport)
	{
		H2C_Parameter[0] |= BIT0;
	}

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], BT FW auto report : %s, FW write 0x68=0x%x\n",
		(bEnableAutoReport? "Enabled!!":"Disabled!!"), H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x68, 1, H2C_Parameter);
#else

#endif
}

VOID
halbtc8812a1ant_BtAutoReport(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bEnableAutoReport
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s BT Auto report = %s\n",
		(bForceExec? "force to":""), ((bEnableAutoReport)? "Enabled":"Disabled")));
	pCoexDm->bCurBtAutoReport = bEnableAutoReport;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreBtAutoReport=%d, bCurBtAutoReport=%d\n",
			pCoexDm->bPreBtAutoReport, pCoexDm->bCurBtAutoReport));

		if(pCoexDm->bPreBtAutoReport == pCoexDm->bCurBtAutoReport)
			return;
	}
	halbtc8812a1ant_SetBtAutoReport(pBtCoexist, pCoexDm->bCurBtAutoReport);

	pCoexDm->bPreBtAutoReport = pCoexDm->bCurBtAutoReport;
}

VOID
halbtc8812a1ant_FwDacSwingLvl(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	u1Byte			fwDacSwingLvl
	)
{
	return;
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s set FW Dac Swing level = %d\n",
		(bForceExec? "force to":""), fwDacSwingLvl));
	pCoexDm->curFwDacSwingLvl = fwDacSwingLvl;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], preFwDacSwingLvl=%d, curFwDacSwingLvl=%d\n",
			pCoexDm->preFwDacSwingLvl, pCoexDm->curFwDacSwingLvl));

		if(pCoexDm->preFwDacSwingLvl == pCoexDm->curFwDacSwingLvl)
			return;
	}

	halbtc8812a1ant_SetFwDacSwingLevel(pBtCoexist, pCoexDm->curFwDacSwingLvl);

	pCoexDm->preFwDacSwingLvl = pCoexDm->curFwDacSwingLvl;
}

VOID
halbtc8812a1ant_SetSwRfRxLpfCorner(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bRxRfShrinkOn
	)
{
	if(bRxRfShrinkOn)
	{
		//Shrink RF Rx LPF corner
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Shrink RF Rx LPF corner!!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff, 0xf0ff7);
	}
	else
	{
		//Resume RF Rx LPF corner
		// After initialized, we can use pCoexDm->btRf0x1eBackup
		if(pBtCoexist->bInitilized)
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Resume RF Rx LPF corner!!\n"));
			pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff, pCoexDm->btRf0x1eBackup);
		}
	}
}

VOID
halbtc8812a1ant_RfShrink(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bRxRfShrinkOn
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn Rx RF Shrink = %s\n",
		(bForceExec? "force to":""), ((bRxRfShrinkOn)? "ON":"OFF")));
	pCoexDm->bCurRfRxLpfShrink = bRxRfShrinkOn;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreRfRxLpfShrink=%d, bCurRfRxLpfShrink=%d\n",
			pCoexDm->bPreRfRxLpfShrink, pCoexDm->bCurRfRxLpfShrink));

		if(pCoexDm->bPreRfRxLpfShrink == pCoexDm->bCurRfRxLpfShrink)
			return;
	}
	halbtc8812a1ant_SetSwRfRxLpfCorner(pBtCoexist, pCoexDm->bCurRfRxLpfShrink);

	pCoexDm->bPreRfRxLpfShrink = pCoexDm->bCurRfRxLpfShrink;
}

VOID
halbtc8812a1ant_SetSwPenaltyTxRateAdaptive(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bLowPenaltyRa
	)
{
	u1Byte	tmpU1;

	tmpU1 = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x4fd);
	tmpU1 |= BIT0;
	if(bLowPenaltyRa)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Tx rate adaptive, set low penalty!!\n"));
		tmpU1 &= ~BIT2;
	}
	else
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Tx rate adaptive, set normal!!\n"));
		tmpU1 |= BIT2;
	}

	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x4fd, tmpU1);
}

VOID
halbtc8812a1ant_LowPenaltyRa(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bLowPenaltyRa
	)
{
	return;
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn LowPenaltyRA = %s\n",
		(bForceExec? "force to":""), ((bLowPenaltyRa)? "ON":"OFF")));
	pCoexDm->bCurLowPenaltyRa = bLowPenaltyRa;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreLowPenaltyRa=%d, bCurLowPenaltyRa=%d\n",
			pCoexDm->bPreLowPenaltyRa, pCoexDm->bCurLowPenaltyRa));

		if(pCoexDm->bPreLowPenaltyRa == pCoexDm->bCurLowPenaltyRa)
			return;
	}
	halbtc8812a1ant_SetSwPenaltyTxRateAdaptive(pBtCoexist, pCoexDm->bCurLowPenaltyRa);

	pCoexDm->bPreLowPenaltyRa = pCoexDm->bCurLowPenaltyRa;
}

VOID
halbtc8812a1ant_SetDacSwingReg(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u4Byte			level
	)
{
	u1Byte	val=(u1Byte)level;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Write SwDacSwing = 0x%x\n", level));
	pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0xc5b, 0x3e, val);
}

VOID
halbtc8812a1ant_SetSwFullTimeDacSwing(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bSwDacSwingOn,
	IN	u4Byte			swDacSwingLvl
	)
{
	if(bSwDacSwingOn)
	{
		halbtc8812a1ant_SetDacSwingReg(pBtCoexist, swDacSwingLvl);
	}
	else
	{
		halbtc8812a1ant_SetDacSwingReg(pBtCoexist, 0x18);
	}
}


VOID
halbtc8812a1ant_DacSwing(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bDacSwingOn,
	IN	u4Byte			dacSwingLvl
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn DacSwing=%s, dacSwingLvl=0x%x\n",
		(bForceExec? "force to":""), ((bDacSwingOn)? "ON":"OFF"), dacSwingLvl));
	pCoexDm->bCurDacSwingOn = bDacSwingOn;
	pCoexDm->curDacSwingLvl = dacSwingLvl;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreDacSwingOn=%d, preDacSwingLvl=0x%x, bCurDacSwingOn=%d, curDacSwingLvl=0x%x\n",
			pCoexDm->bPreDacSwingOn, pCoexDm->preDacSwingLvl,
			pCoexDm->bCurDacSwingOn, pCoexDm->curDacSwingLvl));

		if( (pCoexDm->bPreDacSwingOn == pCoexDm->bCurDacSwingOn) &&
			(pCoexDm->preDacSwingLvl == pCoexDm->curDacSwingLvl) )
			return;
	}
	delay_ms(30);
	halbtc8812a1ant_SetSwFullTimeDacSwing(pBtCoexist, bDacSwingOn, dacSwingLvl);

	pCoexDm->bPreDacSwingOn = pCoexDm->bCurDacSwingOn;
	pCoexDm->preDacSwingLvl = pCoexDm->curDacSwingLvl;
}

VOID
halbtc8812a1ant_SetAdcBackOff(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bAdcBackOff
	)
{
	if(bAdcBackOff)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB BackOff Level On!\n"));
		pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0x8db, 0x60, 0x3);
	}
	else
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB BackOff Level Off!\n"));
		pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0x8db, 0x60, 0x1);
	}
}

VOID
halbtc8812a1ant_AdcBackOff(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bAdcBackOff
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn AdcBackOff = %s\n",
		(bForceExec? "force to":""), ((bAdcBackOff)? "ON":"OFF")));
	pCoexDm->bCurAdcBackOff = bAdcBackOff;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreAdcBackOff=%d, bCurAdcBackOff=%d\n",
			pCoexDm->bPreAdcBackOff, pCoexDm->bCurAdcBackOff));

		if(pCoexDm->bPreAdcBackOff == pCoexDm->bCurAdcBackOff)
			return;
	}
	halbtc8812a1ant_SetAdcBackOff(pBtCoexist, pCoexDm->bCurAdcBackOff);

	pCoexDm->bPreAdcBackOff = pCoexDm->bCurAdcBackOff;
}

VOID
halbtc8812a1ant_SetAgcTable(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bAgcTableEn
	)
{
	u1Byte		rssiAdjustVal=0;

	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xef, 0xfffff, 0x02000);
	if(bAgcTableEn)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table On!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x3fa58);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x37a58);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x2fa58);
		rssiAdjustVal = 8;
	}
	else
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table Off!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x39258);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x31258);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x29258);
	}
	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xef, 0xfffff, 0x0);

	// set rssiAdjustVal for wifi module.
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON, &rssiAdjustVal);
}


VOID
halbtc8812a1ant_AgcTable(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bAgcTableEn
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s %s Agc Table\n",
		(bForceExec? "force to":""), ((bAgcTableEn)? "Enable":"Disable")));
	pCoexDm->bCurAgcTableEn = bAgcTableEn;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreAgcTableEn=%d, bCurAgcTableEn=%d\n",
			pCoexDm->bPreAgcTableEn, pCoexDm->bCurAgcTableEn));

		if(pCoexDm->bPreAgcTableEn == pCoexDm->bCurAgcTableEn)
			return;
	}
	halbtc8812a1ant_SetAgcTable(pBtCoexist, bAgcTableEn);

	pCoexDm->bPreAgcTableEn = pCoexDm->bCurAgcTableEn;
}

VOID
halbtc8812a1ant_SetCoexTable(
	IN	PBTC_COEXIST	pBtCoexist,
	IN	u4Byte		val0x6c0,
	IN	u4Byte		val0x6c4,
	IN	u4Byte		val0x6c8,
	IN	u1Byte		val0x6cc
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c0=0x%x\n", val0x6c0));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c0, val0x6c0);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c4=0x%x\n", val0x6c4));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c4, val0x6c4);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c8=0x%x\n", val0x6c8));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c8, val0x6c8);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6cc=0x%x\n", val0x6cc));
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x6cc, val0x6cc);
}

VOID
halbtc8812a1ant_CoexTable(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	u4Byte			val0x6c0,
	IN	u4Byte			val0x6c4,
	IN	u4Byte			val0x6c8,
	IN	u1Byte			val0x6cc
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s write Coex Table 0x6c0=0x%x, 0x6c4=0x%x, 0x6c8=0x%x, 0x6cc=0x%x\n",
		(bForceExec? "force to":""), val0x6c0, val0x6c4, val0x6c8, val0x6cc));
	pCoexDm->curVal0x6c0 = val0x6c0;
	pCoexDm->curVal0x6c4 = val0x6c4;
	pCoexDm->curVal0x6c8 = val0x6c8;
	pCoexDm->curVal0x6cc = val0x6cc;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], preVal0x6c0=0x%x, preVal0x6c4=0x%x, preVal0x6c8=0x%x, preVal0x6cc=0x%x !!\n",
			pCoexDm->preVal0x6c0, pCoexDm->preVal0x6c4, pCoexDm->preVal0x6c8, pCoexDm->preVal0x6cc));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], curVal0x6c0=0x%x, curVal0x6c4=0x%x, curVal0x6c8=0x%x, curVal0x6cc=0x%x !!\n",
			pCoexDm->curVal0x6c0, pCoexDm->curVal0x6c4, pCoexDm->curVal0x6c8, pCoexDm->curVal0x6cc));

		if( (pCoexDm->preVal0x6c0 == pCoexDm->curVal0x6c0) &&
			(pCoexDm->preVal0x6c4 == pCoexDm->curVal0x6c4) &&
			(pCoexDm->preVal0x6c8 == pCoexDm->curVal0x6c8) &&
			(pCoexDm->preVal0x6cc == pCoexDm->curVal0x6cc) )
			return;
	}
	halbtc8812a1ant_SetCoexTable(pBtCoexist, val0x6c0, val0x6c4, val0x6c8, val0x6cc);

	pCoexDm->preVal0x6c0 = pCoexDm->curVal0x6c0;
	pCoexDm->preVal0x6c4 = pCoexDm->curVal0x6c4;
	pCoexDm->preVal0x6c8 = pCoexDm->curVal0x6c8;
	pCoexDm->preVal0x6cc = pCoexDm->curVal0x6cc;
}

VOID
halbtc8812a1ant_SetFwIgnoreWlanAct(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bEnable
	)
{
	u1Byte	dataLen=3;
	u1Byte	buf[5] = {0};

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], %s BT Ignore Wlan_Act\n",
		(bEnable? "Enable":"Disable")));

	buf[0] = dataLen;
	buf[1] = 0x1;			// OP_Code
	buf[2] = 0x1;			// OP_Code_Length
	if(bEnable)
		buf[3] = 0x1; 		// OP_Code_Content
	else
		buf[3] = 0x0;

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_COEX, (PVOID)&buf[0]);
}

VOID
halbtc8812a1ant_IgnoreWlanAct(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bEnable
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s turn Ignore WlanAct %s\n",
		(bForceExec? "force to":""), (bEnable? "ON":"OFF")));
	pCoexDm->bCurIgnoreWlanAct = bEnable;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreIgnoreWlanAct = %d, bCurIgnoreWlanAct = %d!!\n",
			pCoexDm->bPreIgnoreWlanAct, pCoexDm->bCurIgnoreWlanAct));

		if(pCoexDm->bPreIgnoreWlanAct == pCoexDm->bCurIgnoreWlanAct)
			return;
	}
	halbtc8812a1ant_SetFwIgnoreWlanAct(pBtCoexist, bEnable);

	pCoexDm->bPreIgnoreWlanAct = pCoexDm->bCurIgnoreWlanAct;
}

VOID
halbtc8812a1ant_SetFwPstdma(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			byte1,
	IN	u1Byte			byte2,
	IN	u1Byte			byte3,
	IN	u1Byte			byte4,
	IN	u1Byte			byte5
	)
{
	u1Byte			H2C_Parameter[5] ={0};

	H2C_Parameter[0] = byte1;
	H2C_Parameter[1] = byte2;
	H2C_Parameter[2] = byte3;
	H2C_Parameter[3] = byte4;
	H2C_Parameter[4] = byte5;

	pCoexDm->psTdmaPara[0] = byte1;
	pCoexDm->psTdmaPara[1] = byte2;
	pCoexDm->psTdmaPara[2] = byte3;
	pCoexDm->psTdmaPara[3] = byte4;
	pCoexDm->psTdmaPara[4] = byte5;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write 0x60(5bytes)=0x%x%08x\n",
		H2C_Parameter[0],
		H2C_Parameter[1]<<24|H2C_Parameter[2]<<16|H2C_Parameter[3]<<8|H2C_Parameter[4]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x60, 5, H2C_Parameter);
}

VOID
halbtc8812a1ant_SetLpsRpwm(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			lpsVal,
	IN	u1Byte			rpwmVal
	)
{
	u1Byte	lps=lpsVal;
	u1Byte	rpwm=rpwmVal;

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_1ANT_LPS, &lps);
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_1ANT_RPWM, &rpwm);

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_INC_FORCE_EXEC_PWR_CMD_CNT, NULL);
}

VOID
halbtc8812a1ant_LpsRpwm(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	u1Byte			lpsVal,
	IN	u1Byte			rpwmVal
	)
{
	BOOLEAN	bForceExecPwrCmd=FALSE;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s set lps/rpwm=0x%x/0x%x \n",
		(bForceExec? "force to":""), lpsVal, rpwmVal));
	pCoexDm->curLps = lpsVal;
	pCoexDm->curRpwm = rpwmVal;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], preLps/curLps=0x%x/0x%x, preRpwm/curRpwm=0x%x/0x%x!!\n",
			pCoexDm->preLps, pCoexDm->curLps, pCoexDm->preRpwm, pCoexDm->curRpwm));

		if( (pCoexDm->preLps == pCoexDm->curLps) &&
			(pCoexDm->preRpwm == pCoexDm->curRpwm) )
		{
			return;
		}
	}
	halbtc8812a1ant_SetLpsRpwm(pBtCoexist, lpsVal, rpwmVal);

	pCoexDm->preLps = pCoexDm->curLps;
	pCoexDm->preRpwm = pCoexDm->curRpwm;
}

VOID
halbtc8812a1ant_SwMechanism1(
	IN	PBTC_COEXIST	pBtCoexist,
	IN	BOOLEAN		bShrinkRxLPF,
	IN	BOOLEAN 	bLowPenaltyRA,
	IN	BOOLEAN		bLimitedDIG,
	IN	BOOLEAN		bBTLNAConstrain
	)
{
	//halbtc8812a1ant_RfShrink(pBtCoexist, NORMAL_EXEC, bShrinkRxLPF);
	//halbtc8812a1ant_LowPenaltyRa(pBtCoexist, NORMAL_EXEC, bLowPenaltyRA);

	//no limited DIG
	//halbtc8812a1ant_SetBtLnaConstrain(pBtCoexist, NORMAL_EXEC, bBTLNAConstrain);
}

VOID
halbtc8812a1ant_SwMechanism2(
	IN	PBTC_COEXIST	pBtCoexist,
	IN	BOOLEAN		bAGCTableShift,
	IN	BOOLEAN 	bADCBackOff,
	IN	BOOLEAN		bSWDACSwing,
	IN	u4Byte		dacSwingLvl
	)
{
	//halbtc8812a1ant_AgcTable(pBtCoexist, NORMAL_EXEC, bAGCTableShift);
	//halbtc8812a1ant_AdcBackOff(pBtCoexist, NORMAL_EXEC, bADCBackOff);
	//halbtc8812a1ant_DacSwing(pBtCoexist, NORMAL_EXEC, bSWDACSwing, dacSwingLvl);
}

VOID
halbtc8812a1ant_PsTdma(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bForceExec,
	IN	BOOLEAN			bTurnOn,
	IN	u1Byte			type
	)
{
	BOOLEAN			bTurnOnByCnt=FALSE;
	u1Byte			psTdmaTypeByCnt=0, rssiAdjustVal=0;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s turn %s PS TDMA, type=%d\n",
		(bForceExec? "force to":""), (bTurnOn? "ON":"OFF"), type));
	pCoexDm->bCurPsTdmaOn = bTurnOn;
	pCoexDm->curPsTdma = type;

	if(!bForceExec)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPrePsTdmaOn = %d, bCurPsTdmaOn = %d!!\n",
			pCoexDm->bPrePsTdmaOn, pCoexDm->bCurPsTdmaOn));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], prePsTdma = %d, curPsTdma = %d!!\n",
			pCoexDm->prePsTdma, pCoexDm->curPsTdma));

		if( (pCoexDm->bPrePsTdmaOn == pCoexDm->bCurPsTdmaOn) &&
			(pCoexDm->prePsTdma == pCoexDm->curPsTdma) )
			return;
	}
	if(bTurnOn)
	{
		switch(type)
		{
			default:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x1a, 0x1a, 0x0, 0x58);
				break;
			case 1:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x1a, 0x1a, 0x0, 0x48);
				rssiAdjustVal = 11;
				break;
			case 2:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x12, 0x12, 0x0, 0x48);
				rssiAdjustVal = 14;
				break;
			case 3:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x25, 0x3, 0x10, 0x40);
				break;
			case 4:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x15, 0x3, 0x14, 0x0);
				rssiAdjustVal = 17;
				break;
			case 5:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x61, 0x15, 0x3, 0x31, 0x0);
				break;
			case 6:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0xa, 0x3, 0x0, 0x0);
				break;
			case 7:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0xc, 0x5, 0x0, 0x0);
				break;
			case 8:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x25, 0x3, 0x10, 0x0);
				break;
			case 9:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0xa, 0xa, 0x0, 0x48);
				rssiAdjustVal = 18;
				break;
			case 10:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0xa, 0xa, 0x0, 0x40);
				break;
			case 11:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x5, 0x5, 0x0, 0x48);
				rssiAdjustVal = 20;
				break;
			case 12:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xeb, 0xa, 0x3, 0x31, 0x18);
				break;

			case 15:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0xa, 0x3, 0x8, 0x0);
				break;
			case 16:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x15, 0x3, 0x10, 0x0);
				rssiAdjustVal = 18;
				break;

			case 18:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x25, 0x3, 0x10, 0x0);
				rssiAdjustVal = 14;
				break;

			case 20:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0x25, 0x25, 0x0, 0x0);
				break;
			case 21:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x20, 0x3, 0x10, 0x40);
				break;
			case 22:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x13, 0x8, 0x8, 0x0, 0x40);
				break;
			case 23:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0x25, 0x3, 0x31, 0x18);
				rssiAdjustVal = 22;
				break;
			case 24:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0x15, 0x3, 0x31, 0x18);
				rssiAdjustVal = 22;
				break;
			case 25:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0xa, 0x3, 0x31, 0x18);
				rssiAdjustVal = 22;
				break;
			case 26:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0xa, 0x3, 0x31, 0x18);
				rssiAdjustVal = 22;
				break;
			case 27:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0x25, 0x3, 0x31, 0x98);
				rssiAdjustVal = 22;
				break;
			case 28:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x69, 0x25, 0x3, 0x31, 0x0);
				break;
			case 29:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xab, 0x1a, 0x1a, 0x1, 0x8);
				break;
			case 30:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x93, 0x15, 0x3, 0x14, 0x0);
				break;
			case 31:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x1a, 0x1a, 0, 0x58);
				break;
			case 32:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xab, 0xa, 0x3, 0x31, 0x88);
				break;
			case 33:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xa3, 0x25, 0x3, 0x30, 0x88);
				break;
			case 34:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x1a, 0x1a, 0x0, 0x8);
				break;
			case 35:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0x0, 0x8);
				break;
			case 36:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0xd3, 0x12, 0x3, 0x14, 0x58);
				break;
		}
	}
	else
	{
		// disable PS tdma
		switch(type)
		{
			case 8:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x8, 0x0, 0x0, 0x0, 0x0);
				pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x4);
				break;
			case 0:
			default:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x0, 0x0, 0x0, 0x0, 0x0);
				delay_ms(5);
				pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x20);
				break;
			case 9:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x0, 0x0, 0x0, 0x0, 0x0);
				pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x4);
				break;
			case 10:
				halbtc8812a1ant_SetFwPstdma(pBtCoexist, 0x0, 0x0, 0x0, 0x8, 0x0);
				delay_ms(5);
				pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x20);
				break;
		}
	}
	rssiAdjustVal =0;
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_RSSI_ADJ_VAL_FOR_1ANT_COEX_TYPE, &rssiAdjustVal);

	// update pre state
	pCoexDm->bPrePsTdmaOn = pCoexDm->bCurPsTdmaOn;
	pCoexDm->prePsTdma = pCoexDm->curPsTdma;
}

VOID
halbtc8812a1ant_CoexAllOff(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	// fw all off
	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
	halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	// sw all off
	halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
	halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);


	// hw all off
	halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
}

VOID
halbtc8812a1ant_WifiParaAdjust(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bEnable
	)
{
	if(bEnable)
	{
		halbtc8812a1ant_LowPenaltyRa(pBtCoexist, NORMAL_EXEC, TRUE);
	}
	else
	{
		halbtc8812a1ant_LowPenaltyRa(pBtCoexist, NORMAL_EXEC, FALSE);
	}
}

BOOLEAN
halbtc8812a1ant_IsCommonAction(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	BOOLEAN			bCommon=FALSE, bWifiConnected=FALSE, bWifiBusy=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);

	//halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);

	if(!bWifiConnected &&
		BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi non connected-idle + BT non connected-idle!!\n"));
		halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

 		halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
		halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	}
	else if(bWifiConnected &&
		(BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) )
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi connected + BT non connected-idle!!\n"));
		halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);

      		halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
		halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	}
	else if(!bWifiConnected &&
		(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) )
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi non connected-idle + BT connected-idle!!\n"));
		halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

		halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
		halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	}
	else if(bWifiConnected &&
		(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) )
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi connected + BT connected-idle!!\n"));
		halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

		halbtc8812a1ant_SwMechanism1(pBtCoexist,TRUE,TRUE,TRUE,TRUE);
		halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	}
	else if(!bWifiConnected &&
		(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE != pCoexDm->btStatus) )
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi non connected-idle + BT Busy!!\n"));
		halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

		halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
		halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	}
	else
	{
		halbtc8812a1ant_SwMechanism1(pBtCoexist,TRUE,TRUE,TRUE,TRUE);

		bCommon = FALSE;
	}

	return bCommon;
}


VOID
halbtc8812a1ant_TdmaDurationAdjustForAcl(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	static s4Byte		up,dn,m,n,WaitCount;
	s4Byte			result;   //0: no change, +1: increase WiFi duration, -1: decrease WiFi duration
	u1Byte			retryCount=0, btInfoExt;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], halbtc8812a1ant_TdmaDurationAdjustForAcl()\n"));
	if(pCoexDm->bResetTdmaAdjust)
	{
		pCoexDm->bResetTdmaAdjust = FALSE;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], first run TdmaDurationAdjust()!!\n"));

		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
		pCoexDm->psTdmaDuAdjType = 2;
		//============
		up = 0;
		dn = 0;
		m = 1;
		n= 3;
		result = 0;
		WaitCount = 0;
	}
	else
	{
		//accquire the BT TRx retry count from BT_Info byte2
		retryCount = pCoexSta->btRetryCnt;
		btInfoExt = pCoexSta->btInfoExt;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], retryCount = %d\n", retryCount));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], up=%d, dn=%d, m=%d, n=%d, WaitCount=%d\n",
			up, dn, m, n, WaitCount));
		result = 0;
		WaitCount++;

		if(retryCount == 0)  // no retry in the last 2-second duration
		{
			up++;
			dn--;

			if (dn <= 0)
				dn = 0;

			if(up >= n)	// if 連續 n 個2秒 retry count為0, 則調寬WiFi duration
			{
				WaitCount = 0;
				n = 3;
				up = 0;
				dn = 0;
				result = 1;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Increase wifi duration!!\n"));
			}
		}
		else if (retryCount <= 3)	// <=3 retry in the last 2-second duration
		{
			up--;
			dn++;

			if (up <= 0)
				up = 0;

			if (dn == 2)	// if 連續 2 個2秒 retry count< 3, 則調窄WiFi duration
			{
				if (WaitCount <= 2)
					m++; // 避免一直在兩個level中來回
				else
					m = 1;

				if ( m >= 20) //m 最大值 = 20 ' 最大120秒 recheck是否調整 WiFi duration.
					m = 20;

				n = 3*m;
				up = 0;
				dn = 0;
				WaitCount = 0;
				result = -1;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Decrease wifi duration for retryCounter<3!!\n"));
			}
		}
		else  //retry count > 3, 只要1次 retry count > 3, 則調窄WiFi duration
		{
			if (WaitCount == 1)
				m++; // 避免一直在兩個level中來回
			else
				m = 1;

			if ( m >= 20) //m 最大值 = 20 ' 最大120秒 recheck是否調整 WiFi duration.
				m = 20;

			n = 3*m;
			up = 0;
			dn = 0;
			WaitCount = 0;
			result = -1;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Decrease wifi duration for retryCounter>3!!\n"));
		}

		if(result == -1)
		{
			if( (BT_INFO_8812A_1ANT_A2DP_BASIC_RATE(btInfoExt)) &&
				((pCoexDm->curPsTdma == 1) ||(pCoexDm->curPsTdma == 2)) )
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
				pCoexDm->psTdmaDuAdjType = 9;
			}
			else if(pCoexDm->curPsTdma == 1)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
				pCoexDm->psTdmaDuAdjType = 2;
			}
			else if(pCoexDm->curPsTdma == 2)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
				pCoexDm->psTdmaDuAdjType = 9;
			}
			else if(pCoexDm->curPsTdma == 9)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
				pCoexDm->psTdmaDuAdjType = 11;
			}
		}
		else if(result == 1)
		{
			if( (BT_INFO_8812A_1ANT_A2DP_BASIC_RATE(btInfoExt)) &&
				((pCoexDm->curPsTdma == 1) ||(pCoexDm->curPsTdma == 2)) )
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
				pCoexDm->psTdmaDuAdjType = 9;
			}
			else if(pCoexDm->curPsTdma == 11)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
				pCoexDm->psTdmaDuAdjType = 9;
			}
			else if(pCoexDm->curPsTdma == 9)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
				pCoexDm->psTdmaDuAdjType = 2;
			}
			else if(pCoexDm->curPsTdma == 2)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
				pCoexDm->psTdmaDuAdjType = 1;
			}
		}

		if( pCoexDm->curPsTdma != 1 &&
			pCoexDm->curPsTdma != 2 &&
			pCoexDm->curPsTdma != 9 &&
			pCoexDm->curPsTdma != 11 )
		{
			// recover to previous adjust type
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, pCoexDm->psTdmaDuAdjType);
		}
	}
}

u1Byte
halbtc8812a1ant_PsTdmaTypeByWifiRssi(
	IN	s4Byte	wifiRssi,
	IN	s4Byte	preWifiRssi,
	IN	u1Byte	wifiRssiThresh
	)
{
	u1Byte	psTdmaType=0;

	if(wifiRssi > preWifiRssi)
	{
		if(wifiRssi > (wifiRssiThresh+5))
		{
			psTdmaType = 26;
		}
		else
		{
			psTdmaType = 25;
		}
	}
	else
	{
		if(wifiRssi > wifiRssiThresh)
		{
			psTdmaType = 26;
		}
		else
		{
			psTdmaType = 25;
		}
	}

	return psTdmaType;
}

VOID
halbtc8812a1ant_PsTdmaCheckForPowerSaveState(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bNewPsState
	)
{
	u1Byte	lpsMode=0x0;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U1_LPS_MODE, &lpsMode);

	if(lpsMode)	// already under LPS state
	{
		if(bNewPsState)
		{
			// keep state under LPS, do nothing.
		}
		else
		{
			// will leave LPS state, turn off psTdma first
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
		}
	}
	else						// NO PS state
	{
		if(bNewPsState)
		{
			// will enter LPS state, turn off psTdma first
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
		}
		else
		{
			// keep state under NO PS state, do nothing.
		}
	}
}

// SCO only or SCO+PAN(HS)
VOID
halbtc8812a1ant_ActionSco(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte	wifiRssiState;
	u4Byte	wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 4);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			  halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			  halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}


VOID
halbtc8812a1ant_ActionHid(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte	wifiRssiState, btRssiState;
	u4Byte	wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

//A2DP only / PAN(EDR) only/ A2DP+PAN(HS)
VOID
halbtc8812a1ant_ActionA2dp(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8812a1ant_ActionA2dpPanHs(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState, btInfoExt;
	u4Byte		wifiBw;

	btInfoExt = pCoexSta->btInfoExt;
	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8812a1ant_ActionPanEdr(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}


//PAN(HS) only
VOID
halbtc8812a1ant_ActionPanHs(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// fw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
		}
		else
		{
			halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);
		}

		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// fw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
		}
		else
		{
			halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);
		}

		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

//PAN(EDR)+A2DP
VOID
halbtc8812a1ant_ActionPanEdrA2dp(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState, btInfoExt;
	u4Byte		wifiBw;

	btInfoExt = pCoexSta->btInfoExt;
	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8812a1ant_ActionPanEdrHid(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

// HID+A2DP+PAN(EDR)
VOID
halbtc8812a1ant_ActionHidA2dpPanEdr(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState, btInfoExt;
	u4Byte		wifiBw;

	btInfoExt = pCoexSta->btInfoExt;
	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8812a1ant_ActionHidA2dp(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u1Byte		wifiRssiState, btRssiState, btInfoExt;
	u4Byte		wifiBw;

	btInfoExt = pCoexSta->btInfoExt;
	wifiRssiState = halbtc8812a1ant_WifiRssiState(pBtCoexist, 0, 2, 25, 0);
	btRssiState = halbtc8812a1ant_BtRssiState(2, 50, 0);

	if(halbtc8812a1ant_NeedToDecBtPwr(pBtCoexist))
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, TRUE);
	else
		halbtc8812a1ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, FALSE);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 == wifiBw)
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,TRUE,FALSE,0x18);
		}
	}
	else
	{
		// sw mechanism
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
			(wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) )
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,TRUE,TRUE,FALSE,0x18);
		}
		else
		{
			halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8812a1ant_ActionHs(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN			bHsConnecting
	)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action for HS, bHsConnecting=%d!!!\n", bHsConnecting));
	halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);

	if(bHsConnecting)
	{
		halbtc8812a1ant_CoexTable(pBtCoexist, FORCE_EXEC, 0xaaaaaaaa, 0xaaaaaaaa, 0xffff, 0x3);
	}
	else
	{
		if((pCoexSta->highPriorityTx+pCoexSta->highPriorityRx+
			pCoexSta->lowPriorityTx+pCoexSta->lowPriorityRx)<=1200)
			halbtc8812a1ant_CoexTable(pBtCoexist, FORCE_EXEC, 0xaaaaaaaa, 0xaaaaaaaa, 0xffff, 0x3);
		else
			halbtc8812a1ant_CoexTable(pBtCoexist, FORCE_EXEC, 0xffffffff, 0xffffffff, 0xffff, 0x3);
	}
}


VOID
halbtc8812a1ant_ActionWifiNotConnected(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	BOOLEAN		bHsConnecting=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_CONNECTING, &bHsConnecting);

	halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

	if(bHsConnecting)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HS is connecting!!!\n"));
		halbtc8812a1ant_ActionHs(pBtCoexist, bHsConnecting);
	}
	else
	{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
}

VOID
halbtc8812a1ant_ActionWifiNotConnectedAssoAuthScan(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO	pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN			bHsConnecting=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_CONNECTING, &bHsConnecting);
	halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

	if(bHsConnecting)
	{
		halbtc8812a1ant_ActionHs(pBtCoexist, bHsConnecting);
	}
	else if(pBtCoexist->btInfo.bBtDisabled)
	{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else if(BT_8812A_1ANT_BT_STATUS_INQ_PAGE == pCoexDm->btStatus)
{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 30);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else if( (BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) ||
		(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) )
	{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 28);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus)
	{
		if(pStackInfo->bHidExist)
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 35);
		else
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 29);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
	}
	else if( (BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
		(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
		{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5aea5aea, 0x5aea5aea, 0xffff, 0x3);
	}
	else
	{
		//error condition, should not reach here, record error number for debugging.
		pCoexDm->errorCondition = 1;
	}
}

VOID
halbtc8812a1ant_ActionWifiConnectedScan(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO	pStackInfo=&pBtCoexist->stackInfo;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], ActionConnectedScan()===>\n"));

	if(pBtCoexist->btInfo.bBtDisabled)
	{
		halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else
	{
		halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
		halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
		// power save must executed before psTdma.
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);

		// psTdma
		if(BT_8812A_1ANT_BT_STATUS_INQ_PAGE == pCoexDm->btStatus)
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], ActionConnectedScan(), bt is under inquiry/page scan\n"));
			if(pStackInfo->bScoExist)
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 32);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
			}
			else
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 30);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
			}
		}
		else if( (BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) ||
			(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) )
		{
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
		}
		else if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus)
		{
			if(pStackInfo->bHidExist)
		{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 34);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
			}
			else
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
			}
		}
		else if( (BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
			(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
		{
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 33);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
		}
		else
		{
			//error condition, should not reach here
			pCoexDm->errorCondition = 2;
		}
	}
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], ActionConnectedScan()<===\n"));
}

VOID
halbtc8812a1ant_ActionWifiConnectedSpecialPacket(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO	pStackInfo=&pBtCoexist->stackInfo;

	halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

	if(pBtCoexist->btInfo.bBtDisabled)
	{
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else
	{
		if(BT_8812A_1ANT_BT_STATUS_INQ_PAGE == pCoexDm->btStatus)
		{
			if(pStackInfo->bScoExist)
		{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 32);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
			}
			else
			{
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 30);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
			}
		}
		else if( (BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) ||
			(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) )
		{
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 28);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
		}
		else if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus)
		{
			if(pStackInfo->bHidExist)
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 35);
			else
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 29);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
		}
		else if( (BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
			(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
		{
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5aea5aea, 0x5aea5aea, 0xffff, 0x3);
		}
		else
		{
			//error condition, should not reach here
			pCoexDm->errorCondition = 3;
		}
	}
}

VOID
halbtc8812a1ant_ActionWifiConnected(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO	pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN 	bWifiConnected=FALSE, bWifiBusy=FALSE, bBtHsOn=FALSE;
	BOOLEAN		bScan=FALSE, bLink=FALSE, bRoam=FALSE;
	BOOLEAN		bHsConnecting=FALSE, bUnder4way=FALSE;
	u4Byte		wifiBw;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], CoexForWifiConnect()===>\n"));

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	if(!bWifiConnected)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], CoexForWifiConnect(), return for wifi not connected<===\n"));
		return;
	}

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_4_WAY_PROGRESS, &bUnder4way);
	if(bUnder4way)
	{
		halbtc8812a1ant_ActionWifiConnectedSpecialPacket(pBtCoexist);
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], CoexForWifiConnect(), return for wifi is under 4way<===\n"));
		return;
	}

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_CONNECTING, &bHsConnecting);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_SCAN, &bScan);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_LINK, &bLink);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_ROAM, &bRoam);
	if(bScan || bLink || bRoam)
	{
		halbtc8812a1ant_ActionWifiConnectedScan(pBtCoexist);
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], CoexForWifiConnect(), return for wifi is under scan<===\n"));
		return;
	}

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);
	if(!bWifiBusy)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi associated-idle!!!\n"));
		if(pBtCoexist->btInfo.bBtDisabled)
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
			halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
		}
		else
		{
			if(BT_8812A_1ANT_BT_STATUS_INQ_PAGE == pCoexDm->btStatus)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], bt is under inquiry/page scan!!!\n"));
				if(pStackInfo->bScoExist)
				{
					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 32);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
				}
				else
				{
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
					halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 30);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
				}
			}
			else if(BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus)
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
				halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x26, 0x0);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
			}
			else if(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus)
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
				halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x26, 0x0);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
			}
			else if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus)
			{
				if(pStackInfo->bHidExist && pStackInfo->numOfLink==1)
				{
					// hid only
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5fff5fff, 0x5fff5fff, 0xffff, 0x3);
					pCoexDm->bResetTdmaAdjust = TRUE;
				}
				else
				{
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
					halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);

					if(pStackInfo->bHidExist)
					{
						if(pStackInfo->bA2dpExist)
						{
							// hid+a2dp
							halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						else if(pStackInfo->bPanExist)
						{
							if(bBtHsOn)
							{
								// hid+hs
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							else
							{
								// hid+pan
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						else
						{
							pCoexDm->errorCondition = 4;
						}
						pCoexDm->bResetTdmaAdjust = TRUE;
					}
					else if(pStackInfo->bA2dpExist)
					{
						if(pStackInfo->bPanExist)
						{
							if(bBtHsOn)
							{
								// a2dp+hs
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							else
							{
								// a2dp+pan
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 36);
							}
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
							pCoexDm->bResetTdmaAdjust = TRUE;
						}
						else
						{
							// a2dp only
							halbtc8812a1ant_TdmaDurationAdjustForAcl(pBtCoexist);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
					}
					else if(pStackInfo->bPanExist)
					{
						// pan only
						if(bBtHsOn)
						{
							pCoexDm->errorCondition = 5;
						}
						else
						{
							halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						pCoexDm->bResetTdmaAdjust = TRUE;
					}
					else
					{
						// temp state, do nothing!!!
						//DbgPrint("error 6, pCoexDm->btStatus=%d\n", pCoexDm->btStatus);
						//DbgPrint("error 6, pStackInfo->numOfLink=%d, pStackInfo->bHidExist=%d, pStackInfo->bA2dpExist=%d, pStackInfo->bPanExist=%d, pStackInfo->bScoExist=%d\n",
							//pStackInfo->numOfLink, pStackInfo->bHidExist, pStackInfo->bA2dpExist, pStackInfo->bPanExist, pStackInfo->bScoExist);
						//pCoexDm->errorCondition = 6;
					}
				}
			}
			else if( (BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
				(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5aea5aea, 0x5aea5aea, 0xffff, 0x3);
			}
			else
			{
				//error condition, should not reach here
				pCoexDm->errorCondition = 7;
			}
		}
	}
	else
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi busy!!!\n"));
		if(pBtCoexist->btInfo.bBtDisabled)
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
			halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
		}
		else
		{
			if(bBtHsOn)
			{
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HS is under progress!!!\n"));
				//DbgPrint("pCoexDm->btStatus = 0x%x\n", pCoexDm->btStatus);
				halbtc8812a1ant_ActionHs(pBtCoexist, bHsConnecting);
			}
			else if(BT_8812A_1ANT_BT_STATUS_INQ_PAGE == pCoexDm->btStatus)
			{
				if(pStackInfo->bScoExist)
				{
					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 32);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
				}
				else
				{
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
					halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 30);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
				}
			}
			else if(BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus)
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5a5a5a5a, 0x5a5a5a5a, 0xffff, 0x3);
			}
			else if(BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus)
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
				if(bBtHsOn)
				{
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HS is under progress!!!\n"));
					halbtc8812a1ant_ActionHs(pBtCoexist, bHsConnecting);
				}
				else
				{
					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5a5a5a5a, 0x5a5a5a5a, 0xffff, 0x3);
				}
			}
			else if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus)
			{
				if(pStackInfo->bHidExist && pStackInfo->numOfLink==1)
				{
					// hid only
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

					halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
					halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5fff5fff, 0x5fff5fff, 0xffff, 0x3);
					pCoexDm->bResetTdmaAdjust = TRUE;
				}
				else
				{
					halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
					halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
					// power save must executed before psTdma.
					pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);

					if(pStackInfo->bHidExist)
					{
						if(pStackInfo->bA2dpExist)
						{
							// hid+a2dp
							halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						else if(pStackInfo->bPanExist)
						{
							if(bBtHsOn)
							{
								// hid+hs
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							else
							{
								// hid+pan
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						else
						{
							pCoexDm->errorCondition = 8;
						}
						pCoexDm->bResetTdmaAdjust = TRUE;
					}
					else if(pStackInfo->bA2dpExist)
					{
						if(pStackInfo->bPanExist)
						{
							if(bBtHsOn)
							{
								// a2dp+hs
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							}
							else
							{
								// a2dp+pan
								halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 36);
							}
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
							pCoexDm->bResetTdmaAdjust = TRUE;
						}
						else
						{
							// a2dp only
							halbtc8812a1ant_TdmaDurationAdjustForAcl(pBtCoexist);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
					}
					else if(pStackInfo->bPanExist)
					{
						// pan only
						if(bBtHsOn)
						{
							pCoexDm->errorCondition = 9;
						}
						else
						{
							halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
							halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x5afa5afa, 0xffff, 0x3);
						}
						pCoexDm->bResetTdmaAdjust = TRUE;
					}
					else
					{
						//DbgPrint("error 10, pStackInfo->numOfLink=%d, pStackInfo->bHidExist=%d, pStackInfo->bA2dpExist=%d, pStackInfo->bPanExist=%d, pStackInfo->bScoExist=%d\n",
							//pStackInfo->numOfLink, pStackInfo->bHidExist, pStackInfo->bA2dpExist, pStackInfo->bPanExist, pStackInfo->bScoExist);
						pCoexDm->errorCondition = 10;
					}
				}
			}
			else if( (BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
				(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
			{
				halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
				// power save must executed before psTdma.
				pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);

				halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 8);
				halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x5aea5aea, 0x5aea5aea, 0xffff, 0x3);
			}
			else
			{
				//DbgPrint("error 11, pCoexDm->btStatus=%d\n", pCoexDm->btStatus);
				//DbgPrint("error 11, pStackInfo->numOfLink=%d, pStackInfo->bHidExist=%d, pStackInfo->bA2dpExist=%d, pStackInfo->bPanExist=%d, pStackInfo->bScoExist=%d\n",
					//pStackInfo->numOfLink, pStackInfo->bHidExist, pStackInfo->bA2dpExist, pStackInfo->bPanExist, pStackInfo->bScoExist);
				//error condition, should not reach here
				pCoexDm->errorCondition = 11;
			}
		}
	}
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], CoexForWifiConnect()<===\n"));
}

VOID
halbtc8812a1ant_RunSwCoexistMechanism(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN				bWifiUnder5G=FALSE, bWifiBusy=FALSE, bWifiConnected=FALSE;
	u1Byte				btInfoOriginal=0, btRetryCnt=0;
	u1Byte				algorithm=0;

	return;
	if(pStackInfo->bProfileNotified)
	{
		algorithm = halbtc8812a1ant_ActionAlgorithm(pBtCoexist);
		pCoexDm->curAlgorithm = algorithm;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Algorithm = %d \n", pCoexDm->curAlgorithm));

		if(halbtc8812a1ant_IsCommonAction(pBtCoexist))
		{
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action common.\n"));
		}
		else
		{
			switch(pCoexDm->curAlgorithm)
			{
				case BT_8812A_1ANT_COEX_ALGO_SCO:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = SCO.\n"));
					halbtc8812a1ant_ActionSco(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_HID:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = HID.\n"));
					halbtc8812a1ant_ActionHid(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_A2DP:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = A2DP.\n"));
					halbtc8812a1ant_ActionA2dp(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_A2DP_PANHS:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = A2DP+PAN(HS).\n"));
					halbtc8812a1ant_ActionA2dpPanHs(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_PANEDR:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = PAN(EDR).\n"));
					halbtc8812a1ant_ActionPanEdr(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_PANHS:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = HS mode.\n"));
					halbtc8812a1ant_ActionPanHs(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_PANEDR_A2DP:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = PAN+A2DP.\n"));
					halbtc8812a1ant_ActionPanEdrA2dp(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_PANEDR_HID:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = PAN(EDR)+HID.\n"));
					halbtc8812a1ant_ActionPanEdrHid(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_HID_A2DP_PANEDR:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = HID+A2DP+PAN.\n"));
					halbtc8812a1ant_ActionHidA2dpPanEdr(pBtCoexist);
					break;
				case BT_8812A_1ANT_COEX_ALGO_HID_A2DP:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = HID+A2DP.\n"));
					halbtc8812a1ant_ActionHidA2dp(pBtCoexist);
					break;
				default:
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action algorithm = coexist All Off!!\n"));
					halbtc8812a1ant_CoexAllOff(pBtCoexist);
					break;
			}
			pCoexDm->preAlgorithm = pCoexDm->curAlgorithm;
		}
	}
}

VOID
halbtc8812a1ant_RunCoexistMechanism(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN				bWifiUnder5G=FALSE, bWifiBusy=FALSE, bWifiConnected=FALSE;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism()===>\n"));

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_UNDER_5G, &bWifiUnder5G);

	if(bWifiUnder5G)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism(), return for 5G <===\n"));
		return;
	}

	if(pBtCoexist->bManualControl)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism(), return for Manual CTRL <===\n"));
		return;
	}

	if(pBtCoexist->bStopCoexDm)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism(), return for Stop Coex DM <===\n"));
		return;
	}

	halbtc8812a1ant_RunSwCoexistMechanism(pBtCoexist);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	if(pBtCoexist->btInfo.bBtDisabled)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], bt is disabled!!!\n"));
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);
		if(bWifiBusy)
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		}
		else
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, TRUE);
			halbtc8812a1ant_LpsRpwm(pBtCoexist, NORMAL_EXEC, 0x0, 0x4);
			// power save must executed before psTdma.
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_ENTER_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		}
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
	}
	else if(pCoexSta->bUnderIps)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi is under IPS !!!\n"));
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
		halbtc8812a1ant_CoexTable(pBtCoexist, NORMAL_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
		halbtc8812a1ant_WifiParaAdjust(pBtCoexist, FALSE);
	}
	else if(!bWifiConnected)
	{
		BOOLEAN	bScan=FALSE, bLink=FALSE, bRoam=FALSE;

		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi is non connected-idle !!!\n"));

		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_SCAN, &bScan);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_LINK, &bLink);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_ROAM, &bRoam);

		if(bScan || bLink || bRoam)
			halbtc8812a1ant_ActionWifiNotConnectedAssoAuthScan(pBtCoexist);
		else
			halbtc8812a1ant_ActionWifiNotConnected(pBtCoexist);
	}
	else	// wifi LPS/Busy
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi is NOT under IPS!!!\n"));
		halbtc8812a1ant_WifiParaAdjust(pBtCoexist, TRUE);
		halbtc8812a1ant_ActionWifiConnected(pBtCoexist);
	}
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism()<===\n"));
}

VOID
halbtc8812a1ant_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	BOOLEAN		bWifiConnected=FALSE;
	// force to reset coex mechanism

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	if(!bWifiConnected) // non-connected scan
	{
		halbtc8812a1ant_ActionWifiNotConnected(pBtCoexist);
	}
	else	// wifi is connected
	{
		halbtc8812a1ant_ActionWifiConnected(pBtCoexist);
	}

	halbtc8812a1ant_FwDacSwingLvl(pBtCoexist, FORCE_EXEC, 6);
	halbtc8812a1ant_DecBtPwr(pBtCoexist, FORCE_EXEC, FALSE);

	// sw all off
	halbtc8812a1ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
	halbtc8812a1ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

	halbtc8812a1ant_CoexTable(pBtCoexist, FORCE_EXEC, 0x55555555, 0x55555555, 0xffff, 0x3);
}

//============================================================
// work around function start with wa_halbtc8812a1ant_
//============================================================
//============================================================
// extern function start with EXhalbtc8812a1ant_
//============================================================
VOID
EXhalbtc8812a1ant_InitHwConfig(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	u4Byte	u4Tmp=0;
	u2Byte	u2Tmp=0;
	u1Byte	u1Tmp=0;

	BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], 1Ant Init HW Config!!\n"));

	// backup rf 0x1e value
	pCoexDm->btRf0x1eBackup =
		pBtCoexist->fBtcGetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff);

	//ant sw control to BT
	 pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x900, 0x00000400);
	 pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x76d, 0x1);
	 pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xcb3, 0x77);
	 pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xcb7, 0x40);

	// 0x790[5:0]=0x5
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x790);
	u1Tmp &= 0xc0;
	u1Tmp |= 0x5;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x790, u1Tmp);

	// PTA parameter
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x6cc, 0x0);
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c8, 0xffff);
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c4, 0x55555555);
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c0, 0x55555555);

	// coex parameters
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x778, 0x1);

	// enable counter statistics
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x76e, 0x4);

	// enable PTA
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x40, 0x20);

	// bt clock related
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x4);
	u1Tmp |= BIT7;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x4, u1Tmp);

	// bt clock related
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x7);
	u1Tmp |= BIT1;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x7, u1Tmp);
}

VOID
EXhalbtc8812a1ant_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], Coex Mechanism Init!!\n"));

	pBtCoexist->bStopCoexDm = FALSE;

	halbtc8812a1ant_InitCoexDm(pBtCoexist);
}

VOID
EXhalbtc8812a1ant_DisplayCoexInfo(
	IN	PBTC_COEXIST		pBtCoexist
	)
{
	PBTC_BOARD_INFO		pBoardInfo=&pBtCoexist->boardInfo;
	PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;
	pu1Byte				cliBuf=pBtCoexist->cliBuf;
	u1Byte				u1Tmp[4], i, btInfoExt, psTdmaCase=0;
	u4Byte				u4Tmp[4];
	BOOLEAN				bRoam=FALSE, bScan=FALSE, bLink=FALSE, bWifiUnder5G=FALSE;
	BOOLEAN				bBtHsOn=FALSE, bWifiBusy=FALSE;
	s4Byte				wifiRssi=0, btHsRssi=0;
	u4Byte				wifiBw, wifiTrafficDir;
	u1Byte				wifiDot11Chnl, wifiHsChnl;

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ============[BT Coexist info]============");
	CL_PRINTF(cliBuf);

	if(pBtCoexist->bManualControl)
	{
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ============[Under Manual Control]============");
		CL_PRINTF(cliBuf);
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ==========================================");
		CL_PRINTF(cliBuf);
	}
	if(pBtCoexist->bStopCoexDm)
	{
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ============[Coex is STOPPED]============");
		CL_PRINTF(cliBuf);
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ==========================================");
		CL_PRINTF(cliBuf);
	}

	if(!pBoardInfo->bBtExist)
	{
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n BT not exists !!!");
		CL_PRINTF(cliBuf);
		return;
	}

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d ", "Ant PG number/ Ant mechanism:", \
		pBoardInfo->pgAntNum, pBoardInfo->btdmAntNum);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s / %d", "BT stack/ hci ext ver", \
		((pStackInfo->bProfileNotified)? "Yes":"No"), pStackInfo->hciVersion);
	CL_PRINTF(cliBuf);
	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_BT_FW_VER);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U1_WIFI_DOT11_CHNL, &wifiDot11Chnl);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U1_WIFI_HS_CHNL, &wifiHsChnl);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d / %d(%d)", "Dot11 channel / HsChnl(HsMode)", \
		wifiDot11Chnl, wifiHsChnl, bBtHsOn);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x ", "H2C Wifi inform bt chnl Info", \
		pCoexDm->wifiChnlInfo[0], pCoexDm->wifiChnlInfo[1],
		pCoexDm->wifiChnlInfo[2]);
	CL_PRINTF(cliBuf);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_S4_WIFI_RSSI, &wifiRssi);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_S4_HS_RSSI, &btHsRssi);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "Wifi rssi/ HS rssi", \
		wifiRssi, btHsRssi);
	CL_PRINTF(cliBuf);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_SCAN, &bScan);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_LINK, &bLink);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_ROAM, &bRoam);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d ", "Wifi bLink/ bRoam/ bScan", \
		bLink, bRoam, bScan);
	CL_PRINTF(cliBuf);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_UNDER_5G, &bWifiUnder5G);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_TRAFFIC_DIRECTION, &wifiTrafficDir);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s / %s/ %s ", "Wifi status", \
		(bWifiUnder5G? "5G":"2.4G"),
		((BTC_WIFI_BW_LEGACY==wifiBw)? "Legacy": (((BTC_WIFI_BW_HT40==wifiBw)? "HT40":"HT20"))),
		((!bWifiBusy)? "idle": ((BTC_WIFI_TRAFFIC_TX==wifiTrafficDir)? "uplink":"downlink")));
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = [%s/ %d/ %d] ", "BT [status/ rssi/ retryCnt]", \
		((pCoexSta->bC2hBtInquiryPage)?("inquiry/page scan"):((BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus)? "non-connected idle":
		(  (BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus)? "connected-idle":"busy"))),
		pCoexSta->btRssi, pCoexSta->btRetryCnt);
	CL_PRINTF(cliBuf);

	if(pStackInfo->bProfileNotified)
	{
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d / %d / %d / %d", "SCO/HID/PAN/A2DP", \
			pStackInfo->bScoExist, pStackInfo->bHidExist, pStackInfo->bPanExist, pStackInfo->bA2dpExist);
		CL_PRINTF(cliBuf);

		pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_BT_LINK_INFO);
	}

	btInfoExt = pCoexSta->btInfoExt;
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s", "BT Info A2DP rate", \
		(btInfoExt&BIT0)? "Basic rate":"EDR rate");
	CL_PRINTF(cliBuf);

	for(i=0; i<BT_INFO_SRC_8812A_1ANT_MAX; i++)
	{
		if(pCoexSta->btInfoC2hCnt[i])
		{
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x %02x %02x(%d)", GLBtInfoSrc8812a1Ant[i], \
				pCoexSta->btInfoC2h[i][0], pCoexSta->btInfoC2h[i][1],
				pCoexSta->btInfoC2h[i][2], pCoexSta->btInfoC2h[i][3],
				pCoexSta->btInfoC2h[i][4], pCoexSta->btInfoC2h[i][5],
				pCoexSta->btInfoC2h[i][6], pCoexSta->btInfoC2hCnt[i]);
			CL_PRINTF(cliBuf);
		}
	}
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s/%s, (0x%x/0x%x)", "PS state, IPS/LPS, (lps/rpwm)", \
		((pCoexSta->bUnderIps? "IPS ON":"IPS OFF")),
		((pCoexSta->bUnderLps? "LPS ON":"LPS OFF")),
		pBtCoexist->btInfo.lps1Ant,
		pBtCoexist->btInfo.rpwm1Ant);
	CL_PRINTF(cliBuf);
	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_FW_PWR_MODE_CMD);

	if(!pBtCoexist->bManualControl)
	{
		// Sw mechanism
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Sw mechanism]============");
		CL_PRINTF(cliBuf);

		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d ", "SM1[ShRf/ LpRA/ LimDig/ btLna]", \
			pCoexDm->bCurRfRxLpfShrink, pCoexDm->bCurLowPenaltyRa, pCoexDm->bLimitedDig, pCoexDm->bCurBtLnaConstrain);
		CL_PRINTF(cliBuf);
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d(0x%x) ", "SM2[AgcT/ AdcB/ SwDacSwing(lvl)]", \
			pCoexDm->bCurAgcTableEn, pCoexDm->bCurAdcBackOff, pCoexDm->bCurDacSwingOn, pCoexDm->curDacSwingLvl);
		CL_PRINTF(cliBuf);

		// Fw mechanism
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Fw mechanism]============");
		CL_PRINTF(cliBuf);

		psTdmaCase = pCoexDm->curPsTdma;
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x case-%d", "PS TDMA", \
			pCoexDm->psTdmaPara[0], pCoexDm->psTdmaPara[1],
			pCoexDm->psTdmaPara[2], pCoexDm->psTdmaPara[3],
			pCoexDm->psTdmaPara[4], psTdmaCase);
		CL_PRINTF(cliBuf);

		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x ", "Latest error condition(should be 0)", \
			pCoexDm->errorCondition);
		CL_PRINTF(cliBuf);

		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d ", "DecBtPwr/ IgnWlanAct", \
			pCoexDm->bCurDecBtPwr, pCoexDm->bCurIgnoreWlanAct);
		CL_PRINTF(cliBuf);
	}

	// Hw setting
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Hw setting]============");
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "RF-A, 0x1e initVal", \
		pCoexDm->btRf0x1eBackup);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x778);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "0x778", \
		u1Tmp[0]);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x92c);
	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x930);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x92c/ 0x930", \
		(u1Tmp[0]), u4Tmp[0]);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x40);
	u1Tmp[1] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x4f);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x40/ 0x4f", \
		u1Tmp[0], u1Tmp[1]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x550);
	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x522);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x550(bcn ctrl)/0x522", \
		u4Tmp[0], u1Tmp[0]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xc50);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "0xc50(dig)", \
		u4Tmp[0]);
	CL_PRINTF(cliBuf);

#if 0
	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xf48);
	u4Tmp[1] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xf4c);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0xf48/ 0xf4c (FA cnt)", \
		u4Tmp[0], u4Tmp[1]);
	CL_PRINTF(cliBuf);
#endif

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c0);
	u4Tmp[1] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c4);
	u4Tmp[2] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c8);
	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x6cc);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x/ 0x%x/ 0x%x", "0x6c0/0x6c4/0x6c8/0x6cc(coexTable)", \
		u4Tmp[0], u4Tmp[1], u4Tmp[2], u1Tmp[0]);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "0x770(hp rx[31:16]/tx[15:0])", \
		pCoexSta->highPriorityRx, pCoexSta->highPriorityTx);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "0x774(lp rx[31:16]/tx[15:0])", \
		pCoexSta->lowPriorityRx, pCoexSta->lowPriorityTx);
	CL_PRINTF(cliBuf);

	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_COEX_STATISTICS);
}


VOID
EXhalbtc8812a1ant_IpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	u4Byte	u4Tmp=0;

	if(pBtCoexist->bManualControl ||	pBtCoexist->bStopCoexDm)
		return;

	if(BTC_IPS_ENTER == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], IPS ENTER notify\n"));
		pCoexSta->bUnderIps = TRUE;

		// 0x4c[23]=1
		u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x4c);
		u4Tmp |= BIT23;
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x4c, u4Tmp);

		halbtc8812a1ant_CoexAllOff(pBtCoexist);
	}
	else if(BTC_IPS_LEAVE == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], IPS LEAVE notify\n"));
		pCoexSta->bUnderIps = FALSE;
		//halbtc8812a1ant_InitCoexDm(pBtCoexist);
	}
}

VOID
EXhalbtc8812a1ant_LpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	if(pBtCoexist->bManualControl || pBtCoexist->bStopCoexDm)
		return;

	if(BTC_LPS_ENABLE == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], LPS ENABLE notify\n"));
		pCoexSta->bUnderLps = TRUE;
	}
	else if(BTC_IPS_LEAVE == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], LPS DISABLE notify\n"));
		pCoexSta->bUnderLps = FALSE;
	}
}

VOID
EXhalbtc8812a1ant_ScanNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	PBTC_STACK_INFO 	pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN 		bWifiConnected=FALSE;

	if(pBtCoexist->bManualControl ||pBtCoexist->bStopCoexDm)
		return;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	if(BTC_SCAN_START == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], SCAN START notify\n"));
		if(!bWifiConnected)	// non-connected scan
		{
			//set 0x550[3]=1 before PsTdma
			//halbtc8812a1ant_Reg0x550Bit3(pBtCoexist, TRUE);
			halbtc8812a1ant_ActionWifiNotConnectedAssoAuthScan(pBtCoexist);
		}
		else	// wifi is connected
		{
			halbtc8812a1ant_ActionWifiConnectedScan(pBtCoexist);
		}
	}
	else if(BTC_SCAN_FINISH == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], SCAN FINISH notify\n"));
		if(!bWifiConnected)	// non-connected scan
		{
			//halbtc8812a1ant_Reg0x550Bit3(pBtCoexist, FALSE);
			halbtc8812a1ant_ActionWifiNotConnected(pBtCoexist);
		}
		else
		{
			halbtc8812a1ant_ActionWifiConnected(pBtCoexist);
		}
	}
}

VOID
EXhalbtc8812a1ant_ConnectNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	)
{
	PBTC_STACK_INFO 	pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN			bWifiConnected=FALSE;

	if(pBtCoexist->bManualControl ||pBtCoexist->bStopCoexDm)
		return;

	if(BTC_ASSOCIATE_START == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], CONNECT START notify\n"));
		if(pBtCoexist->btInfo.bBtDisabled)
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		}
		else
		{
			halbtc8812a1ant_ActionWifiNotConnectedAssoAuthScan(pBtCoexist);
		}
	}
	else if(BTC_ASSOCIATE_FINISH == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], CONNECT FINISH notify\n"));

		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
		if(!bWifiConnected) // non-connected scan
		{
			//halbtc8812a1ant_Reg0x550Bit3(pBtCoexist, FALSE);
			halbtc8812a1ant_ActionWifiNotConnected(pBtCoexist);
		}
		else
		{
			halbtc8812a1ant_ActionWifiConnected(pBtCoexist);
		}
	}
}

VOID
EXhalbtc8812a1ant_MediaStatusNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
	)
{
	u1Byte			dataLen=5;
	u1Byte			buf[6] = {0};
	u1Byte			H2C_Parameter[3] ={0};
	BOOLEAN			bWifiUnder5G=FALSE;
	u4Byte			wifiBw;
	u1Byte			wifiCentralChnl;

	if(pBtCoexist->bManualControl ||pBtCoexist->bStopCoexDm)
		return;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_UNDER_5G, &bWifiUnder5G);

	// only 2.4G we need to inform bt the chnl mask
	if(!bWifiUnder5G)
	{
		if(BTC_MEDIA_CONNECT == type)
		{
			H2C_Parameter[0] = 0x1;
		}
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U1_WIFI_CENTRAL_CHNL, &wifiCentralChnl);
		H2C_Parameter[1] = wifiCentralChnl;
		if(BTC_WIFI_BW_HT40 == wifiBw)
			H2C_Parameter[2] = 0x30;
		else
			H2C_Parameter[2] = 0x20;
	}

	pCoexDm->wifiChnlInfo[0] = H2C_Parameter[0];
	pCoexDm->wifiChnlInfo[1] = H2C_Parameter[1];
	pCoexDm->wifiChnlInfo[2] = H2C_Parameter[2];

	buf[0] = dataLen;
	buf[1] = 0x5;				// OP_Code
	buf[2] = 0x3;				// OP_Code_Length
	buf[3] = H2C_Parameter[0]; 	// OP_Code_Content
	buf[4] = H2C_Parameter[1];
	buf[5] = H2C_Parameter[2];

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_CTRL_BT_COEX, (PVOID)&buf[0]);
}

VOID
EXhalbtc8812a1ant_SpecialPacketNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
	)
{
	BOOLEAN 	bSecurityLink=FALSE;

	if(pBtCoexist->bManualControl ||pBtCoexist->bStopCoexDm)
		return;

	//if(type == BTC_PACKET_DHCP)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], special Packet(%d) notify\n", type));
		if(pBtCoexist->btInfo.bBtDisabled)
		{
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
		}
		else
		{
			halbtc8812a1ant_ActionWifiConnectedSpecialPacket(pBtCoexist);
		}
	}
}

VOID
EXhalbtc8812a1ant_BtInfoNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	pu1Byte			tmpBuf,
	IN	u1Byte			length
	)
{
	u1Byte			btInfo=0;
	u1Byte			i, rspSource=0;
	static u4Byte		setBtLnaCnt=0, setBtPsdMode=0;
	BOOLEAN			bBtBusy=FALSE, bLimitedDig=FALSE;
	BOOLEAN			bWifiConnected=FALSE;
	BOOLEAN			bRejApAggPkt=FALSE;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify()===>\n"));


	rspSource = tmpBuf[0]&0xf;
	if(rspSource >= BT_INFO_SRC_8812A_1ANT_MAX)
		rspSource = BT_INFO_SRC_8812A_1ANT_WIFI_FW;
	pCoexSta->btInfoC2hCnt[rspSource]++;

	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], Bt info[%d], length=%d, hex data=[", rspSource, length));
	for(i=0; i<length; i++)
	{
		pCoexSta->btInfoC2h[rspSource][i] = tmpBuf[i];
		if(i == 1)
			btInfo = tmpBuf[i];
		if(i == length-1)
		{
			BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("0x%2x]\n", tmpBuf[i]));
		}
		else
		{
			BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("0x%2x, ", tmpBuf[i]));
		}
	}

	if(pBtCoexist->bManualControl)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), return for Manual CTRL<===\n"));
		return;
	}
	if(pBtCoexist->bStopCoexDm)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), return for Coex STOPPED!!<===\n"));
		return;
	}

	if(BT_INFO_SRC_8812A_1ANT_WIFI_FW != rspSource)
	{
		pCoexSta->btRetryCnt =
			pCoexSta->btInfoC2h[rspSource][2];

		pCoexSta->btRssi =
			pCoexSta->btInfoC2h[rspSource][3]*2+10;

		pCoexSta->btInfoExt =
			pCoexSta->btInfoC2h[rspSource][4];

		// Here we need to resend some wifi info to BT
		// because bt is reset and loss of the info.
		if( (pCoexSta->btInfoExt & BIT1) )
		{
			pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
			if(bWifiConnected)
			{
				EXhalbtc8812a1ant_MediaStatusNotify(pBtCoexist, BTC_MEDIA_CONNECT);
			}
			else
			{
				EXhalbtc8812a1ant_MediaStatusNotify(pBtCoexist, BTC_MEDIA_DISCONNECT);
			}

			setBtPsdMode = 0;
		}

		// test-chip bt patch doesn't support, temporary remove.
		// need to add back when mp-chip. 12/20/2012
#if 0
		if(setBtPsdMode <= 3)
		{
			halbtc8812a1ant_SetBtPsdMode(pBtCoexist, FORCE_EXEC, 0xd);
			setBtPsdMode++;
		}

		if(pCoexDm->bCurBtLnaConstrain)
		{
			if( (pCoexSta->btInfoExt & BIT2) )
			{
			}
			else
			{
				if(setBtLnaCnt <= 3)
				{
					halbtc8812a1ant_SetBtLnaConstrain(pBtCoexist, FORCE_EXEC, TRUE);
					setBtLnaCnt++;
				}
			}
		}
		else
		{
			setBtLnaCnt = 0;
		}
#endif
		// test-chip bt patch only rsp the status for BT_RSP,
		// so temporary we consider the following only under BT_RSP
		if(BT_INFO_SRC_8812A_1ANT_BT_RSP == rspSource)
		{
			if( (pCoexSta->btInfoExt & BIT3) )
			{
			#if 0// temp disable because bt patch report the wrong value.
				halbtc8812a1ant_IgnoreWlanAct(pBtCoexist, FORCE_EXEC, FALSE);
			#endif
			}
			else
			{
				// BT already NOT ignore Wlan active, do nothing here.
			}

			if( (pCoexSta->btInfoExt & BIT4) )
			{
				// BT auto report already enabled, do nothing
			}
			else
			{
				halbtc8812a1ant_BtAutoReport(pBtCoexist, FORCE_EXEC, TRUE);
			}
		}
	}

	// check BIT2 first ==> check if bt is under inquiry or page scan
	if(btInfo & BT_INFO_8812A_1ANT_B_INQ_PAGE)
	{
		pCoexSta->bC2hBtInquiryPage = TRUE;
		pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_INQ_PAGE;
	}
	else
	{
		pCoexSta->bC2hBtInquiryPage = FALSE;
		if(!(btInfo&BT_INFO_8812A_1ANT_B_CONNECTION))
		{
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_NON_CONNECTED_IDLE;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt non-connected idle!!!\n"));
		}
		else if(btInfo == BT_INFO_8812A_1ANT_B_CONNECTION)	// connection exists but no busy
		{
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_CONNECTED_IDLE;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt connected-idle!!!\n"));
		}
		else if((btInfo&BT_INFO_8812A_1ANT_B_SCO_ESCO) ||
			(btInfo&BT_INFO_8812A_1ANT_B_SCO_BUSY))
		{
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_SCO_BUSY;
			bRejApAggPkt = TRUE;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt sco busy!!!\n"));
		}
		else if(btInfo&BT_INFO_8812A_1ANT_B_ACL_BUSY)
		{
			if(BT_8812A_1ANT_BT_STATUS_ACL_BUSY != pCoexDm->btStatus)
				pCoexDm->bResetTdmaAdjust = TRUE;
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_ACL_BUSY;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt acl busy!!!\n"));
		}
#if 0
		else if(btInfo&BT_INFO_8812A_1ANT_B_SCO_ESCO)
		{
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt acl/sco busy!!!\n"));
		}
#endif
		else
		{
			//DbgPrint("error, undefined btInfo=0x%x\n", btInfo);
			pCoexDm->btStatus = BT_8812A_1ANT_BT_STATUS_MAX;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), bt non-defined state!!!\n"));
		}

		// send delete BA to disable aggregation
		//pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_TO_REJ_AP_AGG_PKT, &bRejApAggPkt);
	}

	if( (BT_8812A_1ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus) ||
		(BT_8812A_1ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
		(BT_8812A_1ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) )
	{
		bBtBusy = TRUE;
	}
	else
	{
		bBtBusy = FALSE;
	}
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_TRAFFIC_BUSY, &bBtBusy);

	if(bBtBusy)
	{
		bLimitedDig = TRUE;
	}
	else
	{
		bLimitedDig = FALSE;
	}
	pCoexDm->bLimitedDig = bLimitedDig;
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_LIMITED_DIG, &bLimitedDig);

	halbtc8812a1ant_RunCoexistMechanism(pBtCoexist);
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify()<===\n"));
}

VOID
EXhalbtc8812a1ant_StackOperationNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
	)
{
	if(BTC_STACK_OP_INQ_PAGE_PAIR_START == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], StackOP Inquiry/page/pair start notify\n"));
	}
	else if(BTC_STACK_OP_INQ_PAGE_PAIR_FINISH == type)
	{
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], StackOP Inquiry/page/pair finish notify\n"));
	}
}

VOID
EXhalbtc8812a1ant_HaltNotify(
	IN	PBTC_COEXIST			pBtCoexist
	)
{
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], Halt notify\n"));

	halbtc8812a1ant_IgnoreWlanAct(pBtCoexist, FORCE_EXEC, TRUE);
	halbtc8812a1ant_PsTdma(pBtCoexist, FORCE_EXEC, FALSE, 0);
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x4f, 0xf);
	halbtc8812a1ant_WifiParaAdjust(pBtCoexist, FALSE);
}

VOID
EXhalbtc8812a1ant_PnpNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				pnpState
	)
{
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], Pnp notify\n"));

	if(BTC_WIFI_PNP_SLEEP == pnpState)
	{
		pBtCoexist->bStopCoexDm = TRUE;
		halbtc8812a1ant_IgnoreWlanAct(pBtCoexist, FORCE_EXEC, TRUE);
		halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
		halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
	}
	else if(BTC_WIFI_PNP_WAKE_UP == pnpState)
	{

	}
}

VOID
EXhalbtc8812a1ant_Periodical(
	IN	PBTC_COEXIST			pBtCoexist
	)
{
	BOOLEAN			bWifiUnder5G=FALSE;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Periodical()===>\n"));
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], 1Ant Periodical!!\n"));

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_UNDER_5G, &bWifiUnder5G);

	if(bWifiUnder5G)
	{
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Periodical(), return for 5G<===\n"));
		halbtc8812a1ant_CoexAllOff(pBtCoexist);
		return;
	}

	halbtc8812a1ant_QueryBtInfo(pBtCoexist);
	halbtc8812a1ant_MonitorBtCtr(pBtCoexist);
	halbtc8812a1ant_MonitorBtEnableDisable(pBtCoexist);
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Periodical()<===\n"));
}

VOID
EXhalbtc8812a1ant_DbgControl(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				opCode,
	IN	u1Byte				opLen,
	IN	pu1Byte				pData
	)
{
	switch(opCode)
	{
		case BTC_DBG_SET_COEX_NORMAL:
			pBtCoexist->bManualControl = FALSE;
			halbtc8812a1ant_InitCoexDm(pBtCoexist);
			break;
		case BTC_DBG_SET_COEX_WIFI_ONLY:
			pBtCoexist->bManualControl = TRUE;
			halbtc8812a1ant_PsTdmaCheckForPowerSaveState(pBtCoexist, FALSE);
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_LEAVE_LPS, NULL);
			halbtc8812a1ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 9);
			break;
		case BTC_DBG_SET_COEX_BT_ONLY:
			// todo
			break;
		default:
			break;
	}
}
#endif

