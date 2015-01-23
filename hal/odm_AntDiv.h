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

#endif //#ifndef	__ODMANTDIV_H__
