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

#ifndef __INC_MP_FW_HW_IMG_8723B_H
#define __INC_MP_FW_HW_IMG_8723B_H


/******************************************************************************
*                           FW_AP.TXT
******************************************************************************/

void
ODM_ReadFirmware_MP_8723B_FW_AP_WoWLAN(
     PDM_ODM_T    pDM_Odm,
     u8       *pFirmware,
     u32       *pFirmwareSize
);

/******************************************************************************
*                           FW_BT.TXT
******************************************************************************/

void
ODM_ReadFirmware_MP_8723B_FW_BT(
     PDM_ODM_T    pDM_Odm,
     u8       *pFirmware,
     u32       *pFirmwareSize
);

/******************************************************************************
*                           FW_NIC.TXT
******************************************************************************/

void
ODM_ReadFirmware_MP_8723B_FW_NIC(
     PDM_ODM_T    pDM_Odm,
     u8       *pFirmware,
     u32       *pFirmwareSize
);

/******************************************************************************
*                           FW_WoWLAN.TXT
******************************************************************************/

void
ODM_ReadFirmware_MP_8723B_FW_WoWLAN(
     PDM_ODM_T    pDM_Odm,
     u8       *pFirmware,
     u32       *pFirmwareSize
);

#endif
