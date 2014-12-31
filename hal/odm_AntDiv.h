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
 
#ifndef	__ODMANTDIV_H__
#define    __ODMANTDIV_H__



#define ANT1_2G 0 // = ANT2_5G
#define ANT2_2G 1 // = ANT1_5G

//Antenna Diversty Control Type
#define	ODM_AUTO_ANT	0
#define	ODM_FIX_MAIN_ANT	1
#define	ODM_FIX_AUX_ANT	2

#define	TX_BY_REG	0

#define ODM_ANTDIV_SUPPORT		(ODM_RTL8723B)
#define ODM_N_ANTDIV_SUPPORT		(ODM_RTL8723B)

#define ODM_OLD_IC_ANTDIV_SUPPORT		(0)

#define ODM_ANTDIV_2G_SUPPORT_IC			(ODM_RTL8723B)
#define ODM_ANTDIV_5G_SUPPORT_IC			(0)
#define ODM_ANTDIV_2G	BIT0
#define ODM_ANTDIV_5G	BIT1

#define ANTDIV_ON 1
#define ANTDIV_OFF 0

#define INIT_ANTDIV_TIMMER 0
#define CANCEL_ANTDIV_TIMMER 1
#define RELEASE_ANTDIV_TIMMER 2

void
ODM_StopAntennaSwitchDm(
	IN	PDM_ODM_T	pDM_Odm
	);
void
ODM_SetAntConfig(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte		antSetting	// 0=A, 1=B, 2=C, ....
	);

#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))

#define SwAntDivRestAfterLink	ODM_SwAntDivRestAfterLink
void ODM_SwAntDivRestAfterLink(	IN	PDM_ODM_T	pDM_Odm);

void
ODM_UpdateRxIdleAnt(
	IN	 	PDM_ODM_T 		pDM_Odm, 
	IN		 u1Byte			Ant
);

void
odm_AntselStatistics(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			MacId,
	IN		u4Byte			RxPWDBAll
);

void
ODM_SW_AntDiv_Callback(void *FunctionContext);

void
odm_S0S1_SwAntDivByCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Step
	);

void
odm_AntselStatisticsOfCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			RxPWDBAll
);

void
odm_S0S1_SwAntDivByCtrlFrame_ProcessRSSI(
	IN		PDM_ODM_T				pDM_Odm,
	IN		PODM_PHY_INFO_T		pPhyInfo,
	IN		PODM_PACKET_INFO_T		pPktinfo
	);

void
ODM_AntDivInit(
	IN		 PDM_ODM_T		pDM_Odm 
);

void
ODM_AntDivReset(
	IN		PDM_ODM_T		pDM_Odm 
);

void
ODM_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
);

void
ODM_Process_RSSIForAntDiv(	
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN		PODM_PHY_INFO_T				pPhyInfo,
	IN		PODM_PACKET_INFO_T			pPktinfo
);

void
ODM_SetTxAntByTxInfo(
	IN		PDM_ODM_T		pDM_Odm,
	IN		pu1Byte			pDesc,
	IN		u1Byte			macId	
);

void
ODM_AntDiv_Config(
	IN		PDM_ODM_T		pDM_Odm
);


void
ODM_UpdateRxIdleAnt_8723B(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Ant,
	IN		u4Byte			DefaultAnt, 
	IN		u4Byte			OptionalAnt
);

void
ODM_AntDivTimers(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte			state
);

#endif //#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))
#endif //#ifndef	__ODMANTDIV_H__
