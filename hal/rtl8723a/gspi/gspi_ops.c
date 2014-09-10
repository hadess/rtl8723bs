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
 *******************************************************************************/
#define _GSPI_OPS_C_

#include <drv_types.h>
#include <gspi_ops.h>
#include <rtl8723a_spec.h>
#include <rtl8723a_hal.h>

extern u8 spi_read8(ADAPTER *Adapter, u32 addr, s32 *err);
extern u16 spi_read16(ADAPTER *Adapter, u32 addr, s32 *err);
extern u32 spi_read32(ADAPTER *Adapter, u32 addr, s32 *err);
extern void spi_write8(ADAPTER *Adapter, u32 addr, u8 v, s32 *err);
extern void spi_write16(ADAPTER *Adapter, u32 addr, u16 v, s32 *err);
extern void spi_write32(ADAPTER *Adapter, u32 addr, u32 v, s32 *err);
unsigned int spi_write8_endian(ADAPTER *Adapter, unsigned int addr, unsigned int buf, u32 big);
void spi_write_tx_fifo(ADAPTER *Adapter, u8 *buf, int len, u32 fifo);
int spi_read_rx_fifo(ADAPTER *Adapter, unsigned char *buf, int len, struct spi_more_data *pmore_data);


u8 _spi_read8(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 val;

_func_enter_;

	val = spi_read8(pintfhdl->padapter, addr, NULL);

_func_exit_;

	return val;
}

u16 _spi_read16(struct intf_hdl *pintfhdl, u32 addr)
{
	u16 val;

_func_enter_;

	val = spi_read16(pintfhdl->padapter, addr, NULL);

_func_exit_;

	return val;
}

u32 _spi_read32(struct intf_hdl *pintfhdl, u32 addr)
{
	u32 val;

_func_enter_;

	val = spi_read32(pintfhdl->padapter, addr, NULL);

_func_exit_;

	return val;
}

s32 _spi_readN(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pbuf)
{
	DBG_8192C("Oops: %s not supported in GSPI\n", __func__);
	return 0;
}

s32 _spi_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	s32 err;

_func_enter_;

	spi_write8(pintfhdl->padapter, addr, val, &err);

_func_exit_;

	return err;
}

s32 _spi_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	s32 err;

_func_enter_;

	spi_write16(pintfhdl->padapter, addr, val, &err);

_func_exit_;

	return err;
}

s32 _spi_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	s32 err;

_func_enter_;

	spi_write32(pintfhdl->padapter, addr, val, &err);

_func_exit_;

	return err;
}

s32 _spi_writeN(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8* pbuf)
{
	DBG_8192C("Oops: %s not supported in GSPI\n", __func__);
	return 0;
}

void _spi_read_mem(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	DBG_8192C("Oops: %s not supported in GSPI\n", __func__);
}

void _spi_write_mem(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *wmem)
{
	DBG_8192C("Oops: %s not supported in GSPI\n", __func__);
}

/*
 * Description:
 *	Read from RX FIFO
 *	Round read size to block size,
 *	and make sure data transfer will be done in one command.
 *
 * Parameters:
 *	pintfhdl	a pointer of intf_hdl
 *	addr		port ID
 *	cnt			size to read
 *	rmem		address to put data
 *
 * Return:
 *	_SUCCESS(1)		Success
 *	_FAIL(0)		Fail
 */
static u32 _spi_read_port(
	struct intf_hdl *pintfhdl,
	u32 addr,
	u32 cnt,
	u8 *mem)
{
	struct spi_more_data more_data = {0};

	DBG_8192C("%s \n", __func__);

	spi_read_rx_fifo(pintfhdl->padapter, mem, cnt, &more_data);

	return 0;
}

/*
 *	Description:
 *		Translate QSEL to hardware tx FIFO address
 */
static u32 hwaddr2txfifo(u32 addr)
{
	u32 qsel;

	switch (addr)
	{
		case WLAN_TX_HIQ_DEVICE_ID:
			qsel = TX_HIQ_DOMAIN;
		 	break;
		case WLAN_TX_MIQ_DEVICE_ID:
			qsel = TX_MIQ_DOMAIN;
			break;
		case WLAN_TX_LOQ_DEVICE_ID:
			qsel = TX_LOQ_DOMAIN;
			break;
		default:
			qsel = TX_HIQ_DOMAIN;
			break;
	}

	return qsel;
}

static u32 _spi_write_port(
	struct intf_hdl *pintfhdl,
	u32 addr,
	u32 cnt,
	u8 *mem)
{
	u8 remain_len = 0;
	u32 w_sz = cnt;

	remain_len = w_sz%4;
	if (remain_len != 0)
		w_sz += 4 -remain_len;

	//DBG_8192C("%s fifo:%d cnt:%d w_sz:%d\n", __func__, addr, cnt, w_sz);
	spi_write_tx_fifo(pintfhdl->padapter, mem, w_sz, hwaddr2txfifo(addr));

	return 0;
}

void spi_set_intf_ops(struct _io_ops *pops)
{
_func_enter_;

	pops->_read8 = &_spi_read8;
	pops->_read16 = &_spi_read16;
	pops->_read32 = &_spi_read32;
	pops->_read_mem = &_spi_read_mem;
	pops->_read_port = &_spi_read_port;

	pops->_write8 = &_spi_write8;
	pops->_write16 = &_spi_write16;
	pops->_write32 = &_spi_write32;
	pops->_writeN = &_spi_writeN;
	pops->_write_mem = &_spi_write_mem;
	pops->_write_port = &_spi_write_port;//rtw_write_port
_func_exit_;
}

//
//	Description:
//		Initialize SDIO Host Interrupt Mask configuration variables for future use.
//
//	Assumption:
//		Using SDIO Local register ONLY for configuration.
//
//	Created by Roger, 2011.02.11.
//
void InitInterrupt8723ASdio(PADAPTER padapter)
{
	HAL_DATA_TYPE *pHalData;


	pHalData = GET_HAL_DATA(padapter);
	pHalData->sdio_himr = (u32)(			\
								SPI_HISR_RX_REQUEST			|
								//SPI_HISR_RXFOVW			|
								SPI_HISR_TXERR			|
#if defined(CONFIG_BT_COEXIST) || defined(CONFIG_MP_INCLUDED)
								SPI_HISR_C2HCMD				|
#endif
#ifdef CONFIG_LPS_LCLK
								SPI_HISR_CPWM1					|
#endif
								0);
}

//
//	Description:
//		Clear corresponding SDIO Host ISR interrupt service.
//
//	Assumption:
//		Using SDIO Local register ONLY for configuration.
//
//	Created by Roger, 2011.02.11.
//
void ClearInterrupt8723ASdio(PADAPTER padapter)
{
	u32 tmp = 0;
	tmp = rtw_read32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HISR);
	rtw_write32(padapter, SPI_LOCAL_OFFSET|SDIO_REG_HISR, tmp);
//	padapter->IsrContent.IntArray[0] = 0;
	padapter->IsrContent = 0;
}

//
//	Description:
//		Initialize System Host Interrupt Mask configuration variables for future use.
//
//	Created by Roger, 2011.08.03.
//
void InitSysInterrupt8723ASdio(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;


	pHalData = GET_HAL_DATA(padapter);

	pHalData->SysIntrMask = (			\
			//HSIMR_GPIO12_0_INT_EN			|
			//HSIMR_SPS_OCP_INT_EN			|
			//HSIMR_RON_INT_EN				|
			//HSIMR_PDNINT_EN				|
			//HSIMR_GPIO9_INT_EN				|
			0);
}
//
//	Description:
//		Enalbe SDIO Host Interrupt Mask configuration on SDIO local domain.
//
//	Assumption:
//		1. Using SDIO Local register ONLY for configuration.
//		2. PASSIVE LEVEL
//
//	Created by Roger, 2011.02.11.
//
void EnableInterrupt8723ASdio(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	pHalData = GET_HAL_DATA(padapter);

	rtw_write32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HIMR, pHalData->sdio_himr);
}

//
//	Description:
//		Disable SDIO Host IMR configuration to mask unnecessary interrupt service.
//
//	Assumption:
//		Using SDIO Local register ONLY for configuration.
//
//	Created by Roger, 2011.02.11.
//
void DisableInterrupt8723ASdio(PADAPTER padapter)
{
	rtw_write32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HIMR, SPI_HIMR_DISABLED);
}

//
//	Description:
//		Update SDIO Host Interrupt Mask configuration on SDIO local domain.
//
//	Assumption:
//		1. Using SDIO Local register ONLY for configuration.
//		2. PASSIVE LEVEL
//
//	Created by Roger, 2011.02.11.
//
void UpdateInterruptMask8723ASdio(PADAPTER padapter, u32 AddMSR, u32 RemoveMSR)
{
	HAL_DATA_TYPE *pHalData;


	pHalData = GET_HAL_DATA(padapter);

	if (AddMSR)
		pHalData->sdio_himr |= AddMSR;

	if (RemoveMSR)
		pHalData->sdio_himr &= (~RemoveMSR);

	DisableInterrupt8723ASdio(padapter);
	EnableInterrupt8723ASdio(padapter);
}

static void spi_clean_rxfifo(PADAPTER padapter, u32 size)
{
	u8 *pbuf;
	u32 bufSize, readsize;
	s32 ret;
	struct spi_more_data more_data = {0};


	pbuf = NULL;
	bufSize = adapter_to_dvobj(padapter)->intf_data.block_transfer_len;
	do {
		pbuf = rtw_malloc(bufSize);
		if (pbuf) break;
		bufSize /= 2;
		if (bufSize == 0) {
			DBG_8192C(KERN_ERR "%s: Can't allocate any memory for RX!!\n", __func__);
			return;
		}
	} while (1);

	while (size)
	{
		u8 remain_len = 0;

		readsize = (size>bufSize)?bufSize:size;
		remain_len = readsize%4;
		if (remain_len != 0)
			readsize += 4 -remain_len;

		ret = spi_read_rx_fifo(padapter, pbuf, readsize, &more_data);
		if (ret == _FAIL) {
			DBG_8192C(KERN_ERR "%s: read port FAIL! size=%d\n", __func__, readsize);
			break;
		}
		size -= readsize;
	}

	rtw_mfree(pbuf, bufSize);
}

static struct recv_buf* spi_recv_rxfifo(PADAPTER padapter, u32 size, struct spi_more_data* more_data)
{
	u32 readsize, allocsize, ret;
	u8 *preadbuf;
	_pkt *ppkt;
	struct recv_priv *precvpriv;
	struct recv_buf	*precvbuf;
	u8 remain_len = 0;

	readsize = size;
	remain_len = size%4;
	if (remain_len != 0)
		readsize += 4 -remain_len;

	//3 1. alloc skb
	// align to block size
	allocsize = _RND(readsize, adapter_to_dvobj(padapter)->intf_data.block_transfer_len);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)) // http://www.mail-archive.com/netdev@vger.kernel.org/msg17214.html
	ppkt = __dev_alloc_skb(allocsize, GFP_KERNEL);
#else
	ppkt = __netdev_alloc_skb(padapter->pnetdev, allocsize, GFP_KERNEL);
#endif
	if (ppkt == NULL) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: alloc_skb fail! alloc=%d read=%d\n", __FUNCTION__, allocsize, readsize));
#ifdef TODO
		spi_clean_rxfifo(padapter, size);
#endif
		return NULL;
	}

	//3 2. read data from rxfifo
	preadbuf = skb_put(ppkt, readsize);
//	rtw_read_port(padapter, WLAN_RX0FF_DEVICE_ID, readsize, preadbuf);
	//DBG_8192C("%s readsize:%d\n", __func__, readsize);
	ret = spi_read_rx_fifo(padapter, preadbuf, readsize, more_data);
	if (ret == _FAIL) {
		dev_kfree_skb_any(ppkt);
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: read port FAIL!\n", __FUNCTION__));
		return NULL;
	}

	//3 3. alloc recvbuf
	precvpriv = &padapter->recvpriv;
	precvbuf = rtw_dequeue_recvbuf(&precvpriv->free_recv_buf_queue);
	if (precvbuf == NULL) {
		dev_kfree_skb_any(ppkt);
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: alloc recvbuf FAIL!\n", __FUNCTION__));
		return NULL;
	}

	if (0) {
		u32 i = 0;

		DBG_8192C("%s dump pkt: len:%d\n", __func__, readsize);
		for (i = 0; i < readsize; i = i + 4) {
			DBG_8192C("%8.8x \n", *((u32*)(ppkt->data + i)));
		}
	}
	//3 4. init recvbuf
	precvbuf->pskb = ppkt;

	precvbuf->len = ppkt->len;

	precvbuf->phead = ppkt->head;
	precvbuf->pdata = ppkt->data;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	precvbuf->ptail = ppkt->head + ppkt->tail;
	precvbuf->pend = ppkt->head + ppkt->end;
#else
	precvbuf->ptail = ppkt->tail;
	precvbuf->pend = ppkt->end;
#endif

	return precvbuf;
}

static void spi_rxhandler(PADAPTER padapter, struct recv_buf *precvbuf)
{
	struct recv_priv *precvpriv;
	_queue *ppending_queue;


	precvpriv = &padapter->recvpriv;
	ppending_queue = &precvpriv->recv_buf_pending_queue;

	if (_rtw_queue_empty(ppending_queue) == _TRUE)
	{
		//3 1. enqueue recvbuf
		rtw_enqueue_recvbuf(precvbuf, ppending_queue);

		//3 2. schedule tasklet
#ifdef PLATFORM_LINUX
#ifndef CONFIG_GSPI_HCI
		tasklet_schedule(&precvpriv->recv_tasklet);
#endif
#endif
	} else
		rtw_enqueue_recvbuf(precvbuf, ppending_queue);

}

void spi_recv_work_callback(PADAPTER padapter)
{
	PHAL_DATA_TYPE phal = GET_HAL_DATA(padapter);
	struct recv_buf *precvbuf;
	struct spi_more_data more_data = {0};

	phal->SdioRxFIFOSize = rtw_read16(padapter, SPI_LOCAL_OFFSET | SPI_REG_RX0_REQ_LEN);
	//DBG_8192C("%s: RX Request, size=%d\n", __func__, phal->SdioRxFIFOSize);
	do {
		more_data.more_data = 0;
		more_data.len = 0;

		if (phal->SdioRxFIFOSize == 0)
		{
			u16 val = 0;
			s32 ret;

			val = spi_read16(padapter, SPI_LOCAL_OFFSET | SPI_REG_RX0_REQ_LEN, &ret);
			if (!ret) {
				phal->SdioRxFIFOSize = val;
				DBG_8192C("%s: RX_REQUEST, read RXFIFOsize again size=%d\n", __func__, phal->SdioRxFIFOSize);
			} else {
				DBG_8192C(KERN_ERR "%s: RX_REQUEST, read RXFIFOsize ERROR!!\n", __func__);
			}
		}

		if (phal->SdioRxFIFOSize != 0)
		{
#ifdef RTL8723A_SDIO_LOOPBACK
			sd_recv_loopback(padapter, phal->SdioRxFIFOSize);
#else
			precvbuf = spi_recv_rxfifo(padapter, phal->SdioRxFIFOSize, &more_data);
			if (precvbuf)
				spi_rxhandler(padapter, precvbuf);
			if (more_data.more_data) {
				phal->SdioRxFIFOSize = more_data.len;
			}
#endif
		}
	} while (more_data.more_data);
#ifdef PLATFORM_LINUX
#ifdef CONFIG_GSPI_HCI
	tasklet_schedule(&padapter->recvpriv.recv_tasklet);
#endif
#endif
}

void spi_int_dpc(PADAPTER padapter, u32 sdio_hisr)
{
	PHAL_DATA_TYPE phal;
	static u32 last_c2h;


	phal = GET_HAL_DATA(padapter);

	if (sdio_hisr & SPI_HISR_CPWM1)
	{
		struct reportpwrstate_parm report;

		report.state = spi_read8(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HCPWM1, NULL);
#ifdef CONFIG_LPS_LCLK
		cpwm_int_hdl(padapter, &report);
#endif
	}

	if (sdio_hisr & SPI_HISR_TXERR)
	{
		u32 status;

		status = rtw_read32(padapter, REG_TXDMA_STATUS);
		rtw_write32(padapter, REG_TXDMA_STATUS, status);
		//DBG_8192C("%s: SPI_HISR_TXERR (0x%08x)\n", __func__, status);
	}

	if (sdio_hisr & SPI_HISR_TXBCNOK)
	{
		DBG_8192C("%s: SPI_HISR_TXBCNOK\n", __func__);
	}

	if (sdio_hisr & SPI_HISR_TXBCNERR)
	{
		DBG_8192C("%s: SPI_HISR_TXBCNERR\n", __func__);
	}

	if (sdio_hisr & SPI_HISR_C2HCMD)
	{
		DBG_8192C("%s: C2H Command\n", __func__);
		last_c2h = rtw_c2h_wk_cmd(padapter);
	} else if (last_c2h != _SUCCESS) {
		/* check C2H stuck */
		u8 have_c2h = 0;

		have_c2h = rtw_read8(padapter, REG_C2HEVT_CLEAR);

		if (have_c2h) {
			DBG_871X_LEVEL(_drv_err_, "%s: c2h stuck\n", __FUNCTION__);
			last_c2h = rtw_c2h_wk_cmd(padapter);
		} else {
			last_c2h = _SUCCESS;
		}
	}

	if (sdio_hisr & SPI_HISR_RX_REQUEST)// || sdio_hisr & SPI_HISR_RXFOVW)
	{
		struct dvobj_priv *dvobj = padapter->dvobj;
		PGSPI_DATA pgspi_data = &dvobj->intf_data;

		if (pgspi_data->priv_wq)
			queue_delayed_work(pgspi_data->priv_wq, &pgspi_data->recv_work, 0);
	}
}

void spi_int_hdl(PADAPTER padapter)
{
	u8 data[6];
	s32 ret;
	PHAL_DATA_TYPE phal;
	u32 sdio_hisr = 0;


	if ((padapter->bDriverStopped == _TRUE) ||
	    (padapter->bSurpriseRemoved == _TRUE))
		goto exit;

	phal = GET_HAL_DATA(padapter);

	sdio_hisr = spi_read32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HISR, &ret);
	//if (sdio_hisr)
	//	DBG_8192C("%s hisr:%x\n", __func__, sdio_hisr);
	if (ret) {
		DBG_8192C(KERN_ERR "%s: read SDIO_REG_HISR FAIL!!\n", __func__);
		goto exit;
	}

	if (sdio_hisr & phal->sdio_himr)
	{
		u32 v32;

		sdio_hisr &= phal->sdio_himr;

		// clear HISR
		v32 = sdio_hisr & MASK_SPI_HISR_CLEAR;
		if (v32) {
			spi_write32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HISR, v32, &ret);
		}

		spi_int_dpc(padapter, sdio_hisr);
	} else {
		//RT_TRACE(_module_hci_ops_c_, _drv_err_,
		//		("%s: HISR(0x%08x) and HIMR(0x%08x) not match!\n",
		//		__FUNCTION__, sdio_hisr, phal->sdio_himr));
	}


exit:
	return;
}

//
//	Description:
//		Query SDIO Local register to query current the number of Free TxPacketBuffer page.
//
//	Assumption:
//		1. Running at PASSIVE_LEVEL
//		2. RT_TX_SPINLOCK is NOT acquired.
//
//	Created by Roger, 2011.01.28.
//
u8 HalQueryTxBufferStatus8723ASdio(PADAPTER padapter)
{
	PHAL_DATA_TYPE phal;
	u32 NumOfFreePage;
//	_irqL irql;


	phal = GET_HAL_DATA(padapter);

	NumOfFreePage = rtw_read32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_FREE_TXPG);

//	_enter_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);
	_rtw_memcpy(phal->SdioTxFIFOFreePage, &NumOfFreePage, 4);
	RT_TRACE(_module_hci_ops_c_, _drv_notice_,
			("%s: Free page for HIQ(%#x),MIDQ(%#x),LOWQ(%#x),PUBQ(%#x)\n",
			__FUNCTION__,
			phal->SdioTxFIFOFreePage[HI_QUEUE_IDX],
			phal->SdioTxFIFOFreePage[MID_QUEUE_IDX],
			phal->SdioTxFIFOFreePage[LOW_QUEUE_IDX],
			phal->SdioTxFIFOFreePage[PUBLIC_QUEUE_IDX]));
//	_exit_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);

	return _TRUE;
}

#ifdef CONFIG_WOWLAN
void DisableInterruptButCpwm28723ASdio(PADAPTER padapter)
{
	u32 himr, tmp;

#ifdef CONFIG_CONCURRENT_MODE
	if ((padapter->isprimary == _FALSE) && padapter->pbuddy_adapter){
		padapter = padapter->pbuddy_adapter;
	}
#endif
	tmp = rtw_read32(padapter, SDIO_LOCAL_BASE | SDIO_REG_HIMR);
	DBG_871X("DisableInterruptButCpwm28723ASdio(): Read SDIO_REG_HIMR: 0x%08x\n", tmp);

	himr = cpu_to_le32(SPI_HIMR_DISABLED) | SPI_HISR_CPWM2;
	rtw_write32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HIMR, himr);

	tmp = rtw_read32(padapter, SPI_LOCAL_OFFSET | SDIO_REG_HIMR);
	DBG_871X("DisableInterruptButCpwm28723ASdio(): Read again SDIO_REG_HIMR: 0x%08x\n", tmp);
}

u8 RecvOnePkt(PADAPTER padapter, u32 size)
{
	struct recv_buf *precvbuf;
	struct spi_more_data more_data = {0};
	u8 res = _FALSE;

	DBG_871X("+%s: size: %d+\n", __func__, size);

	if (padapter == NULL) {
		DBG_871X(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return _FALSE;
	}

	if(size) {
		precvbuf = spi_recv_rxfifo(padapter, size, &more_data);

		if (precvbuf) {
			printk("Start Recv One Pkt.\n");
			spi_rxhandler(padapter, precvbuf);
			res = _TRUE;
		}else{
			printk("Recv One Pkt buf alloc fail.\n");
			res = _FALSE;
		}
	}
	DBG_871X("-%s-\n", __func__);
	return res;
}
#endif //CONFIG_WOWLAN

