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
#define _RECV_OSDEP_C_

#include <drv_types.h>
#include <linux/jiffies.h>

int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb)
{
	int res = _SUCCESS;
	u8	shift_sz = 0;
	u32	skb_len, alloc_sz;
	_pkt	 *pkt_copy = NULL;	
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;


	if(pdata == NULL)
	{		
		precvframe->u.hdr.pkt = NULL;
		res = _FAIL;
		return res;
	}	


	//	Modified by Albert 20101213
	//	For 8 bytes IP header alignment.
	shift_sz = pattrib->qos ? 6:0;//	Qos data, wireless lan header length is 26

	skb_len = pattrib->pkt_len;

	// for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet.
	// modify alloc_sz for recvive crc error packet by thomas 2011-06-02
	if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
	{
		//alloc_sz = 1664;	//1664 is 128 alignment.
		alloc_sz = (skb_len <= 1650) ? 1664:(skb_len + 14);		
	}
	else 
	{
		alloc_sz = skb_len;
		//	6 is for IP header 8 bytes alignment in QoS packet case.
		//	8 is for skb->data 4 bytes alignment.
		alloc_sz += 14;
	}	

	pkt_copy = rtw_skb_alloc(alloc_sz);

	if(pkt_copy)
	{
		pkt_copy->dev = padapter->pnetdev;
		precvframe->u.hdr.pkt = pkt_copy;
		precvframe->u.hdr.rx_head = pkt_copy->data;
		precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
		skb_reserve(pkt_copy, 8 - ((SIZE_PTR)( pkt_copy->data) & 7 ));//force pkt_copy->data at 8-byte alignment address
		skb_reserve(pkt_copy, shift_sz);//force ip_hdr at 8-byte alignment address according to shift_sz.
		memcpy(pkt_copy->data, pdata, skb_len);
		precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
	}
	else
	{
		if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
		{				
			DBG_871X("%s: alloc_skb fail , drop frag frame \n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}

		if(pskb == NULL)
		{
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}
			
		precvframe->u.hdr.pkt = rtw_skb_clone(pskb);
		if(precvframe->u.hdr.pkt)
		{
			precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pdata;
			precvframe->u.hdr.rx_end =  pdata + alloc_sz;
		}
		else
		{
			DBG_871X("%s: rtw_skb_clone fail\n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			//goto _exit_recvbuf2recvframe;
			res = _FAIL;
		}
	}		

exit_rtw_os_recv_resource_alloc:

	return res;

}

void rtw_os_free_recvframe(union recv_frame *precvframe)
{
	if(precvframe->u.hdr.pkt)
	{
		rtw_skb_free(precvframe->u.hdr.pkt);//free skb by driver

		precvframe->u.hdr.pkt = NULL;
	}
}

//init os related resource in struct recv_priv
int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter)
{
	int	res=_SUCCESS;

	return res;
}

//alloc os related resource in union recv_frame
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe)
{
	int	res=_SUCCESS;
	
	precvframe->u.hdr.pkt_newalloc = precvframe->u.hdr.pkt = NULL;

	return res;
}

//free os related resource in union recv_frame
void rtw_os_recv_resource_free(struct recv_priv *precvpriv)
{
	sint i;
	union recv_frame *precvframe;
	precvframe = (union recv_frame*) precvpriv->precv_frame_buf;

	for(i=0; i < NR_RECVFRAME; i++)
	{
		if(precvframe->u.hdr.pkt)
		{
			rtw_skb_free(precvframe->u.hdr.pkt);//free skb by driver
			precvframe->u.hdr.pkt = NULL;
		}
		precvframe++;
	}
}

//alloc os related resource in struct recv_buf
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf)
{
	int res=_SUCCESS;

	return res;
}

//free os related resource in struct recv_buf
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;

	if(precvbuf->pskb)
	{
		rtw_skb_free(precvbuf->pskb);
	}
	return ret;

}

_pkt *rtw_os_alloc_msdu_pkt(union recv_frame *prframe, u16 nSubframe_Length, u8 *pdata)
{
	u16	eth_type;
	u8	*data_ptr;
	_pkt *sub_skb;
	struct rx_pkt_attrib *pattrib;

	pattrib = &prframe->u.hdr.attrib;

	sub_skb = rtw_skb_alloc(nSubframe_Length + 12);
	if(sub_skb)
	{
		skb_reserve(sub_skb, 12);
		data_ptr = (u8 *)skb_put(sub_skb, nSubframe_Length);
		memcpy(data_ptr, (pdata + ETH_HLEN), nSubframe_Length);
	}
	else
	{
		sub_skb = rtw_skb_clone(prframe->u.hdr.pkt);
		if(sub_skb)
		{
			sub_skb->data = pdata + ETH_HLEN;
			sub_skb->len = nSubframe_Length;
			skb_set_tail_pointer(sub_skb, nSubframe_Length);
		}
		else
		{
			DBG_871X("%s(): rtw_skb_clone() Fail!!!\n",__FUNCTION__);
			return NULL;
		}
	}

	eth_type = RTW_GET_BE16(&sub_skb->data[6]);

	if (sub_skb->len >= 8 &&
		((!memcmp(sub_skb->data, rtw_rfc1042_header, SNAP_SIZE) &&
		  eth_type != ETH_P_AARP && eth_type != ETH_P_IPX) ||
		 !memcmp(sub_skb->data, rtw_bridge_tunnel_header, SNAP_SIZE) )) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
		skb_pull(sub_skb, SNAP_SIZE);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->src, ETH_ALEN);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->dst, ETH_ALEN);
	} else {
		u16 len;
		/* Leave Ethernet header part of hdr and full payload */
		len = htons(sub_skb->len);
		memcpy(skb_push(sub_skb, 2), &len, 2);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->src, ETH_ALEN);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->dst, ETH_ALEN);
	}

	return sub_skb;
}

void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt, struct rx_pkt_attrib *pattrib)
{
	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;
	int ret;

	/* Indicat the packets to upper layer */
	if (pkt) {
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == true)
		{
		 	_pkt *pskb2=NULL;
		 	struct sta_info *psta = NULL;
		 	struct sta_priv *pstapriv = &padapter->stapriv;
			int bmcast = IS_MCAST(pattrib->dst);

			//DBG_871X("bmcast=%d\n", bmcast);

			if(memcmp(pattrib->dst, myid(&padapter->eeprompriv), ETH_ALEN))
			{
				//DBG_871X("not ap psta=%p, addr=%pM\n", psta, pattrib->dst);

				if(bmcast)
				{
					psta = rtw_get_bcmc_stainfo(padapter);
					pskb2 = rtw_skb_clone(pkt);
				} else {
					psta = rtw_get_stainfo(pstapriv, pattrib->dst);
				}

				if(psta)
				{
					struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;			

					//DBG_871X("directly forwarding to the rtw_xmit_entry\n");

					//skb->ip_summed = CHECKSUM_NONE;
					pkt->dev = pnetdev;				
					skb_set_queue_mapping(pkt, rtw_recv_select_queue(pkt));

					_rtw_xmit_entry(pkt, pnetdev);

					if(bmcast && (pskb2 != NULL) ) {
						pkt = pskb2;
						DBG_COUNTER(padapter->rx_logs.os_indicate_ap_mcast);
					} else {
						DBG_COUNTER(padapter->rx_logs.os_indicate_ap_forward);
						return;
					}
				}
			}
			else// to APself
			{
				//DBG_871X("to APSelf\n");
				DBG_COUNTER(padapter->rx_logs.os_indicate_ap_self);
			}
		}

		pkt->protocol = eth_type_trans(pkt, padapter->pnetdev);
		pkt->dev = padapter->pnetdev;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_RX
		if ( (pattrib->tcpchk_valid == 1) && (pattrib->tcp_chkrpt == 1) ) {
			pkt->ip_summed = CHECKSUM_UNNECESSARY;
		} else {
			pkt->ip_summed = CHECKSUM_NONE;
		}
#else /* !CONFIG_TCP_CSUM_OFFLOAD_RX */
		pkt->ip_summed = CHECKSUM_NONE;
#endif //CONFIG_TCP_CSUM_OFFLOAD_RX

		ret = rtw_netif_rx(padapter->pnetdev, pkt);
		if (ret == NET_RX_SUCCESS)
			DBG_COUNTER(padapter->rx_logs.os_netif_ok);
		else
			DBG_COUNTER(padapter->rx_logs.os_netif_err);
	}
}

void rtw_handle_tkip_mic_err(_adapter *padapter,u8 bgroup)
{
	enum nl80211_key_type key_type = 0;
	union iwreq_data wrqu;
	struct iw_michaelmicfailure    ev;
	struct mlme_priv*              pmlmepriv  = &padapter->mlmepriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;	
	unsigned long cur_time = 0;

	if( psecuritypriv->last_mic_err_time == 0 )
	{
		psecuritypriv->last_mic_err_time = jiffies;
	}
	else
	{
		cur_time = jiffies;

		if( cur_time - psecuritypriv->last_mic_err_time < 60*HZ )
		{
			psecuritypriv->btkip_countermeasure = true;
			psecuritypriv->last_mic_err_time = 0;
			psecuritypriv->btkip_countermeasure_time = cur_time;
		}
		else
		{
			psecuritypriv->last_mic_err_time = jiffies;
		}
	}

	if ( bgroup )
	{
		key_type |= NL80211_KEYTYPE_GROUP;
	}
	else
	{
		key_type |= NL80211_KEYTYPE_PAIRWISE;
	}

	cfg80211_michael_mic_failure(padapter->pnetdev, (u8 *)&pmlmepriv->assoc_bssid[ 0 ], key_type, -1,
		NULL, GFP_ATOMIC);

	memset( &ev, 0x00, sizeof( ev ) );
	if ( bgroup )
	{
	    ev.flags |= IW_MICFAILURE_GROUP;
	}
	else
	{
	    ev.flags |= IW_MICFAILURE_PAIRWISE;
	}

	ev.src_addr.sa_family = ARPHRD_ETHER;
	memcpy( ev.src_addr.sa_data, &pmlmepriv->assoc_bssid[ 0 ], ETH_ALEN );

	memset( &wrqu, 0x00, sizeof( wrqu ) );
	wrqu.data.length = sizeof( ev );
}

void rtw_hostapd_mlme_rx(_adapter *padapter, union recv_frame *precv_frame)
{
}

#ifdef CONFIG_AUTO_AP_MODE
static void rtw_os_ksocket_send(_adapter *padapter, union recv_frame *precv_frame)
{	
	_pkt *skb = precv_frame->u.hdr.pkt;	
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_info *psta = precv_frame->u.hdr.psta;
		
	DBG_871X("eth rx: got eth_type=0x%x\n", pattrib->eth_type);					
		
	if (psta && psta->isrc && psta->pid>0)
	{
		u16 rx_pid;

		rx_pid = *(u16*)(skb->data+ETH_HLEN);
			
		DBG_871X("eth rx(pid=0x%x): sta("MAC_FMT") pid=0x%x\n", 
			rx_pid, MAC_ARG(psta->hwaddr), psta->pid);

		if(rx_pid == psta->pid)
		{
			int i;
			u16 len = *(u16*)(skb->data+ETH_HLEN+2);
			//u16 ctrl_type = *(u16*)(skb->data+ETH_HLEN+4);

			//DBG_871X("eth, RC: len=0x%x, ctrl_type=0x%x\n", len, ctrl_type); 
			DBG_871X("eth, RC: len=0x%x\n", len);

			for(i=0;i<len;i++)
				DBG_871X("0x%x\n", *(skb->data+ETH_HLEN+4+i));
				//DBG_871X("0x%x\n", *(skb->data+ETH_HLEN+6+i));

			DBG_871X("eth, RC-end\n"); 
		}
		
	}		

}
#endif //CONFIG_AUTO_AP_MODE

int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	_queue	*pfree_recv_queue;
	_pkt *skb;
	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

_func_enter_;

	DBG_COUNTER(padapter->rx_logs.os_indicate);

	precvpriv = &(padapter->recvpriv);
	pfree_recv_queue = &(precvpriv->free_recv_queue);

#ifdef CONFIG_WAPI_SUPPORT
	if (rtw_wapi_check_for_drop(padapter,precv_frame))
	{
		WAPI_TRACE(WAPI_ERR, "%s(): Rx Reorder Drop case!!\n", __FUNCTION__);
		goto _recv_indicatepkt_drop;
	}
#endif

	skb = precv_frame->u.hdr.pkt;
	if(skb == NULL)
	{
		RT_TRACE(_module_recv_osdep_c_,_drv_err_,("rtw_recv_indicatepkt():skb==NULL something wrong!!!!\n"));
		goto _recv_indicatepkt_drop;
	}

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():skb != NULL !!!\n"));		
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():precv_frame->u.hdr.rx_head=%p  precv_frame->hdr.rx_data=%p\n", precv_frame->u.hdr.rx_head, precv_frame->u.hdr.rx_data));
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("precv_frame->hdr.rx_tail=%p precv_frame->u.hdr.rx_end=%p precv_frame->hdr.len=%d \n", precv_frame->u.hdr.rx_tail, precv_frame->u.hdr.rx_end, precv_frame->u.hdr.len));

	skb->data = precv_frame->u.hdr.rx_data;

	skb_set_tail_pointer(skb, precv_frame->u.hdr.len);

	skb->len = precv_frame->u.hdr.len;

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n skb->head=%p skb->data=%p skb->tail=%p skb->end=%p skb->len=%d\n", skb->head, skb->data, skb_tail_pointer(skb), skb_end_pointer(skb), skb->len));

#ifdef CONFIG_AUTO_AP_MODE	
	if (0x8899 == pattrib->eth_type)
	{
		rtw_os_ksocket_send(padapter, precv_frame);

		//goto _recv_indicatepkt_drop;
	}
#endif //CONFIG_AUTO_AP_MODE

	rtw_os_recv_indicate_pkt(padapter, skb, pattrib);

	precv_frame->u.hdr.pkt = NULL; // pointers to NULL before rtw_free_recvframe()

	rtw_free_recvframe(precv_frame, pfree_recv_queue);

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n rtw_recv_indicatepkt :after rtw_os_recv_indicate_pkt!!!!\n"));

_func_exit_;

        return _SUCCESS;

_recv_indicatepkt_drop:

	 //enqueue back to free_recv_queue
	 if(precv_frame)
		 rtw_free_recvframe(precv_frame, pfree_recv_queue);

	 DBG_COUNTER(padapter->rx_logs.os_indicate_err);
	 return _FAIL;

_func_exit_;

}

void rtw_os_read_port(_adapter *padapter, struct recv_buf *precvbuf)
{
	struct recv_priv *precvpriv = &padapter->recvpriv;

	precvbuf->pskb = NULL;
}
void _rtw_reordering_ctrl_timeout_handler (void *FunctionContext);
void _rtw_reordering_ctrl_timeout_handler (void *FunctionContext)
{
	struct recv_reorder_ctrl *preorder_ctrl = (struct recv_reorder_ctrl *)FunctionContext;
	rtw_reordering_ctrl_timeout_handler(preorder_ctrl);
}

void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl)
{
	_adapter *padapter = preorder_ctrl->padapter;

	_init_timer(&(preorder_ctrl->reordering_ctrl_timer), padapter->pnetdev, _rtw_reordering_ctrl_timeout_handler, preorder_ctrl);

}

