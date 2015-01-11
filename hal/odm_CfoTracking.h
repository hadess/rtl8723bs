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

#ifndef	__ODMCFOTRACK_H__
#define    __ODMCFOTRACK_H__

#define		CFO_TH_XTAL_HIGH			20			// kHz
#define		CFO_TH_XTAL_LOW			10			// kHz
#define		CFO_TH_ATC					80			// kHz

typedef struct _CFO_TRACKING_
{
	bool			bATCStatus;
	bool			largeCFOHit;
	bool			bAdjust;
	u1Byte			CrystalCap;
	u1Byte			DefXCap;
	int				CFO_tail[2];
	int				CFO_ave_pre;
	u4Byte			packetCount;
	u4Byte			packetCount_pre;

	bool			bForceXtalCap;
	bool			bReset;
}CFO_TRACKING, *PCFO_TRACKING;

void
ODM_CfoTrackingReset(
	IN		void *					pDM_VOID
);

void
ODM_CfoTrackingInit(
	IN		void *					pDM_VOID
);

void
ODM_CfoTracking(
	IN		void *					pDM_VOID
);

void
ODM_ParsingCFO(
	IN		void *					pDM_VOID,
	IN		void *					pPktinfo_VOID,
	IN     	s8* 					pcfotail
);

#endif
