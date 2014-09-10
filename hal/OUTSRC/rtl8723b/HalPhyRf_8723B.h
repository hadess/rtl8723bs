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

#ifndef __HAL_PHY_RF_8723B_H__
#define __HAL_PHY_RF_8723B_H__

/*--------------------------Define Parameters-------------------------------*/
#define	IQK_DELAY_TIME_8723B		10		//ms
#define	IQK_DEFERRED_TIME_8723B		4
#define	index_mapping_NUM_8723B		15
#define	AVG_THERMAL_NUM_8723B		4
#define	RF_T_METER_8723B					0x42	//


void ConfigureTxpowerTrack_8723B(
	PTXPWRTRACK_CFG	pConfig
	);

void DoIQK_8723B(
	PDM_ODM_T	pDM_Odm,
	u1Byte 		DeltaThermalIndex,
	u1Byte		ThermalValue,
	u1Byte 		Threshold
	);

VOID
ODM_TxPwrTrackSetPwr_8723B(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD 	Method,
	u1Byte 				RFPath,
	u1Byte 				ChannelMappedIndex
	);

//1 7.	IQK

void
PHY_IQCalibrate_8723B(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN PADAPTER	Adapter,
#endif
	IN	BOOLEAN 	bReCovery);


//
// LC calibrate
//
VOID
PHY_LCCalibrate_8723B(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm
#else
	IN	PADAPTER	pAdapter
#endif
	);

VOID
_PHY_LCCalibrate_8723B(
	IN PDM_ODM_T		pDM_Odm
	);


void PHY_SetRFPathSwitch_8723B(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain
	);


//
// AP calibrate
//
void
PHY_APCalibrate_8723B(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
							IN 	s1Byte		delta);

#endif	// #ifndef __HAL_PHY_RF_8188E_H__

