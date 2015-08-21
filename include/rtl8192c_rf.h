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
#ifndef _RTL8192C_RF_H_
#define _RTL8192C_RF_H_


/*  */
/*  RF RL6052 Series API */
/*  */
void		rtl8192c_RF_ChangeTxPath(	IN	struct adapter *	Adapter,
										IN	u16		DataRate);
void		rtl8192c_PHY_RF6052SetBandwidth(
										IN	struct adapter *				Adapter,
										IN	CHANNEL_WIDTH		Bandwidth);
void	rtl8192c_PHY_RF6052SetCckTxPower(
										IN	struct adapter *	Adapter,
										IN	u8*		pPowerlevel);
void	rtl8192c_PHY_RF6052SetOFDMTxPower(
										IN	struct adapter *	Adapter,
										IN	u8*		pPowerLevel,
										IN	u8		Channel);
int	PHY_RF6052_Config8192C(	IN	struct adapter *		Adapter	);

/*--------------------------Exported Function prototype---------------------*/


#endif/* End of HalRf.h */
