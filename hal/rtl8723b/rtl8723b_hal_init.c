/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#define _HAL_INIT_C_

#include <drv_types.h>
#include <rtw_byteorder.h>
#include <rtw_efuse.h>
#include <rtw_btcoex.h>

#include <rtl8723b_hal.h>
#include "rtw_bt_mp.h"
#include "rtl8723b_cmd.h"
#include "rtl8723b_pg.h"
#include <halbt.h>

static VOID
_FWDownloadEnable(
	IN	PADAPTER		padapter,
	IN	BOOLEAN			enable
	)
{
	u8	tmp, count = 0;

	if(enable)
	{
		// 8051 enable
		tmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
		rtw_write8(padapter, REG_SYS_FUNC_EN+1, tmp|0x04);

		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp|0x01);

		do{
			tmp = rtw_read8(padapter, REG_MCUFWDL);
			if(tmp & 0x01)
				break;
			rtw_write8(padapter, REG_MCUFWDL, tmp|0x01);
			rtw_msleep_os(1);
		}while(count++<100);
		if(count > 0)
			DBG_871X("%s: !!!!!!!!Write 0x80 Fail!: count = %d\n", __func__, count);

		// 8051 reset
		tmp = rtw_read8(padapter, REG_MCUFWDL+2);
		rtw_write8(padapter, REG_MCUFWDL+2, tmp&0xf7);
	}
	else
	{
		// MCU firmware download disable.
		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp&0xfe);
	}
}

static int
_BlockWrite(
	IN		PADAPTER		padapter,
	IN		PVOID		buffer,
	IN		u32			buffSize
	)
{
	int ret = _SUCCESS;

	u32			blockSize_p1 = 4;	// (Default) Phase #1 : PCI muse use 4-byte write to download FW
	u32			blockSize_p2 = 8;	// Phase #2 : Use 8-byte, if Phase#1 use big size to write FW.
	u32			blockSize_p3 = 1;	// Phase #3 : Use 1-byte, the remnant of FW image.
	u32			blockCount_p1 = 0, blockCount_p2 = 0, blockCount_p3 = 0;
	u32			remainSize_p1 = 0, remainSize_p2 = 0;
	u8			*bufferPtr	= (u8*)buffer;
	u32			i=0, offset=0;

#ifdef CONFIG_USB_HCI
	blockSize_p1 = 254;
#endif

//	printk("====>%s %d\n", __func__, __LINE__);

	//3 Phase #1
	blockCount_p1 = buffSize / blockSize_p1;
	remainSize_p1 = buffSize % blockSize_p1;

	if (blockCount_p1) {
		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P1] buffSize(%d) blockSize_p1(%d) blockCount_p1(%d) remainSize_p1(%d)\n",
				buffSize, blockSize_p1, blockCount_p1, remainSize_p1));
	}

	for (i = 0; i < blockCount_p1; i++)
	{
#ifdef CONFIG_USB_HCI
		ret = rtw_writeN(padapter, (FW_8723B_START_ADDRESS + i * blockSize_p1), blockSize_p1, (bufferPtr + i * blockSize_p1));
#else
		ret = rtw_write32(padapter, (FW_8723B_START_ADDRESS + i * blockSize_p1), le32_to_cpu(*((u32*)(bufferPtr + i * blockSize_p1))));
#endif
		if(ret == _FAIL) {
			printk("====>%s %d i:%d\n", __func__, __LINE__, i);
			goto exit;
		}
	}

	//3 Phase #2
	if (remainSize_p1)
	{
		offset = blockCount_p1 * blockSize_p1;

		blockCount_p2 = remainSize_p1/blockSize_p2;
		remainSize_p2 = remainSize_p1%blockSize_p2;

		if (blockCount_p2) {
				RT_TRACE(_module_hal_init_c_, _drv_notice_,
						("_BlockWrite: [P2] buffSize_p2(%d) blockSize_p2(%d) blockCount_p2(%d) remainSize_p2(%d)\n",
						(buffSize-offset), blockSize_p2 ,blockCount_p2, remainSize_p2));
		}

#ifdef CONFIG_USB_HCI
		for (i = 0; i < blockCount_p2; i++) {
			ret = rtw_writeN(padapter, (FW_8723B_START_ADDRESS + offset + i*blockSize_p2), blockSize_p2, (bufferPtr + offset + i*blockSize_p2));

			if(ret == _FAIL)
				goto exit;
		}
#endif
	}

	//3 Phase #3
	if (remainSize_p2)
	{
		offset = (blockCount_p1 * blockSize_p1) + (blockCount_p2 * blockSize_p2);

		blockCount_p3 = remainSize_p2 / blockSize_p3;

		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P3] buffSize_p3(%d) blockSize_p3(%d) blockCount_p3(%d)\n",
				(buffSize-offset), blockSize_p3, blockCount_p3));

		for(i = 0 ; i < blockCount_p3 ; i++){
			ret = rtw_write8(padapter, (FW_8723B_START_ADDRESS + offset + i), *(bufferPtr + offset + i));

			if(ret == _FAIL) {
				printk("====>%s %d i:%d\n", __func__, __LINE__, i);
				goto exit;
			}
		}
	}
exit:
	return ret;
}

static int
_PageWrite(
	IN		PADAPTER	padapter,
	IN		u32			page,
	IN		PVOID		buffer,
	IN		u32			size
	)
{
	u8 value8;
	u8 u8Page = (u8) (page & 0x07) ;

	value8 = (rtw_read8(padapter, REG_MCUFWDL+2) & 0xF8) | u8Page ;
	rtw_write8(padapter, REG_MCUFWDL+2,value8);

	return _BlockWrite(padapter,buffer,size);
}

static VOID
_FillDummy(
	u8*		pFwBuf,
	u32*	pFwLen
	)
{
	u32	FwLen = *pFwLen;
	u8	remain = (u8)(FwLen%4);
	remain = (remain==0)?0:(4-remain);

	while(remain>0)
	{
		pFwBuf[FwLen] = 0;
		FwLen++;
		remain--;
	}

	*pFwLen = FwLen;
}

static int
_WriteFW(
	IN		PADAPTER		padapter,
	IN		PVOID			buffer,
	IN		u32			size
	)
{
	// Since we need dynamic decide method of dwonload fw, so we call this function to get chip version.
	// We can remove _ReadChipVersion from ReadpadapterInfo8192C later.
	int ret = _SUCCESS;
	u32 	pageNums,remainSize ;
	u32 	page, offset;
	u8		*bufferPtr = (u8*)buffer;

#ifdef CONFIG_PCI_HCI
	// 20100120 Joseph: Add for 88CE normal chip.
	// Fill in zero to make firmware image to dword alignment.
	_FillDummy(bufferPtr, &size);
#endif

	pageNums = size / MAX_PAGE_SIZE ;
	//RT_ASSERT((pageNums <= 4), ("Page numbers should not greater then 4 \n"));
	remainSize = size % MAX_PAGE_SIZE;

	for (page = 0; page < pageNums; page++) {
		offset = page * MAX_PAGE_SIZE;
		ret = _PageWrite(padapter, page, bufferPtr+offset, MAX_PAGE_SIZE);

		if(ret == _FAIL) {
			printk("====>%s %d\n", __func__, __LINE__);
			goto exit;
		}
	}
	if (remainSize) {
		offset = pageNums * MAX_PAGE_SIZE;
		page = pageNums;
		ret = _PageWrite(padapter, page, bufferPtr+offset, remainSize);

		if(ret == _FAIL) {
			printk("====>%s %d\n", __func__, __LINE__);
			goto exit;
		}
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("_WriteFW Done- for Normal chip.\n"));

exit:
	return ret;
}

#if 0
void _8051Reset8723(PADAPTER padapter)
{
	u8 u1bTmp;

	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp&(~BIT2));
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp|(BIT2));
	DBG_871X("=====> _8051Reset8723(): 8051 reset success .\n");

}
#endif

static s32 _FWFreeToGo(PADAPTER padapter)
{
	u32	counter = 0;
	u32	value32;

	// polling CheckSum report
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & FWDL_ChkSum_rpt) break;
	} while (counter++ < POLLING_READY_TIMEOUT_COUNT);

	if (counter >= POLLING_READY_TIMEOUT_COUNT) {
		RT_TRACE(_module_hal_init_c_, _drv_err_, ("%s: chksum report fail! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32));
		return _FAIL;
	}
	RT_TRACE(_module_hal_init_c_, _drv_notice_, ("%s: Checksum report OK! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32));

	value32 = rtw_read32(padapter, REG_MCUFWDL);
	value32 |= MCUFWDL_RDY;
	value32 &= ~WINTINI_RDY;
	rtw_write32(padapter, REG_MCUFWDL, value32);

	//_8051Reset8723(padapter);
	rtl8723b_FirmwareSelfReset(padapter);

	// polling for FW ready
	counter = 0;
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & WINTINI_RDY) {
			RT_TRACE(_module_hal_init_c_, _drv_info_, ("%s: Polling FW ready success!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32));
			return _SUCCESS;
		}
		rtw_udelay_os(5);
	} while (counter++ < POLLING_READY_TIMEOUT_COUNT);

	RT_TRACE(_module_hal_init_c_, _drv_err_, ("%s: Polling FW ready fail!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32));
	return _FAIL;
}

#define IS_FW_81xxC(padapter)	(((GET_HAL_DATA(padapter))->FirmwareSignature & 0xFFF0) == 0x88C0)

void rtl8723b_FirmwareSelfReset(PADAPTER padapter)
{
	u8	u1bTmp;

	//0x1c[8] = 0
	u1bTmp = rtw_read8(padapter, REG_RSV_CTRL+1);
	u1bTmp &= ~BIT0;
	rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp);

	//0x3[2] = 0
	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
	u1bTmp &= ~BIT2;
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp);

	//0x1c[8] = 1
	u1bTmp = rtw_read8(padapter, REG_RSV_CTRL+1);
	u1bTmp |= BIT0;
	rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp);

	//0x3[2] = 1
	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
	u1bTmp |= BIT2;
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp);
}

//
#ifdef CONFIG_MP_INCLUDED

int _WriteBTFWtoTxPktBuf8723B(
	IN		PADAPTER		Adapter,
	IN		PVOID			buffer,
	IN		u4Byte			FwBufLen,
	IN		u1Byte			times
	)
{
	int			rtStatus = _SUCCESS;
	//u4Byte				value32;
	//u1Byte				numHQ, numLQ, numPubQ;//, txpktbuf_bndy;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	u1Byte				BcnValidReg;
	u1Byte				count=0, DLBcnCount=0;
	pu1Byte 			FwbufferPtr = (pu1Byte)buffer;
	//PRT_TCB 			pTcb, ptempTcb;
	//PRT_TX_LOCAL_BUFFER pBuf;
	BOOLEAN 			bRecover=_FALSE;
	pu1Byte 			ReservedPagePacket = NULL;
	pu1Byte 			pGenBufReservedPagePacket = NULL;
	u4Byte				TotalPktLen;
	//u1Byte				tmpReg422;
	//u1Byte				u1bTmp;
	u8 			*pframe;
	struct xmit_priv	*pxmitpriv = &(Adapter->xmitpriv);
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	u8 			txdesc_offset = TXDESC_OFFSET;

#if 1//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	TotalPktLen = FwBufLen;
#else
	TotalPktLen = FwBufLen+pHalData->HWDescHeadLength;
#endif
	pGenBufReservedPagePacket = rtw_zmalloc(TotalPktLen);//GetGenTempBuffer (Adapter, TotalPktLen);
	if (!pGenBufReservedPagePacket)
		return _FAIL;

	ReservedPagePacket = (u1Byte *)pGenBufReservedPagePacket;

	_rtw_memset(ReservedPagePacket, 0, TotalPktLen);

#if 1//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	_rtw_memcpy(ReservedPagePacket, FwbufferPtr, FwBufLen);

#else
	PlatformMoveMemory(ReservedPagePacket+Adapter->HWDescHeadLength , FwbufferPtr, FwBufLen);
#endif

	//---------------------------------------------------------
	// 1. Pause BCN
	//---------------------------------------------------------
	//Set REG_CR bit 8. DMA beacon by SW.
#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	u1bTmp = PlatformEFIORead1Byte(Adapter, REG_CR+1);
	PlatformEFIOWrite1Byte(Adapter,  REG_CR+1, (u1bTmp|BIT0));
#else
	// Remove for temparaily because of the code on v2002 is not sync to MERGE_TMEP for USB/SDIO.
	// De not remove this part on MERGE_TEMP. by tynli.
	//pHalData->RegCR_1 |= (BIT0);
	//PlatformEFIOWrite1Byte(Adapter,  REG_CR+1, pHalData->RegCR_1);
#endif

	// Disable Hw protection for a time which revserd for Hw sending beacon.
	// Fix download reserved page packet fail that access collision with the protection time.
	// 2010.05.11. Added by tynli.
	SetBcnCtrlReg(Adapter, 0, BIT(3));
	SetBcnCtrlReg(Adapter, BIT(4), 0);

#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	tmpReg422 = PlatformEFIORead1Byte(Adapter, REG_FWHW_TXQ_CTRL+2);
	if( tmpReg422&BIT6)
		bRecover = TRUE;
	PlatformEFIOWrite1Byte(Adapter, REG_FWHW_TXQ_CTRL+2,  tmpReg422&(~BIT6));
#else
	// Set FWHW_TXQ_CTRL 0x422[6]=0 to tell Hw the packet is not a real beacon frame.
	if(pHalData->RegFwHwTxQCtrl & BIT(6))
		bRecover=_TRUE;
	PlatformEFIOWrite1Byte(Adapter, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl&(~BIT(6))));
	pHalData->RegFwHwTxQCtrl &= (~ BIT(6));
#endif

	//---------------------------------------------------------
	// 2. Adjust LLT table to an even boundary.
	//---------------------------------------------------------
#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	if(LLT_table_init(Adapter, TRUE, 10) == RT_STATUS_FAILURE)
	{
		DbgPrint("Init self define for BT Fw patch LLT table fail.\n");
		return RT_STATUS_FAILURE;
	}
#endif

	//---------------------------------------------------------
	// 3. Write Fw to Tx packet buffer by reseverd page.
	//---------------------------------------------------------
	do
	{
		// download rsvd page.
		// Clear beacon valid check bit.
		BcnValidReg = PlatformEFIORead1Byte(Adapter, REG_TDECTRL+2);
		PlatformEFIOWrite1Byte(Adapter, REG_TDECTRL+2, BcnValidReg&(~BIT(0)));

		//BT patch is big, we should set 0x209 < 0x40 suggested from Gimmy
		RT_TRACE(_module_mp_, _drv_info_,("0x209:%x\n",
					PlatformEFIORead1Byte(Adapter, REG_TDECTRL+1)));//209 < 0x40

		PlatformEFIOWrite1Byte(Adapter, REG_TDECTRL+1, (0x90-0x20*(times-1)));
		DBG_871X("0x209:0x%x\n", PlatformEFIORead1Byte(Adapter, REG_TDECTRL+1));
		RT_TRACE(_module_mp_, _drv_info_,("0x209:%x\n",
					PlatformEFIORead1Byte(Adapter, REG_TDECTRL+1)));

#if 0
		// Acquice TX spin lock before GetFwBuf and send the packet to prevent system deadlock.
		// Advertised by Roger. Added by tynli. 2010.02.22.
		PlatformAcquireSpinLock(Adapter, RT_TX_SPINLOCK);
		if(MgntGetFWBuffer(Adapter, &pTcb, &pBuf))
		{
			PlatformMoveMemory(pBuf->Buffer.VirtualAddress, ReservedPagePacket, TotalPktLen);
			CmdSendPacket(Adapter, pTcb, pBuf, TotalPktLen, DESC_PACKET_TYPE_NORMAL, FALSE);
		}
		else
			dbgdump("SetFwRsvdPagePkt(): MgntGetFWBuffer FAIL!!!!!!!!.\n");
		PlatformReleaseSpinLock(Adapter, RT_TX_SPINLOCK);
#else
		/*---------------------------------------------------------
		tx reserved_page_packet
		----------------------------------------------------------*/
			if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL) {
						rtStatus = _FAIL;
						goto exit;
			}
			//update attribute
			pattrib = &pmgntframe->attrib;
			update_mgntframe_attrib(Adapter, pattrib);

			pattrib->qsel = QSLT_BEACON;
			pattrib->pktlen = pattrib->last_txcmdsz = FwBufLen ;

			//_rtw_memset(pmgntframe->buf_addr, 0, TotalPktLen+txdesc_size);
			//pmgntframe->buf_addr = ReservedPagePacket ;

		_rtw_memcpy( (u8*) (pmgntframe->buf_addr + txdesc_offset), ReservedPagePacket, FwBufLen);
		DBG_871X("[%d]===>TotalPktLen + TXDESC_OFFSET TotalPacketLen:%d \n", DLBcnCount, (FwBufLen + txdesc_offset));

		     dump_mgntframe(Adapter, pmgntframe);

#endif
#if 1
		// check rsvd page download OK.
		BcnValidReg = PlatformEFIORead1Byte(Adapter, REG_TDECTRL+2);
		while(!(BcnValidReg & BIT(0)) && count <200)
		{
			count++;
			//PlatformSleepUs(10);
			rtw_msleep_os(1);
			BcnValidReg = PlatformEFIORead1Byte(Adapter, REG_TDECTRL+2);
			RT_TRACE(_module_mp_, _drv_notice_,("Poll 0x20A = %x\n", BcnValidReg));
		}
		DLBcnCount++;
		//DBG_871X("##0x208:%08x,0x210=%08x\n",PlatformEFIORead4Byte(Adapter, REG_TDECTRL),PlatformEFIORead4Byte(Adapter, 0x210));
	}while((!(BcnValidReg&BIT(0))) && DLBcnCount<5);


#endif
	if(DLBcnCount >=5){
		DBG_871X(" check rsvd page download OK DLBcnCount =%d  \n",DLBcnCount);
		rtStatus = _FAIL;
		goto exit;
	}

	if(!(BcnValidReg&BIT(0)))
	{
		DBG_871X("_WriteFWtoTxPktBuf(): 1 Download RSVD page failed!\n");
		rtStatus = _FAIL;
		goto exit;
	}

	//---------------------------------------------------------
	// 4. Set Tx boundary to the initial value
	//---------------------------------------------------------


	//---------------------------------------------------------
	// 5. Reset beacon setting to the initial value.
	//	 After _CheckWLANFwPatchBTFwReady().
	//---------------------------------------------------------

exit:

	if(pGenBufReservedPagePacket)
	{
		DBG_871X("_WriteBTFWtoTxPktBuf8723B => rtw_mfree pGenBufReservedPagePacket!\n");
		rtw_mfree((u8*)pGenBufReservedPagePacket, TotalPktLen);
		}
	return rtStatus;
}



//
// Description: Determine the contents of H2C BT_FW_PATCH Command sent to FW.
// 2011.10.20 by tynli
//
void
SetFwBTFwPatchCmd(
	IN PADAPTER	Adapter,
	IN u16		FwSize
	)
{
	u8 u1BTFwPatchParm[H2C_8723B_BT_FW_PATCH_LEN]={0};
	u8 addr0 = 0;
	u8 addr1 = 0xa0;
	u8 addr2 = 0x10;
	u8 addr3 = 0x80;

	RT_TRACE(_module_mp_, _drv_notice_,("SetFwBTFwPatchCmd(): FwSize = %d\n", FwSize));

	SET_8723B_H2CCMD_BT_FW_PATCH_SIZE(u1BTFwPatchParm, FwSize);
	SET_8723B_H2CCMD_BT_FW_PATCH_ADDR0(u1BTFwPatchParm, addr0);
	SET_8723B_H2CCMD_BT_FW_PATCH_ADDR1(u1BTFwPatchParm, addr1);
	SET_8723B_H2CCMD_BT_FW_PATCH_ADDR2(u1BTFwPatchParm, addr2);
	SET_8723B_H2CCMD_BT_FW_PATCH_ADDR3(u1BTFwPatchParm, addr3);

	FillH2CCmd8723B(Adapter, H2C_8723B_BT_FW_PATCH, H2C_8723B_BT_FW_PATCH_LEN, u1BTFwPatchParm);

	RT_TRACE(_module_mp_, _drv_notice_,("<----SetFwBTFwPatchCmd(): FwSize = %d \n", FwSize));
}

void
SetFwBTPwrCmd(
	IN PADAPTER	Adapter,
	IN u1Byte	PwrIdx
	)
{
	u1Byte		u1BTPwrIdxParm[H2C_8723B_FORCE_BT_TXPWR_LEN]={0};

	RT_TRACE(_module_mp_, _drv_info_,("SetFwBTPwrCmd(): idx = %d\n", PwrIdx));
	SET_8723B_H2CCMD_BT_PWR_IDX(u1BTPwrIdxParm, PwrIdx);

	RT_TRACE(_module_mp_, _drv_info_,("SetFwBTPwrCmd(): %x %x %x\n",
		u1BTPwrIdxParm[0],u1BTPwrIdxParm[1],u1BTPwrIdxParm[2]));

	FillH2CCmd8723B(Adapter, H2C_8723B_FORCE_BT_TXPWR, H2C_8723B_FORCE_BT_TXPWR_LEN, u1BTPwrIdxParm);
}

//
// Description: WLAN Fw will write BT Fw to BT XRAM and signal driver.
//
// 2011.10.20. by tynli.
//
int
_CheckWLANFwPatchBTFwReady(
	IN	PADAPTER			Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte	count=0;
	u1Byte	u1bTmp;
	int ret = _FAIL;

	//---------------------------------------------------------
	// Check if BT FW patch procedure is ready.
	//---------------------------------------------------------
	do{
		u1bTmp = PlatformEFIORead1Byte(Adapter, REG_HMEBOX_DBG_0_8723B);
		if(u1bTmp&BIT(7))
		{
			ret = _SUCCESS;
			break;
		}

		count++;
		RT_TRACE(_module_mp_, _drv_info_,("0x81=%x, wait for 50 ms (%d) times.\n",
					u1bTmp, count));
		rtw_msleep_os(50); // 50ms
	}while(!(u1bTmp&BIT(7)) && count < 50);

	RT_TRACE(_module_mp_, _drv_notice_,("_CheckWLANFwPatchBTFwReady():"
				" Polling ready bit 0x88[7] for %d times.\n", count));

	if(count >= 50)
	{
		RT_TRACE(_module_mp_, _drv_notice_,("_CheckWLANFwPatchBTFwReady():"
				" Polling ready bit 0x88[7] FAIL!!\n"));
	}

	//---------------------------------------------------------
	// Reset beacon setting to the initial value.
	//---------------------------------------------------------
#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	if(LLT_table_init(Adapter, FALSE, 0) == RT_STATUS_FAILURE)
	{
		dbgdump("Init self define for BT Fw patch LLT table fail.\n");
		//return RT_STATUS_FAILURE;
	}
#endif
	SetBcnCtrlReg(Adapter, BIT(3), 0);
	SetBcnCtrlReg(Adapter, 0, BIT(4));

	// To make sure that if there exists an adapter which would like to send beacon.
	// If exists, the origianl value of 0x422[6] will be 1, we should check this to
	// prevent from setting 0x422[6] to 0 after download reserved page, or it will cause
	// the beacon cannot be sent by HW.
	// 2010.06.23. Added by tynli.
#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	u1bTmp = PlatformEFIORead1Byte(Adapter, REG_FWHW_TXQ_CTRL+2);
	PlatformEFIOWrite1Byte(Adapter, REG_FWHW_TXQ_CTRL+2, (u1bTmp|BIT6));
#else
	PlatformEFIOWrite1Byte(Adapter, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl|BIT(6)));
	pHalData->RegFwHwTxQCtrl |= BIT(6);
#endif

	// Clear CR[8] or beacon packet will not be send to TxBuf anymore.
#if 0//(DEV_BUS_TYPE == RT_PCI_INTERFACE)
	u1bTmp = PlatformEFIORead1Byte(Adapter, REG_CR+1);
	PlatformEFIOWrite1Byte(Adapter, REG_CR+1, (u1bTmp&(~BIT0)));
#else
	// Remove for temparaily because of the code on v2002 is not sync to MERGE_TMEP for USB/SDIO.
	// De not remove this part on MERGE_TEMP. by tynli.
	//pHalData->RegCR_1 &= (~BIT0);
	//PlatformEFIOWrite1Byte(Adapter,  REG_CR+1, pHalData->RegCR_1);
#endif
	return ret;
}

/* As the size of bt firmware is more than 16k which is too big for some platforms, we divide it
 * into four parts to transfer. The last parameter of _WriteBTFWtoTxPktBuf8723B is used to indicate
 * the location of every part. We call the first 4096 byte of bt firmware as part 1, the second 4096
 * part as part 2, the third 4096 part as part 3, the remain as part 4. First we transform the part
 * 4 and set the register 0x209 to 0x90, then the 32 bytes description are added to the head of part
 * 4, and those bytes are putted at the location 0x90. Second we transform the part 3 and set the
 * register 0x209 to 0x70. The 32 bytes description and part 3(4196 bytes) are putted at the location
 * 0x70. It can contain 4196 bytes between 0x70 and 0x90. So the last 32 bytes os part 3 will cover the
 * 32 bytes description of part4. Using this method, we can put the whole bt firmware to 0x30 and only
 * has 32 bytes descrption at the head of part 1.
*/
int
FirmwareDownloadBT(IN PADAPTER Adapter, PRT_FIRMWARE_8723B pFirmware)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	int rtStatus = _SUCCESS;

	// BT pacth
	u1Byte		*BTFwImage;
	u4Byte		BTFwImageLen;

	u1Byte		*pBTFirmwareBuf;
	u4Byte		BTFirmwareLen;
	int		download_time;
	int		i;

	//
	// Patch BT Fw. Download BT RAM code to Tx packet buffer.
	//
	if (Adapter->bBTFWReady){
		DBG_871X("BT Firmware is ready!!\n");
		return _FAIL;
	}
	BTFwImage = (pu1Byte)Rtl8723BFwBTImgArray;
	BTFwImageLen = Rtl8723BFwBTImgArrayLength;
		DBG_871X("BT Firmware is size= %zu!!\n",sizeof(Rtl8723BFwBTImgArray));

	// Download BT patch Fw.
	RT_TRACE(_module_mp_, _drv_info_,("Download BT Fw (patch version) from header.\n"));
	DBG_871X("Download BT Fw (patch version) from header.\n");

#ifdef CONFIG_EMBEDDED_FWIMG
		pFirmware->szBTFwBuffer = BTFwImage;
		DBG_871X("CONFIG_EMBEDDED_FWIMG pFirmware->szBTFwBuffer = BTFwImage;\n");
#else
		DBG_871X("_rtw_memcpy BTFwImage to pFirmware->szBTFwBuffer.\n");
		_rtw_memcpy(pFirmware->szBTFwBuffer, (PVOID)BTFwImage, BTFwImageLen);
#endif
	pFirmware->ulBTFwLength = BTFwImageLen;
		RT_TRACE(_module_mp_, _drv_notice_,("Download BT Fw (patch version) from header "
			"pFirmware->ulBTFwLength:%d.\n", pFirmware->ulBTFwLength));

	// BT FW
#if 0
	pBTFirmwareBuf = BTFwImage;
	//pFirmware->szBTFwBuffer;
	BTFirmwareLen = BTFwImageLen;
	//pFirmware->ulBTFwLength;
#else
	pBTFirmwareBuf = pFirmware->szBTFwBuffer;
	BTFirmwareLen = pFirmware->ulBTFwLength;

#endif
	//for h2c cam here should be set to  true
	Adapter->bFWReady = _TRUE;
	DBG_871X("FirmwareDownloadBT to _WriteBTFWtoTxPktBuf8723B !\n");

	download_time = BTFirmwareLen / 4096 + 1;
	DBG_871X("download_time is %d \n", download_time);
	for(i = (download_time-1); i >= 0; i--)
	{
		if( i == (download_time - 1))
		{
			rtStatus = _WriteBTFWtoTxPktBuf8723B(Adapter, pBTFirmwareBuf+(4096*i), (BTFirmwareLen-(4096*i)), 1);
			DBG_871X("start %d, len %d, time 1\n", 4096*i, BTFirmwareLen-(4096*i));
		}
		else
		{
			rtStatus = _WriteBTFWtoTxPktBuf8723B(Adapter, pBTFirmwareBuf+(4096*i), 4096, (download_time-i));
			DBG_871X("start %d, len 4096, time %d\n", 4096*i, download_time-i);
		}

		if(rtStatus != _SUCCESS)
		{
			DBG_871X("BT Firmware download to Tx packet buffer fail!\n" );
			return rtStatus;
		}
	}


	Adapter->bBTFWReady = _TRUE;
	SetFwBTFwPatchCmd(Adapter, (u2Byte)BTFirmwareLen);
	_CheckWLANFwPatchBTFwReady(Adapter);

	DBG_871X("<===FirmwareDownloadBT(),return %s!\n",rtStatus?"SUCCESS":"FAIL");
	return rtStatus;

}

#endif


#ifdef CONFIG_WOWLAN
#define ArrayLength_8723S_FW_WoWLAN   22132
extern const u8 Array_8723S_FW_WoWLAN[ArrayLength_8723S_FW_WoWLAN];
#endif //CONFIG_WOWLAN

//
//	Description:
//		Download 8192C firmware code.
//
//
s32 rtl8723b_FirmwareDownload(PADAPTER padapter, BOOLEAN  bUsedWoWLANFw)
{
	s32	rtStatus = _SUCCESS;
	u8 writeFW_retry = 0;
	u32 fwdl_start_time;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	s8 			R8723BFwImageFileName[] ={RTL8723B_FW_IMG};
	u8			*FwImage;
	u32			FwImageLen;
	u8			*pFwImageFileName;
#ifdef CONFIG_WOWLAN
	u8			*FwImageWoWLAN;
	u32			FwImageWoWLANLen;
#endif
	u8			*pucMappedFile = NULL;
	PRT_FIRMWARE_8723B	pFirmware = NULL;
	PRT_FIRMWARE_8723B	pBTFirmware = NULL;
	PRT_8723B_FIRMWARE_HDR		pFwHdr = NULL;
	u8			*pFirmwareBuf;
	u32			FirmwareLen;

	RT_TRACE(_module_hal_init_c_, _drv_info_, ("+%s\n", __FUNCTION__));
#ifdef CONFIG_WOWLAN
	RT_TRACE(_module_hal_init_c_, _drv_notice_, ("+%s, bUsedWoWLANFw:%d\n", __FUNCTION__,bUsedWoWLANFw));
#endif
	pFirmware = (PRT_FIRMWARE_8723B)rtw_zmalloc(sizeof(RT_FIRMWARE_8723B));
	pBTFirmware = (PRT_FIRMWARE_8723B)rtw_zmalloc(sizeof(RT_FIRMWARE_8723B));

	if(!pFirmware||!pBTFirmware)
	{
		rtStatus = _FAIL;
		goto Exit;
	}

#ifdef CONFIG_EMBEDDED_FWIMG
	pFirmware->eFWSource = FW_SOURCE_HEADER_FILE;
#else
	pFirmware->eFWSource = FW_SOURCE_IMG_FILE; // We should decided by Reg.
#endif

	switch(pFirmware->eFWSource)
	{
		case FW_SOURCE_IMG_FILE:
			//TODO:
			break;
		case FW_SOURCE_HEADER_FILE:
			ODM_ConfigFWWithHeaderFile(&pHalData->odmpriv, CONFIG_FW_NIC, (u8 *)&(pFirmware->szFwBuffer), &(pFirmware->ulFwLength));
			DBG_871X(" ===> %s fw:%s, size: %d\n", "Firmware for NIC",__FUNCTION__, pFirmware->ulFwLength);

#ifdef CONFIG_WOWLAN
			ODM_ConfigFWWithHeaderFile(&pHalData->odmpriv, CONFIG_FW_WoWLAN, (u8 *)&(pFirmware->szWoWLANFwBuffer), &(pFirmware->ulWoWLANFwLength));
			DBG_871X(" ===> %s fw:%s, size: %d\n", "Firmware for WoWLAN",__FUNCTION__, pFirmware->ulWoWLANFwLength);
#endif //CONFIG_WOWLAN

			if (pFirmware->ulFwLength > FW_8723B_SIZE) {
				rtStatus = _FAIL;
				RT_TRACE(_module_hal_init_c_, _drv_err_, ("Firmware size exceed 0x%X. Check it.\n", FW_8723B_SIZE) );
				goto Exit;
			}
			break;
	}

#ifdef CONFIG_WOWLAN
	if(bUsedWoWLANFw) {
		pFirmwareBuf = pFirmware->szWoWLANFwBuffer;
		FirmwareLen = pFirmware->ulWoWLANFwLength;
	} else
#endif //CONFIG_WOWLAN
	{
		pFirmwareBuf = pFirmware->szFwBuffer;
		FirmwareLen = pFirmware->ulFwLength;
	}

	// To Check Fw header. Added by tynli. 2009.12.04.
	pFwHdr = (PRT_8723B_FIRMWARE_HDR)pFirmwareBuf;

	pHalData->FirmwareVersion =  le16_to_cpu(pFwHdr->Version);
	pHalData->FirmwareSubVersion = pFwHdr->Subversion;
	pHalData->FirmwareSignature = le16_to_cpu(pFwHdr->Signature);

	DBG_871X("%s: fw_ver=%x fw_subver=%x sig=0x%x\n",
		  __FUNCTION__, pHalData->FirmwareVersion, pHalData->FirmwareSubVersion, pHalData->FirmwareSignature);

	if (IS_FW_HEADER_EXIST(pFwHdr))
	{
		DBG_871X("%s(): Shift for fw header!\n", __func__);
		// Shift 32 bytes for FW header
		pFirmwareBuf = pFirmwareBuf + 32;
		FirmwareLen = FirmwareLen - 32;
	}

	/* before download fw we should set REG_TCR BIT(6) BIT(7) to 0 */
	/* to avoid null 0 issue under LPS*/
	rtw_write8(padapter, REG_TCR, rtw_read8(padapter, REG_TCR) & 0x3F);

	// Suggested by Filen. If 8051 is running in RAM code, driver should inform Fw to reset by itself,
	// or it will cause download Fw fail. 2010.02.01. by tynli.
	if (rtw_read8(padapter, REG_MCUFWDL) & RAM_DL_SEL) //8051 RAM code
	{
		rtw_write8(padapter, REG_MCUFWDL, 0x04);
		rtl8723b_FirmwareSelfReset(padapter);
	}

	DBG_871X("rtl8723b_FirmwareDownload(): FwDownloadenable !\n");
	_FWDownloadEnable(padapter, _TRUE);
	fwdl_start_time = rtw_get_current_time();
	while(1) {
		//reset the FWDL chksum
		rtw_write8(padapter, REG_MCUFWDL, rtw_read8(padapter, REG_MCUFWDL)|FWDL_ChkSum_rpt);
		rtStatus = _WriteFW(padapter, pFirmwareBuf, FirmwareLen);

		if(rtStatus == _SUCCESS
			||(rtw_get_passing_time_ms(fwdl_start_time) > 500 && writeFW_retry++ >= 3)
		)
			break;

		DBG_871X("%s writeFW_retry:%u, time after fwdl_start_time:%ums\n", __FUNCTION__
			, writeFW_retry
			, rtw_get_passing_time_ms(fwdl_start_time)
		);
	}
	_FWDownloadEnable(padapter, _FALSE);
	if(_SUCCESS != rtStatus){
		DBG_871X("DL Firmware failed!\n");
		goto Exit;
	}

	DBG_871X("rtl8723b_FirmwareDownload(): Free to go !\n");

	rtStatus = _FWFreeToGo(padapter);
	if (_SUCCESS != rtStatus) {
		RT_TRACE(_module_hal_init_c_, _drv_err_, ("DL Firmware failed!\n"));
		goto Exit;
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("Firmware is ready to run!\n"));

#ifdef CONFIG_MP_INCLUDED//BT_MP
	if (padapter->registrypriv.mp_mode == 1)
	{
		rtw_write8(padapter, 0x81, rtw_read8(padapter, 0x81)|BIT0);
		DBG_871X("rtl8723b_FirmwareDownload go to FirmwareDownloadBT !\n");
		FirmwareDownloadBT(padapter, pBTFirmware);
	}
#endif

Exit:
	if (pFirmware)
		rtw_mfree((u8*)pFirmware, sizeof(RT_FIRMWARE_8723B));
	if (pBTFirmware)
		rtw_mfree((u8*)pBTFirmware, sizeof(RT_FIRMWARE_8723B));
	DBG_871X(" <=== rtl8723b_FirmwareDownload()\n");
	return rtStatus;
}

void rtl8723b_InitializeFirmwareVars(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);

	// Init Fw LPS related.
	adapter_to_pwrctl(padapter)->bFwCurrentInPSMode = _FALSE;

	//Init H2C cmd.
	rtw_write8(padapter, REG_HMETFR, 0x0f);

	// Init H2C counter. by tynli. 2009.12.09.
	pHalData->LastHMEBoxNum = 0;
//	pHalData->H2CQueueHead = 0;
//	pHalData->H2CQueueTail = 0;
//	pHalData->H2CStopInsertQueue = _FALSE;
}

#ifdef CONFIG_WOWLAN
//===========================================

//
// Description: Prepare some information to Fw for WoWLAN.
//					(1) Download wowlan Fw.
//					(2) Download RSVD page packets.
//					(3) Enable AP offload if needed.
//
// 2011.04.12 by tynli.
//
VOID
SetFwRelatedForWoWLAN8723b(
		IN		PADAPTER			padapter,
		IN		u8					bHostIsGoingtoSleep
)
{
		int				status=_FAIL;
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
		u8				bRecover = _FALSE;
	//
	// 1. Before WoWLAN we need to re-download WoWLAN Fw.
	//
	status = rtl8723b_FirmwareDownload(padapter, bHostIsGoingtoSleep);
	if(status != _SUCCESS)
	{
		DBG_871X("SetFwRelatedForWoWLAN8723b(): Try again!!\n");
		status = rtl8723b_FirmwareDownload(padapter, bHostIsGoingtoSleep);
	}
	if(status != _SUCCESS) {
		DBG_871X("SetFwRelatedForWoWLAN8723b(): Re-Download Firmware failed!!\n");
		return;
	} else {
		DBG_871X("SetFwRelatedForWoWLAN8723b(): Re-Download Firmware Success !!\n");
	}
	//
	// 2. Re-Init the variables about Fw related setting.
	//
	rtl8723b_InitializeFirmwareVars(padapter);
#ifdef CONFIG_BT_COEXIST
	rtl8723b_set_FwAntSelReverse_cmd(padapter);
#endif
}
#endif //CONFIG_WOWLAN

static void rtl8723b_free_hal_data(PADAPTER padapter)
{
_func_enter_;
	if (padapter->HalData) {
		rtw_mfree(padapter->HalData, sizeof(HAL_DATA_TYPE));
		padapter->HalData = NULL;
	}
_func_exit_;
}

//===========================================================
//				Efuse related code
//===========================================================
static u8
hal_EfuseSwitchToBank(
	PADAPTER	padapter,
	u8			bank,
	u8			bPseudoTest)
{
	u8 bRet = _FALSE;
	u32 value32 = 0;
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL pEfuseHal = &pHalData->EfuseHal;
#endif


	DBG_8192C("%s: Efuse switch bank to %d\n", __FUNCTION__, bank);
	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeEfuseBank = bank;
#else
		fakeEfuseBank = bank;
#endif
		bRet = _TRUE;
	}
	else
	{
		value32 = rtw_read32(padapter, EFUSE_TEST);
		bRet = _TRUE;
		switch (bank)
		{
			case 0:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_WIFI_SEL_0);
				break;
			case 1:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_0);
				break;
			case 2:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_1);
				break;
			case 3:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_2);
				break;
			default:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_WIFI_SEL_0);
				bRet = _FALSE;
				break;
		}
		rtw_write32(padapter, EFUSE_TEST, value32);
	}

	return bRet;
}

static void
Hal_GetEfuseDefinition(
	PADAPTER	padapter,
	u8			efuseType,
	u8			type,
	void		*pOut,
	u8			bPseudoTest)
{
	switch (type)
	{
		case TYPE_EFUSE_MAX_SECTION:
			{
				u8 *pMax_section;
				pMax_section = (u8*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pMax_section = EFUSE_WIFI_MAX_SECTION;
				else
					*pMax_section = EFUSE_BT_MAX_SECTION;
			}
			break;

		case TYPE_EFUSE_REAL_CONTENT_LEN:
			{
				u16 *pu2Tmp;
				pu2Tmp = (u16*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu2Tmp = EFUSE_REAL_CONTENT_LEN_8723A;
				else
					*pu2Tmp = EFUSE_BT_REAL_CONTENT_LEN;
			}
			break;

		case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
			{
				u16	*pu2Tmp;
				pu2Tmp = (u16*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu2Tmp = (EFUSE_REAL_CONTENT_LEN_8723A-EFUSE_OOB_PROTECT_BYTES);
				else
					*pu2Tmp = (EFUSE_BT_REAL_BANK_CONTENT_LEN-EFUSE_PROTECT_BYTES_BANK);
			}
			break;

		case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
			{
				u16 *pu2Tmp;
				pu2Tmp = (u16*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu2Tmp = (EFUSE_REAL_CONTENT_LEN_8723A-EFUSE_OOB_PROTECT_BYTES);
				else
					*pu2Tmp = (EFUSE_BT_REAL_CONTENT_LEN-(EFUSE_PROTECT_BYTES_BANK*3));
			}
			break;

		case TYPE_EFUSE_MAP_LEN:
			{
				u16 *pu2Tmp;
				pu2Tmp = (u16*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu2Tmp = EFUSE_MAX_MAP_LEN;
				else
					*pu2Tmp = EFUSE_BT_MAP_LEN;
			}
			break;

		case TYPE_EFUSE_PROTECT_BYTES_BANK:
			{
				u8 *pu1Tmp;
				pu1Tmp = (u8*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu1Tmp = EFUSE_OOB_PROTECT_BYTES;
				else
					*pu1Tmp = EFUSE_PROTECT_BYTES_BANK;
			}
			break;

		case TYPE_EFUSE_CONTENT_LEN_BANK:
			{
				u16 *pu2Tmp;
				pu2Tmp = (u16*)pOut;

				if (efuseType == EFUSE_WIFI)
					*pu2Tmp = EFUSE_REAL_CONTENT_LEN_8723A;
				else
					*pu2Tmp = EFUSE_BT_REAL_BANK_CONTENT_LEN;
			}
			break;

		default:
			{
				u8 *pu1Tmp;
				pu1Tmp = (u8*)pOut;
				*pu1Tmp = 0;
			}
			break;
	}
}

#define VOLTAGE_V25		0x03
#define LDOE25_SHIFT	28

static void
Hal_EfusePowerSwitch(
	PADAPTER	padapter,
	u8			bWrite,
	u8			PwrState)
{
	u8	tempval;
	u16	tmpV16;

	if (PwrState == _TRUE)
	{
		rtw_write8(padapter, REG_EFUSE_ACCESS, EFUSE_ACCESS_ON);

		// 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid
		tmpV16 = rtw_read16(padapter, REG_SYS_ISO_CTRL);
		if (!(tmpV16 & PWC_EV12V)) {
			tmpV16 |= PWC_EV12V ;
			rtw_write16(padapter, REG_SYS_ISO_CTRL, tmpV16);
		}
		// Reset: 0x0000h[28], default valid
		tmpV16 =  rtw_read16(padapter, REG_SYS_FUNC_EN);
		if (!(tmpV16 & FEN_ELDR)) {
			tmpV16 |= FEN_ELDR ;
			rtw_write16(padapter, REG_SYS_FUNC_EN, tmpV16);
		}

		// Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid
		tmpV16 = rtw_read16(padapter, REG_SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN))  || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN | ANA8M) ;
			rtw_write16(padapter, REG_SYS_CLKR, tmpV16);
		}

		if (bWrite == _TRUE)
		{
			// Enable LDO 2.5V before read/write action
			tempval = rtw_read8(padapter, EFUSE_TEST+3);
			tempval &= 0x0F;
			tempval |= (VOLTAGE_V25 << 4);
			rtw_write8(padapter, EFUSE_TEST+3, (tempval | 0x80));
		}
	}
	else
	{
		rtw_write8(padapter, REG_EFUSE_ACCESS, EFUSE_ACCESS_OFF);

		if (bWrite == _TRUE) {
			// Disable LDO 2.5V after read/write action
			tempval = rtw_read8(padapter, EFUSE_TEST+3);
			rtw_write8(padapter, EFUSE_TEST+3, (tempval & 0x7F));
		}
	}
}

static void
hal_ReadEFuse_WiFi(
	PADAPTER	padapter,
	u16			_offset,
	u16			_size_byte,
	u8			*pbuf,
	u8			bPseudoTest)
{
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
#endif
	u8	*efuseTbl = NULL;
	u16	eFuse_Addr=0;
	u8	offset, wden;
	u8	efuseHeader, efuseExtHdr, efuseData;
	u16	i, total, used;
	u8	efuse_usage = 0;

	//DBG_871X("YJ: ====>%s():_offset=%d _size_byte=%d bPseudoTest=%d\n", __func__, _offset, _size_byte, bPseudoTest);
	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if ((_offset+_size_byte) > EFUSE_MAX_MAP_LEN)
	{
		DBG_8192C("%s: Invalid offset(%#x) with read bytes(%#x)!!\n", __FUNCTION__, _offset, _size_byte);
		return;
	}

	efuseTbl = (u8*)rtw_malloc(EFUSE_MAX_MAP_LEN);
	if (efuseTbl == NULL)
	{
		DBG_8192C("%s: alloc efuseTbl fail!\n", __FUNCTION__);
		return;
	}
	// 0xff will be efuse default value instead of 0x00.
	_rtw_memset(efuseTbl, 0xFF, EFUSE_MAX_MAP_LEN);


#ifdef CONFIG_DEBUG
if(0)
{
	for(i=0; i<256; i++)
		ReadEFuseByte(padapter, i, &efuseTbl[i], _FALSE);
	DBG_871X("Efuse Content:\n");
	for(i=0; i<256; i++)
	{
		if (i % 16 == 0)
			printk("\n");
		printk("%02X ", efuseTbl[i]);
	}
	printk("\n");
}
#endif


	// switch bank back to bank 0 for later BT and wifi use.
	hal_EfuseSwitchToBank(padapter, 0, bPseudoTest);

	while (AVAILABLE_EFUSE_ADDR(eFuse_Addr))
	{
		ReadEFuseByte(padapter, eFuse_Addr++, &efuseHeader, bPseudoTest);
		if (efuseHeader == 0xFF)
		{
			DBG_8192C("%s: data end at address=%#x\n", __FUNCTION__, eFuse_Addr);
			break;
		}
		//DBG_8192C("%s: efuse[0x%X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseHeader);

		// Check PG header for section num.
		if (EXT_HEADER(efuseHeader))		//extended header
		{
			offset = GET_HDR_OFFSET_2_0(efuseHeader);
			//DBG_8192C("%s: extended header offset=0x%X\n", __FUNCTION__, offset);

			ReadEFuseByte(padapter, eFuse_Addr++, &efuseExtHdr, bPseudoTest);
			//DBG_8192C("%s: efuse[0x%X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseExtHdr);
			if (ALL_WORDS_DISABLED(efuseExtHdr))
			{
				continue;
			}

			offset |= ((efuseExtHdr & 0xF0) >> 1);
			wden = (efuseExtHdr & 0x0F);
		}
		else
		{
			offset = ((efuseHeader >> 4) & 0x0f);
			wden = (efuseHeader & 0x0f);
		}
		//DBG_8192C("%s: Offset=%d Worden=0x%X\n", __FUNCTION__, offset, wden);

		if (offset < EFUSE_WIFI_MAX_SECTION)
		{
			u16 addr;
			// Get word enable value from PG header
//			DBG_8192C("%s: Offset=%d Worden=0x%X\n", __FUNCTION__, offset, wden);

			addr = offset * PGPKT_DATA_SIZE;
			for (i=0; i<EFUSE_MAX_WORD_UNIT; i++)
			{
				// Check word enable condition in the section
				if (!(wden & (0x01<<i)))
				{
					ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest);
//					DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseData);
					efuseTbl[addr] = efuseData;

					ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest);
//					DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseData);
					efuseTbl[addr+1] = efuseData;
				}
				addr += 2;
			}
		}
		else
		{
			DBG_8192C(KERN_ERR "%s: offset(%d) is illegal!!\n", __FUNCTION__, offset);
			eFuse_Addr += Efuse_CalculateWordCnts(wden)*2;
		}
	}

	// Copy from Efuse map to output pointer memory!!!
	for (i=0; i<_size_byte; i++)
		pbuf[i] = efuseTbl[_offset+i];

#ifdef CONFIG_DEBUG
if(1)
{
	DBG_871X("Efuse Realmap:\n");
	for(i=0; i<_size_byte; i++)
	{
		if (i % 16 == 0)
			printk("\n");
		printk("%02X ", pbuf[i]);
	}
	printk("\n");
}
#endif
	// Calculate Efuse utilization
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &total, bPseudoTest);
	used = eFuse_Addr - 1;
	efuse_usage = (u8)((used*100)/total);
	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeEfuseUsedBytes = used;
#else
		fakeEfuseUsedBytes = used;
#endif
	}
	else
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_BYTES, (u8*)&used);
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_USAGE, (u8*)&efuse_usage);
	}

	if (efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_MAX_MAP_LEN);
}

static VOID
hal_ReadEFuse_BT(
	PADAPTER	padapter,
	u16			_offset,
	u16			_size_byte,
	u8			*pbuf,
	u8			bPseudoTest
	)
{
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
#endif
	u8	*efuseTbl;
	u8	bank;
	u16	eFuse_Addr;
	u8	efuseHeader, efuseExtHdr, efuseData;
	u8	offset, wden;
	u16	i, total, used;
	u8	efuse_usage;


	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if ((_offset+_size_byte) > EFUSE_BT_MAP_LEN)
	{
		DBG_8192C("%s: Invalid offset(%#x) with read bytes(%#x)!!\n", __FUNCTION__, _offset, _size_byte);
		return;
	}

	efuseTbl = rtw_malloc(EFUSE_BT_MAP_LEN);
	if (efuseTbl == NULL) {
		DBG_8192C("%s: efuseTbl malloc fail!\n", __FUNCTION__);
		return;
	}
	// 0xff will be efuse default value instead of 0x00.
	_rtw_memset(efuseTbl, 0xFF, EFUSE_BT_MAP_LEN);

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_BANK, &total, bPseudoTest);

	for (bank=1; bank<EFUSE_MAX_BANK; bank++)
	{
		if (hal_EfuseSwitchToBank(padapter, bank, bPseudoTest) == _FALSE)
		{
			DBG_8192C("%s: hal_EfuseSwitchToBank Fail!!\n", __FUNCTION__);
			goto exit;
		}

		eFuse_Addr = 0;

		while (AVAILABLE_EFUSE_ADDR(eFuse_Addr))
		{
			ReadEFuseByte(padapter, eFuse_Addr++, &efuseHeader, bPseudoTest);
			if (efuseHeader == 0xFF) break;
//			DBG_8192C("%s: efuse[%#X]=0x%02x (header)\n", __FUNCTION__, (((bank-1)*EFUSE_REAL_CONTENT_LEN)+eFuse_Addr-1), efuseHeader);

			// Check PG header for section num.
			if (EXT_HEADER(efuseHeader))		//extended header
			{
				offset = GET_HDR_OFFSET_2_0(efuseHeader);
//				DBG_8192C("%s: extended header offset_2_0=0x%X\n", __FUNCTION__, offset_2_0);

				ReadEFuseByte(padapter, eFuse_Addr++, &efuseExtHdr, bPseudoTest);
//				DBG_8192C("%s: efuse[%#X]=0x%02x (ext header)\n", __FUNCTION__, (((bank-1)*EFUSE_REAL_CONTENT_LEN)+eFuse_Addr-1), efuseExtHdr);
				if (ALL_WORDS_DISABLED(efuseExtHdr))
				{
					continue;
				}

				offset |= ((efuseExtHdr & 0xF0) >> 1);
				wden = (efuseExtHdr & 0x0F);
			}
			else
			{
				offset = ((efuseHeader >> 4) & 0x0f);
				wden = (efuseHeader & 0x0f);
			}

			if (offset < EFUSE_BT_MAX_SECTION)
			{
				u16 addr;

				// Get word enable value from PG header
//				DBG_8192C("%s: Offset=%d Worden=%#X\n", __FUNCTION__, offset, wden);

				addr = offset * PGPKT_DATA_SIZE;
				for (i=0; i<EFUSE_MAX_WORD_UNIT; i++)
				{
					// Check word enable condition in the section
					if (!(wden & (0x01<<i)))
					{
						ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest);
//						DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseData);
						efuseTbl[addr] = efuseData;

						ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest);
//						DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, eFuse_Addr-1, efuseData);
						efuseTbl[addr+1] = efuseData;
					}
					addr += 2;
				}
			}
			else
			{
				DBG_8192C(KERN_ERR "%s: offset(%d) is illegal!!\n", __FUNCTION__, offset);
				eFuse_Addr += Efuse_CalculateWordCnts(wden)*2;
			}
		}

		if ((eFuse_Addr-1) < total)
		{
			DBG_8192C("%s: bank(%d) data end at %#x\n", __FUNCTION__, bank, eFuse_Addr-1);
			break;
		}
	}

	// switch bank back to bank 0 for later BT and wifi use.
	hal_EfuseSwitchToBank(padapter, 0, bPseudoTest);

	// Copy from Efuse map to output pointer memory!!!
	for (i=0; i<_size_byte; i++)
		pbuf[i] = efuseTbl[_offset+i];

	//
	// Calculate Efuse utilization.
	//
	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &total, bPseudoTest);
	used = (EFUSE_BT_REAL_BANK_CONTENT_LEN*(bank-1)) + eFuse_Addr - 1;
	efuse_usage = (u8)((used*100)/total);
	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeBTEfuseUsedBytes = used;
#else
		fakeBTEfuseUsedBytes = used;
#endif
	}
	else
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_BT_BYTES, (u8*)&used);
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_BT_USAGE, (u8*)&efuse_usage);
	}

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_BT_MAP_LEN);
}

static void
Hal_ReadEFuse(
	PADAPTER	padapter,
	u8			efuseType,
	u16			_offset,
	u16			_size_byte,
	u8			*pbuf,
	u8			bPseudoTest)
{
	if (efuseType == EFUSE_WIFI)
		hal_ReadEFuse_WiFi(padapter, _offset, _size_byte, pbuf, bPseudoTest);
	else
		hal_ReadEFuse_BT(padapter, _offset, _size_byte, pbuf, bPseudoTest);
}

static u16
hal_EfuseGetCurrentSize_WiFi(
	PADAPTER	padapter,
	u8			bPseudoTest)
{
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
#endif
	u16	efuse_addr=0;
	u8	hoffset=0, hworden=0;
	u8	efuse_data, word_cnts=0;


	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		efuse_addr = (u16)pEfuseHal->fakeEfuseUsedBytes;
#else
		efuse_addr = (u16)fakeEfuseUsedBytes;
#endif
	}
	else
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_EFUSE_BYTES, (u8*)&efuse_addr);
	}
	DBG_8192C("%s: start_efuse_addr=0x%X\n", __FUNCTION__, efuse_addr);

	// switch bank back to bank 0 for later BT and wifi use.
	hal_EfuseSwitchToBank(padapter, 0, bPseudoTest);

	while (AVAILABLE_EFUSE_ADDR(efuse_addr))
	{
		if (efuse_OneByteRead(padapter, efuse_addr, &efuse_data, bPseudoTest) == _FALSE)
		{
			DBG_8192C(KERN_ERR "%s: efuse_OneByteRead Fail! addr=0x%X !!\n", __FUNCTION__, efuse_addr);
			break;
		}

		if (efuse_data == 0xFF) break;

		if (EXT_HEADER(efuse_data))
		{
			hoffset = GET_HDR_OFFSET_2_0(efuse_data);
			efuse_addr++;
			efuse_OneByteRead(padapter, efuse_addr, &efuse_data, bPseudoTest);
			if (ALL_WORDS_DISABLED(efuse_data))
			{
				continue;
			}

			hoffset |= ((efuse_data & 0xF0) >> 1);
			hworden = efuse_data & 0x0F;
		}
		else
		{
			hoffset = (efuse_data>>4) & 0x0F;
			hworden = efuse_data & 0x0F;
		}

		word_cnts = Efuse_CalculateWordCnts(hworden);
		efuse_addr += (word_cnts*2)+1;
	}

	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeEfuseUsedBytes = efuse_addr;
#else
		fakeEfuseUsedBytes = efuse_addr;
#endif
	}
	else
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_BYTES, (u8*)&efuse_addr);
	}
	DBG_8192C("%s: CurrentSize=%d\n", __FUNCTION__, efuse_addr);

	return efuse_addr;
}

static u16
hal_EfuseGetCurrentSize_BT(
	PADAPTER	padapter,
	u8			bPseudoTest)
{
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
#endif
	u16 btusedbytes;
	u16	efuse_addr;
	u8	bank, startBank;
	u8	hoffset=0, hworden=0;
	u8	efuse_data, word_cnts=0;
	u16	retU2=0;


	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		btusedbytes = pEfuseHal->fakeBTEfuseUsedBytes;
#else
		btusedbytes = fakeBTEfuseUsedBytes;
#endif
	}
	else
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_EFUSE_BT_BYTES, (u8*)&btusedbytes);
	}
	efuse_addr = (u16)((btusedbytes%EFUSE_BT_REAL_BANK_CONTENT_LEN));
	startBank = (u8)(1+(btusedbytes/EFUSE_BT_REAL_BANK_CONTENT_LEN));

	DBG_8192C("%s: start from bank=%d addr=0x%X\n", __FUNCTION__, startBank, efuse_addr);

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_BANK, &retU2, bPseudoTest);

	for (bank=startBank; bank<EFUSE_MAX_BANK; bank++)
	{
		if (hal_EfuseSwitchToBank(padapter, bank, bPseudoTest) == _FALSE)
		{
			DBG_8192C(KERN_ERR "%s: switch bank(%d) Fail!!\n", __FUNCTION__, bank);
			bank = EFUSE_MAX_BANK;
			break;
		}

		// only when bank is switched we have to reset the efuse_addr.
		if (bank != startBank)
			efuse_addr = 0;

		while (AVAILABLE_EFUSE_ADDR(efuse_addr))
		{
			if (efuse_OneByteRead(padapter, efuse_addr, &efuse_data, bPseudoTest) == _FALSE)
			{
				DBG_8192C(KERN_ERR "%s: efuse_OneByteRead Fail! addr=0x%X !!\n", __FUNCTION__, efuse_addr);
				bank = EFUSE_MAX_BANK;
				break;
			}

			if (efuse_data == 0xFF) break;

			if (EXT_HEADER(efuse_data))
			{
				hoffset = GET_HDR_OFFSET_2_0(efuse_data);
				efuse_addr++;
				efuse_OneByteRead(padapter, efuse_addr, &efuse_data, bPseudoTest);
				if (ALL_WORDS_DISABLED(efuse_data))
				{
					efuse_addr++;
					continue;
				}

				hoffset |= ((efuse_data & 0xF0) >> 1);
				hworden = efuse_data & 0x0F;
			}
			else
			{
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = Efuse_CalculateWordCnts(hworden);
			//read next header
			efuse_addr += (word_cnts*2)+1;
		}

		// Check if we need to check next bank efuse
		if (efuse_addr < retU2)
		{
			break;// don't need to check next bank.
		}
	}

	retU2 = ((bank-1)*EFUSE_BT_REAL_BANK_CONTENT_LEN) + efuse_addr;
	if (bPseudoTest)
	{
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeBTEfuseUsedBytes = retU2;
#else
		fakeBTEfuseUsedBytes = retU2;
#endif
	}
	else
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_EFUSE_BT_BYTES, (u8*)&retU2);
	}

	DBG_8192C("%s: CurrentSize=%d\n", __FUNCTION__, retU2);
	return retU2;
}

static u16
Hal_EfuseGetCurrentSize(
	PADAPTER	pAdapter,
	u8			efuseType,
	u8			bPseudoTest)
{
	u16	ret = 0;

	if (efuseType == EFUSE_WIFI)
		ret = hal_EfuseGetCurrentSize_WiFi(pAdapter, bPseudoTest);
	else
		ret = hal_EfuseGetCurrentSize_BT(pAdapter, bPseudoTest);

	return ret;
}

static u8
Hal_EfuseWordEnableDataWrite(
	PADAPTER	padapter,
	u16			efuse_addr,
	u8			word_en,
	u8			*data,
	u8			bPseudoTest)
{
	u16	tmpaddr = 0;
	u16	start_addr = efuse_addr;
	u8	badworden = 0x0F;
	u8	tmpdata[PGPKT_DATA_SIZE];


//	DBG_8192C("%s: efuse_addr=%#x word_en=%#x\n", __FUNCTION__, efuse_addr, word_en);
	_rtw_memset(tmpdata, 0xFF, PGPKT_DATA_SIZE);

	if (!(word_en & BIT(0)))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(padapter, start_addr++, data[0], bPseudoTest);
		efuse_OneByteWrite(padapter, start_addr++, data[1], bPseudoTest);

		efuse_OneByteRead(padapter, tmpaddr, &tmpdata[0], bPseudoTest);
		efuse_OneByteRead(padapter, tmpaddr+1, &tmpdata[1], bPseudoTest);
		if ((data[0]!=tmpdata[0]) || (data[1]!=tmpdata[1])) {
			badworden &= (~BIT(0));
		}
	}
	if (!(word_en & BIT(1)))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(padapter, start_addr++, data[2], bPseudoTest);
		efuse_OneByteWrite(padapter, start_addr++, data[3], bPseudoTest);

		efuse_OneByteRead(padapter, tmpaddr, &tmpdata[2], bPseudoTest);
		efuse_OneByteRead(padapter, tmpaddr+1, &tmpdata[3], bPseudoTest);
		if ((data[2]!=tmpdata[2]) || (data[3]!=tmpdata[3])) {
			badworden &= (~BIT(1));
		}
	}
	if (!(word_en & BIT(2)))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(padapter, start_addr++, data[4], bPseudoTest);
		efuse_OneByteWrite(padapter, start_addr++, data[5], bPseudoTest);

		efuse_OneByteRead(padapter, tmpaddr, &tmpdata[4], bPseudoTest);
		efuse_OneByteRead(padapter, tmpaddr+1, &tmpdata[5], bPseudoTest);
		if ((data[4]!=tmpdata[4]) || (data[5]!=tmpdata[5])) {
			badworden &= (~BIT(2));
		}
	}
	if (!(word_en & BIT(3)))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(padapter, start_addr++, data[6], bPseudoTest);
		efuse_OneByteWrite(padapter, start_addr++, data[7], bPseudoTest);

		efuse_OneByteRead(padapter, tmpaddr, &tmpdata[6], bPseudoTest);
		efuse_OneByteRead(padapter, tmpaddr+1, &tmpdata[7], bPseudoTest);
		if ((data[6]!=tmpdata[6]) || (data[7]!=tmpdata[7])) {
			badworden &= (~BIT(3));
		}
	}

	return badworden;
}

static s32
Hal_EfusePgPacketRead(
	PADAPTER	padapter,
	u8			offset,
	u8			*data,
	u8			bPseudoTest)
{
	u8	bDataEmpty = _TRUE;
	u8	efuse_data, word_cnts=0;
	u16	efuse_addr=0;
	u8	hoffset=0, hworden=0;
	u8	i;
	u8	max_section = 0;
	s32	ret;


	if (data == NULL)
		return _FALSE;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAX_SECTION, &max_section, bPseudoTest);
	if (offset > max_section)
	{
		DBG_8192C("%s: Packet offset(%d) is illegal(>%d)!\n", __FUNCTION__, offset, max_section);
		return _FALSE;
	}

	_rtw_memset(data, 0xFF, PGPKT_DATA_SIZE);
	ret = _TRUE;

	//
	// <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// Skip dummy parts to prevent unexpected data read from Efuse.
	// By pass right now. 2009.02.19.
	//
	while (AVAILABLE_EFUSE_ADDR(efuse_addr))
	{
		if (efuse_OneByteRead(padapter, efuse_addr++, &efuse_data, bPseudoTest) == _FALSE)
		{
			ret = _FALSE;
			break;
		}

		if (efuse_data == 0xFF) break;

		if (EXT_HEADER(efuse_data))
		{
			hoffset = GET_HDR_OFFSET_2_0(efuse_data);
			efuse_OneByteRead(padapter, efuse_addr++, &efuse_data, bPseudoTest);
			if (ALL_WORDS_DISABLED(efuse_data))
			{
				DBG_8192C("%s: Error!! All words disabled!\n", __FUNCTION__);
				continue;
			}

			hoffset |= ((efuse_data & 0xF0) >> 1);
			hworden = efuse_data & 0x0F;
		}
		else
		{
			hoffset = (efuse_data>>4) & 0x0F;
			hworden =  efuse_data & 0x0F;
		}

		if (hoffset == offset)
		{
			for (i=0; i<EFUSE_MAX_WORD_UNIT; i++)
			{
				// Check word enable condition in the section
				if (!(hworden & (0x01<<i)))
				{
					ReadEFuseByte(padapter, efuse_addr++, &efuse_data, bPseudoTest);
//					DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, efuse_addr+tmpidx, efuse_data);
					data[i*2] = efuse_data;

					ReadEFuseByte(padapter, efuse_addr++, &efuse_data, bPseudoTest);
//					DBG_8192C("%s: efuse[%#X]=0x%02X\n", __FUNCTION__, efuse_addr+tmpidx, efuse_data);
					data[(i*2)+1] = efuse_data;
				}
			}
		}
		else
		{
			word_cnts = Efuse_CalculateWordCnts(hworden);
			efuse_addr += word_cnts*2;
		}
	}

	return ret;
}

static u8
hal_EfusePgCheckAvailableAddr(
	PADAPTER	pAdapter,
	u8			efuseType,
	u8		bPseudoTest)
{
	u16	max_available=0;
	u16 current_size;


	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &max_available, bPseudoTest);
//	DBG_8192C("%s: max_available=%d\n", __FUNCTION__, max_available);

	current_size = Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest);
	if (current_size >= max_available)
	{
		DBG_8192C("%s: Error!! current_size(%d)>max_available(%d)\n", __FUNCTION__, current_size, max_available);
		return _FALSE;
	}
	return _TRUE;
}

static void
hal_EfuseConstructPGPkt(
	u8 				offset,
	u8				word_en,
	u8				*pData,
	PPGPKT_STRUCT	pTargetPkt)
{
	_rtw_memset(pTargetPkt->data, 0xFF, PGPKT_DATA_SIZE);
	pTargetPkt->offset = offset;
	pTargetPkt->word_en = word_en;
	efuse_WordEnableDataRead(word_en, pData, pTargetPkt->data);
	pTargetPkt->word_cnts = Efuse_CalculateWordCnts(pTargetPkt->word_en);
}

#if 0
static u8
wordEnMatched(
	PPGPKT_STRUCT	pTargetPkt,
	PPGPKT_STRUCT	pCurPkt,
	u8				*pWden)
{
	u8	match_word_en = 0x0F;	// default all words are disabled
	u8	i;

	// check if the same words are enabled both target and current PG packet
	if (((pTargetPkt->word_en & BIT(0)) == 0) &&
		((pCurPkt->word_en & BIT(0)) == 0))
	{
		match_word_en &= ~BIT(0);				// enable word 0
	}
	if (((pTargetPkt->word_en & BIT(1)) == 0) &&
		((pCurPkt->word_en & BIT(1)) == 0))
	{
		match_word_en &= ~BIT(1);				// enable word 1
	}
	if (((pTargetPkt->word_en & BIT(2)) == 0) &&
		((pCurPkt->word_en & BIT(2)) == 0))
	{
		match_word_en &= ~BIT(2);				// enable word 2
	}
	if (((pTargetPkt->word_en & BIT(3)) == 0) &&
		((pCurPkt->word_en & BIT(3)) == 0))
	{
		match_word_en &= ~BIT(3);				// enable word 3
	}

	*pWden = match_word_en;

	if (match_word_en != 0xf)
		return _TRUE;
	else
		return _FALSE;
}

static u8
hal_EfuseCheckIfDatafollowed(
	PADAPTER		pAdapter,
	u8				word_cnts,
	u16				startAddr,
	u8				bPseudoTest)
{
	u8 bRet=_FALSE;
	u8 i, efuse_data;

	for (i=0; i<(word_cnts*2); i++)
	{
		if (efuse_OneByteRead(pAdapter, (startAddr+i) ,&efuse_data, bPseudoTest) == _FALSE)
		{
			DBG_8192C("%s: efuse_OneByteRead FAIL!!\n", __FUNCTION__);
			bRet = _TRUE;
			break;
		}

		if (efuse_data != 0xFF)
		{
			bRet = _TRUE;
			break;
		}
	}

	return bRet;
}
#endif

static u8
hal_EfusePartialWriteCheck(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
	u8	bRet=_FALSE;
	u16	startAddr=0, efuse_max_available_len=0, efuse_max=0;
	u8	efuse_data=0;
#if 0
	u8	i, cur_header=0;
	u8	new_wden=0, matched_wden=0, badworden=0;
	PGPKT_STRUCT	curPkt;
#endif


	EFUSE_GetEfuseDefinition(padapter, efuseType, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &efuse_max_available_len, bPseudoTest);
	EFUSE_GetEfuseDefinition(padapter, efuseType, TYPE_EFUSE_CONTENT_LEN_BANK, &efuse_max, bPseudoTest);

	if (efuseType == EFUSE_WIFI)
	{
		if (bPseudoTest)
		{
#ifdef HAL_EFUSE_MEMORY
			startAddr = (u16)pEfuseHal->fakeEfuseUsedBytes;
#else
			startAddr = (u16)fakeEfuseUsedBytes;
#endif
		}
		else
		{
			rtw_hal_get_hwreg(padapter, HW_VAR_EFUSE_BYTES, (u8*)&startAddr);
		}
	}
	else
	{
		if (bPseudoTest)
		{
#ifdef HAL_EFUSE_MEMORY
			startAddr = (u16)pEfuseHal->fakeBTEfuseUsedBytes;
#else
			startAddr = (u16)fakeBTEfuseUsedBytes;
#endif
		}
		else
		{
			rtw_hal_get_hwreg(padapter, HW_VAR_EFUSE_BT_BYTES, (u8*)&startAddr);
		}
	}
	startAddr %= efuse_max;
//	DBG_8192C("%s: startAddr=%#X\n", __FUNCTION__, startAddr);

	while (1)
	{
		if (startAddr >= efuse_max_available_len)
		{
			bRet = _FALSE;
			DBG_8192C("%s: startAddr(%d) >= efuse_max_available_len(%d)\n",
				__FUNCTION__, startAddr, efuse_max_available_len);
			break;
		}

		if (efuse_OneByteRead(padapter, startAddr, &efuse_data, bPseudoTest) && (efuse_data!=0xFF))
		{
#if 1
			bRet = _FALSE;
			DBG_8192C("%s: Something Wrong! last bytes(%#X=0x%02X) is not 0xFF\n",
				__FUNCTION__, startAddr, efuse_data);
			break;
#else
			if (EXT_HEADER(efuse_data))
			{
				cur_header = efuse_data;
				startAddr++;
				efuse_OneByteRead(padapter, startAddr, &efuse_data, bPseudoTest);
				if (ALL_WORDS_DISABLED(efuse_data))
				{
					DBG_8192C("%s: Error condition, all words disabled!", __FUNCTION__);
					bRet = _FALSE;
					break;
				}
				else
				{
					curPkt.offset = ((cur_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					curPkt.word_en = efuse_data & 0x0F;
				}
			}
			else
			{
				cur_header  =  efuse_data;
				curPkt.offset = (cur_header>>4) & 0x0F;
				curPkt.word_en = cur_header & 0x0F;
			}

			curPkt.word_cnts = Efuse_CalculateWordCnts(curPkt.word_en);
			// if same header is found but no data followed
			// write some part of data followed by the header.
			if ((curPkt.offset == pTargetPkt->offset) &&
				(hal_EfuseCheckIfDatafollowed(padapter, curPkt.word_cnts, startAddr+1, bPseudoTest) == _FALSE) &&
				wordEnMatched(pTargetPkt, &curPkt, &matched_wden) == _TRUE)
			{
				DBG_8192C("%s: Need to partial write data by the previous wrote header\n", __FUNCTION__);
				// Here to write partial data
				badworden = Efuse_WordEnableDataWrite(padapter, startAddr+1, matched_wden, pTargetPkt->data, bPseudoTest);
				if (badworden != 0x0F)
				{
					u32	PgWriteSuccess=0;
					// if write fail on some words, write these bad words again
					if (efuseType == EFUSE_WIFI)
						PgWriteSuccess = Efuse_PgPacketWrite(padapter, pTargetPkt->offset, badworden, pTargetPkt->data, bPseudoTest);
					else
						PgWriteSuccess = Efuse_PgPacketWrite_BT(padapter, pTargetPkt->offset, badworden, pTargetPkt->data, bPseudoTest);

					if (!PgWriteSuccess)
					{
						bRet = _FALSE;	// write fail, return
						break;
					}
				}
				// partial write ok, update the target packet for later use
				for (i=0; i<4; i++)
				{
					if ((matched_wden & (0x1<<i)) == 0)	// this word has been written
					{
						pTargetPkt->word_en |= (0x1<<i);	// disable the word
					}
				}
				pTargetPkt->word_cnts = Efuse_CalculateWordCnts(pTargetPkt->word_en);
			}
			// read from next header
			startAddr = startAddr + (curPkt.word_cnts*2) + 1;
#endif
		}
		else
		{
			// not used header, 0xff
			*pAddr = startAddr;
//			DBG_8192C("%s: Started from unused header offset=%d\n", __FUNCTION__, startAddr));
			bRet = _TRUE;
			break;
		}
	}

	return bRet;
}

static u8
hal_EfusePgPacketWrite1ByteHeader(
	PADAPTER		pAdapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u8	bRet=_FALSE;
	u8	pg_header=0, tmp_header=0;
	u16	efuse_addr=*pAddr;
	u8	repeatcnt=0;


//	DBG_8192C("%s\n", __FUNCTION__);
	pg_header = ((pTargetPkt->offset << 4) & 0xf0) | pTargetPkt->word_en;

	do {
		efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
		efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);
		if (tmp_header != 0xFF) break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
		{
			DBG_8192C("%s: Repeat over limit for pg_header!!\n", __FUNCTION__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header)
	{
		DBG_8192C(KERN_ERR "%s: PG Header Fail!!(pg=0x%02X read=0x%02X)\n", __FUNCTION__, pg_header, tmp_header);
		return _FALSE;
	}

	*pAddr = efuse_addr;

	return _TRUE;
}

static u8
hal_EfusePgPacketWrite2ByteHeader(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u16	efuse_addr, efuse_max_available_len=0;
	u8	pg_header=0, tmp_header=0;
	u8	repeatcnt=0;


//	DBG_8192C("%s\n", __FUNCTION__);
	EFUSE_GetEfuseDefinition(padapter, efuseType, TYPE_AVAILABLE_EFUSE_BYTES_BANK, &efuse_max_available_len, bPseudoTest);

	efuse_addr = *pAddr;
	if (efuse_addr >= efuse_max_available_len)
	{
		DBG_8192C("%s: addr(%d) over avaliable(%d)!!\n", __FUNCTION__, efuse_addr, efuse_max_available_len);
		return _FALSE;
	}

	pg_header = ((pTargetPkt->offset & 0x07) << 5) | 0x0F;
//	DBG_8192C("%s: pg_header=0x%x\n", __FUNCTION__, pg_header);

	do {
		efuse_OneByteWrite(padapter, efuse_addr, pg_header, bPseudoTest);
		efuse_OneByteRead(padapter, efuse_addr, &tmp_header, bPseudoTest);
		if (tmp_header != 0xFF) break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
		{
			DBG_8192C("%s: Repeat over limit for pg_header!!\n", __FUNCTION__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header)
	{
		DBG_8192C(KERN_ERR "%s: PG Header Fail!!(pg=0x%02X read=0x%02X)\n", __FUNCTION__, pg_header, tmp_header);
		return _FALSE;
	}

	// to write ext_header
	efuse_addr++;
	pg_header = ((pTargetPkt->offset & 0x78) << 1) | pTargetPkt->word_en;

	do {
		efuse_OneByteWrite(padapter, efuse_addr, pg_header, bPseudoTest);
		efuse_OneByteRead(padapter, efuse_addr, &tmp_header, bPseudoTest);
		if (tmp_header != 0xFF) break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
		{
			DBG_8192C("%s: Repeat over limit for ext_header!!\n", __FUNCTION__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header)	//offset PG fail
	{
		DBG_8192C(KERN_ERR "%s: PG EXT Header Fail!!(pg=0x%02X read=0x%02X)\n", __FUNCTION__, pg_header, tmp_header);
		return _FALSE;
	}

	*pAddr = efuse_addr;

	return _TRUE;
}

static u8
hal_EfusePgPacketWriteHeader(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u8 bRet=_FALSE;

	if (pTargetPkt->offset >= EFUSE_MAX_SECTION_BASE)
	{
		bRet = hal_EfusePgPacketWrite2ByteHeader(padapter, efuseType, pAddr, pTargetPkt, bPseudoTest);
	}
	else
	{
		bRet = hal_EfusePgPacketWrite1ByteHeader(padapter, efuseType, pAddr, pTargetPkt, bPseudoTest);
	}

	return bRet;
}

static u8
hal_EfusePgPacketWriteData(
	PADAPTER		pAdapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u16	efuse_addr;
	u8	badworden;


	efuse_addr = *pAddr;
	badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr+1, pTargetPkt->word_en, pTargetPkt->data, bPseudoTest);
	if (badworden != 0x0F)
	{
		DBG_8192C("%s: Fail!!\n", __FUNCTION__);
		return _FALSE;
	}

//	DBG_8192C("%s: ok\n", __FUNCTION__);
	return _TRUE;
}

static s32
Hal_EfusePgPacketWrite(
	PADAPTER	padapter,
	u8 			offset,
	u8			word_en,
	u8			*pData,
	u8			bPseudoTest)
{
	PGPKT_STRUCT targetPkt;
	u16 startAddr=0;
	u8 efuseType=EFUSE_WIFI;

	if (!hal_EfusePgCheckAvailableAddr(padapter, efuseType, bPseudoTest))
		return _FALSE;

	hal_EfuseConstructPGPkt(offset, word_en, pData, &targetPkt);

	if (!hal_EfusePartialWriteCheck(padapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if (!hal_EfusePgPacketWriteHeader(padapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if (!hal_EfusePgPacketWriteData(padapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	return _TRUE;
}

static u8
Hal_EfusePgPacketWrite_BT(
	PADAPTER	pAdapter,
	u8 			offset,
	u8			word_en,
	u8			*pData,
	u8			bPseudoTest)
{
	PGPKT_STRUCT targetPkt;
	u16 startAddr=0;
	u8 efuseType=EFUSE_BT;

	if(!hal_EfusePgCheckAvailableAddr(pAdapter, efuseType, bPseudoTest))
		return _FALSE;

	hal_EfuseConstructPGPkt(offset, word_en, pData, &targetPkt);

	if(!hal_EfusePartialWriteCheck(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if(!hal_EfusePgPacketWriteHeader(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if(!hal_EfusePgPacketWriteData(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	return _TRUE;
}

static HAL_VERSION
ReadChipVersion8723B(
	IN	PADAPTER	padapter
	)
{
	u32				value32;
	HAL_VERSION		ChipVersion;
	HAL_DATA_TYPE	*pHalData;

//YJ,TODO, move read chip type here
	pHalData = GET_HAL_DATA(padapter);

	value32 = rtw_read32(padapter, REG_SYS_CFG);
	ChipVersion.ICType = CHIP_8723B;
	ChipVersion.ChipType = ((value32 & RTL_ID) ? TEST_CHIP : NORMAL_CHIP);
	ChipVersion.RFType = RF_TYPE_1T1R ;
	ChipVersion.VendorType = ((value32 & VENDOR_ID) ? CHIP_VENDOR_UMC : CHIP_VENDOR_TSMC);
	ChipVersion.CUTVersion = (value32 & CHIP_VER_RTL_MASK)>>CHIP_VER_RTL_SHIFT; // IC version (CUT)

	// For regulator mode. by tynli. 2011.01.14
	pHalData->RegulatorMode = ((value32 & SPS_SEL) ? RT_LDO_REGULATOR : RT_SWITCHING_REGULATOR);

	value32 = rtw_read32(padapter, REG_GPIO_OUTSTS);
	ChipVersion.ROMVer = ((value32 & RF_RL_ID) >> 20);	// ROM code version.

	// For multi-function consideration. Added by Roger, 2010.10.06.
	pHalData->MultiFunc = RT_MULTI_FUNC_NONE;
	value32 = rtw_read32(padapter, REG_MULTI_FUNC_CTRL);
	pHalData->MultiFunc |= ((value32 & WL_FUNC_EN) ? RT_MULTI_FUNC_WIFI : 0);
	pHalData->MultiFunc |= ((value32 & BT_FUNC_EN) ? RT_MULTI_FUNC_BT : 0);
	pHalData->MultiFunc |= ((value32 & GPS_FUNC_EN) ? RT_MULTI_FUNC_GPS : 0);
	pHalData->PolarityCtl = ((value32 & WL_HWPDN_SL) ? RT_POLARITY_HIGH_ACT : RT_POLARITY_LOW_ACT);
//#if DBG
#if 1
	dump_chip_info(ChipVersion);
#endif
	pHalData->VersionID = ChipVersion;

	if (IS_1T2R(ChipVersion))
		pHalData->rf_type = RF_1T2R;
	else if (IS_2T2R(ChipVersion))
		pHalData->rf_type = RF_2T2R;
	else
		pHalData->rf_type = RF_1T1R;

	MSG_8192C("RF_Type is %x!!\n", pHalData->rf_type);

	return ChipVersion;
}


static void rtl8723b_read_chip_version(PADAPTER padapter)
{
	ReadChipVersion8723B(padapter);
}

//====================================================================================
//
// 20100209 Joseph:
// This function is used only for 92C to set REG_BCN_CTRL(0x550) register.
// We just reserve the value of the register in variable pHalData->RegBcnCtrlVal and then operate
// the value of the register via atomic operation.
// This prevents from race condition when setting this register.
// The value of pHalData->RegBcnCtrlVal is initialized in HwConfigureRTL8192CE() function.
//
void SetBcnCtrlReg(PADAPTER padapter, u8 SetBits, u8 ClearBits)
{
	PHAL_DATA_TYPE pHalData;
	u32 addr;
	u8 *pRegBcnCtrlVal;


	pHalData = GET_HAL_DATA(padapter);
	pRegBcnCtrlVal = (u8*)&pHalData->RegBcnCtrlVal;

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		addr = REG_BCN_CTRL_1;
		pRegBcnCtrlVal++;
	}
	else
#endif
	{
		addr = REG_BCN_CTRL;
	}

	*pRegBcnCtrlVal = rtw_read8(padapter, addr);
	*pRegBcnCtrlVal |= SetBits;
	*pRegBcnCtrlVal &= ~ClearBits;

#if 0
//#ifdef CONFIG_SDIO_HCI
	if (pHalData->sdio_himr & (SDIO_HIMR_TXBCNOK_MSK | SDIO_HIMR_TXBCNERR_MSK))
		*pRegBcnCtrlVal |= EN_TXBCN_RPT;
#endif

	rtw_write8(padapter, addr, *pRegBcnCtrlVal);
}

void rtl8723b_InitBeaconParameters(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);


	rtw_write16(padapter, REG_BCN_CTRL, 0x1010);
	pHalData->RegBcnCtrlVal = 0x1010;

	// TODO: Remove these magic number
	rtw_write16(padapter, REG_TBTT_PROHIBIT, 0x6404);// ms
	// Firmware will control REG_DRVERLYINT when power saving is enable,
	// so don't set this register on STA mode.
	if (check_fwstate(&padapter->mlmepriv, WIFI_STATION_STATE) == _FALSE)
		rtw_write8(padapter, REG_DRVERLYINT, DRIVER_EARLY_INT_TIME); // 5ms
	rtw_write8(padapter, REG_BCNDMATIM, BCN_DMA_ATIME_INT_TIME); // 2ms

	// Suggested by designer timchen. Change beacon AIFS to the largest number
	// beacause test chip does not contension before sending beacon. by tynli. 2009.11.03
	rtw_write16(padapter, REG_BCNTCFG, 0x660F);
}

void rtl8723b_InitBeaconMaxError(PADAPTER padapter, u8 InfraMode)
{
#ifdef RTL8192CU_ADHOC_WORKAROUND_SETTING
	rtw_write8(padapter, REG_BCN_MAX_ERR, 0xFF);
#else
	//rtw_write8(Adapter, REG_BCN_MAX_ERR, (InfraMode ? 0xFF : 0x10));
#endif
}

static void ResumeTxBeacon(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);


	// 2010.03.01. Marked by tynli. No need to call workitem beacause we record the value
	// which should be read from register to a global variable.

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("+ResumeTxBeacon\n"));

	pHalData->RegFwHwTxQCtrl |= BIT(6);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+1, 0xff);
	pHalData->RegReg542 |= BIT(0);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);
}

static void StopTxBeacon(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);


	// 2010.03.01. Marked by tynli. No need to call workitem beacause we record the value
	// which should be read from register to a global variable.

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("+StopTxBeacon\n"));

	pHalData->RegFwHwTxQCtrl &= ~BIT(6);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, pHalData->RegFwHwTxQCtrl);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+1, 0x64);
	pHalData->RegReg542 &= ~BIT(0);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);

	CheckFwRsvdPageContent(padapter);  // 2010.06.23. Added by tynli.
}

static void _BeaconFunctionEnable(PADAPTER padapter, u8 Enable, u8 Linked)
{
	SetBcnCtrlReg(padapter, DIS_TSF_UDT | EN_BCN_FUNCTION | DIS_BCNQ_SUB, 0);
	rtw_write8(padapter, REG_RD_CTRL+1, 0x6F);
}

static void rtl8723b_SetBeaconRelatedRegisters(PADAPTER padapter)
{
	u32 value32;
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;


	//reset TSF, enable update TSF, correcting TSF On Beacon

	//REG_BCN_INTERVAL
	//REG_BCNDMATIM
	//REG_ATIMWND
	//REG_TBTT_PROHIBIT
	//REG_DRVERLYINT
	//REG_BCN_MAX_ERR
	//REG_BCNTCFG //(0x510)
	//REG_DUAL_TSF_RST
	//REG_BCN_CTRL //(0x550)

	//
	// ATIM window
	//
	rtw_write16(padapter, REG_ATIMWND, 2);

	//
	// Beacon interval (in unit of TU).
	//
	rtw_write16(padapter, REG_BCN_INTERVAL, pmlmeinfo->bcn_interval);

	rtl8723b_InitBeaconParameters(padapter);

	rtw_write8(padapter, REG_SLOT, 0x09);

	//
	// Reset TSF Timer to zero, added by Roger. 2008.06.24
	//
	value32 = rtw_read32(padapter, REG_TCR);
	value32 &= ~TSFRST;
	rtw_write32(padapter, REG_TCR, value32);

	value32 |= TSFRST;
	rtw_write32(padapter, REG_TCR, value32);

	// NOTE: Fix test chip's bug (about contention windows's randomness)
	if (check_fwstate(&padapter->mlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE|WIFI_AP_STATE) == _TRUE)
	{
		rtw_write8(padapter, REG_RXTSF_OFFSET_CCK, 0x50);
		rtw_write8(padapter, REG_RXTSF_OFFSET_OFDM, 0x50);
	}

	_BeaconFunctionEnable(padapter, _TRUE, _TRUE);

	ResumeTxBeacon(padapter);
	SetBcnCtrlReg(padapter, DIS_BCNQ_SUB, 0);
}

void rtl8723b_GetHalODMVar(
	PADAPTER				Adapter,
	HAL_ODM_VARIABLE		eVariable,
	PVOID					pValue1,
	BOOLEAN					bSet)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T podmpriv = &pHalData->odmpriv;
	switch(eVariable){
		case HAL_ODM_STA_INFO:
			break;
		default:
			break;
	}
}

void rtl8723b_SetHalODMVar(
	PADAPTER				Adapter,
	HAL_ODM_VARIABLE		eVariable,
	PVOID					pValue1,
	BOOLEAN					bSet)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T podmpriv = &pHalData->odmpriv;
	switch(eVariable){
		case HAL_ODM_STA_INFO:
			{
				struct sta_info *psta = (struct sta_info *)pValue1;
				#ifdef CONFIG_CONCURRENT_MODE
				//get Primary adapter's odmpriv
				if(Adapter->adapter_type > PRIMARY_ADAPTER && Adapter->pbuddy_adapter){
					pHalData = GET_HAL_DATA(Adapter->pbuddy_adapter);
					podmpriv = &pHalData->odmpriv;
				}
				#endif

				if(bSet){
					DBG_8192C("Set STA_(%d) info\n",psta->mac_id);
					ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS,psta->mac_id,psta);
				}
				else{
					DBG_8192C("Clean STA_(%d) info\n",psta->mac_id);
					ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS,psta->mac_id,NULL);
				}
			}
			break;
		case HAL_ODM_P2P_STATE:
				ODM_CmnInfoUpdate(podmpriv,ODM_CMNINFO_WIFI_DIRECT,bSet);
			break;
		case HAL_ODM_WIFI_DISPLAY_STATE:
				ODM_CmnInfoUpdate(podmpriv,ODM_CMNINFO_WIFI_DISPLAY,bSet);
			break;
		default:
			break;
	}
}
void hal_notch_filter_8723b(_adapter *adapter, bool enable)
{
	if (enable) {
		DBG_871X("Enable notch filter\n");
		rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) | BIT1);
	} else {
		DBG_871X("Disable notch filter\n");
		rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) & ~BIT1);
	}
}
void rtl8723b_set_hal_ops(struct hal_ops *pHalFunc)
{
	pHalFunc->free_hal_data = &rtl8723b_free_hal_data;

	pHalFunc->dm_init = &rtl8723b_init_dm_priv;
	pHalFunc->dm_deinit = &rtl8723b_deinit_dm_priv;

	pHalFunc->read_chip_version = &rtl8723b_read_chip_version;

	pHalFunc->set_bwmode_handler = &PHY_SetBWMode8723B;
	pHalFunc->set_channel_handler = &PHY_SwChnl8723B;

	pHalFunc->hal_dm_watchdog = &rtl8723b_HalDmWatchDog;

	pHalFunc->SetBeaconRelatedRegistersHandler = &rtl8723b_SetBeaconRelatedRegisters;

	pHalFunc->Add_RateATid = &rtl8723b_Add_RateATid;

	pHalFunc->run_thread= &rtl8723b_start_thread;
	pHalFunc->cancel_thread= &rtl8723b_stop_thread;

	pHalFunc->read_bbreg = &PHY_QueryBBReg_8723B;
	pHalFunc->write_bbreg = &PHY_SetBBReg_8723B;
	pHalFunc->read_rfreg = &PHY_QueryRFReg_8723B;
	pHalFunc->write_rfreg = &PHY_SetRFReg_8723B;

	// Efuse related function
	pHalFunc->EfusePowerSwitch = &Hal_EfusePowerSwitch;
	pHalFunc->ReadEFuse = &Hal_ReadEFuse;
	pHalFunc->EFUSEGetEfuseDefinition = &Hal_GetEfuseDefinition;
	pHalFunc->EfuseGetCurrentSize = &Hal_EfuseGetCurrentSize;
	pHalFunc->Efuse_PgPacketRead = &Hal_EfusePgPacketRead;
	pHalFunc->Efuse_PgPacketWrite = &Hal_EfusePgPacketWrite;
	pHalFunc->Efuse_WordEnableDataWrite = &Hal_EfuseWordEnableDataWrite;
	pHalFunc->Efuse_PgPacketWrite_BT = &Hal_EfusePgPacketWrite_BT;

#ifdef DBG_CONFIG_ERROR_DETECT
	pHalFunc->sreset_init_value = &sreset_init_value;
	pHalFunc->sreset_reset_value = &sreset_reset_value;
	pHalFunc->silentreset = &rtl8723b_silentreset_for_specific_platform;
	pHalFunc->sreset_xmit_status_check = &rtl8723b_sreset_xmit_status_check;
	pHalFunc->sreset_linked_status_check  = &rtl8723b_sreset_linked_status_check;
	pHalFunc->sreset_get_wifi_status  = &sreset_get_wifi_status;
#endif
	pHalFunc->GetHalODMVarHandler = &rtl8723b_GetHalODMVar;
	pHalFunc->SetHalODMVarHandler = &rtl8723b_SetHalODMVar;

#ifdef CONFIG_XMIT_THREAD_MODE
	pHalFunc->xmit_thread_handler = &hal_xmit_handler;
#endif
	pHalFunc->hal_notch_filter = &hal_notch_filter_8723b;
}

void rtl8723b_InitAntenna_Selection(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 val;


	pHalData = GET_HAL_DATA(padapter);

	val = rtw_read8(padapter, REG_LEDCFG2);
	// Let 8051 take control antenna settting
	val |= BIT(7); // DPDT_SEL_EN, 0x4C[23]
	rtw_write8(padapter, REG_LEDCFG2, val);
}

void rtl8723b_CheckAntenna_Selection(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 val;


	pHalData = GET_HAL_DATA(padapter);

	val = rtw_read8(padapter, REG_LEDCFG2);
	// Let 8051 take control antenna settting
	if(!(val &BIT(7))){
		val |= BIT(7); // DPDT_SEL_EN, 0x4C[23]
		rtw_write8(padapter, REG_LEDCFG2, val);
	}
}
void rtl8723b_DeinitAntenna_Selection(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 val;


	pHalData = GET_HAL_DATA(padapter);
	val = rtw_read8(padapter, REG_LEDCFG2);
	// Let 8051 take control antenna settting
	val &= ~BIT(7); // DPDT_SEL_EN, clear 0x4C[23]
	rtw_write8(padapter, REG_LEDCFG2, val);

}

void rtl8723b_init_default_value(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	struct dm_priv *pdmpriv;
	u8 i;


	pHalData = GET_HAL_DATA(padapter);
	pdmpriv = &pHalData->dmpriv;

	// init default value
	pHalData->fw_ractrl = _FALSE;
	pHalData->bIQKInitialized = _FALSE;
	if (!adapter_to_pwrctl(padapter)->bkeepfwalive)
		pHalData->LastHMEBoxNum = 0;

	pHalData->bIQKInitialized = _FALSE;

	// init dm default value
	pdmpriv->TM_Trigger = 0;//for IQK
//	pdmpriv->binitialized = _FALSE;
//	pdmpriv->prv_traffic_idx = 3;
//	pdmpriv->initialize = 0;

	pdmpriv->ThermalValue_HP_index = 0;
	for (i=0; i<HP_THERMAL_NUM; i++)
		pdmpriv->ThermalValue_HP[i] = 0;

	// init Efuse variables
	pHalData->EfuseUsedBytes = 0;
	pHalData->EfuseUsedPercentage = 0;
#ifdef HAL_EFUSE_MEMORY
	pHalData->EfuseHal.fakeEfuseBank = 0;
	pHalData->EfuseHal.fakeEfuseUsedBytes = 0;
	_rtw_memset(pHalData->EfuseHal.fakeEfuseContent, 0xFF, EFUSE_MAX_HW_SIZE);
	_rtw_memset(pHalData->EfuseHal.fakeEfuseInitMap, 0xFF, EFUSE_MAX_MAP_LEN);
	_rtw_memset(pHalData->EfuseHal.fakeEfuseModifiedMap, 0xFF, EFUSE_MAX_MAP_LEN);
	pHalData->EfuseHal.BTEfuseUsedBytes = 0;
	pHalData->EfuseHal.BTEfuseUsedPercentage = 0;
	_rtw_memset(pHalData->EfuseHal.BTEfuseContent, 0xFF, EFUSE_MAX_BT_BANK*EFUSE_MAX_HW_SIZE);
	_rtw_memset(pHalData->EfuseHal.BTEfuseInitMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset(pHalData->EfuseHal.BTEfuseModifiedMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	pHalData->EfuseHal.fakeBTEfuseUsedBytes = 0;
	_rtw_memset(pHalData->EfuseHal.fakeBTEfuseContent, 0xFF, EFUSE_MAX_BT_BANK*EFUSE_MAX_HW_SIZE);
	_rtw_memset(pHalData->EfuseHal.fakeBTEfuseInitMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset(pHalData->EfuseHal.fakeBTEfuseModifiedMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
#endif
}

u8 GetEEPROMSize8723B(PADAPTER padapter)
{
	u8 size = 0;
	u32	cr;

	cr = rtw_read16(padapter, REG_9346CR);
	// 6: EEPROM used is 93C46, 4: boot from E-Fuse.
	size = (cr & BOOT_FROM_EEPROM) ? 6 : 4;

	MSG_8192C("EEPROM type is %s\n", size==4 ? "E-FUSE" : "93C46");

	return size;
}

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
//-------------------------------------------------------------------------
//
// LLT R/W/Init function
//
//-------------------------------------------------------------------------
s32 _LLTWrite(PADAPTER padapter, u32 address, u32 data)
{
	s32	status = _SUCCESS;
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | _LLT_OP(_LLT_WRITE_ACCESS);
	u16	LLTReg = REG_LLT_INIT;


	status = rtw_write32(padapter, LLTReg, value);
	if (_FAIL == status)
		return _FAIL;

	//polling
	do {
		value = rtw_read32(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			break;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling write LLT done at address %d!\n", address));
			status = _FAIL;
			break;
		}
	} while (count++);

	return status;
}

u8 _LLTRead(PADAPTER padapter, u32 address)
{
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_OP(_LLT_READ_ACCESS);
	u16	LLTReg = REG_LLT_INIT;


	rtw_write32(padapter, LLTReg, value);

	//polling and get value
	do {
		value = rtw_read32(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			return (u8)value;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling read LLT done at address %d!\n", address));
			break;
		}
	} while (count++);

	return 0xFF;
}

s32 InitLLTTable(PADAPTER padapter, u32 boundary)
{
	s32	status = _SUCCESS;
	u32	i;
	u32	txpktbuf_bndy = boundary;
	u32	Last_Entry_Of_TxPktBuf = LAST_ENTRY_OF_TX_PKT_BUFFER;
	HAL_DATA_TYPE *pHalData	= GET_HAL_DATA(padapter);

	for (i = 0; i < (txpktbuf_bndy - 1); i++) {
		status = _LLTWrite(padapter, i, i + 1);
		if (_SUCCESS != status) {
			return status;
		}
	}

	// end of list
	status = _LLTWrite(padapter, (txpktbuf_bndy - 1), 0xFF);
	if (_SUCCESS != status) {
		return status;
	}

	// Make the other pages as ring buffer
	// This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer.
	// Otherwise used as local loopback buffer.
	for (i = txpktbuf_bndy; i < Last_Entry_Of_TxPktBuf; i++) {
		status = _LLTWrite(padapter, i, (i + 1));
		if (_SUCCESS != status) {
			return status;
		}
	}

	// Let last entry point to the start entry of ring buffer
	status = _LLTWrite(padapter, Last_Entry_Of_TxPktBuf, txpktbuf_bndy);
	if (_SUCCESS != status) {
		return status;
	}

	return status;
}
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
void _DisableGPIO(PADAPTER	padapter)
{
/***************************************
j. GPIO_PIN_CTRL 0x44[31:0]=0x000		//
k.Value = GPIO_PIN_CTRL[7:0]
l. GPIO_PIN_CTRL 0x44[31:0] = 0x00FF0000 | (value <<8); //write external PIN level
m. GPIO_MUXCFG 0x42 [15:0] = 0x0780
n. LEDCFG 0x4C[15:0] = 0x8080
***************************************/
	u8	value8;
	u16	value16;
	u32	value32;
	u32	u4bTmp;


	//1. Disable GPIO[7:0]
	rtw_write16(padapter, REG_GPIO_PIN_CTRL+2, 0x0000);
	value32 = rtw_read32(padapter, REG_GPIO_PIN_CTRL) & 0xFFFF00FF;
	u4bTmp = value32 & 0x000000FF;
	value32 |= ((u4bTmp<<8) | 0x00FF0000);
	rtw_write32(padapter, REG_GPIO_PIN_CTRL, value32);

	if (IS_HARDWARE_TYPE_8723AU(padapter) ||
		IS_HARDWARE_TYPE_8723AS(padapter))
	{
		//
		// <Roger_Notes> For RTL8723u multi-function configuration which was autoload from Efuse offset 0x0a and 0x0b,
		// WLAN HW GPIO[9], GPS HW GPIO[10] and BT HW GPIO[11].
		// Added by Roger, 2010.10.07.
		//
		//2. Disable GPIO[8] and GPIO[12]
		rtw_write16(padapter, REG_GPIO_IO_SEL_2, 0x0000); // Configure all pins as input mode.
	    	value32 = rtw_read32(padapter, REG_GPIO_PIN_CTRL_2) & 0xFFFF001F;
		u4bTmp = value32 & 0x0000001F;
//		if( IS_MULTI_FUNC_CHIP(padapter) )
//			value32 |= ((u4bTmp<<8) | 0x00110000); // Set pin 8 and pin 12 to output mode.
//		else
			value32 |= ((u4bTmp<<8) | 0x001D0000); // Set pin 8, 10, 11 and pin 12 to output mode.
		rtw_write32(padapter, REG_GPIO_PIN_CTRL_2, value32);
	}
	else
	{
		//2. Disable GPIO[10:8]
		rtw_write8(padapter, REG_MAC_PINMUX_CFG, 0x00);
		value16 = rtw_read16(padapter, REG_GPIO_IO_SEL) & 0xFF0F;
		value8 = (u8) (value16&0x000F);
		value16 |= ((value8<<4) | 0x0780);
		rtw_write16(padapter, REG_GPIO_IO_SEL, value16);
	}

	//3. Disable LED0 & 1
	if(IS_HARDWARE_TYPE_8192DU(padapter))
	{
		rtw_write16(padapter, REG_LEDCFG0, 0x8888);
	}
	else
	{
		rtw_write16(padapter, REG_LEDCFG0, 0x8080);
	}
//	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> Disable GPIO and LED.\n"));
} //end of _DisableGPIO()

void _DisableRFAFEAndResetBB8192C(PADAPTER padapter)
{
/**************************************
a.	TXPAUSE 0x522[7:0] = 0xFF             //Pause MAC TX queue
b.	RF path 0 offset 0x00 = 0x00            // disable RF
c. 	APSD_CTRL 0x600[7:0] = 0x40
d.	SYS_FUNC_EN 0x02[7:0] = 0x16		//reset BB state machine
e.	SYS_FUNC_EN 0x02[7:0] = 0x14		//reset BB state machine
***************************************/
    	u8 eRFPath = 0, value8 = 0;

	rtw_write8(padapter, REG_TXPAUSE, 0xFF);

	PHY_SetRFReg(padapter, (RF_RADIO_PATH_E)eRFPath, 0x0, bMaskByte0, 0x0);

	value8 |= APSDOFF;
	rtw_write8(padapter, REG_APSD_CTRL, value8);//0x40

	// Set BB reset at first
	value8 = 0 ;
	value8 |= (FEN_USBD | FEN_USBA | FEN_BB_GLB_RSTn);
	rtw_write8(padapter, REG_SYS_FUNC_EN, value8 );//0x16

	// Set global reset.
	value8 &= ~FEN_BB_GLB_RSTn;
	rtw_write8(padapter, REG_SYS_FUNC_EN, value8); //0x14

	// 2010/08/12 MH We need to set BB/GLBAL reset to save power for SS mode.

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> RF off and reset BB.\n"));
}

void _DisableRFAFEAndResetBB(PADAPTER padapter)
{
#if 0
	if (IS_HARDWARE_TYPE_8192D(padapter))
		_DisableRFAFEAndResetBB8192D(padapter);
	else
#endif
		_DisableRFAFEAndResetBB8192C(padapter);
}

void _ResetDigitalProcedure1_92C(PADAPTER padapter, BOOLEAN bWithoutHWSM)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	if (IS_FW_81xxC(padapter) && (pHalData->FirmwareVersion <= 0x20))
	{
		#if 0
/*****************************
		f.	SYS_FUNC_EN 0x03[7:0]=0x54		// reset MAC register, DCORE
		g.	MCUFWDL 0x80[7:0]=0				// reset MCU ready status
******************************/
	u32	value32 = 0;
		rtw_write8(padapter, REG_SYS_FUNC_EN+1, 0x54);
		rtw_write8(padapter, REG_MCUFWDL, 0);
		#else
		/*****************************
		f.	MCUFWDL 0x80[7:0]=0				// reset MCU ready status
		g.	SYS_FUNC_EN 0x02[10]= 0			// reset MCU register, (8051 reset)
		h.	SYS_FUNC_EN 0x02[15-12]= 5		// reset MAC register, DCORE
		i.     SYS_FUNC_EN 0x02[10]= 1			// enable MCU register, (8051 enable)
		******************************/
			u16 valu16 = 0;
			rtw_write8(padapter, REG_MCUFWDL, 0);

			valu16 = rtw_read16(padapter, REG_SYS_FUNC_EN);
			rtw_write16(padapter, REG_SYS_FUNC_EN, (valu16 & (~FEN_CPUEN)));//reset MCU ,8051

			valu16 = rtw_read16(padapter, REG_SYS_FUNC_EN)&0x0FFF;
			rtw_write16(padapter, REG_SYS_FUNC_EN, (valu16 |(FEN_HWPDN|FEN_ELDR)));//reset MAC

			valu16 = rtw_read16(padapter, REG_SYS_FUNC_EN);
			rtw_write16(padapter, REG_SYS_FUNC_EN, (valu16 | FEN_CPUEN));//enable MCU ,8051
		#endif
	}
	else
	{
		u8 retry_cnts = 0;

		// 2010/08/12 MH For USB SS, we can not stop 8051 when we are trying to
		// enter IPS/HW&SW radio off. For S3/S4/S5/Disable, we can stop 8051 because
		// we will init FW when power on again.
		//if(!pDevice->RegUsbSS)
		{	// If we want to SS mode, we can not reset 8051.
			if(rtw_read8(padapter, REG_MCUFWDL) & BIT1)
			{ //IF fw in RAM code, do reset


				if(padapter->bFWReady)
				{
					// 2010/08/25 MH Accordign to RD alfred's suggestion, we need to disable other
					// HRCV INT to influence 8051 reset.
					rtw_write8(padapter, REG_FWIMR, 0x20);
					// 2011/02/15 MH According to Alex's suggestion, close mask to prevent incorrect FW write operation.
					rtw_write8(padapter, REG_FTIMR, 0x00);
					rtw_write8(padapter, REG_FSIMR, 0x00);

					rtw_write8(padapter, REG_HMETFR+3, 0x20);//8051 reset by self

					while( (retry_cnts++ <100) && (FEN_CPUEN &rtw_read16(padapter, REG_SYS_FUNC_EN)))
					{
						rtw_udelay_os(50);//us
						// 2010/08/25 For test only We keep on reset 5051 to prevent fail.
						//rtw_write8(padapter, REG_HMETFR+3, 0x20);//8051 reset by self
					}
//					RT_ASSERT((retry_cnts < 100), ("8051 reset failed!\n"));

					if (retry_cnts >= 100)
					{
						// if 8051 reset fail we trigger GPIO 0 for LA
						//rtw_write32(	padapter,
						//						REG_GPIO_PIN_CTRL,
						//						0x00010100);
						// 2010/08/31 MH According to Filen's info, if 8051 reset fail, reset MAC directly.
						rtw_write8(padapter, REG_SYS_FUNC_EN+1, 0x50);	//Reset MAC and Enable 8051
						rtw_mdelay_os(10);
					}
//					else
//					RT_TRACE(COMP_INIT, DBG_LOUD, ("=====> 8051 reset success (%d) .\n",retry_cnts));
				}
			}
//			else
//			{
//				RT_TRACE(COMP_INIT, DBG_LOUD, ("=====> 8051 in ROM.\n"));
//			}
			rtw_write8(padapter, REG_SYS_FUNC_EN+1, 0x54);	//Reset MAC and Enable 8051
			rtw_write8(padapter, REG_MCUFWDL, 0);
		}
	}

	//if(pDevice->RegUsbSS)
		//bWithoutHWSM = TRUE;	// Sugest by Filen and Issau.

	if(bWithoutHWSM)
	{
		//HAL_DATA_TYPE		*pHalData	= GET_HAL_DATA(padapter);
	/*****************************
		Without HW auto state machine
	g.	SYS_CLKR 0x08[15:0] = 0x30A3			//disable MAC clock
	h.	AFE_PLL_CTRL 0x28[7:0] = 0x80			//disable AFE PLL
	i.	AFE_XTAL_CTRL 0x24[15:0] = 0x880F		//gated AFE DIG_CLOCK
	j.	SYS_ISO_CTRL 0x00[7:0] = 0xF9			// isolated digital to PON
	******************************/
		//rtw_write16(padapter, REG_SYS_CLKR, 0x30A3);
		//if(!pDevice->RegUsbSS)
		// 2011/01/26 MH SD4 Scott suggest to fix UNC-B cut bug.
		//if (IS_81xxC_VENDOR_UMC_B_CUT(pHalData->VersionID))
			//rtw_write16(padapter, REG_SYS_CLKR, (0x70A3|BIT6));  //modify to 0x70A3 by Scott.
		//else
			rtw_write16(padapter, REG_SYS_CLKR, 0x70A3);  //modify to 0x70A3 by Scott.
		rtw_write8(padapter, REG_AFE_PLL_CTRL, 0x80);
		rtw_write16(padapter, REG_AFE_XTAL_CTRL, 0x880F);
		//if(!pDevice->RegUsbSS)
			rtw_write8(padapter, REG_SYS_ISO_CTRL, 0xF9);
	}
	else
	{
		// Disable all RF/BB power
		rtw_write8(padapter, REG_RF_CTRL, 0x00);
	}
//	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> Reset Digital.\n"));

}

void _ResetDigitalProcedure1(PADAPTER padapter, BOOLEAN bWithoutHWSM)
{
#if 0
	if(IS_HARDWARE_TYPE_8192D(padapter))
		_ResetDigitalProcedure1_92D(padapter, bWithoutHWSM);
	else
#endif
		_ResetDigitalProcedure1_92C(padapter, bWithoutHWSM);
}

void _ResetDigitalProcedure2(PADAPTER padapter)
{
	//HAL_DATA_TYPE		*pHalData	= GET_HAL_DATA(padapter);
/*****************************
k.	SYS_FUNC_EN 0x03[7:0] = 0x44			// disable ELDR runction
l.	SYS_CLKR 0x08[15:0] = 0x3083			// disable ELDR clock
m.	SYS_ISO_CTRL 0x01[7:0] = 0x83			// isolated ELDR to PON
******************************/
	//rtw_write8(padapter, REG_SYS_FUNC_EN+1, 0x44); //marked by Scott.
	// 2011/01/26 MH SD4 Scott suggest to fix UNC-B cut bug.
	//if (IS_81xxC_VENDOR_UMC_B_CUT(pHalData->VersionID))
		//rtw_write16(padapter, REG_SYS_CLKR, 0x70a3|BIT6);
	//else
		rtw_write16(padapter, REG_SYS_CLKR, 0x70a3); //modify to 0x70a3 by Scott.
	rtw_write8(padapter, REG_SYS_ISO_CTRL+1, 0x82); //modify to 0x82 by Scott.
}

void _DisableAnalog(PADAPTER padapter, BOOLEAN bWithoutHWSM)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	u16 value16 = 0;
	u8 value8 = 0;


	if (bWithoutHWSM)
	{
		/*****************************
		n.	LDOA15_CTRL 0x20[7:0] = 0x04		// disable A15 power
		o.	LDOV12D_CTRL 0x21[7:0] = 0x54		// disable digital core power
		r.	When driver call disable, the ASIC will turn off remaining clock automatically
		******************************/

		rtw_write8(padapter, REG_LDOA15_CTRL, 0x04);
		//rtw_write8(padapter, REG_LDOV12D_CTRL, 0x54);

		value8 = rtw_read8(padapter, REG_LDOV12D_CTRL);
		value8 &= (~LDV12_EN);
		rtw_write8(padapter, REG_LDOV12D_CTRL, value8);
//		RT_TRACE(COMP_INIT, DBG_LOUD, (" REG_LDOV12D_CTRL Reg0x21:0x%02x.\n",value8));
	}

	/*****************************
	h.	SPS0_CTRL 0x11[7:0] = 0x23			//enter PFM mode
	i.	APS_FSMCO 0x04[15:0] = 0x4802		// set USB suspend
	******************************/
	value8 = 0x23;
	if (IS_81xxC_VENDOR_UMC_B_CUT(pHalData->VersionID))
		value8 |= BIT3;

	rtw_write8(padapter, REG_SPS0_CTRL, value8);

	if(bWithoutHWSM)
	{
		//value16 |= (APDM_HOST | /*AFSM_HSUS |*/PFM_ALDN);
		// 2010/08/31 According to Filen description, we need to use HW to shut down 8051 automatically.
		// Becasue suspend operatione need the asistance of 8051 to wait for 3ms.
		value16 |= (APDM_HOST | AFSM_HSUS | PFM_ALDN);
	}
	else
	{
		value16 |= (APDM_HOST | AFSM_HSUS | PFM_ALDN);
	}

	rtw_write16(padapter, REG_APS_FSMCO, value16);//0x4802

	rtw_write8(padapter, REG_RSV_CTRL, 0x0e);

#if 0
	//tynli_test for suspend mode.
	if(!bWithoutHWSM){
		rtw_write8(padapter, 0xfe10, 0x19);
	}
#endif

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> Disable Analog Reg0x04:0x%04x.\n",value16));
}

// HW Auto state machine
s32 CardDisableHWSM(PADAPTER padapter, u8 resetMCU)
{
	int rtStatus = _SUCCESS;


	if (padapter->bSurpriseRemoved){
		return rtStatus;
	}
	//==== RF Off Sequence ====
	_DisableRFAFEAndResetBB(padapter);

	//  ==== Reset digital sequence   ======
	_ResetDigitalProcedure1(padapter, _FALSE);

	//  ==== Pull GPIO PIN to balance level and LED control ======
	_DisableGPIO(padapter);

	//  ==== Disable analog sequence ===
	_DisableAnalog(padapter, _FALSE);

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("======> Card disable finished.\n"));

	return rtStatus;
}

// without HW Auto state machine
s32 CardDisableWithoutHWSM(PADAPTER padapter)
{
	s32 rtStatus = _SUCCESS;


	//RT_TRACE(COMP_INIT, DBG_LOUD, ("======> Card Disable Without HWSM .\n"));
	if (padapter->bSurpriseRemoved) {
		return rtStatus;
	}

	//==== RF Off Sequence ====
	_DisableRFAFEAndResetBB(padapter);

	//  ==== Reset digital sequence   ======
	_ResetDigitalProcedure1(padapter, _TRUE);

	//  ==== Pull GPIO PIN to balance level and LED control ======
	_DisableGPIO(padapter);

	//  ==== Reset digital sequence   ======
	_ResetDigitalProcedure2(padapter);

	//  ==== Disable analog sequence ===
	_DisableAnalog(padapter, _TRUE);

	//RT_TRACE(COMP_INIT, DBG_LOUD, ("<====== Card Disable Without HWSM .\n"));
	return rtStatus;
}

#endif
BOOLEAN
Hal_GetChnlGroup8723B(
	IN	u8 Channel,
	OUT u8 *pGroup
	)
{
	BOOLEAN bIn24G=TRUE;

	if(Channel <= 14)
	{
		bIn24G=TRUE;

		if      (1  <= Channel && Channel <= 2 )   *pGroup = 0;
		else if (3  <= Channel && Channel <= 5 )   *pGroup = 1;
		else if (6  <= Channel && Channel <= 8 )   *pGroup = 2;
		else if (9  <= Channel && Channel <= 11)   *pGroup = 3;
		else if (12 <= Channel && Channel <= 14)   *pGroup = 4;
		else
		{
			RT_TRACE(_module_hci_hal_init_c_, _drv_notice_,("==>Hal_GetChnlGroup8723B in 2.4 G, but Channel %d in Group not found \n", Channel));
		}
	}
	else
	{
		bIn24G=FALSE;

		if      (36   <= Channel && Channel <=  42)   *pGroup = 0;
		else if (44   <= Channel && Channel <=  48)   *pGroup = 1;
		else if (50   <= Channel && Channel <=  58)   *pGroup = 2;
		else if (60   <= Channel && Channel <=  64)   *pGroup = 3;
		else if (100  <= Channel && Channel <= 106)   *pGroup = 4;
		else if (108  <= Channel && Channel <= 114)   *pGroup = 5;
		else if (116  <= Channel && Channel <= 122)   *pGroup = 6;
		else if (124  <= Channel && Channel <= 130)   *pGroup = 7;
		else if (132  <= Channel && Channel <= 138)   *pGroup = 8;
		else if (140  <= Channel && Channel <= 144)   *pGroup = 9;
		else if (149  <= Channel && Channel <= 155)   *pGroup = 10;
		else if (157  <= Channel && Channel <= 161)   *pGroup = 11;
		else if (165  <= Channel && Channel <= 171)   *pGroup = 12;
		else if (173  <= Channel && Channel <= 177)   *pGroup = 13;
		else
		{
			RT_TRACE(_module_hci_hal_init_c_, _drv_notice_,("==>Hal_GetChnlGroup8723B in 5G, but Channel %d in Group not found \n",Channel));
		}

	}
	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("<==Hal_GetChnlGroup8723B,  (%s) Channel = %d, Group =%d,\n",
                                  (bIn24G) ? "2.4G" : "5G", Channel, *pGroup));
	return bIn24G;
}

void
Hal_InitPGData(
	PADAPTER	padapter,
	u8			*PROMContent)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
//	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	u32			i;
	u16			value16;

	if(_FALSE == pEEPROM->bautoload_fail_flag)
	{ // autoload OK.
//		if (IS_BOOT_FROM_EEPROM(padapter))
		if (_TRUE == pEEPROM->EepromOrEfuse)
		{
			// Read all Content from EEPROM or EFUSE.
			for(i = 0; i < HWSET_MAX_SIZE; i += 2)
			{
//				value16 = EF2Byte(ReadEEprom(pAdapter, (u2Byte) (i>>1)));
//				*((u16*)(&PROMContent[i])) = value16;
			}
		}
		else
		{
			// Read EFUSE real map to shadow.
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, _FALSE);
			_rtw_memcpy((void*)PROMContent, (void*)pEEPROM->efuse_eeprom_data, HWSET_MAX_SIZE);
		}
	}
	else
	{//autoload fail
		RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("AutoLoad Fail reported from CR9346!!\n"));
//		pHalData->AutoloadFailFlag = _TRUE;
		//update to default value 0xFF
		if (_FALSE == pEEPROM->EepromOrEfuse)
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, _FALSE);
		_rtw_memcpy((void*)PROMContent, (void*)pEEPROM->efuse_eeprom_data, HWSET_MAX_SIZE);
	}
}

void
Hal_EfuseParseIDCode(
	IN	PADAPTER	padapter,
	IN	u8			*hwinfo
	)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
//	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	u16			EEPROMId;


	// Checl 0x8129 again for making sure autoload status!!
	EEPROMId = le16_to_cpu(*((u16*)hwinfo));
	if (EEPROMId != RTL_EEPROM_ID)
	{
		DBG_8192C("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		pEEPROM->bautoload_fail_flag = _TRUE;
	}
	else
	{
		pEEPROM->bautoload_fail_flag = _FALSE;
	}

	RT_TRACE(_module_hal_init_c_, _drv_notice_, ("EEPROM ID=0x%04x\n", EEPROMId));
}

static void
Hal_EEValueCheck(
	IN		u8		EEType,
	IN		PVOID		pInValue,
	OUT		PVOID		pOutValue
	)
{
	switch(EEType)
	{
		case EETYPE_TX_PWR_8723B:
			{
				u8	*pIn, *pOut;
				pIn = (u8*)pInValue;
				pOut = (u8*)pOutValue;
				if(*pIn >= 0 && *pIn <= 63)
				{
					*pOut = *pIn;
				}
				else
				{
					RT_TRACE(_module_hci_hal_init_c_, _drv_err_, ("EETYPE_TX_PWR_8723B, value=%d is invalid, set to default=0x%x\n",
						*pIn, EEPROM_Default_TxPowerLevel));
					*pOut = EEPROM_Default_TxPowerLevel;
				}
			}
			break;
		default:
			break;
	}
}

static u8
Hal_GetChnlGroup(
	IN	u8 chnl
	)
{
	u8	group=0;

	if (chnl < 3)			// Cjanel 1-3
		group = 0;
	else if (chnl < 9)		// Channel 4-9
		group = 1;
	else					// Channel 10-14
		group = 2;

	return group;
}

void
Hal_ReadPowerValueFromPROM_8723B(
	IN	PADAPTER 		Adapter,
	IN	PTxPowerInfo24G	pwrInfo24G,
	IN	u8 				* PROMContent,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte rfPath, eeAddr=EEPROM_TX_PWR_INX_8723B, group,TxCount=0;

	_rtw_memset(pwrInfo24G, 0, sizeof(TxPowerInfo24G));

	if(0xFF == PROMContent[eeAddr+1])
		AutoLoadFail = TRUE;

	if(AutoLoadFail)
	{
		DBG_871X("%s(): Use Default value!\n", __func__);
		for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
		{
			//2.4G default value
			for(group = 0 ; group < MAX_CHNL_GROUP_24G; group++)
			{
				pwrInfo24G->IndexCCK_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
			}
			for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
			{
				if(TxCount==0)
				{
					pwrInfo24G->BW20_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_HT20_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_OFDM_DIFF;
				}
				else
				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				}
			}
		}

		return;
	}

	pHalData->bTXPowerDataReadFromEEPORM = TRUE;		//YJ,move,120316

	for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
	{
		//2 2.4G default value
		for(group = 0 ; group < MAX_CHNL_GROUP_24G; group++)
		{
			pwrInfo24G->IndexCCK_Base[rfPath][group] =	PROMContent[eeAddr++];
			if(pwrInfo24G->IndexCCK_Base[rfPath][group] == 0xFF)
			{
				pwrInfo24G->IndexCCK_Base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
			}
		}
		for(group = 0 ; group < MAX_CHNL_GROUP_24G-1; group++)
		{
			pwrInfo24G->IndexBW40_Base[rfPath][group] =	PROMContent[eeAddr++];
			if(pwrInfo24G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
		}
		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{
			if(TxCount==0)
			{
				pwrInfo24G->BW40_Diff[rfPath][TxCount] = 0;
				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_24G_HT20_DIFF;
				else
				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
				 	if(pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}

				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_24G_OFDM_DIFF;
				else
				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if(pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				pwrInfo24G->CCK_Diff[rfPath][TxCount] = 0;
				eeAddr++;
			}
			else
			{
				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				else
				{
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if(pwrInfo24G->BW40_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW40_Diff[rfPath][TxCount] |= 0xF0;
				}

				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				else
				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if(pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				eeAddr++;

				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				else
				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if(pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}

				if(PROMContent[eeAddr] == 0xFF)
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				else
				{
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if(pwrInfo24G->CCK_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->CCK_Diff[rfPath][TxCount] |= 0xF0;
				}
				eeAddr++;
			}
		}
	}
}


void
Hal_EfuseParseTxPowerInfo_8723B(
	IN	PADAPTER 		padapter,
	IN	u8*			PROMContent,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	TxPowerInfo24G	pwrInfo24G;
	u8			rfPath, ch, group, TxCount=1;

	Hal_ReadPowerValueFromPROM_8723B(padapter, &pwrInfo24G, PROMContent, AutoLoadFail);
	for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
	{
		for(ch = 0 ; ch < CHANNEL_MAX_NUMBER; ch++)
		{
			Hal_GetChnlGroup8723B(ch+1, &group);

			if(ch == 14-1)
			{
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][5];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}
			else
			{
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][group];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}

#ifdef CONFIG_CMCC_TEST
			if (padapter->drv_in_test == CMCC_TP_TEST)
				pHalData->Index24G_CCK_Base[rfPath][ch] += 2; /* add 1dbm CCK power */
#endif

#ifdef CONFIG_DEBUG
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("Index24G_CCK_Base[%d][%d] = 0x%x\n",rfPath,ch ,pHalData->Index24G_CCK_Base[rfPath][ch]));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("Index24G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index24G_BW40_Base[rfPath][ch]));
#endif
		}

		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{
			pHalData->CCK_24G_Diff[rfPath][TxCount]=pwrInfo24G.CCK_Diff[rfPath][TxCount];
			pHalData->OFDM_24G_Diff[rfPath][TxCount]=pwrInfo24G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_24G_Diff[rfPath][TxCount]=pwrInfo24G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_24G_Diff[rfPath][TxCount]=pwrInfo24G.BW40_Diff[rfPath][TxCount];

#ifdef CONFIG_DEBUG
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("--------------------------------------- 2.4G ---------------------------------------\n"));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("CCK_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->CCK_24G_Diff[rfPath][TxCount]));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("OFDM_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->OFDM_24G_Diff[rfPath][TxCount]));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("BW20_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW20_24G_Diff[rfPath][TxCount]));
			RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("BW40_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW40_24G_Diff[rfPath][TxCount]));
#endif
		}
	}

	// 2010/10/19 MH Add Regulator recognize for CU.
	if(!AutoLoadFail)
	{
		pHalData->EEPROMRegulatory = (PROMContent[EEPROM_RF_BOARD_OPTION_8723B]&0x7);	//bit0~2
		if(PROMContent[EEPROM_RF_BOARD_OPTION_8723B] == 0xFF)
			pHalData->EEPROMRegulatory = (EEPROM_DEFAULT_BOARD_OPTION&0x7);	//bit0~2
	}
	else
	{
		pHalData->EEPROMRegulatory = 0;
	}
	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("EEPROMRegulatory = 0x%x\n", pHalData->EEPROMRegulatory));
}

VOID
Hal_EfuseParseBTCoexistInfo_8723B(
	IN PADAPTER			padapter,
	IN u8*			hwinfo,
	IN BOOLEAN			AutoLoadFail
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	u8			tempval;
	u32			tmpu4;

	if (!AutoLoadFail)
	{
		tmpu4 = rtw_read32(padapter, REG_MULTI_FUNC_CTRL);
		if (tmpu4 & BT_FUNC_EN)
			pHalData->EEPROMBluetoothCoexist = 1;
		else
			pHalData->EEPROMBluetoothCoexist = 0;
		pHalData->EEPROMBluetoothType = BT_RTL8723B;

		// The following need to be checked with newer version of
		// eeprom spec
		tempval = hwinfo[EEPROM_RF_BT_SETTING_8723B];
		pHalData->EEPROMBluetoothAntNum =( (tempval&0x1) == 0?2:1);					// bit [0]
	}
	else
	{
		pHalData->EEPROMBluetoothCoexist = 0;
		pHalData->EEPROMBluetoothType = BT_RTL8723B;
		pHalData->EEPROMBluetoothAntNum = Ant_x2;
	}
#ifdef CONFIG_BT_COEXIST
	BT_InitHalVars(padapter);
#endif
	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("%s(): EEPROMBluetoothCoexist=%d EEPROMBluetoothAntNum = %d\n", __func__, pHalData->EEPROMBluetoothCoexist, pHalData->EEPROMBluetoothAntNum));
}

VOID
Hal_EfuseParsePackage_8723B(
	IN	PADAPTER		padapter,
	IN	u8*			hwinfo)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	u16	EEPROMId = le16_to_cpu(*((u16*)hwinfo));
	u8 buff = 0xFF;
	u32 res;

	//0x1FB bit[0:2]
	res = rtw_efuse_access(padapter, _FALSE, EEPROM_PACKAGE_METHOD_8723BS, 1, &buff);
	
	if (EEPROMId != RTL_EEPROM_ID || res == _FAIL)
		pHalData->EEPROMPackageMethod = EEPROM_PACKAGE_METHOD_DEFAULT;
	else
		pHalData->EEPROMPackageMethod = buff & 0x07;

	if (EEPROM_PACKAGE_METHOD_TFBGA79 == pHalData->EEPROMPackageMethod)
		pHalData->bUseExtSPDT = _FALSE;
	else
		pHalData->bUseExtSPDT = _TRUE;
	
	DBG_871X("Hal_EfuseParsePackage_8723B(), 0x1FB:%x EEPROMPackageMethod = %d bUseExtSPDT:%d\n",
		buff, pHalData->EEPROMPackageMethod, pHalData->bUseExtSPDT);
}

VOID
Hal_EfuseParseEEPROMVer_8723B(
	IN	PADAPTER		padapter,
	IN	u8*			hwinfo,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

//	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("%s(): AutoLoadFail = %d\n", __func__, AutoLoadFail));
	if(!AutoLoadFail)
		pHalData->EEPROMVersion = hwinfo[EEPROM_VERSION_8723B];
	else
		pHalData->EEPROMVersion = 1;
	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("Hal_EfuseParseEEPROMVer(), EEVer = %d\n",
		pHalData->EEPROMVersion));
}

VOID
Hal_EfuseParseChnlPlan_8723B(
	IN	PADAPTER		padapter,
	IN	u8*			hwinfo,
	IN	BOOLEAN			AutoLoadFail
	)
{
	padapter->mlmepriv.ChannelPlan = hal_com_get_channel_plan(
		padapter
		, hwinfo?hwinfo[EEPROM_ChannelPlan_8723B]:0xFF
		, padapter->registrypriv.channel_plan
		, RT_CHANNEL_DOMAIN_WORLD_WIDE_13
		, AutoLoadFail
	);

	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("EEPROM ChannelPlan=0x%02x\n", padapter->mlmepriv.ChannelPlan));
}

VOID
Hal_EfuseParseCustomerID_8723B(
	IN	PADAPTER		padapter,
	IN	u8*			hwinfo,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	if (!AutoLoadFail)
	{
		pHalData->EEPROMCustomerID = hwinfo[EEPROM_CustomID_8723B];
	}
	else
	{
		pHalData->EEPROMCustomerID = 0;
	}
	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("EEPROM Customer ID: 0x%2x\n", pHalData->EEPROMCustomerID));
}

VOID
Hal_EfuseParseAntennaDiversity_8723B(
	IN	PADAPTER		pAdapter,
	IN	u8				* hwinfo,
	IN	BOOLEAN			AutoLoadFail
	)
{
#if 0
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if(!AutoLoadFail)
	{
		// Antenna Diversity setting.
		pHalData->AntDivCfg = (hwinfo[RF_OPTION1_8723A]&0x18)>>3;

		if(BT_1Ant(pAdapter))
			pHalData->AntDivCfg = 0;
		pHalData->ReverseDPDT = (hwinfo[RF_OPTION1_8723A]&BIT5) >> 5;
	}
	else
	{
		pHalData->AntDivCfg = 0;
		pHalData->ReverseDPDT = 1;
	}

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("EEPROM SWAS: bHwAntDiv = %x, TRxAntDivType = %x\n",
                                                pHalData->AntDivCfg, pHalData->TRxAntDivType));
#endif
}

VOID
Hal_EfuseParseXtal_8723B(
	IN	PADAPTER		pAdapter,
	IN	u8			* hwinfo,
	IN	BOOLEAN		AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if(!AutoLoadFail)
	{
		pHalData->CrystalCap = hwinfo[EEPROM_XTAL_8723B];
		if(pHalData->CrystalCap == 0xFF)
			pHalData->CrystalCap = EEPROM_Default_CrystalCap_8723B;	   //what value should 8812 set?
	}
	else
	{
		pHalData->CrystalCap = EEPROM_Default_CrystalCap_8723B;
	}
	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("EEPROM CrystalCap: 0x%2x\n", pHalData->CrystalCap));
}


void
Hal_EfuseParseThermalMeter_8723B(
	PADAPTER	padapter,
	u8			*PROMContent,
	u8			AutoLoadFail
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);

//	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("%s(): AutoLoadFail = %d\n", __func__, AutoLoadFail));
	//
	// ThermalMeter from EEPROM
	//
	if (_FALSE == AutoLoadFail)
		pHalData->EEPROMThermalMeter = PROMContent[EEPROM_THERMAL_METER_8723B];
	else
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_8723B;

	if ((pHalData->EEPROMThermalMeter == 0xff) || (_TRUE == AutoLoadFail))
	{
		pHalData->bAPKThermalMeterIgnore = _TRUE;
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_8723B;
	}

	RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("EEPROM ThermalMeter=0x%x\n", pHalData->EEPROMThermalMeter));
}

void rtl8723b_cal_txdesc_chksum(PTXDESC ptxdesc)
{
	u16	*usPtr = (u16*)ptxdesc;
	u32 count;
	u32 index;
	u16 checksum = 0;

	/*  checksume is always calculated by first 32 bytes, */
	/*  and it doesn't depend on TX DESC length. */
	count = 16;

	for (index = 0; index < count; index++) {
		checksum ^= le16_to_cpu(*(usPtr + index));
	}

	ptxdesc->checksum = (checksum & 0x0000ffff);
}

static u8 fill_txdesc_sectype(struct pkt_attrib *pattrib)
{
	u8 sectype = 0;
	if ((pattrib->encrypt > 0) && !pattrib->bswenc)
	{
		switch (pattrib->encrypt)
		{
			// SEC_TYPE
			case _WEP40_:
			case _WEP104_:
			case _TKIP_:
			case _TKIP_WTMIC_:
				sectype = 1;
				break;

#ifdef CONFIG_WAPI_SUPPORT
			case _SMS4_:
				sectype = 2;
				break;
#endif
			case _AES_:
				sectype = 3;
				break;

			case _NO_PRIVACY_:
			default:
					break;
		}
	}
	return sectype;
}

static void fill_txdesc_vcs_8723b(struct pkt_attrib *pattrib, PTXDESC ptxdesc)
{
	//DBG_8192C("cvs_mode=%d\n", pattrib->vcs_mode);

	switch (pattrib->vcs_mode)
	{
		case RTS_CTS:
			ptxdesc->rtsen = 1;
			break;

		case CTS_TO_SELF:
			ptxdesc->cts2self = 1;
			break;

		case NONE_VCS:
		default:
			break;
	}

	if(pattrib->vcs_mode) {
		ptxdesc->hw_rts_en = 1; // ENABLE HW RTS

		// Set RTS BW
		if(pattrib->ht_en)
		{
			switch (pattrib->ch_offset)
			{
				case HAL_PRIME_CHNL_OFFSET_DONT_CARE:
					ptxdesc->rts_sc = 0;
					break;

				case HAL_PRIME_CHNL_OFFSET_LOWER:
					ptxdesc->rts_sc = 1;
					break;

				case HAL_PRIME_CHNL_OFFSET_UPPER:
					ptxdesc->rts_sc = 2;
					break;

				default:
					ptxdesc->rts_sc = 3; // Duplicate
					break;
			}
		}
	}
}

static void fill_txdesc_phy_8723b(struct pkt_attrib *pattrib, PTXDESC ptxdesc)
{
	//DBG_8192C("bwmode=%d, ch_off=%d\n", pattrib->bwmode, pattrib->ch_offset);

	if (pattrib->ht_en)
	{
		if (pattrib->bwmode & HT_CHANNEL_WIDTH_40)
			ptxdesc->data_bw = 1;

		switch (pattrib->ch_offset)
		{
			case HAL_PRIME_CHNL_OFFSET_DONT_CARE:
				ptxdesc->data_sc = 0;
				break;

			case HAL_PRIME_CHNL_OFFSET_LOWER:
				ptxdesc->data_sc = 1;
				break;

			case HAL_PRIME_CHNL_OFFSET_UPPER:
				ptxdesc->data_sc = 2;
				break;

			default:
				ptxdesc->data_sc = 3; // Duplicate
				break;
		}
	}
}

static void rtl8723b_fill_default_txdesc(
	struct xmit_frame *pxmitframe,
	u8 *pbuf)
{
	PADAPTER padapter;
	HAL_DATA_TYPE *pHalData;
	struct dm_priv *pdmpriv;
	struct mlme_ext_priv *pmlmeext;
	struct mlme_ext_info *pmlmeinfo;
	struct pkt_attrib *pattrib;
	PTXDESC ptxdesc;
	s32 bmcst;

	_rtw_memset(pbuf, 0, TXDESC_SIZE);

	padapter = pxmitframe->padapter;
	pHalData = GET_HAL_DATA(padapter);
	pdmpriv = &pHalData->dmpriv;
	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &(pmlmeext->mlmext_info);

	pattrib = &pxmitframe->attrib;
	bmcst = IS_MCAST(pattrib->ra);

	ptxdesc = (PTXDESC)pbuf;

	if (pxmitframe->frame_tag == DATA_FRAMETAG)
	{
		ptxdesc->macid = pattrib->mac_id; // CAM_ID(MAC_ID)

		if (pattrib->ampdu_en == _TRUE)
			ptxdesc->agg_en = 1; // AGG EN
		else
			ptxdesc->bk = 1; // AGG BK

		ptxdesc->qsel = pattrib->qsel;
#ifdef CONFIG_BT_COEXIST
		if(pHalData->bBtScoBusy){
			ptxdesc->rate_id = 9; //define by register 0x444
		}
		else
#endif
		ptxdesc->rate_id = pattrib->raid;

		ptxdesc->sectype = fill_txdesc_sectype(pattrib);

		ptxdesc->seq = pattrib->seqnum;

		if ((pattrib->ether_type != 0x888e) &&
			(pattrib->ether_type != 0x0806) &&
			(pattrib->ether_type != 0x88B4) &&
			(pattrib->dhcp_pkt != 1))
		{
			// Non EAP & ARP & DHCP type data packet

			fill_txdesc_vcs_8723b(pattrib, ptxdesc);
			fill_txdesc_phy_8723b(pattrib, ptxdesc);

			ptxdesc->rtsrate = 8; // RTS Rate=24M
			ptxdesc->data_ratefb_lmt = 0x1F;
			ptxdesc->rts_ratefb_lmt = 0xF;

#ifdef CONFIG_CMCC_TEST
			if (padapter->drv_in_test == CMCC_TP_TEST)
				ptxdesc->data_short = 1; /* use cck short premble */
#endif
			if (padapter->drv_in_test == CMCC_RF_TEST) {
				ptxdesc->userate = 1; // driver uses rate
				ptxdesc->rate_id = 9; //define by register 0x444
			}


			// use REG_INIDATA_RATE_SEL value
			ptxdesc->datarate = pdmpriv->INIDATA_RATE[pattrib->mac_id];

#ifdef CONFIG_WOWLAN
			if(TRUE == adapter_to_pwrctl(padapter)->wowlan_mode)
			{
				ptxdesc->userate = 1; // driver uses rate
				if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT)
					ptxdesc->data_short = 1;// DATA_SHORT
				ptxdesc->datarate = MRateToHwRate(pmlmeext->tx_rate);
			}
#endif
#if 0
			ptxdesc->userate = 1; // driver uses rate

			if (pattrib->ht_en)
				ptxdesc->data_short = 1; // SGI

			ptxdesc->datarate = 0x13; // init rate - mcs7
#endif
		}
		else
		{
			// EAP data packet and ARP packet.
			// Use the 1M data rate to send the EAP/ARP packet.
			// This will maybe make the handshake smooth.

			ptxdesc->bk = 1; // AGG BK
			ptxdesc->userate = 1; // driver uses rate
			if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT)
				ptxdesc->data_short = 1;// DATA_SHORT
			ptxdesc->datarate = MRateToHwRate(pmlmeext->tx_rate);
			//DBG_871X("YJ: %s(): ARP Data: userate=%d, datarate=0x%x\n", __func__, ptxdesc->userate, ptxdesc->datarate);
		}
#if defined(CONFIG_USB_TX_AGGREGATION) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		ptxdesc->usb_txagg_num = pxmitframe->agg_num;
#endif
	}
	else if (pxmitframe->frame_tag == MGNT_FRAMETAG)
	{
//		RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("%s: MGNT_FRAMETAG\n", __FUNCTION__));

		ptxdesc->macid = pattrib->mac_id; // CAM_ID(MAC_ID)
		ptxdesc->qsel = pattrib->qsel;
		ptxdesc->rate_id = pattrib->raid; // Rate ID
		ptxdesc->seq = pattrib->seqnum;
		ptxdesc->userate = 1; // driver uses rate, 1M
		ptxdesc->rty_lmt_en = 1; // retry limit enable
		ptxdesc->data_rt_lmt = 6; // retry limit = 6

#ifdef CONFIG_INTEL_PROXIM
		if((padapter->proximity.proxim_on==_TRUE)&&(pattrib->intel_proxim==_TRUE)){
			DBG_871X("\n %s pattrib->rate=%d\n",__FUNCTION__,pattrib->rate);
			ptxdesc->datarate = pattrib->rate;
		}
		else
#endif
		{
			ptxdesc->datarate = MRateToHwRate(pmlmeext->tx_rate);
		}
	}
	else if (pxmitframe->frame_tag == TXAGG_FRAMETAG)
	{
		RT_TRACE(_module_hal_xmit_c_, _drv_warning_, ("%s: TXAGG_FRAMETAG\n", __FUNCTION__));
	}
#ifdef CONFIG_MP_INCLUDED
	else if (pxmitframe->frame_tag == MP_FRAMETAG)
	{
		struct tx_desc *pdesc;

		pdesc = (struct tx_desc*)ptxdesc;
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("%s: MP_FRAMETAG\n", __FUNCTION__));
		fill_txdesc_for_mp(padapter, pdesc);

		pdesc->txdw0 = le32_to_cpu(pdesc->txdw0);
		pdesc->txdw1 = le32_to_cpu(pdesc->txdw1);
		pdesc->txdw2 = le32_to_cpu(pdesc->txdw2);
		pdesc->txdw3 = le32_to_cpu(pdesc->txdw3);
		pdesc->txdw4 = le32_to_cpu(pdesc->txdw4);
		pdesc->txdw5 = le32_to_cpu(pdesc->txdw5);
		pdesc->txdw6 = le32_to_cpu(pdesc->txdw6);
		pdesc->txdw7 = le32_to_cpu(pdesc->txdw7);
#ifdef CONFIG_PCI_HCI
		pdesc->txdw8 = le32_to_cpu(pdesc->txdw8);
		pdesc->txdw9 = le32_to_cpu(pdesc->txdw9);
		pdesc->txdw10 = le32_to_cpu(pdesc->txdw10);
		pdesc->txdw11 = le32_to_cpu(pdesc->txdw11);
		pdesc->txdw12 = le32_to_cpu(pdesc->txdw12);
		pdesc->txdw13 = le32_to_cpu(pdesc->txdw13);
		pdesc->txdw14 = le32_to_cpu(pdesc->txdw14);
		pdesc->txdw15 = le32_to_cpu(pdesc->txdw15);
#endif
	}
#endif
	else
	{
		RT_TRACE(_module_hal_xmit_c_, _drv_warning_, ("%s: frame_tag=0x%x\n", __FUNCTION__, pxmitframe->frame_tag));

		ptxdesc->macid = 4; // CAM_ID(MAC_ID)
		ptxdesc->rate_id = 6; // Rate ID
		ptxdesc->seq = pattrib->seqnum;
		ptxdesc->userate = 1; // driver uses rate
		ptxdesc->datarate = MRateToHwRate(pmlmeext->tx_rate);
	}

	ptxdesc->pktlen = pattrib->last_txcmdsz;

	ptxdesc->offset = TXDESC_SIZE + OFFSET_SZ;
	if (bmcst) ptxdesc->bmc = 1;
	ptxdesc->ls = 1;
	ptxdesc->fs = 1;
	ptxdesc->own = 1;

	// 2009.11.05. tynli_test. Suggested by SD4 Filen for FW LPS.
	// (1) The sequence number of each non-Qos frame / broadcast / multicast /
	// mgnt frame should be controled by Hw because Fw will also send null data
	// which we cannot control when Fw LPS enable.
	// --> default enable non-Qos data sequense number. 2010.06.23. by tynli.
	// (2) Enable HW SEQ control for beacon packet, because we use Hw beacon.
	// (3) Use HW Qos SEQ to control the seq num of Ext port non-Qos packets.
	// 2010.06.23. Added by tynli.
	if (!pattrib->qos_en)
	{
		// Hw set sequence number
		ptxdesc->hwseq_en = 1; // HWSEQ_EN
		ptxdesc->hwseq_sel = 0; // HWSEQ_SEL
	}
}

/*
 *	Description:
 *
 *	Parameters:
 *		pxmitframe	xmitframe
 *		pbuf		where to fill tx desc
 */
void rtl8723b_update_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf)
{
	PADAPTER padapter = pxmitframe->padapter;
	struct tx_desc *pdesc;

	rtl8723b_fill_default_txdesc(pxmitframe, pbuf);

	pdesc = (struct tx_desc*)pbuf;
	pdesc->txdw0 = cpu_to_le32(pdesc->txdw0);
	pdesc->txdw1 = cpu_to_le32(pdesc->txdw1);
	pdesc->txdw2 = cpu_to_le32(pdesc->txdw2);
	pdesc->txdw3 = cpu_to_le32(pdesc->txdw3);
	pdesc->txdw4 = cpu_to_le32(pdesc->txdw4);
	pdesc->txdw5 = cpu_to_le32(pdesc->txdw5);
	pdesc->txdw6 = cpu_to_le32(pdesc->txdw6);
	pdesc->txdw7 = cpu_to_le32(pdesc->txdw7);
	pdesc->txdw8 = cpu_to_le32(pdesc->txdw8);
	pdesc->txdw9 = cpu_to_le32(pdesc->txdw9);

#ifdef CONFIG_PCI_HCI
	pdesc->txdw8 = cpu_to_le32(pdesc->txdw8);
	pdesc->txdw9 = cpu_to_le32(pdesc->txdw9);
	pdesc->txdw10 = cpu_to_le32(pdesc->txdw10);
	pdesc->txdw11 = cpu_to_le32(pdesc->txdw11);
	pdesc->txdw12 = cpu_to_le32(pdesc->txdw12);
	pdesc->txdw13 = cpu_to_le32(pdesc->txdw13);
	pdesc->txdw14 = cpu_to_le32(pdesc->txdw14);
	pdesc->txdw15 = cpu_to_le32(pdesc->txdw15);
#endif

	rtl8723b_cal_txdesc_chksum((PTXDESC)pdesc);
}

//
// Description: In normal chip, we should send some packet to Hw which will be used by Fw
//			in FW LPS mode. The function is to fill the Tx descriptor of this packets, then
//			Fw can tell Hw to send these packet derectly.
// Added by tynli. 2009.10.15.
//
//type1:pspoll, type2:null
void rtl8723b_fill_fake_txdesc(
	PADAPTER	padapter,
	u8*			pDesc,
	u32			BufferLen,
	u8			type,
	u8			IsBTQosNull)
{
	PTXDESC ptxdesc;
	u8 desc_size = TXDESC_SIZE;

	ptxdesc = (PTXDESC)pDesc;
	_rtw_memset(pDesc, 0, desc_size);

	ptxdesc->own = 1;
	ptxdesc->fs = 1;
	ptxdesc->ls = 1;

	ptxdesc->offset = (desc_size + OFFSET_SZ) & 0xFF;

	ptxdesc->pktlen = BufferLen & 0xffff; /*  Buffer size + command header */

	ptxdesc->qsel = QSLT_MGNT & 0x1f; /* Fixed queue of Mgnt queue */

	/* Set NAVUSEHDR to prevent Ps-poll AId filed to be changed to error vlaue by Hw. */
	if (type == 1)
	{
		ptxdesc->navusehdr = 1;
	}
	else
	{
		ptxdesc->hwseq_sel = 1; /* Hw set sequence number */
		ptxdesc->tailpage = 28; /* set bit3 to 1. Suugested by TimChen. 2009.12.29. */
	}

	if (_TRUE == IsBTQosNull)
	{
		ptxdesc->bt_null = 1; /*  BT NULL */
	}

	ptxdesc->userate = 1;

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	/*  USB interface drop packet if the checksum of descriptor isn't correct. */
	/*  Using this checksum can let hardware recovery from packet bulk out error (e.g. Cancel URC, Bulk out error.). */
	rtl8723b_cal_txdesc_chksum(ptxdesc);
#endif
}

#ifdef CONFIG_CONCURRENT_MODE
int reset_tsf(PADAPTER Adapter, u8 reset_port )
{
	u8 reset_cnt_before = 0, reset_cnt_after = 0, loop_cnt = 0;
	u32 reg_reset_tsf_cnt = (IFACE_PORT0==reset_port) ?
				REG_FW_RESET_TSF_CNT_0:REG_FW_RESET_TSF_CNT_1;

	rtw_scan_abort(Adapter->pbuddy_adapter);	/*	site survey will cause reset_tsf fail	*/
	reset_cnt_after = reset_cnt_before = rtw_read8(Adapter,reg_reset_tsf_cnt);
	rtl8723b_reset_tsf(Adapter, reset_port);

	while ((reset_cnt_after == reset_cnt_before ) && (loop_cnt < 10)) {
		rtw_msleep_os(100);
		loop_cnt++;
		reset_cnt_after = rtw_read8(Adapter, reg_reset_tsf_cnt);
	}

	return(loop_cnt >= 10) ? _FAIL : _TRUE;
}
#endif

static void hw_var_set_opmode(PADAPTER padapter, u8 variable, u8 *val)
{
	u8 val8;
	u8 mode = *((u8 *)val);

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		// disable Port1 TSF update
		val8 = rtw_read8(padapter, REG_BCN_CTRL_1);
		val8 |= DIS_TSF_UDT;
		rtw_write8(padapter, REG_BCN_CTRL_1, val8);

		// set net_type
		val8 = rtw_read8(padapter, MSR) & 0x03;
		val8 |= (mode<<2);
		rtw_write8(padapter, MSR, val8);

		DBG_871X("%s()-%d mode = %d\n", __FUNCTION__, __LINE__, mode);

		if ((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_))
		{
			if (!check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE))
			{
				StopTxBeacon(padapter);
			}

			rtw_write8(padapter, REG_BCN_CTRL_1, 0x11);//disable atim wnd and disable beacon function
		}
		else if ((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/)
		{
			ResumeTxBeacon(padapter);
			rtw_write8(padapter, REG_BCN_CTRL_1, 0x1a);
		}
		else if (mode == _HW_STATE_AP_)
		{
			ResumeTxBeacon(padapter);

			rtw_write8(padapter, REG_BCN_CTRL_1, 0x12);

			// Set RCR
			//rtw_write32(padapter, REG_RCR, 0x70002a8e);//CBSSID_DATA must set to 0
			//rtw_write32(padapter, REG_RCR, 0x7000228e);//CBSSID_DATA must set to 0
			rtw_write32(padapter, REG_RCR, 0x7000208e);//CBSSID_DATA must set to 0,reject ICV_ERR packet
			// enable to rx data frame
			rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);
			// enable to rx ps-poll
			rtw_write16(padapter, REG_RXFLTMAP1, 0x0400);

			// Beacon Control related register for first time
			rtw_write8(padapter, REG_BCNDMATIM, 0x02); // 2ms

			//rtw_write8(padapter, REG_BCN_MAX_ERR, 0xFF);
			rtw_write8(padapter, REG_ATIMWND_1, 0x0a); // 10ms for port1
			rtw_write16(padapter, REG_BCNTCFG, 0x00);
			rtw_write16(padapter, REG_TBTT_PROHIBIT, 0xff04);
			rtw_write16(padapter, REG_TSFTR_SYN_OFFSET, 0x7fff);// +32767 (~32ms)

			// reset TSF2
			rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(1));

			// enable BCN1 Function for if2
			// don't enable update TSF1 for if2 (due to TSF update when beacon/probe rsp are received)
			rtw_write8(padapter, REG_BCN_CTRL_1, (DIS_TSF_UDT|EN_BCN_FUNCTION | EN_TXBCN_RPT|DIS_BCNQ_SUB));

#ifdef CONFIG_CONCURRENT_MODE
			if (check_buddy_fwstate(padapter, WIFI_FW_NULL_STATE))
			{
				val8 = rtw_read8(padapter, REG_BCN_CTRL);
				val8 &= ~EN_BCN_FUNCTION;
				rtw_write8(padapter, REG_BCN_CTRL, val8);
			}
#endif
			//BCN1 TSF will sync to BCN0 TSF with offset(0x518) if if1_sta linked
			//rtw_write8(padapter, REG_BCN_CTRL_1, rtw_read8(padapter, REG_BCN_CTRL_1)|BIT(5));
			//rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(3));

			//dis BCN0 ATIM  WND if if1 is station
			rtw_write8(padapter, REG_BCN_CTRL, rtw_read8(padapter, REG_BCN_CTRL)|DIS_ATIM);

#ifdef CONFIG_TSF_RESET_OFFLOAD
			// Reset TSF for STA+AP concurrent mode
			if (check_buddy_fwstate(padapter, (WIFI_STATION_STATE|WIFI_ASOC_STATE)))
			{
				if (reset_tsf(padapter, IFACE_PORT1) == _FALSE)
					DBG_871X("ERROR! %s()-%d: Reset port1 TSF fail\n",
						__FUNCTION__, __LINE__);
			}
#endif // CONFIG_TSF_RESET_OFFLOAD
		}
	}
	else //else for port0
#endif // CONFIG_CONCURRENT_MODE
	{
		// disable Port0 TSF update
		val8 = rtw_read8(padapter, REG_BCN_CTRL);
		val8 |= DIS_TSF_UDT;
		rtw_write8(padapter, REG_BCN_CTRL, val8);

		// set net_type
		val8 = rtw_read8(padapter, MSR)&0x0c;
		val8 |= mode;
		rtw_write8(padapter, MSR, val8);

		DBG_871X("%s()-%d mode = %d\n", __FUNCTION__, __LINE__, mode);

		if ((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_))
		{
#ifdef CONFIG_CONCURRENT_MODE
			if (!check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE))
#endif // CONFIG_CONCURRENT_MODE
			{
				StopTxBeacon(padapter);
			}

			rtw_write8(padapter, REG_BCN_CTRL, 0x19); // disable atim wnd
			//rtw_write8(padapter,REG_BCN_CTRL, 0x18);
		}
		else if ((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/)
		{
			ResumeTxBeacon(padapter);
			rtw_write8(padapter, REG_BCN_CTRL, 0x1a);
		}
		else if (mode == _HW_STATE_AP_)
		{

			ResumeTxBeacon(padapter);

			rtw_write8(padapter, REG_BCN_CTRL, 0x12);

			//Set RCR
			//rtw_write32(padapter, REG_RCR, 0x70002a8e);//CBSSID_DATA must set to 0
			//rtw_write32(padapter, REG_RCR, 0x7000228e);//CBSSID_DATA must set to 0
			rtw_write32(padapter, REG_RCR, 0x7000208e);//CBSSID_DATA must set to 0,reject ICV_ERR packet
			//enable to rx data frame
			rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);
			//enable to rx ps-poll
			rtw_write16(padapter, REG_RXFLTMAP1, 0x0400);

			//Beacon Control related register for first time
			rtw_write8(padapter, REG_BCNDMATIM, 0x02); // 2ms

			//rtw_write8(padapter, REG_BCN_MAX_ERR, 0xFF);
			rtw_write8(padapter, REG_ATIMWND, 0x0a); // 10ms
			rtw_write16(padapter, REG_BCNTCFG, 0x00);
			rtw_write16(padapter, REG_TBTT_PROHIBIT, 0xff04);
			rtw_write16(padapter, REG_TSFTR_SYN_OFFSET, 0x7fff);// +32767 (~32ms)

			//reset TSF
			rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(0));

			//enable BCN0 Function for if1
			//don't enable update TSF0 for if1 (due to TSF update when beacon/probe rsp are received)
			rtw_write8(padapter, REG_BCN_CTRL, (DIS_TSF_UDT|EN_BCN_FUNCTION|EN_TXBCN_RPT|DIS_BCNQ_SUB));

#ifdef CONFIG_CONCURRENT_MODE
			if (check_buddy_fwstate(padapter, WIFI_FW_NULL_STATE))
			{
				val8 = rtw_read8(padapter, REG_BCN_CTRL_1);
				val8 &= ~EN_BCN_FUNCTION;
				rtw_write8(padapter, REG_BCN_CTRL_1, val8);
			}
#endif // CONFIG_CONCURRENT_MODE

			// dis BCN1 ATIM  WND if if2 is station
			val8 = rtw_read8(padapter, REG_BCN_CTRL_1);
			val8 |= DIS_ATIM;
			rtw_write8(padapter, REG_BCN_CTRL_1, val8);
#ifdef CONFIG_TSF_RESET_OFFLOAD
			// Reset TSF for STA+AP concurrent mode
			if (check_buddy_fwstate(padapter, (WIFI_STATION_STATE|WIFI_ASOC_STATE)))
			{
				if (reset_tsf(padapter, IFACE_PORT0) == _FALSE)
					DBG_871X("ERROR! %s()-%d: Reset port0 TSF fail\n",
						__FUNCTION__, __LINE__);
			}
#endif	// CONFIG_TSF_RESET_OFFLOAD
		}
	}
}


static void hw_var_set_macaddr(PADAPTER padapter, u8 variable, u8 *val)
{
	u8 idx = 0;
	u32 reg_macid;

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		reg_macid = REG_MACID1;
	}
	else
#endif
	{
		reg_macid = REG_MACID;
	}

	for (idx = 0 ; idx < 6; idx++)
	{
		rtw_write8(padapter, (reg_macid+idx), val[idx]);
	}
}

static void hw_var_set_bssid(PADAPTER padapter, u8 variable, u8 *val)
{
	u8	idx = 0;
	u32 reg_bssid;

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		reg_bssid = REG_BSSID1;
	}
	else
#endif
	{
		reg_bssid = REG_BSSID;
	}

	for (idx = 0 ; idx < 6; idx++)
	{
		rtw_write8(padapter, (reg_bssid+idx), val[idx]);
	}
}

static void hw_var_set_correct_tsf(PADAPTER padapter, u8 variable, u8 *val)
{
	u64 tsf;
	u32 reg_tsftr;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;


	//tsf = pmlmeext->TSFValue - ((u32)pmlmeext->TSFValue % (pmlmeinfo->bcn_interval*1024)) - 1024; //us
	tsf = pmlmeext->TSFValue - rtw_modular64(pmlmeext->TSFValue, (pmlmeinfo->bcn_interval*1024)) - 1024; //us

	if (((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) ||
		((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{
		//pHalData->RegTxPause |= STOP_BCNQ;BIT(6)
		//rtw_write8(padapter, REG_TXPAUSE, (rtw_read8(Adapter, REG_TXPAUSE)|BIT(6)));
		StopTxBeacon(padapter);
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		reg_tsftr = REG_TSFTR1;
	}
	else
#endif
	{
		reg_tsftr = REG_TSFTR;
	}

	// disable related TSF function
	SetBcnCtrlReg(padapter, 0, EN_BCN_FUNCTION);

	rtw_write32(padapter, reg_tsftr, tsf);
	rtw_write32(padapter, reg_tsftr+4, tsf>>32);

#ifdef CONFIG_CONCURRENT_MODE

	// Update buddy port's TSF if it is SoftAP for beacon TX issue!
	if ( (pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE
		&& check_buddy_fwstate(padapter, WIFI_AP_STATE)
	) {
		//disable related TSF function
		if (padapter->pbuddy_adapter)
			SetBcnCtrlReg(padapter->pbuddy_adapter, 0, EN_BCN_FUNCTION);
		if (padapter->iface_type == IFACE_PORT1)
		{
			reg_tsftr = REG_TSFTR;
		}
		else
		{
			reg_tsftr = REG_TSFTR1;
		}

		rtw_write32(padapter, reg_tsftr, tsf);
		rtw_write32(padapter, reg_tsftr+4, tsf>>32);

		//enable related TSF function
		if (padapter->pbuddy_adapter)
			SetBcnCtrlReg(padapter->pbuddy_adapter,  EN_BCN_FUNCTION,0);
	}
#endif
	//enable related TSF function
	SetBcnCtrlReg(padapter, EN_BCN_FUNCTION, 0);

#ifdef CONFIG_TSF_RESET_OFFLOAD
	// Reset TSF for STA+AP concurrent mode
	if ( (pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE
			&& check_buddy_fwstate(padapter, WIFI_AP_STATE) ) {
		if (padapter->iface_type == IFACE_PORT1) {
			if (reset_tsf(padapter, IFACE_PORT0) == _FALSE)
				DBG_871X("ERROR! %s()-%d: Reset port0 TSF fail\n",
				__FUNCTION__, __LINE__);
		} else {
			if (reset_tsf(padapter, IFACE_PORT1) == _FALSE)
				DBG_871X("ERROR! %s()-%d: Reset port1 TSF fail\n",
				__FUNCTION__, __LINE__);
		}
	}
#endif	// CONFIG_TSF_RESET_OFFLOAD

	if (((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) ||
		((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{
		//pHalData->RegTxPause &= (~STOP_BCNQ);
		//rtw_write8(padapter, REG_TXPAUSE, (rtw_read8(padapter, REG_TXPAUSE)&(~BIT(6))));
		ResumeTxBeacon(padapter);
	}
}

static void hw_var_set_mlme_disconnect(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
#ifdef CONFIG_CONCURRENT_MODE
	if (check_buddy_mlmeinfo_state(padapter, _HW_STATE_NOLINK_))
#endif
	{
		// Set RCR to not to receive data frame when NO LINK state
		//rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR) & ~RCR_ADF);
		// reject all data frames
		rtw_write16(padapter, REG_RXFLTMAP2, 0);
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type == IFACE_PORT1)
	{
		// reset TSF1
		rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(1));

		// disable update TSF1
		SetBcnCtrlReg(padapter, DIS_TSF_UDT, 0);

		// disable Port1's beacon function
		rtw_write8(padapter, REG_BCN_CTRL_1, rtw_read8(padapter, REG_BCN_CTRL_1)&(~BIT(3)));
	}
	else
#endif
	{
		// reset TSF
		rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(0));

		// disable update TSF
		SetBcnCtrlReg(padapter, DIS_TSF_UDT, 0);
	}
}

void hw_var_set_ant_outof_coex(PADAPTER padapter, u8 AntForWifi)
{
	u32 u4Tmp = 0;
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);

	DBG_871X_LEVEL(_drv_always_, "%s AntForWifi:%x \n", __func__, AntForWifi);

#ifdef CONFIG_BT_COEXIST

	if (pHalData->bUseExtSPDT) {
		rtw_write16(padapter, 0x930, 0x77);
#ifdef CONFIG_ANTENNA_AT_AUX_PORT
		rtw_write16(padapter, 0x92c, AntForWifi ? 0x2 : 0x1); //for wifi
		rtw_write8(padapter, 0x64, AntForWifi ? 0x1 : 0x0);
#else
		rtw_write16(padapter, 0x92c, AntForWifi ? 0x1 : 0x2);
		rtw_write8(padapter, 0x64, AntForWifi ? 0x0 : 0x1);
#endif
	} else {
#ifdef CONFIG_ANTENNA_AT_AUX_PORT
		rtw_write16(padapter, 0x948, AntForWifi ? 0x280 : 0);
#else
		rtw_write16(padapter, 0x948, AntForWifi ? 0 : 0x280);
#endif
	}

#else //!CONFIG_BT_COEXIST

	u4Tmp = rtw_read32(padapter, REG_BT_WIFI_ANTENNA_SWITCH_8723B); //Ant select enable
	u4Tmp |= BIT11;
	rtw_write32(padapter, REG_BT_WIFI_ANTENNA_SWITCH_8723B, u4Tmp);
	rtw_write32(padapter, 0x4c, rtw_read32(padapter, 0x4c) | BIT23);
#ifdef CONFIG_ANTENNA_AT_AUX_PORT
	rtw_write8(padapter, 0x64, AntForWifi ? 0x1 : 0x0);
#else
	rtw_write8(padapter, 0x64, AntForWifi ? 0x0 : 0x1);
#endif
	DBG_871X("%s ant switch: 0x38:%x 0x4c:%x 0x64:%x\n", __func__,
			rtw_read32(padapter, 0x38), rtw_read32(padapter, 0x4c),
			rtw_read32(padapter, 0x64));
#endif//CONFIG_BT_COEXIST
}

#ifdef CONFIG_CONCURRENT_MODE
static void hw_var_set_mlme_sitesurvey(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);

	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	u32 v32;


	if (*val)//under sitesurvey
	{
		// config RCR to receive different BSSID & not to receive data frame
		v32 = rtw_read32(padapter, REG_RCR);
		v32 &= ~(RCR_CBSSID_BCN);
		rtw_write32(padapter, REG_RCR, v32);

		// disable update TSF
		if ((pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE)
			SetBcnCtrlReg(padapter, DIS_TSF_UDT, 0);

		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE) &&
			(check_buddy_fwstate(padapter, _FW_LINKED) == _TRUE))
		{
			StopTxBeacon(padapter);
		}
	}
	else//sitesurvey done
	{
		// enable to rx data frame
		//write32(padapter, REG_RCR, read32(padapter, REG_RCR)|RCR_ADF);
		rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);

		// enable update TSF
		SetBcnCtrlReg(padapter, 0, DIS_TSF_UDT);

		v32 = rtw_read32(padapter, REG_RCR);
		v32 |= RCR_CBSSID_BCN;
		rtw_write32(padapter, REG_RCR, v32);

		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE) &&
			(check_buddy_fwstate(padapter, _FW_LINKED) == _TRUE))
		{
			ResumeTxBeacon(padapter);
#if 0
			// reset TSF 1/2 after ResumeTxBeacon
			if (pbuddy_adapter->iface_type == IFACE_PORT1)
				rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(1));
			else
				rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(0));
#endif

		}
	}
}
#endif

static void hw_var_set_mlme_join(PADAPTER padapter, u8 variable, u8 *val)
{
	u8 RetryLimit = 0x30;
	u8 type = *val;

	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (type == 0) // prepare to join
	{
		u32 v32;

#ifdef CONFIG_CONCURRENT_MODE
		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE) &&
			(check_buddy_fwstate(padapter, _FW_LINKED) == _TRUE))
		{
			StopTxBeacon(padapter);
		}
#endif

		// enable to rx data frame.Accept all data frame
		//rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR)|RCR_ADF);
		rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);

		v32 = rtw_read32(padapter, REG_RCR);
#ifdef CONFIG_CONCURRENT_MODE
		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE))
			v32 |= RCR_CBSSID_BCN;
		else
#endif
		{
			v32 |= RCR_CBSSID_DATA | RCR_CBSSID_BCN;
		}
		rtw_write32(padapter, REG_RCR, v32);

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
			RetryLimit = (pHalData->CustomerID == RT_CID_CCX) ? 7 : 48;
		else // Ad-hoc Mode
			RetryLimit = 0x7;
	}
	else if (type == 1) // joinbss_event callback when join res < 0
	{
#ifdef CONFIG_CONCURRENT_MODE
		if (check_buddy_mlmeinfo_state(padapter, _HW_STATE_NOLINK_))
			rtw_write16(padapter, REG_RXFLTMAP2, 0);

		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE) &&
			(check_buddy_fwstate(padapter, _FW_LINKED) == _TRUE))
		{
			ResumeTxBeacon(padapter);

			// reset TSF 1/2 after ResumeTxBeacon
			rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(1)|BIT(0));
		}
#else
		// config RCR to receive different BSSID & not to receive data frame during linking
		//v32 = rtw_read32(padapter, REG_RCR);
		//v32 &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN);//| RCR_ADF
		//rtw_write32(padapter, REG_RCR, v32);
		rtw_write16(padapter, REG_RXFLTMAP2, 0);
#endif
	}
	else if (type == 2) // sta add event callback
	{
		// enable update TSF
		SetBcnCtrlReg(padapter, 0, DIS_TSF_UDT);

		if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE)
		{
			// fixed beacon issue for 8191su...........
			rtw_write8(padapter, 0x542, 0x02);
			RetryLimit = 0x7;
		}

#ifdef CONFIG_CONCURRENT_MODE
		if (check_buddy_mlmeinfo_state(padapter, WIFI_FW_AP_STATE) &&
			(check_buddy_fwstate(padapter, _FW_LINKED) == _TRUE))
		{
			ResumeTxBeacon(padapter);

			// reset TSF 1/2 after ResumeTxBeacon
			rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(1)|BIT(0));
		}
#endif
	}

	rtw_write16(padapter, REG_RL, RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT);
}

static void process_c2h_event(PADAPTER padapter, PC2H_EVT_HDR pC2hEvent, u8 *c2hBuf)
{
	u8				index = 0;
	PHAL_DATA_TYPE	pHalData=GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (c2hBuf == NULL) {
		DBG_8192C("%s c2hbuff is NULL\n",__FUNCTION__);
		return;
	}

	switch (pC2hEvent->CmdID)
	{
		case C2H_AP_RPT_RSP:
			{
				u4Byte c2h_ap_keeplink = _TRUE;
				if (c2hBuf[2] == 0 && c2hBuf[3] == 0)
					c2h_ap_keeplink = _FALSE;
				else
					c2h_ap_keeplink = _TRUE;

				if (_TRUE == pmlmeext->try_ap_c2h_wait) {
					if (_FALSE == c2h_ap_keeplink) {
						pmlmeext->try_ap_c2h_wait = _FALSE;
						RT_TRACE(_module_hal_init_c_, _drv_err_,("fw tell us link is off\n"));
						receive_disconnect(padapter, pmlmeinfo->network.MacAddress , 65535);
					} else  {
						RT_TRACE(_module_hal_init_c_, _drv_err_,("fw tell us link is on\n"));
					}
				} else {
					RT_TRACE(_module_hal_init_c_, _drv_err_,("we don't need this C2H\n"));
				}
				pmlmeext->check_ap_processing = _FALSE;
			}
			break;
		case C2H_DBG:
			{
				RT_TRACE(_module_hal_init_c_, _drv_info_, ("C2HCommandHandler: %s\n", c2hBuf));
			}
			break;

		case C2H_CCX_TX_RPT:
//			CCX_FwC2HTxRpt(padapter, QueueID, tmpBuf);
			break;

#ifdef CONFIG_BT_COEXIST
#ifdef CONFIG_PCI_HCI
		case C2H_BT_RSSI:
//			fwc2h_ODM(padapter, tmpBuf, &C2hEvent);
			BT_FwC2hBtRssi(padapter, c2hBuf);
			break;
#endif
#endif

		case C2H_EXT_RA_RPT:
//			C2HExtRaRptHandler(padapter, tmpBuf, C2hEvent.CmdLen);
			break;

		case C2H_HW_INFO_EXCH:
			RT_TRACE(_module_hal_init_c_, _drv_info_, ("[BT], C2H_HW_INFO_EXCH\n"));
			for (index = 0; index < pC2hEvent->CmdLen; index++)
			{
				RT_TRACE(_module_hal_init_c_, _drv_info_, ("[BT], tmpBuf[%d]=0x%x\n", index, c2hBuf[index]));
			}
			break;

#ifdef CONFIG_BT_COEXIST
		case C2H_8723B_BT_INFO:
			BT_BtInfoNotify(padapter, c2hBuf, pC2hEvent->CmdLen);
			break;
#endif

#ifdef CONFIG_MP_INCLUDED
		case C2H_8723B_BT_MP_INFO:
			MPTBT_FwC2hBtMpCtrl(padapter, c2hBuf, pC2hEvent->CmdLen);
			break;
#endif
		default:
			break;
	}

#ifndef CONFIG_C2H_PACKET_EN
	// Clear event to notify FW we have read the command.
	// Note:
	//	If this field isn't clear, the FW won't update the next command message.
	rtw_write8(padapter, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
#endif
}

#ifdef CONFIG_C2H_PACKET_EN
void C2HPacketHandler_8723B(PADAPTER padapter, u8 *pbuffer, u16 length)
{
	C2H_EVT_HDR		C2hEvent;
	u8 *tmpBuf=NULL;
#ifdef CONFIG_WOWLAN
	if(TRUE == adapter_to_pwrctl(padapter)->wowlan_mode)
	{
		DBG_871X("%s(): return because wowolan_mode==TRUE! CMDID=%d\n", __func__, pbuffer[0]);
		return;
	}
#endif
	C2hEvent.CmdID = pbuffer[0];
	C2hEvent.CmdSeq = pbuffer[1];
	C2hEvent.CmdLen = length -2;
	tmpBuf = pbuffer+2;

	DBG_871X("%s C2hEvent.CmdID:%x C2hEvent.CmdLen:%x C2hEvent.CmdSeq:%x\n",
			__func__, C2hEvent.CmdID, C2hEvent.CmdLen, C2hEvent.CmdSeq);
	RT_PRINT_DATA(_module_hal_init_c_, _drv_notice_, "C2HPacketHandler_8723B(): Command Content:\n", tmpBuf, C2hEvent.CmdLen);

	process_c2h_event(padapter,&C2hEvent, tmpBuf);

	return;
}
#else
//
//C2H event format:
// Field	 TRIGGER		CONTENT	   CMD_SEQ 	CMD_LEN		 CMD_ID
// BITS	 [127:120]	[119:16]      [15:8]		  [7:4]	 	   [3:0]
//2009.10.08. by tynli.
static void C2HCommandHandler(PADAPTER padapter)
{
	C2H_EVT_HDR		C2hEvent;
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)

	u8				*tmpBuf = NULL;
	u8				index = 0;
	u8				bCmdMsgReady = _FALSE;
	u8				U1bTmp = 0;
//	u8				QueueID = 0;

	_rtw_memset(&C2hEvent, 0, sizeof(C2H_EVT_HDR));

	C2hEvent.CmdID = rtw_read8(padapter, REG_C2HEVT_CMD_ID_8723B);
	C2hEvent.CmdLen = rtw_read8(padapter, REG_C2HEVT_CMD_LEN_8723B);
	C2hEvent.CmdSeq = rtw_read8(padapter, REG_C2HEVT_CMD_ID_8723B + 1);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "C2HCommandHandler(): ",
		&C2hEvent , sizeof(C2hEvent));

	U1bTmp = rtw_read8(padapter, REG_C2HEVT_CLEAR);
	DBG_871X("%s C2hEvent.CmdID:%x C2hEvent.CmdLen:%x C2hEvent.CmdSeq:%x\n",
			__func__, C2hEvent.CmdID, C2hEvent.CmdLen, C2hEvent.CmdSeq);

	if (U1bTmp == C2H_EVT_HOST_CLOSE)
	{
		// Not ready.
		return;
	}
	else if (U1bTmp == C2H_EVT_FW_CLOSE)
	{
		bCmdMsgReady = _TRUE;
	}
	else
	{
		// Not a valid value, reset the clear event.
		goto exit;
	}

	if(C2hEvent.CmdLen == 0)
		goto exit;
	tmpBuf = rtw_zmalloc(C2hEvent.CmdLen);
	if (tmpBuf == NULL)
		goto exit;

	// Read the content
	for (index = 0; index < C2hEvent.CmdLen; index++)
	{
		tmpBuf[index] = rtw_read8(padapter, REG_C2HEVT_CMD_ID_8723B + 2 + index);
	}

	RT_PRINT_DATA(_module_hal_init_c_, _drv_notice_, "C2HCommandHandler(): Command Content:\n", tmpBuf, C2hEvent.CmdLen);

	process_c2h_event(padapter,&C2hEvent, tmpBuf);

	if (tmpBuf)
		rtw_mfree(tmpBuf, C2hEvent.CmdLen);
#endif

#ifdef CONFIG_USB_HCI
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(padapter);

	_rtw_memset(&C2hEvent, 0, sizeof(C2H_EVT_HDR));
	C2hEvent.CmdID = pHalData->C2hArray[USB_C2H_CMDID_OFFSET] & 0xF;
	C2hEvent.CmdLen = (pHalData->C2hArray[USB_C2H_CMDID_OFFSET] & 0xF0) >> 4;
	C2hEvent.CmdSeq =pHalData->C2hArray[USB_C2H_SEQ_OFFSET];
	process_c2h_event(padapter,&C2hEvent,&pHalData->C2hArray[USB_C2H_EVENT_OFFSET]);
#endif
	//REG_C2HEVT_CLEAR have done in process_c2h_event
	return;
exit:
	rtw_write8(padapter, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
	return;
}
#endif

void SetHwReg8723B(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	u8 val8;

_func_enter_;

	switch (variable)
	{
		case HW_VAR_MEDIA_STATUS:
			{
				u8 val8;

				val8 = rtw_read8(padapter, MSR) & 0x0c;
				val8 |= *val;
				rtw_write8(padapter, MSR, val8);
			}
			break;

		case HW_VAR_MEDIA_STATUS1:
			{
				u8 val8;

				val8 = rtw_read8(padapter, MSR) & 0x03;
				val8 |= *val << 2;
				rtw_write8(padapter, MSR, val8);
			}
			break;

		case HW_VAR_SET_OPMODE:
			hw_var_set_opmode(padapter, variable, val);
			break;

		case HW_VAR_MAC_ADDR:
			hw_var_set_macaddr(padapter, variable, val);
			break;

		case HW_VAR_BSSID:
			hw_var_set_bssid(padapter, variable, val);
			break;

		case HW_VAR_BASIC_RATE:
			{
				u16			BrateCfg = 0;
				u8			RateIndex = 0;

				// 2007.01.16, by Emily
				// Select RRSR (in Legacy-OFDM and CCK)
				// For 8190, we select only 24M, 12M, 6M, 11M, 5.5M, 2M, and 1M from the Basic rate.
				// We do not use other rates.
				HalSetBrateCfg(padapter, val, &BrateCfg);

				//2011.03.30 add by Luke Lee
				//CCK 2M ACK should be disabled for some BCM and Atheros AP IOT
				//because CCK 2M has poor TXEVM
				//CCK 5.5M & 11M ACK should be enabled for better performance

				pHalData->BasicRateSet = BrateCfg = (BrateCfg |0xd) & 0x15d;
				BrateCfg |= 0x01; // default enable 1M ACK rate
#ifdef CONFIG_CMCC_TEST
				if (padapter->drv_in_test == CMCC_TP_TEST) {
					BrateCfg |= 0x0D; /* use 11M to send ACK */
					BrateCfg |= BIT(6) | BIT(7) | BIT(8); //CMCC_OFDM_ACK 12/18/24M
				}
#endif
				DBG_8192C("HW_VAR_BASIC_RATE: BrateCfg(%#x)\n", BrateCfg);

				// Set RRSR rate table.
				rtw_write8(padapter, REG_RRSR, BrateCfg&0xff);
				rtw_write8(padapter, REG_RRSR+1, (BrateCfg>>8)&0xff);

				// Set RTS initial rate
				while (BrateCfg > 0x1)
				{
					BrateCfg = (BrateCfg >> 1);
					RateIndex++;
				}
				// Ziv - Check
				rtw_write8(padapter, REG_INIRTS_RATE_SEL, RateIndex);
			}
			break;

		case HW_VAR_TXPAUSE:
			rtw_write8(padapter, REG_TXPAUSE, *val);
			break;

		case HW_VAR_BCN_FUNC:
			if (*val)
				SetBcnCtrlReg(padapter, EN_BCN_FUNCTION | EN_TXBCN_RPT, 0);
			else
				SetBcnCtrlReg(padapter, 0, EN_BCN_FUNCTION | EN_TXBCN_RPT);
			break;

		case HW_VAR_CORRECT_TSF:
			hw_var_set_correct_tsf(padapter, variable, val);
			break;

		case HW_VAR_CHECK_BSSID:
			{
				u32 val32;
				val32 = rtw_read32(padapter, REG_RCR);
				if (*val)
					val32 |= RCR_CBSSID_DATA|RCR_CBSSID_BCN;
				else
					val32 &= ~(RCR_CBSSID_DATA|RCR_CBSSID_BCN);
				rtw_write32(padapter, REG_RCR, val32);
			}
			break;

		case HW_VAR_MLME_DISCONNECT:
			hw_var_set_mlme_disconnect(padapter, variable, val);
			break;

		case HW_VAR_MLME_SITESURVEY:
#ifdef CONFIG_CONCURRENT_MODE
			hw_var_set_mlme_sitesurvey(padapter, variable,  val);
#else
			if (*val)//under sitesurvey
			{
				u32 v32;

				// config RCR to receive different BSSID & not to receive data frame
				v32 = rtw_read32(padapter, REG_RCR);
				v32 &= ~(RCR_CBSSID_BCN);
				rtw_write32(padapter, REG_RCR, v32);
				// reject all data frame
				rtw_write16(padapter, REG_RXFLTMAP2, 0);

				// disable update TSF
				SetBcnCtrlReg(padapter, DIS_TSF_UDT, 0);
			}
			else//sitesurvey done
			{
				struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
				struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;
				u32 v32;

				if ((is_client_associated_to_ap(padapter) == _TRUE) ||
					((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) ||
					((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
				{
					// enable to rx data frame
#if 0
					v32 = rtw_read32(padapter, REG_RCR);
					v32 |= RCR_ADF;
					rtw_write32(padapter, REG_RCR, v32);
#else
					rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);
#endif

					// enable update TSF
					SetBcnCtrlReg(padapter, 0, DIS_TSF_UDT);
				}

				v32 = rtw_read32(padapter, REG_RCR);
				v32 |= RCR_CBSSID_BCN;
				rtw_write32(padapter, REG_RCR, v32);
			}
#endif

#ifdef CONFIG_BT_COEXIST
			BT_ScanNotify(padapter, *val?_TRUE:_FALSE);
			if(BT_1Ant(padapter) == _FALSE && *val == _TRUE){ //enter scan
				DBG_871X("call LPS leave()\n");
				LPS_Leave(padapter);
			}
#endif
			break;

		case HW_VAR_MLME_JOIN:
			hw_var_set_mlme_join(padapter, variable,  val);

#ifdef CONFIG_BT_COEXIST
			switch (*val)
			{
				case 0:
					// prepare to join
					//BT_WifiAssociateNotify(padapter, _TRUE);
					BT_WiFiConnectNotify(padapter, _TRUE);
					break;
				case 1:
					// joinbss_event callback when join res < 0
					//BT_WifiAssociateNotify(padapter, _FALSE);
					BT_WiFiConnectNotify(padapter, _FALSE);
					break;
				case 2:
					// sta add event callback
//					BT_WifiMediaStatusNotify(padapter, RT_MEDIA_CONNECT);
					break;
			}
#endif
			break;

		case HW_VAR_BEACON_INTERVAL:
			rtw_write16(padapter, REG_BCN_INTERVAL, *((u16*)val));
			break;

		case HW_VAR_SLOT_TIME:
			{
				u8 u1bAIFS, aSifsTime;
				struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
				struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;

				rtw_write8(padapter, REG_SLOT, *val);

				if (pmlmeinfo->WMM_enable == 0)
				{
					if (pmlmeext->cur_wireless_mode == WIRELESS_11B)
						aSifsTime = 10;
					else
						aSifsTime = 16;

					u1bAIFS = aSifsTime + (2 * pmlmeinfo->slotTime);

					// <Roger_EXP> Temporary removed, 2008.06.20.
					rtw_write8(padapter, REG_EDCA_VO_PARAM, u1bAIFS);
					rtw_write8(padapter, REG_EDCA_VI_PARAM, u1bAIFS);
					rtw_write8(padapter, REG_EDCA_BE_PARAM, u1bAIFS);
					rtw_write8(padapter, REG_EDCA_BK_PARAM, u1bAIFS);
				}
			}
			break;

		case HW_VAR_RESP_SIFS:
#if 0
			// SIFS for OFDM Data ACK
			rtw_write8(padapter, REG_SIFS_CTX+1, val[0]);
			// SIFS for OFDM consecutive tx like CTS data!
			rtw_write8(padapter, REG_SIFS_TRX+1, val[1]);

			rtw_write8(padapter, REG_SPEC_SIFS+1, val[0]);
			rtw_write8(padapter, REG_MAC_SPEC_SIFS+1, val[0]);

			// 20100719 Joseph: Revise SIFS setting due to Hardware register definition change.
			rtw_write8(padapter, REG_R2T_SIFS+1, val[0]);
			rtw_write8(padapter, REG_T2T_SIFS+1, val[0]);

#else
			//SIFS_Timer = 0x0a0a0808;
			//RESP_SIFS for CCK
			rtw_write8(padapter, REG_R2T_SIFS, val[0]); // SIFS_T2T_CCK (0x08)
			rtw_write8(padapter, REG_R2T_SIFS+1, val[1]); //SIFS_R2T_CCK(0x08)
			//RESP_SIFS for OFDM
			rtw_write8(padapter, REG_T2T_SIFS, val[2]); //SIFS_T2T_OFDM (0x0a)
			rtw_write8(padapter, REG_T2T_SIFS+1, val[3]); //SIFS_R2T_OFDM(0x0a)
#endif
			break;

		case HW_VAR_ACK_PREAMBLE:
			{
				u8 regTmp;
				u8 bShortPreamble = *val;

				// Joseph marked out for Netgear 3500 TKIP channel 7 issue.(Temporarily)
				//regTmp = (pHalData->nCur40MhzPrimeSC)<<5;
				regTmp = 0;
				if (bShortPreamble) regTmp |= 0x80;
				rtw_write8(padapter, REG_RRSR+2, regTmp);
			}
			break;

		case HW_VAR_SEC_CFG:
#ifdef CONFIG_CONCURRENT_MODE
			rtw_write8(padapter, REG_SECCFG, 0x0c|BIT(5));// enable tx enc and rx dec engine, and no key search for MC/BC
#else
			rtw_write8(padapter, REG_SECCFG, *val);
#endif
			break;

		case HW_VAR_DM_FLAG:
			pHalData->odmpriv.SupportAbility = *((u32*)val);
			break;

		case HW_VAR_DM_FUNC_OP:
			if (*val) // save dm flag
				pHalData->odmpriv.BK_SupportAbility = pHalData->odmpriv.SupportAbility;
			else // restore dm flag
				pHalData->odmpriv.SupportAbility = pHalData->odmpriv.BK_SupportAbility;
			break;

		case HW_VAR_DM_FUNC_SET:
			if (*((u32*)val) == DYNAMIC_ALL_FUNC_ENABLE) {
				pHalData->dmpriv.DMFlag = pHalData->dmpriv.InitDMFlag;
				pHalData->odmpriv.SupportAbility = pHalData->dmpriv.InitODMFlag;
			} else {
				pHalData->odmpriv.SupportAbility |= *((u32*)val);
			}
			break;

		case HW_VAR_DM_FUNC_CLR:
			pHalData->odmpriv.SupportAbility &= *((u32*)val);
			break;

		case HW_VAR_CAM_EMPTY_ENTRY:
			{
				u8	ucIndex = *val;
				u8	i;
				u32	ulCommand = 0;
				u32	ulContent = 0;
				u32	ulEncAlgo = CAM_AES;

				for (i=0; i<CAM_CONTENT_COUNT; i++)
				{
					// filled id in CAM config 2 byte
					if (i == 0)
					{
						ulContent |= (ucIndex & 0x03) | ((u16)(ulEncAlgo)<<2);
						//ulContent |= CAM_VALID;
					}
					else
					{
						ulContent = 0;
					}
					// polling bit, and No Write enable, and address
					ulCommand = CAM_CONTENT_COUNT*ucIndex+i;
					ulCommand = ulCommand | CAM_POLLINIG | CAM_WRITE;
					// write content 0 is equall to mark invalid
					rtw_write32(padapter, WCAMI, ulContent);  //delay_ms(40);
					//RT_TRACE(COMP_SEC, DBG_LOUD, ("CAM_empty_entry(): WRITE A4: %lx \n",ulContent));
					rtw_write32(padapter, RWCAM, ulCommand);  //delay_ms(40);
					//RT_TRACE(COMP_SEC, DBG_LOUD, ("CAM_empty_entry(): WRITE A0: %lx \n",ulCommand));
				}
			}
			break;

		case HW_VAR_CAM_INVALID_ALL:
			rtw_write32(padapter, RWCAM, BIT(31)|BIT(30));
			break;

		case HW_VAR_CAM_WRITE:
			{
				u32 cmd;
				u32 *cam_val = (u32*)val;

				rtw_write32(padapter, WCAMI, cam_val[0]);

				cmd = CAM_POLLINIG | CAM_WRITE | cam_val[1];
				rtw_write32(padapter, RWCAM, cmd);
			}
			break;

		case HW_VAR_AC_PARAM_VO:
			rtw_write32(padapter, REG_EDCA_VO_PARAM, *((u32*)val));
			break;

		case HW_VAR_AC_PARAM_VI:
			rtw_write32(padapter, REG_EDCA_VI_PARAM, *((u32*)val));
			break;

		case HW_VAR_AC_PARAM_BE:
			pHalData->AcParam_BE = ((u32*)(val))[0];
			rtw_write32(padapter, REG_EDCA_BE_PARAM, *((u32*)val));
			break;

		case HW_VAR_AC_PARAM_BK:
			rtw_write32(padapter, REG_EDCA_BK_PARAM, *((u32*)val));
			break;

		case HW_VAR_ACM_CTRL:
			{
				u8 ctrl = *((u8*)val);
				u8 hwctrl = 0;

				if (ctrl != 0)
				{
					hwctrl |= AcmHw_HwEn;

					if (ctrl & BIT(1)) // BE
						hwctrl |= AcmHw_BeqEn;

					if (ctrl & BIT(2)) // VI
						hwctrl |= AcmHw_ViqEn;

					if (ctrl & BIT(3)) // VO
						hwctrl |= AcmHw_VoqEn;
				}

				DBG_8192C("[HW_VAR_ACM_CTRL] Write 0x%02X\n", hwctrl);
				rtw_write8(padapter, REG_ACMHWCTRL, hwctrl);
			}
			break;

		case HW_VAR_AMPDU_MIN_SPACE:
			{
				u8	MinSpacingToSet;
				u8	SecMinSpace;

				MinSpacingToSet = *val;
				if (MinSpacingToSet <= 7)
				{
					switch (padapter->securitypriv.dot11PrivacyAlgrthm)
					{
						case _NO_PRIVACY_:
						case _AES_:
							SecMinSpace = 0;
							break;

						case _WEP40_:
						case _WEP104_:
						case _TKIP_:
						case _TKIP_WTMIC_:
							SecMinSpace = 6;
							break;
						default:
							SecMinSpace = 7;
							break;
					}

					if (MinSpacingToSet < SecMinSpace)
						MinSpacingToSet = SecMinSpace;

					//RT_TRACE(COMP_MLME, DBG_LOUD, ("Set HW_VAR_AMPDU_MIN_SPACE: %#x\n", padapter->MgntInfo.MinSpaceCfg));
					MinSpacingToSet |= rtw_read8(padapter, REG_AMPDU_MIN_SPACE) & 0xf8;
					rtw_write8(padapter, REG_AMPDU_MIN_SPACE, MinSpacingToSet);
				}
			}
			break;

		case HW_VAR_AMPDU_FACTOR:
			{
				u8 RegToSet_Normal[4] = {0x41,0xa8,0x72, 0xb9};
				u8 MaxAggNum;
				u8 FactorToSet;
				u8 *pRegToSet;
				u8 index = 0;

				pRegToSet = RegToSet_Normal; // 0xb972a841;
#ifdef CONFIG_BT_COEXIST
				if ((BT_IsBtDisabled(padapter) == _FALSE) &&
					(BT_1Ant(padapter) == _TRUE))
				{
					MaxAggNum = 0x8;
				}
				else
#endif // CONFIG_BT_COEXIST
				{
					MaxAggNum = 0xF;
				}

				FactorToSet = *val;
				if (FactorToSet <= 3)
				{
					FactorToSet = (1 << (FactorToSet + 2));
					if (FactorToSet > MaxAggNum)
						FactorToSet = MaxAggNum;

					for (index=0; index<4; index++)
					{
						if ((pRegToSet[index] & 0xf0) > (FactorToSet << 4))
							pRegToSet[index] = (pRegToSet[index] & 0x0f) | (FactorToSet << 4);

						if ((pRegToSet[index] & 0x0f) > FactorToSet)
							pRegToSet[index] = (pRegToSet[index] & 0xf0) | FactorToSet;

						rtw_write8(padapter, REG_AGGLEN_LMT+index, pRegToSet[index]);
					}

					//RT_TRACE(COMP_MLME, DBG_LOUD, ("Set HW_VAR_AMPDU_FACTOR: %#x\n", FactorToSet));
				}
			}
			break;

		case HW_VAR_RXDMA_AGG_PG_TH:
			rtw_write8(padapter, REG_RXDMA_AGG_PG_TH, *val);
			break;

		case HW_VAR_H2C_FW_PWRMODE:
			{
				u8 psmode = *val;

				// Forece leave RF low power mode for 1T1R to prevent conficting setting in Fw power
				// saving sequence. 2010.06.07. Added by tynli. Suggested by SD3 yschang.
				if ((psmode != PS_MODE_ACTIVE) && (!IS_92C_SERIAL(pHalData->VersionID)))
				{
					ODM_RF_Saving(&pHalData->odmpriv, _TRUE);
				}

				if (psmode != PS_MODE_ACTIVE)	{
					rtl8723b_set_lowpwr_lps_cmd(padapter, _TRUE);
				} else {
					rtl8723b_set_lowpwr_lps_cmd(padapter, _FALSE);
				}
				rtl8723b_set_FwPwrMode_cmd(padapter, psmode);
			}
			break;
		case HW_VAR_H2C_PS_TUNE_PARAM:
			rtl8723b_set_FwPsTuneParam_cmd(padapter);
			break;

		case HW_VAR_H2C_FW_JOINBSSRPT:
			rtl8723b_set_FwJoinBssRpt_cmd(padapter, *val);
			break;

#ifdef CONFIG_P2P
		case HW_VAR_H2C_FW_P2P_PS_OFFLOAD:
			rtl8723b_set_p2p_ps_offload_cmd(padapter, *val);
			break;
#endif //CONFIG_P2P

		case HW_VAR_INITIAL_GAIN:
			{
				DIG_T *pDigTable = &pHalData->odmpriv.DM_DigTable;
				u32 rx_gain = *(u32*)val;

				if (rx_gain == 0xff) {//restore rx gain
					ODM_Write_DIG(&pHalData->odmpriv, pDigTable->BackupIGValue);
				} else {
					pDigTable->BackupIGValue = pDigTable->CurIGValue;
					ODM_Write_DIG(&pHalData->odmpriv, rx_gain);
				}
			}
			break;

#ifdef CONFIG_SW_ANTENNA_DIVERSITY
		case HW_VAR_ANTENNA_DIVERSITY_LINK:
			//SwAntDivRestAfterLink8192C(padapter);
			ODM_SwAntDivRestAfterLink(&pHalData->odmpriv);
			break;

		case HW_VAR_ANTENNA_DIVERSITY_SELECT:
			{
				u8 Optimum_antenna = *val;

				//DBG_8192C("==> HW_VAR_ANTENNA_DIVERSITY_SELECT , Ant_(%s)\n",(Optimum_antenna==2)?"A":"B");

				//PHY_SetBBReg(padapter, rFPGA0_XA_RFInterfaceOE, 0x300, Optimum_antenna);
				ODM_SetAntenna(&pHalData->odmpriv, Optimum_antenna);
			}
			break;
#endif

		case HW_VAR_EFUSE_USAGE:
			pHalData->EfuseUsedPercentage = *val;
			break;

		case HW_VAR_EFUSE_BYTES:
			pHalData->EfuseUsedBytes = *((u16*)val);
			break;

		case HW_VAR_EFUSE_BT_USAGE:
#ifdef HAL_EFUSE_MEMORY
			pHalData->EfuseHal.BTEfuseUsedPercentage = *val;
#endif
			break;

		case HW_VAR_EFUSE_BT_BYTES:
#ifdef HAL_EFUSE_MEMORY
			pHalData->EfuseHal.BTEfuseUsedBytes = *((u16*)val);
#else
			BTEfuseUsedBytes = *((u16*)val);
#endif
			break;

		case HW_VAR_FIFO_CLEARN_UP:
			{
				#define RW_RELEASE_EN		BIT(18)
				#define RXDMA_IDLE			BIT(17)

				struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
				u8 trycnt = 100;

				// pause tx
				rtw_write8(padapter, REG_TXPAUSE, 0xff);

				// keep sn
				padapter->xmitpriv.nqos_ssn = rtw_read16(padapter, REG_NQOS_SEQ);

				if (pwrpriv->bkeepfwalive != _TRUE)
				{
					u32 v32;

					/* RX DMA stop */
					v32 = rtw_read32(padapter, REG_RXPKT_NUM);
					v32 |= RW_RELEASE_EN;
					rtw_write32(padapter, REG_RXPKT_NUM, v32);
					do {
						v32 = rtw_read32(padapter, REG_RXPKT_NUM);// & RXDMA_IDLE;
						/* RXDMA_IDLE set 1 is idle */
						if (v32 & RXDMA_IDLE)
							break;

						DBG_871X("%s HW_VAR_FIFO_CLEARN_UP times:%d %x\n", __func__, trycnt, v32);
					} while (trycnt--);
					if (trycnt == 0) {
						DBG_8192C("Stop RX DMA failed......\n");
					}

					// RQPN Load 0
					rtw_write16(padapter, REG_RQPN_NPQ, 0);
					rtw_write32(padapter, REG_RQPN, 0x80000000);
					rtw_mdelay_os(2);
				}
			}
			break;

		case HW_VAR_CHECK_TXBUF:
#ifdef CONFIG_CONCURRENT_MODE
			{
				u16 v16;
				u32 i;
				u8 RetryLimit = 0x01;

				//rtw_write16(padapter, REG_RL,0x0101);
				v16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
				rtw_write16(padapter, REG_RL, v16);

				for (i=0; i<1000; i++)
				{
					if (rtw_read32(padapter, 0x200) != rtw_read32(padapter, 0x204))
					{
						//DBG_871X("packet in tx packet buffer - 0x204=%x, 0x200=%x (%d)\n", rtw_read32(padapter, 0x204), rtw_read32(padapter, 0x200), i);
						rtw_msleep_os(10);
					}
					else
					{
						DBG_871X("no packet in tx packet buffer (%d)\n", i);
						break;
					}
				}

				RetryLimit = 0x30;
				v16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
				rtw_write16(padapter, REG_RL, v16);
			}
#endif
			break;

		case HW_VAR_APFM_ON_MAC:
			pHalData->bMacPwrCtrlOn = *val;
#ifdef PLATFORM_LINUX
			DBG_8192C("%s: bMacPwrCtrlOn=%d\n", __func__, pHalData->bMacPwrCtrlOn);
#endif
			break;

		case HW_VAR_NAV_UPPER:
			{
				u32 usNavUpper = *((u32*)val);

				if (usNavUpper > HAL_8723B_NAV_UPPER_UNIT * 0xFF)
				{
					RT_TRACE(_module_hal_init_c_, _drv_notice_, ("The setting value (0x%08X us) of NAV_UPPER is larger than (%d * 0xFF)!!!\n", usNavUpper, HAL_8723B_NAV_UPPER_UNIT));
					break;
				}

				// The value of ((usNavUpper + HAL_8723B_NAV_UPPER_UNIT - 1) / HAL_8723B_NAV_UPPER_UNIT)
				// is getting the upper integer.
				usNavUpper = (usNavUpper + HAL_8723B_NAV_UPPER_UNIT - 1) / HAL_8723B_NAV_UPPER_UNIT;
				rtw_write8(padapter, REG_NAV_UPPER, (u8)usNavUpper);
			}
			break;

#ifndef CONFIG_C2H_PACKET_EN
		case HW_VAR_C2H_HANDLE:
			C2HCommandHandler(padapter);
			break;
#endif

		case HW_VAR_BCN_VALID:
			//BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2, write 1 to clear, Clear by sw
			//rtw_write8(padapter, REG_TDECTRL+2, rtw_read8(padapter, REG_TDECTRL+2) | BIT0);
#ifdef CONFIG_CONCURRENT_MODE
			if (padapter->iface_type == IFACE_PORT1)
			{
				DBG_871X("HW_VAR_BCN_VALID: Port 1 Case\n");
				val8 = rtw_read8(padapter, REG_TDECTRL1+2);
				val8 |= BIT(0);
				rtw_write8(padapter, REG_TDECTRL1+2, val8);
			}
			else
#endif // CONFIG_CONCURRENT_MODE
			{
				// BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2, write 1 to clear, Clear by sw
				DBG_871X("HW_VAR_BCN_VALID: Port 0 Case\n");
				val8 = rtw_read8(padapter, REG_TDECTRL+2);
				val8 |= BIT(0);
				rtw_write8(padapter, REG_TDECTRL+2, val8);
			}
			break;

			case HW_VAR_DL_BCN_SEL:
#ifdef CONFIG_CONCURRENT_MODE
			if (padapter->iface_type == IFACE_PORT1)
			{
				// SW_BCN_SEL - Port1
				DBG_871X("HW_VAR_DL_BCN_SEL: set Port 1 SW beacon \n");
				val8 = rtw_read8(padapter, REG_TDECTRL1+2);
				val8 |= BIT(4);
				rtw_write8(padapter, REG_TDECTRL1+2, val8);
			}
			else
#endif // CONFIG_CONCURRENT_MODE
			{
				// SW_BCN_SEL - Port0
				DBG_871X("HW_VAR_DL_BCN_SEL: set Port 0 SW beacon \n");
				val8 = rtw_read8(padapter, REG_TDECTRL1+2);
				val8 &= ~BIT(4);
				rtw_write8(padapter, REG_TDECTRL1+2, val8);
			}
			break;

		case HW_VAR_SWANT_OUTOFCOEX:
			{
				u8 AntForWifi = *val;
				hw_var_set_ant_outof_coex(padapter, AntForWifi);
			}
			break;

		default:
			break;
	}

_func_exit_;
}

void GetHwReg8723B(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	u8 val8;

	switch (variable)
	{
		case HW_VAR_BASIC_RATE:
			*((u16*)val) = pHalData->BasicRateSet;
			break;

		case HW_VAR_TXPAUSE:
			*val = rtw_read8(padapter, REG_TXPAUSE);
			break;

		case HW_VAR_BCN_VALID:
			//BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2
			//val[0] = (BIT0 & rtw_read8(padapter, REG_TDECTRL+2))?_TRUE:_FALSE;
#ifdef CONFIG_CONCURRENT_MODE
			if (padapter->iface_type == IFACE_PORT1)
			{
				val8 = rtw_read8(padapter, REG_TDECTRL1+2);
				*val = (BIT(0) & val8) ? _TRUE:_FALSE;
			}
			else
#endif
			{
				// BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2
				val8 = rtw_read8(padapter, REG_TDECTRL+2);
				DBG_871X("HW_VAR_BCN_VALID: val8=%x \n",val8);
				*val = (BIT(0) & val8) ? _TRUE:_FALSE;
			}
			break;

		case HW_VAR_RF_TYPE:
			*val = pHalData->rf_type;
			break;

		case HW_VAR_DM_FLAG:
			{
				PDM_ODM_T podmpriv = &pHalData->odmpriv;
				*((u32*)val) = podmpriv->SupportAbility;
			}
			break;

		case HW_VAR_FWLPS_RF_ON:
			{
				// When we halt NIC, we should check if FW LPS is leave.
				u32 valRCR;

				if ((padapter->bSurpriseRemoved == _TRUE) ||
					(adapter_to_pwrctl(padapter)->rf_pwrstate == rf_off))
				{
					// If it is in HW/SW Radio OFF or IPS state, we do not check Fw LPS Leave,
					// because Fw is unload.
					*val = _TRUE;
				}
				else
				{
					valRCR = rtw_read32(padapter, REG_RCR);
					valRCR &= 0x00070000;
					if(valRCR)
						*val = _FALSE;
					else
						*val = _TRUE;
				}
			}
			break;

#ifdef CONFIG_ANTENNA_DIVERSITY
		case HW_VAR_CURRENT_ANTENNA:
			*val = pHalData->CurAntenna;
			break;
#endif

		case HW_VAR_EFUSE_USAGE:
			*val = pHalData->EfuseUsedPercentage;
			break;

		case HW_VAR_EFUSE_BYTES:
			*((u16*)val) = pHalData->EfuseUsedBytes;
			break;

		case HW_VAR_EFUSE_BT_USAGE:
#ifdef HAL_EFUSE_MEMORY
			*val = pHalData->EfuseHal.BTEfuseUsedPercentage;
#endif
			break;

		case HW_VAR_EFUSE_BT_BYTES:
#ifdef HAL_EFUSE_MEMORY
			*((u16*)val) = pHalData->EfuseHal.BTEfuseUsedBytes;
#else
			*((u16*)val) = BTEfuseUsedBytes;
#endif
			break;

		case HW_VAR_APFM_ON_MAC:
			*val = pHalData->bMacPwrCtrlOn;
			break;
		case HW_VAR_CHK_HI_QUEUE_EMPTY:
			*val = ((rtw_read32(padapter, REG_HGQ_INFORMATION)&0x0000ff00)==0) ? _TRUE:_FALSE;
			break;
		case HW_VAR_RPWM_TOG:
			*val = rtw_read8(padapter, SDIO_LOCAL_BASE|SDIO_REG_HRPWM1) & BIT7;
			break;
#ifdef CONFIG_WOWLAN
		case HW_VAR_WAKEUP_REASON:
			*val = rtw_read8(padapter, REG_WOWLAN_WAKE_REASON_8723B);
			if(*val == 0xEA)
				*val = 0;
			break;
#endif

	}
}

#ifdef CONFIG_WOWLAN
void Hal_DetectWoWMode(PADAPTER pAdapter)
{
	adapter_to_pwrctl(pAdapter)->bSupportRemoteWakeup = _TRUE;
	DBG_871X("%s\n", __func__);
}
#endif //CONFIG_WOWLAN

#ifdef CONFIG_BT_COEXIST
void rtl8723b_SingleDualAntennaDetection(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	PDM_ODM_T pDM_Odm;
	pSWAT_T pDM_SWAT_Table;
	u8 btAntNum;
	u8 i;


	pHalData = GET_HAL_DATA(padapter);
	pDM_Odm = &pHalData->odmpriv;
	pDM_SWAT_Table= &pDM_Odm->DM_SWAT_Table;

	//
	// <Roger_Notes> RTL8723A Single and Dual antenna dynamic detection mechanism when RF power state is on.
	// We should take power tracking, IQK, LCK, RCK RF read/write operation into consideration.
	// 2011.12.15.
	//
	if (IS_HARDWARE_TYPE_8723A(padapter) && !pHalData->bAntennaDetected)
	{
		u8 btAntNum = BT_GetPgAntNum(padapter);

		// Set default antenna B status
		if (btAntNum == Ant_x2)
			pDM_SWAT_Table->ANTB_ON = _TRUE;
		else if (btAntNum == Ant_x1)
			pDM_SWAT_Table->ANTB_ON = _FALSE;
		else
			pDM_SWAT_Table->ANTB_ON = _TRUE;

		if (pHalData->CustomerID != RT_CID_TOSHIBA )
		{
			for (i=0; i<MAX_ANTENNA_DETECTION_CNT; i++)
			{
				if (ODM_SingleDualAntennaDetection(&pHalData->odmpriv, ANTTESTALL) == _TRUE)
					break;
			}

			// Set default antenna number for BT coexistence
			if (btAntNum == Ant_x2)
				//BT_SetBtCoexCurrAntNum(padapter, pDM_SWAT_Table->ANTB_ON ? 2 : 1);
				BT_SetBtCoexAntNum(padapter, BT_COEX_ANT_TYPE_ANTDIV, pDM_SWAT_Table->ANTB_ON ? 2 : 1);
		}
		pHalData->bAntennaDetected = _TRUE;
	}
}
#endif // CONFIG_BT_COEXIST

void rtl8723b_start_thread(_adapter *padapter)
{
#if (defined CONFIG_SDIO_HCI) || (defined CONFIG_GSPI_HCI)
#ifndef CONFIG_SDIO_TX_TASKLET
	struct xmit_priv *pxmitpriv= &padapter->xmitpriv;

	pxmitpriv->SdioXmitThread = kernel_thread(rtl8723bs_xmit_thread, padapter, CLONE_FS|CLONE_FILES);
	if (pxmitpriv->SdioXmitThread < 0) {
		RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("%s: start rtl8723bs_xmit_thread FAIL!!\n", __FUNCTION__));
	} else {
		_rtw_down_sema(&pxmitpriv->SdioXmitTerminateSema);
	}
#endif
#endif
}

void rtl8723b_stop_thread(_adapter *padapter)
{
#if (defined CONFIG_SDIO_HCI) || (defined CONFIG_GSPI_HCI)
#ifndef CONFIG_SDIO_TX_TASKLET
	struct xmit_priv *pxmitpriv= &padapter->xmitpriv;

	// stop xmit_buf_thread
	if (pxmitpriv->SdioXmitThread ) {
		_rtw_up_sema(&pxmitpriv->SdioXmitSema);
		_rtw_down_sema(&pxmitpriv->SdioXmitTerminateSema);
		pxmitpriv->SdioXmitThread = 0;
	}
#endif
#endif
}

#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
extern void check_bt_status_work(void *data);
void rtl8723bs_init_checkbthang_workqueue(_adapter * adapter)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	adapter->priv_checkbt_wq = alloc_workqueue("sdio_wq", 0, 0);
#else
	adapter->priv_checkbt_wq = create_workqueue("sdio_wq");
#endif
	INIT_DELAYED_WORK(&adapter->checkbt_work, (void*)check_bt_status_work);
}

void rtl8723bs_free_checkbthang_workqueue(_adapter * adapter)
{
	if (adapter->priv_checkbt_wq) {
		cancel_delayed_work_sync(&adapter->checkbt_work);
		flush_workqueue(adapter->priv_checkbt_wq);
		destroy_workqueue(adapter->priv_checkbt_wq);
		adapter->priv_checkbt_wq = NULL;
	}
}

void rtl8723bs_cancle_checkbthang_workqueue(_adapter * adapter)
{
	if (adapter->priv_checkbt_wq) {
		cancel_delayed_work_sync(&adapter->checkbt_work);
	}
}

void rtl8723bs_hal_check_bt_hang(_adapter * adapter)
{
	if (adapter->priv_checkbt_wq)
		queue_delayed_work(adapter->priv_checkbt_wq, &(adapter->checkbt_work), 0);
}
#endif



