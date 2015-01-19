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
#define _RTL8723B_REDESC_C_

#include <rtl8723b_hal.h>

static void process_rssi(_adapter *padapter,union recv_frame *prframe)
{
	u32	last_rssi, tmp_val;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct signal_stat * signal_stat = &padapter->recvpriv.signal_strength_data;

	//DBG_8192C("process_rssi=> pattrib->rssil(%d) signal_strength(%d)\n ",pattrib->RecvSignalPower,pattrib->signal_strength);
	//if(pRfd->Status.bPacketToSelf || pRfd->Status.bPacketBeacon)
	{
		if(signal_stat->update_req) {
			signal_stat->total_num = 0;
			signal_stat->total_val = 0;
			signal_stat->update_req = 0;
		}

		signal_stat->total_num++;
		signal_stat->total_val  += pattrib->phy_info.SignalStrength;
		signal_stat->avg_val = signal_stat->total_val / signal_stat->total_num;
	}

}// Process_UI_RSSI_8192C

static void process_link_qual(_adapter *padapter,union recv_frame *prframe)
{
	u32	last_evm=0, tmpVal;
 	struct rx_pkt_attrib *pattrib;
	struct signal_stat * signal_stat;

	if(prframe == NULL || padapter==NULL){
		return;
	}

	pattrib = &prframe->u.hdr.attrib;
	signal_stat = &padapter->recvpriv.signal_qual_data;

	//DBG_8192C("process_link_qual=> pattrib->signal_qual(%d)\n ",pattrib->signal_qual);

	if(signal_stat->update_req) {
		signal_stat->total_num = 0;
		signal_stat->total_val = 0;
		signal_stat->update_req = 0;
	}

	signal_stat->total_num++;
	signal_stat->total_val  += pattrib->phy_info.SignalQuality;
	signal_stat->avg_val = signal_stat->total_val / signal_stat->total_num;
}// Process_UiLinkQuality8192S


void rtl8723b_process_phy_info(_adapter *padapter, void *prframe)
{
	union recv_frame *precvframe = (union recv_frame *)prframe;
	//
	// Check RSSI
	//
	process_rssi(padapter, precvframe);
	//
	// Check PWDB.
	//
	//process_PWDB(padapter, precvframe);

	//UpdateRxSignalStatistics8192C(Adapter, pRfd);
	//
	// Check EVM
	//
	process_link_qual(padapter,  precvframe);
	#ifdef DBG_RX_SIGNAL_DISPLAY_RAW_DATA
	rtw_store_phy_info( padapter,prframe);
	#endif

}

void rtl8723b_query_rx_desc_status(union recv_frame *precvframe, u8 *pdesc)
{
	struct rx_pkt_attrib	*pattrib = &precvframe->u.hdr.attrib;

	memset(pattrib, 0, sizeof(struct rx_pkt_attrib));

	//Offset 0
	pattrib->pkt_len = (u16)GET_RX_STATUS_DESC_PKT_LEN_8723B(pdesc);//(le32_to_cpu(pdesc->rxdw0)&0x00003fff)
	pattrib->crc_err = (u8)GET_RX_STATUS_DESC_CRC32_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw0) >> 14) & 0x1);
	pattrib->icv_err = (u8)GET_RX_STATUS_DESC_ICV_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw0) >> 15) & 0x1);
	pattrib->drvinfo_sz = (u8)GET_RX_STATUS_DESC_DRVINFO_SIZE_8723B(pdesc) * 8;//((le32_to_cpu(pdesc->rxdw0) >> 16) & 0xf) * 8;//uint 2^3 = 8 bytes
	pattrib->encrypt = (u8)GET_RX_STATUS_DESC_SECURITY_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw0) >> 20) & 0x7);
	pattrib->qos = (u8)GET_RX_STATUS_DESC_QOS_8723B(pdesc);//(( le32_to_cpu( pdesc->rxdw0 ) >> 23) & 0x1);// Qos data, wireless lan header length is 26
	pattrib->shift_sz = (u8)GET_RX_STATUS_DESC_SHIFT_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw0) >> 24) & 0x3);
	pattrib->physt = (u8)GET_RX_STATUS_DESC_PHY_STATUS_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw0) >> 26) & 0x1);
	pattrib->bdecrypted = !GET_RX_STATUS_DESC_SWDEC_8723B(pdesc);//(le32_to_cpu(pdesc->rxdw0) & BIT(27))? 0:1;

	//Offset 4
	pattrib->priority = (u8)GET_RX_STATUS_DESC_TID_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw1) >> 8) & 0xf);
	pattrib->mdata = (u8)GET_RX_STATUS_DESC_MORE_DATA_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw1) >> 26) & 0x1);
	pattrib->mfrag = (u8)GET_RX_STATUS_DESC_MORE_FRAG_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw1) >> 27) & 0x1);//more fragment bit

	//Offset 8
	pattrib->seq_num = (u16)GET_RX_STATUS_DESC_SEQ_8723B(pdesc);//(le32_to_cpu(pdesc->rxdw2) & 0x00000fff);
	pattrib->frag_num = (u8)GET_RX_STATUS_DESC_FRAG_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw2) >> 12) & 0xf);//fragmentation number

	if (GET_RX_STATUS_DESC_RPT_SEL_8723B(pdesc))
		pattrib->pkt_rpt_type = C2H_PACKET;
	else
		pattrib->pkt_rpt_type = NORMAL_RX;

	//Offset 12
	pattrib->data_rate=(u8)GET_RX_STATUS_DESC_RX_RATE_8723B(pdesc);//((le32_to_cpu(pdesc->rxdw3))&0x7f);

	//Offset 16
	//Offset 20

}



