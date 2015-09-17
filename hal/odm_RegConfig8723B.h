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
 ******************************************************************************/
#ifndef __INC_ODM_REGCONFIG_H_8723B
#define __INC_ODM_REGCONFIG_H_8723B

void
odm_ConfigRFReg_8723B(
PDM_ODM_T				pDM_Odm,
u4Byte					Addr,
u4Byte					Data,
 ODM_RF_RADIO_PATH_E     RF_PATH,
u4Byte				    RegAddr
	);

void
odm_ConfigRF_RadioA_8723B(
PDM_ODM_T				pDM_Odm,
u4Byte					Addr,
u4Byte					Data
	);

void
odm_ConfigMAC_8723B(
PDM_ODM_T	pDM_Odm,
u4Byte		Addr,
u1Byte		Data
	);

void
odm_ConfigBB_AGC_8723B(
    IN	PDM_ODM_T	pDM_Odm,
    IN	u4Byte		Addr,
    IN	u4Byte		Bitmask,
    IN	u4Byte		Data
   );

void
odm_ConfigBB_PHY_REG_PG_8723B(
PDM_ODM_T	pDM_Odm,
u4Byte		Band,
u4Byte		RfPath,
u4Byte		TxNum,
    IN	u4Byte		Addr,
    IN	u4Byte		Bitmask,
    IN	u4Byte		Data
   );

void
odm_ConfigBB_PHY_8723B(
PDM_ODM_T	pDM_Odm,
    IN	u4Byte		Addr,
    IN	u4Byte		Bitmask,
    IN	u4Byte		Data
   );

void
odm_ConfigBB_TXPWR_LMT_8723B(
PDM_ODM_T	pDM_Odm,
pu1Byte		Regulation,
pu1Byte		Band,
pu1Byte		Bandwidth,
pu1Byte		RateSection,
pu1Byte		RfPath,
pu1Byte		Channel,
pu1Byte		PowerLimit
   );

#endif
