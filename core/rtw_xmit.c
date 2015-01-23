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
#define _RTW_XMIT_C_

#include <drv_types.h>

static u8 P802_1H_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0xf8 };
static u8 RFC1042_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0x00 };

static void _init_txservq(struct tx_servq *ptxservq)
{
_func_enter_;
	_rtw_init_listhead(&ptxservq->tx_pending);
	_rtw_init_queue(&ptxservq->sta_pending);
	ptxservq->qcnt = 0;
_func_exit_;		
}


void	_rtw_init_sta_xmit_priv(struct sta_xmit_priv *psta_xmitpriv)
{	
	
_func_enter_;

	memset((unsigned char *)psta_xmitpriv, 0, sizeof (struct sta_xmit_priv));

	spin_lock_init(&psta_xmitpriv->lock);
	
	//for(i = 0 ; i < MAX_NUMBLKS; i++)
	//	_init_txservq(&(psta_xmitpriv->blk_q[i]));

	_init_txservq(&psta_xmitpriv->be_q);
	_init_txservq(&psta_xmitpriv->bk_q);
	_init_txservq(&psta_xmitpriv->vi_q);
	_init_txservq(&psta_xmitpriv->vo_q);
	_rtw_init_listhead(&psta_xmitpriv->legacy_dz);
	_rtw_init_listhead(&psta_xmitpriv->apsd);
	
_func_exit_;	

}

s32	_rtw_init_xmit_priv(struct xmit_priv *pxmitpriv, _adapter *padapter)
{
	int i;
	struct xmit_buf *pxmitbuf;
	struct xmit_frame *pxframe;
	sint	res=_SUCCESS;   
	
_func_enter_;   	

	// We don't need to memset padapter->XXX to zero, because adapter is allocated by rtw_zvmalloc().
	//memset((unsigned char *)pxmitpriv, 0, sizeof(struct xmit_priv));
	
	spin_lock_init(&pxmitpriv->lock);
	spin_lock_init(&pxmitpriv->lock_sctx);
	sema_init(&pxmitpriv->xmit_sema, 0);
	sema_init(&pxmitpriv->terminate_xmitthread_sema, 0);

	/* 
	Please insert all the queue initializaiton using _rtw_init_queue below
	*/

	pxmitpriv->adapter = padapter;
	
	//for(i = 0 ; i < MAX_NUMBLKS; i++)
	//	_rtw_init_queue(&pxmitpriv->blk_strms[i]);
	
	_rtw_init_queue(&pxmitpriv->be_pending);
	_rtw_init_queue(&pxmitpriv->bk_pending);
	_rtw_init_queue(&pxmitpriv->vi_pending);
	_rtw_init_queue(&pxmitpriv->vo_pending);
	_rtw_init_queue(&pxmitpriv->bm_pending);

	//_rtw_init_queue(&pxmitpriv->legacy_dz_queue);
	//_rtw_init_queue(&pxmitpriv->apsd_queue);

	_rtw_init_queue(&pxmitpriv->free_xmit_queue);

	/*	
	Please allocate memory with the sz = (struct xmit_frame) * NR_XMITFRAME, 
	and initialize free_xmit_frame below.
	Please also apply  free_txobj to link_up all the xmit_frames...
	*/

	pxmitpriv->pallocated_frame_buf = rtw_zvmalloc(NR_XMITFRAME * sizeof(struct xmit_frame) + 4);
	
	if (pxmitpriv->pallocated_frame_buf  == NULL){
		pxmitpriv->pxmit_frame_buf =NULL;
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xmit_frame fail!\n"));	
		res= _FAIL;
		goto exit;
	}
	pxmitpriv->pxmit_frame_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_frame_buf), 4);
	//pxmitpriv->pxmit_frame_buf = pxmitpriv->pallocated_frame_buf + 4 -
	//						((SIZE_PTR) (pxmitpriv->pallocated_frame_buf) &3);

	pxframe = (struct xmit_frame*) pxmitpriv->pxmit_frame_buf;

	for (i = 0; i < NR_XMITFRAME; i++)
	{
		_rtw_init_listhead(&(pxframe->list));

		pxframe->padapter = padapter;
		pxframe->frame_tag = NULL_FRAMETAG;

		pxframe->pkt = NULL;		

		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;
 
		rtw_list_insert_tail(&(pxframe->list), &(pxmitpriv->free_xmit_queue.queue));

		pxframe++;
	}

	pxmitpriv->free_xmitframe_cnt = NR_XMITFRAME;

	pxmitpriv->frag_len = MAX_FRAG_THRESHOLD;


	//init xmit_buf
	_rtw_init_queue(&pxmitpriv->free_xmitbuf_queue);
	_rtw_init_queue(&pxmitpriv->pending_xmitbuf_queue);

	pxmitpriv->pallocated_xmitbuf = rtw_zvmalloc(NR_XMITBUFF * sizeof(struct xmit_buf) + 4);
	
	if (pxmitpriv->pallocated_xmitbuf  == NULL){
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xmit_buf fail!\n"));
		res= _FAIL;
		goto exit;
	}

	pxmitpriv->pxmitbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_xmitbuf), 4);
	//pxmitpriv->pxmitbuf = pxmitpriv->pallocated_xmitbuf + 4 -
	//						((SIZE_PTR) (pxmitpriv->pallocated_xmitbuf) &3);

	pxmitbuf = (struct xmit_buf*)pxmitpriv->pxmitbuf;

	for (i = 0; i < NR_XMITBUFF; i++)
	{
		_rtw_init_listhead(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->padapter = padapter;
		pxmitbuf->buf_tag = XMITBUF_DATA;

		/* Tx buf allocation may fail sometimes, so sleep and retry. */
		if((res=rtw_os_xmit_resource_alloc(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), true)) == _FAIL) {
			msleep(10);
			res = rtw_os_xmit_resource_alloc(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), true);
			if (res == _FAIL) {
				goto exit;
			}
		}

		pxmitbuf->phead = pxmitbuf->pbuf;
		pxmitbuf->pend = pxmitbuf->pbuf + MAX_XMITBUF_SZ;
		pxmitbuf->len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;

		pxmitbuf->flags = XMIT_VO_QUEUE;

		rtw_list_insert_tail(&pxmitbuf->list, &(pxmitpriv->free_xmitbuf_queue.queue));
		#ifdef DBG_XMIT_BUF
		pxmitbuf->no=i;
		#endif

		pxmitbuf++;
		
	}

	pxmitpriv->free_xmitbuf_cnt = NR_XMITBUFF;

	/* init xframe_ext queue,  the same count as extbuf  */
	_rtw_init_queue(&pxmitpriv->free_xframe_ext_queue);
	
	pxmitpriv->xframe_ext_alloc_addr = rtw_zvmalloc(NR_XMIT_EXTBUFF * sizeof(struct xmit_frame) + 4);
	
	if (pxmitpriv->xframe_ext_alloc_addr  == NULL){
		pxmitpriv->xframe_ext = NULL;
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xframe_ext fail!\n"));	
		res= _FAIL;
		goto exit;
	}
	pxmitpriv->xframe_ext = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->xframe_ext_alloc_addr), 4);
	pxframe = (struct xmit_frame*)pxmitpriv->xframe_ext;

	for (i = 0; i < NR_XMIT_EXTBUFF; i++) {
		_rtw_init_listhead(&(pxframe->list));

		pxframe->padapter = padapter;
		pxframe->frame_tag = NULL_FRAMETAG;

		pxframe->pkt = NULL;		

		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;
		
		pxframe->ext_tag = 1;
 
		rtw_list_insert_tail(&(pxframe->list), &(pxmitpriv->free_xframe_ext_queue.queue));

		pxframe++;
	}
	pxmitpriv->free_xframe_ext_cnt = NR_XMIT_EXTBUFF;

	// Init xmit extension buff
	_rtw_init_queue(&pxmitpriv->free_xmit_extbuf_queue);

	pxmitpriv->pallocated_xmit_extbuf = rtw_zvmalloc(NR_XMIT_EXTBUFF * sizeof(struct xmit_buf) + 4);
	
	if (pxmitpriv->pallocated_xmit_extbuf  == NULL){
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xmit_extbuf fail!\n"));
		res= _FAIL;
		goto exit;
	}

	pxmitpriv->pxmit_extbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_xmit_extbuf), 4);

	pxmitbuf = (struct xmit_buf*)pxmitpriv->pxmit_extbuf;

	for (i = 0; i < NR_XMIT_EXTBUFF; i++)
	{
		_rtw_init_listhead(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->padapter = padapter;
		pxmitbuf->buf_tag = XMITBUF_MGNT;

		if((res=rtw_os_xmit_resource_alloc(padapter, pxmitbuf,MAX_XMIT_EXTBUF_SZ + XMITBUF_ALIGN_SZ, true)) == _FAIL) {
			res= _FAIL;
			goto exit;
		}

		pxmitbuf->phead = pxmitbuf->pbuf;
		pxmitbuf->pend = pxmitbuf->pbuf + MAX_XMIT_EXTBUF_SZ;
		pxmitbuf->len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;

		rtw_list_insert_tail(&pxmitbuf->list, &(pxmitpriv->free_xmit_extbuf_queue.queue));
		#ifdef DBG_XMIT_BUF_EXT
		pxmitbuf->no=i;
		#endif
		pxmitbuf++;
		
	}

	pxmitpriv->free_xmit_extbuf_cnt = NR_XMIT_EXTBUFF;

	for (i = 0; i<CMDBUF_MAX; i++) {
		pxmitbuf = &pxmitpriv->pcmd_xmitbuf[i];
		if (pxmitbuf) {
			_rtw_init_listhead(&pxmitbuf->list);

			pxmitbuf->priv_data = NULL;
			pxmitbuf->padapter = padapter;
			pxmitbuf->buf_tag = XMITBUF_CMD;

			if((res=rtw_os_xmit_resource_alloc(padapter, pxmitbuf, MAX_CMDBUF_SZ+XMITBUF_ALIGN_SZ, true)) == _FAIL) {
				res= _FAIL;
				goto exit;
			}

			pxmitbuf->phead = pxmitbuf->pbuf;
			pxmitbuf->pend = pxmitbuf->pbuf + MAX_CMDBUF_SZ;
			pxmitbuf->len = 0;
			pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
			pxmitbuf->alloc_sz = MAX_CMDBUF_SZ+XMITBUF_ALIGN_SZ;
		}
	}

	rtw_alloc_hwxmits(padapter);
	rtw_init_hwxmits(pxmitpriv->hwxmits, pxmitpriv->hwxmit_entry);

        for (i = 0; i < 4; i ++)
	{
		pxmitpriv->wmm_para_seq[i] = i;
	}

	pxmitpriv->ack_tx = false;
	mutex_init(&pxmitpriv->ack_tx_mutex);
	rtw_sctx_init(&pxmitpriv->ack_tx_ops, 0);	

	rtw_hal_init_xmit_priv(padapter);

exit:

_func_exit_;	

	return res;
}

void _rtw_free_xmit_priv (struct xmit_priv *pxmitpriv)
{
       int i;
      _adapter *padapter = pxmitpriv->adapter;
	struct xmit_frame	*pxmitframe = (struct xmit_frame*) pxmitpriv->pxmit_frame_buf;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmitbuf;
	
 _func_enter_;   

	rtw_hal_free_xmit_priv(padapter);
 
 	if(pxmitpriv->pxmit_frame_buf==NULL)
		goto out;
	
	for(i=0; i<NR_XMITFRAME; i++)
	{	
		rtw_os_xmit_complete(padapter, pxmitframe);

		pxmitframe++;
	}		
	
	for(i=0; i<NR_XMITBUFF; i++)
	{
		rtw_os_xmit_resource_free(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), true);
		
		pxmitbuf++;
	}

	if(pxmitpriv->pallocated_frame_buf) {
		rtw_vmfree(pxmitpriv->pallocated_frame_buf, NR_XMITFRAME * sizeof(struct xmit_frame) + 4);
	}
	

	if(pxmitpriv->pallocated_xmitbuf) {
		rtw_vmfree(pxmitpriv->pallocated_xmitbuf, NR_XMITBUFF * sizeof(struct xmit_buf) + 4);
	}

	/* free xframe_ext queue,  the same count as extbuf  */
	if ((pxmitframe = (struct xmit_frame*)pxmitpriv->xframe_ext)) {
		for (i=0; i<NR_XMIT_EXTBUFF; i++) {
			rtw_os_xmit_complete(padapter, pxmitframe);
			pxmitframe++;
		}
	}
	if (pxmitpriv->xframe_ext_alloc_addr)
		rtw_vmfree(pxmitpriv->xframe_ext_alloc_addr, NR_XMIT_EXTBUFF * sizeof(struct xmit_frame) + 4);

	// free xmit extension buff
	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;
	for(i=0; i<NR_XMIT_EXTBUFF; i++)
	{
		rtw_os_xmit_resource_free(padapter, pxmitbuf,(MAX_XMIT_EXTBUF_SZ + XMITBUF_ALIGN_SZ), true);
		
		pxmitbuf++;
	}

	if(pxmitpriv->pallocated_xmit_extbuf) {
		rtw_vmfree(pxmitpriv->pallocated_xmit_extbuf, NR_XMIT_EXTBUFF * sizeof(struct xmit_buf) + 4);
	}

	for (i=0; i<CMDBUF_MAX; i++) {
		pxmitbuf = &pxmitpriv->pcmd_xmitbuf[i];
		if(pxmitbuf!=NULL)
			rtw_os_xmit_resource_free(padapter, pxmitbuf, MAX_CMDBUF_SZ+XMITBUF_ALIGN_SZ , true);
	}

	rtw_free_hwxmits(padapter);

	mutex_destroy(&pxmitpriv->ack_tx_mutex);	

out:	

_func_exit_;		

}

u8	query_ra_short_GI(struct sta_info *psta)
{
	u8	sgi = false, sgi_20m = false, sgi_40m = false, sgi_80m = false;

	sgi_20m = psta->htpriv.sgi_20m;
	sgi_40m = psta->htpriv.sgi_40m;

	switch(psta->bw_mode){
		case CHANNEL_WIDTH_80:
			sgi = sgi_80m;
			break;
		case CHANNEL_WIDTH_40:
			sgi = sgi_40m;
			break;
		case CHANNEL_WIDTH_20:
		default:
			sgi = sgi_20m;
			break;
	}

	return sgi;
}

static void update_attrib_vcs_info(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	u32	sz;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	//struct sta_info	*psta = pattrib->psta;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

/*
        if(pattrib->psta)
	{
		psta = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(&padapter->stapriv ,&pattrib->ra[0] );
	}

        if(psta==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return;
	}
*/

	if (pattrib->nr_frags != 1)
	{
		sz = padapter->xmitpriv.frag_len;
	}
	else //no frag
	{
		sz = pattrib->last_txcmdsz;
	}

	// (1) RTS_Threshold is compared to the MPDU, not MSDU.
	// (2) If there are more than one frag in  this MSDU, only the first frag uses protection frame.
	//		Other fragments are protected by previous fragment.
	//		So we only need to check the length of first fragment.
	if(pmlmeext->cur_wireless_mode < WIRELESS_11_24N  || padapter->registrypriv.wifi_spec)
	{
		if(sz > padapter->registrypriv.rts_thresh)
		{
			pattrib->vcs_mode = RTS_CTS;
		}
		else
		{
			if(pattrib->rtsen)
				pattrib->vcs_mode = RTS_CTS;
			else if(pattrib->cts2self)
				pattrib->vcs_mode = CTS_TO_SELF;
			else
				pattrib->vcs_mode = NONE_VCS;
		}
	}
	else
	{
		while (true)
		{
			//IOT action
			if((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_ATHEROS) && (pattrib->ampdu_en==true) &&
				(padapter->securitypriv.dot11PrivacyAlgrthm == _AES_ ))
			{
				pattrib->vcs_mode = CTS_TO_SELF;
				break;
			}	
			

			//check ERP protection
			if(pattrib->rtsen || pattrib->cts2self)
			{
				if(pattrib->rtsen)
					pattrib->vcs_mode = RTS_CTS;
				else if(pattrib->cts2self)
					pattrib->vcs_mode = CTS_TO_SELF;

				break;
			}

			//check HT op mode
			if(pattrib->ht_en)
			{
				u8 HTOpMode = pmlmeinfo->HT_protection;
				if((pmlmeext->cur_bwmode && (HTOpMode == 2 || HTOpMode == 3)) ||
					(!pmlmeext->cur_bwmode && HTOpMode == 3) )
				{
					pattrib->vcs_mode = RTS_CTS;
					break;
				}
			}

			//check rts
			if(sz > padapter->registrypriv.rts_thresh)
			{
				pattrib->vcs_mode = RTS_CTS;
				break;
			}

			//to do list: check MIMO power save condition.

			//check AMPDU aggregation for TXOP
			if(pattrib->ampdu_en==true)
			{
				pattrib->vcs_mode = RTS_CTS;
				break;
			}

			pattrib->vcs_mode = NONE_VCS;
			break;
		}
	}

	//for debug : force driver control vrtl_carrier_sense.
	if(padapter->driver_vcs_en==1)
	{
		//u8 driver_vcs_en; //Enable=1, Disable=0 driver control vrtl_carrier_sense.
		//u8 driver_vcs_type;//force 0:disable VCS, 1:RTS-CTS, 2:CTS-to-self when vcs_en=1.
		pattrib->vcs_mode = padapter->driver_vcs_type;
	}	
	
}

static void update_attrib_phy_info(_adapter *padapter, struct pkt_attrib *pattrib, struct sta_info *psta)
{
	struct mlme_ext_priv *mlmeext = &padapter->mlmeextpriv;

	pattrib->rtsen = psta->rtsen;
	pattrib->cts2self = psta->cts2self;
	
	pattrib->mdata = 0;
	pattrib->eosp = 0;
	pattrib->triggered=0;
	pattrib->ampdu_spacing = 0;
	
	//qos_en, ht_en, init rate, ,bw, ch_offset, sgi
	pattrib->qos_en = psta->qos_option;
	
	pattrib->raid = psta->raid;

	if (mlmeext->cur_bwmode < psta->bw_mode)
		pattrib->bwmode = mlmeext->cur_bwmode;
	else
		pattrib->bwmode = psta->bw_mode;

	pattrib->sgi = query_ra_short_GI(psta);

	pattrib->ldpc = psta->ldpc;
	pattrib->stbc = psta->stbc;

	pattrib->ht_en = psta->htpriv.ht_option;
	pattrib->ch_offset = psta->htpriv.ch_offset;
	pattrib->ampdu_en = false;

	if(padapter->driver_ampdu_spacing != 0xFF) //driver control AMPDU Density for peer sta's rx
		pattrib->ampdu_spacing = padapter->driver_ampdu_spacing;
	else
		pattrib->ampdu_spacing = psta->htpriv.rx_ampdu_min_spacing;

	//if(pattrib->ht_en && psta->htpriv.ampdu_enable)
	//{
	//	if(psta->htpriv.agg_enable_bitmap & BIT(pattrib->priority))
	//		pattrib->ampdu_en = true;
	//}	


	pattrib->retry_ctrl = false;

#ifdef CONFIG_AUTO_AP_MODE
	if(psta->isrc && psta->pid>0)
		pattrib->pctrl = true;
#endif

}

static s32 update_attrib_sec_info(_adapter *padapter, struct pkt_attrib *pattrib, struct sta_info *psta)
{
	sint res = _SUCCESS;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	sint bmcast = IS_MCAST(pattrib->ra);

	memset(pattrib->dot118021x_UncstKey.skey,  0, 16);		
	memset(pattrib->dot11tkiptxmickey.skey,  0, 16);
	pattrib->mac_id = psta->mac_id;

	if (psta->ieee8021x_blocked == true)
	{
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("\n psta->ieee8021x_blocked == true \n"));

		pattrib->encrypt = 0;

		if((pattrib->ether_type != 0x888e) && (check_fwstate(pmlmepriv, WIFI_MP_STATE) == false))
		{
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("\npsta->ieee8021x_blocked == true,  pattrib->ether_type(%.4x) != 0x888e\n",pattrib->ether_type));
			#ifdef DBG_TX_DROP_FRAME
			DBG_871X("DBG_TX_DROP_FRAME %s psta->ieee8021x_blocked == true,  pattrib->ether_type(%04x) != 0x888e\n", __FUNCTION__,pattrib->ether_type);
			#endif
			res = _FAIL;
			goto exit;
		}
	}
	else
	{
		GET_ENCRY_ALGO(psecuritypriv, psta, pattrib->encrypt, bmcast);
		
#ifdef CONFIG_WAPI_SUPPORT
		if(pattrib->ether_type == 0x88B4)
			pattrib->encrypt=_NO_PRIVACY_;
#endif

		switch(psecuritypriv->dot11AuthAlgrthm)
		{
			case dot11AuthAlgrthm_Open:
			case dot11AuthAlgrthm_Shared:
			case dot11AuthAlgrthm_Auto:
				pattrib->key_idx = (u8)psecuritypriv->dot11PrivacyKeyIndex;
				break;
			case dot11AuthAlgrthm_8021X:
				if(bmcast)
					pattrib->key_idx = (u8)psecuritypriv->dot118021XGrpKeyid;
				else
					pattrib->key_idx = 0;
				break;
			default:
				pattrib->key_idx = 0;
				break;
		}

		//For WPS 1.0 WEP, driver should not encrypt EAPOL Packet for WPS handshake.
		if (((pattrib->encrypt ==_WEP40_)||(pattrib->encrypt ==_WEP104_)) && (pattrib->ether_type == 0x888e))
			pattrib->encrypt=_NO_PRIVACY_;
		
	}
	
	switch (pattrib->encrypt)
	{
		case _WEP40_:
		case _WEP104_:
			pattrib->iv_len = 4;
			pattrib->icv_len = 4;
			WEP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
			break;

		case _TKIP_:
			pattrib->iv_len = 8;
			pattrib->icv_len = 4;

			if(psecuritypriv->busetkipkey==_FAIL)
			{				
				#ifdef DBG_TX_DROP_FRAME
				DBG_871X("DBG_TX_DROP_FRAME %s psecuritypriv->busetkipkey(%d)==_FAIL drop packet\n", __FUNCTION__, psecuritypriv->busetkipkey);
				#endif
				res =_FAIL;
				goto exit;
			}

			if(bmcast)
				TKIP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
			else
				TKIP_IV(pattrib->iv, psta->dot11txpn, 0);


			memcpy(pattrib->dot11tkiptxmickey.skey, psta->dot11tkiptxmickey.skey, 16);
			
			break;
			
		case _AES_:
			
			pattrib->iv_len = 8;
			pattrib->icv_len = 8;
			
			if(bmcast)
				AES_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
			else
				AES_IV(pattrib->iv, psta->dot11txpn, 0);
			
			break;

#ifdef CONFIG_WAPI_SUPPORT
		case _SMS4_:
			pattrib->iv_len = 18;
			pattrib->icv_len = 16;
			rtw_wapi_get_iv(padapter,pattrib->ra,pattrib->iv);			
			break;
#endif
		default:
			pattrib->iv_len = 0;
			pattrib->icv_len = 0;
			break;
	}

	if(pattrib->encrypt>0)
		memcpy(pattrib->dot118021x_UncstKey.skey, psta->dot118021x_UncstKey.skey, 16);		

	RT_TRACE(_module_rtl871x_xmit_c_, _drv_info_,
		("update_attrib: encrypt=%d  securitypriv.sw_encrypt=%d\n",
		pattrib->encrypt, padapter->securitypriv.sw_encrypt));

	if (pattrib->encrypt &&
		((padapter->securitypriv.sw_encrypt == true) || (psecuritypriv->hw_decrypted == false)))
	{
		pattrib->bswenc = true;
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,
			("update_attrib: encrypt=%d securitypriv.hw_decrypted=%d bswenc=true\n",
			pattrib->encrypt, padapter->securitypriv.sw_encrypt));
	} else {
		pattrib->bswenc = false;
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("update_attrib: bswenc=false\n"));
	}

#ifdef CONFIG_WAPI_SUPPORT
	if(pattrib->encrypt == _SMS4_)
		pattrib->bswenc = false;
#endif

exit:

	return res;
	
}

u8	qos_acm(u8 acm_mask, u8 priority)
{
	u8	change_priority = priority;

	switch (priority)
	{
		case 0:
		case 3:
			if(acm_mask & BIT(1))
				change_priority = 1;
			break;
		case 1:
		case 2:
			break;
		case 4:
		case 5:
			if(acm_mask & BIT(2))
				change_priority = 0;
			break;
		case 6:
		case 7:
			if(acm_mask & BIT(3))
				change_priority = 5;
			break;
		default:
			DBG_871X("qos_acm(): invalid pattrib->priority: %d!!!\n", priority);
			break;
	}

	return change_priority;
}

static void set_qos(struct pkt_file *ppktfile, struct pkt_attrib *pattrib)
{
	struct ethhdr etherhdr;
	struct iphdr ip_hdr;
	s32 UserPriority = 0;


	_rtw_open_pktfile(ppktfile->pkt, ppktfile);
	_rtw_pktfile_read(ppktfile, (unsigned char*)&etherhdr, ETH_HLEN);

	// get UserPriority from IP hdr
	if (pattrib->ether_type == 0x0800) {
		_rtw_pktfile_read(ppktfile, (u8*)&ip_hdr, sizeof(ip_hdr));
//		UserPriority = (ntohs(ip_hdr.tos) >> 5) & 0x3;
		UserPriority = ip_hdr.tos >> 5;
	}
/* 
	else if (pattrib->ether_type == 0x888e) {
		// "When priority processing of data frames is supported,
		// a STA's SME should send EAPOL-Key frames at the highest priority."
		UserPriority = 7;
	}
*/
	pattrib->priority = UserPriority;
	pattrib->hdrlen = WLAN_HDR_A3_QOS_LEN;
	pattrib->subtype = WIFI_QOS_DATA_TYPE;
}

static s32 update_attrib(_adapter *padapter, _pkt *pkt, struct pkt_attrib *pattrib)
{
	uint i;
	struct pkt_file pktfile;
	struct sta_info *psta = NULL;
	struct ethhdr etherhdr;

	sint bmcast;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct qos_priv		*pqospriv= &pmlmepriv->qospriv;
	sint res = _SUCCESS;

 _func_enter_;

	DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib);

	_rtw_open_pktfile(pkt, &pktfile);
	i = _rtw_pktfile_read(&pktfile, (u8*)&etherhdr, ETH_HLEN);

	pattrib->ether_type = ntohs(etherhdr.h_proto);


	memcpy(pattrib->dst, &etherhdr.h_dest, ETH_ALEN);
	memcpy(pattrib->src, &etherhdr.h_source, ETH_ALEN);


	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true)) {
		memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_adhoc);
	}
	else if (check_fwstate(pmlmepriv, WIFI_STATION_STATE)) {
		memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
		memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_sta);
	}
	else if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		memcpy(pattrib->ta, get_bssid(pmlmepriv), ETH_ALEN);
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_ap);
	} 
	else
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_unknown);

	pattrib->pktlen = pktfile.pkt_len;

	if (ETH_P_IP == pattrib->ether_type)
	{
		// The following is for DHCP and ARP packet, we use cck1M to tx these packets and let LPS awake some time 
		// to prevent DHCP protocol fail

		u8 tmp[24];
		
		_rtw_pktfile_read(&pktfile, &tmp[0], 24);

		pattrib->dhcp_pkt = 0;
		if (pktfile.pkt_len > 282) {//MINIMUM_DHCP_PACKET_SIZE) {
			if (ETH_P_IP == pattrib->ether_type) {// IP header
				if (((tmp[21] == 68) && (tmp[23] == 67)) ||
					((tmp[21] == 67) && (tmp[23] == 68))) {
					// 68 : UDP BOOTP client
					// 67 : UDP BOOTP server
					RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("======================update_attrib: get DHCP Packet \n"));
					// Use low rate to send DHCP packet.
					//if(pMgntInfo->IOTAction & HT_IOT_ACT_WA_IOT_Broadcom) 
					//{
					//	tcb_desc->DataRate = MgntQuery_TxRateExcludeCCKRates(ieee);//0xc;//ofdm 6m
					//	tcb_desc->bTxDisableRateFallBack = false;
					//}
					//else
					//	pTcb->DataRate = Adapter->MgntInfo.LowestBasicRate; 
					//RTPRINT(FDM, WA_IOT, ("DHCP TranslateHeader(), pTcb->DataRate = 0x%x\n", pTcb->DataRate)); 
					pattrib->dhcp_pkt = 1;
					DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_dhcp);
				}
			}
		}

		//for parsing ICMP pakcets
		{
			struct iphdr *piphdr = (struct iphdr *)tmp;

			pattrib->icmp_pkt = 0;	
			if(piphdr->protocol == 0x1) // protocol type in ip header 0x1 is ICMP			
			{
				pattrib->icmp_pkt = 1;
				DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_icmp);
			}
		}	

		
	} else if (0x888e == pattrib->ether_type) {
		DBG_871X_LEVEL(_drv_always_, "send eapol packet\n");
	}

	if ( (pattrib->ether_type == 0x888e) || (pattrib->dhcp_pkt == 1) )
	{
		rtw_set_scan_deny(padapter, 3000);
	}

	// If EAPOL , ARP , OR DHCP packet, driver must be in active mode.
#ifdef CONFIG_WAPI_SUPPORT
	if ( (pattrib->ether_type == 0x88B4) || (pattrib->ether_type == 0x0806) || (pattrib->ether_type == 0x888e) || (pattrib->dhcp_pkt == 1) )
#else //!CONFIG_WAPI_SUPPORT
	if (pattrib->icmp_pkt==1)
	{
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_LEAVE, 1);
	}
	else if(pattrib->dhcp_pkt==1)
#endif
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_active);
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_SPECIAL_PACKET, 1);
	}

	bmcast = IS_MCAST(pattrib->ra);
	
	// get sta_info
	if (bmcast) {
		psta = rtw_get_bcmc_stainfo(padapter);
	} else {
		psta = rtw_get_stainfo(pstapriv, pattrib->ra);
		if (psta == NULL)	{ // if we cannot get psta => drop the pkt
			DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_err_ucast_sta);
			RT_TRACE(_module_rtl871x_xmit_c_, _drv_alert_, ("\nupdate_attrib => get sta_info fail, ra:" MAC_FMT"\n", MAC_ARG(pattrib->ra)));
			#ifdef DBG_TX_DROP_FRAME
			DBG_871X("DBG_TX_DROP_FRAME %s get sta_info fail, ra:" MAC_FMT"\n", __FUNCTION__, MAC_ARG(pattrib->ra));
			#endif
			res =_FAIL;
			goto exit;
		}
		else if((check_fwstate(pmlmepriv, WIFI_AP_STATE)==true)&&(!(psta->state & _FW_LINKED)))
		{
			DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_err_ucast_ap_link);
			res =_FAIL;
			goto exit;
		}
	}

	if(psta == NULL)
	{		// if we cannot get psta => drop the pkt
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_err_sta);
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_alert_, ("\nupdate_attrib => get sta_info fail, ra:" MAC_FMT "\n", MAC_ARG(pattrib->ra)));
		#ifdef DBG_TX_DROP_FRAME
		DBG_871X("DBG_TX_DROP_FRAME %s get sta_info fail, ra:" MAC_FMT"\n", __FUNCTION__, MAC_ARG(pattrib->ra));
		#endif
		res = _FAIL;
		goto exit;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_err_link);
		DBG_871X("%s, psta("MAC_FMT")->state(0x%x) != _FW_LINKED\n", __func__, MAC_ARG(psta->hwaddr), psta->state);
		return _FAIL;
	}



	//TODO:_lock
	if(update_attrib_sec_info(padapter, pattrib, psta) == _FAIL)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_upd_attrib_err_sec);
		res = _FAIL;
		goto exit;
	}

	update_attrib_phy_info(padapter, pattrib, psta);

	//DBG_8192C("%s ==> mac_id(%d)\n",__FUNCTION__,pattrib->mac_id );
	
	pattrib->psta = psta;
	//TODO:_unlock
	
	pattrib->pctrl = 0;	
		
	pattrib->ack_policy = 0;
	// get ether_hdr_len
	pattrib->pkt_hdrlen = ETH_HLEN;//(pattrib->ether_type == 0x8100) ? (14 + 4 ): 14; //vlan tag

	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA_TYPE;	
	pattrib->priority = 0;
	
	if (check_fwstate(pmlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE))
	{
		if(pattrib->qos_en)
			set_qos(&pktfile, pattrib);
	}
	else
	{
		if(pqospriv->qos_option)
		{
			set_qos(&pktfile, pattrib);

			if(pmlmepriv->acm_mask != 0)
			{
				pattrib->priority = qos_acm(pmlmepriv->acm_mask, pattrib->priority);
			}
		}
	}

	//pattrib->priority = 5; //force to used VI queue, for testing

	rtw_set_tx_chksum_offload(pkt, pattrib);

exit:

_func_exit_;

	return res;
}

static s32 xmitframe_addmic(_adapter *padapter, struct xmit_frame *pxmitframe){
	sint 			curfragnum,length;
	u8	*pframe, *payload,mic[8];
	struct	mic_data		micdata;
	//struct	sta_info		*stainfo;
	struct	qos_priv   *pqospriv= &(padapter->mlmepriv.qospriv);	
	struct	pkt_attrib	 *pattrib = &pxmitframe->attrib;
	struct 	security_priv	*psecuritypriv=&padapter->securitypriv;
	struct	xmit_priv		*pxmitpriv=&padapter->xmitpriv;
	u8 priority[4]={0x0,0x0,0x0,0x0};
	u8 hw_hdr_offset = 0;
	sint bmcst = IS_MCAST(pattrib->ra);

/*
	if(pattrib->psta)
	{
		stainfo = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		stainfo=rtw_get_stainfo(&padapter->stapriv ,&pattrib->ra[0]);
	}	
	
	if(stainfo==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}

	if(!(stainfo->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, stainfo->state);
		return _FAIL;
	}
*/

_func_enter_;

	hw_hdr_offset = TXDESC_OFFSET;
	
	if(pattrib->encrypt ==_TKIP_)//if(psecuritypriv->dot11PrivacyAlgrthm==_TKIP_PRIVACY_) 
	{
		//encode mic code
		//if(stainfo!= NULL)
		{
			u8 null_key[16]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

			pframe = pxmitframe->buf_addr + hw_hdr_offset;
			
			if(bmcst)
			{
				if(!memcmp(psecuritypriv->dot118021XGrptxmickey[psecuritypriv->dot118021XGrpKeyid].skey, null_key, 16)){
					//DbgPrint("\nxmitframe_addmic:stainfo->dot11tkiptxmickey==0\n");
					//msleep(10);
					return _FAIL;
				}				
				//start to calculate the mic code
				rtw_secmicsetkey(&micdata, psecuritypriv->dot118021XGrptxmickey[psecuritypriv->dot118021XGrpKeyid].skey);
			}
			else
			{
				if(!memcmp(&pattrib->dot11tkiptxmickey.skey[0],null_key, 16)){
					//DbgPrint("\nxmitframe_addmic:stainfo->dot11tkiptxmickey==0\n");
					//msleep(10);
					return _FAIL;
				}
				//start to calculate the mic code
				rtw_secmicsetkey(&micdata, &pattrib->dot11tkiptxmickey.skey[0]);
			}
			
			if(pframe[1]&1){   //ToDS==1
				rtw_secmicappend(&micdata, &pframe[16], 6);  //DA
				if(pframe[1]&2)  //From Ds==1
					rtw_secmicappend(&micdata, &pframe[24], 6);
				else
				rtw_secmicappend(&micdata, &pframe[10], 6);		
			}	
			else{	//ToDS==0
				rtw_secmicappend(&micdata, &pframe[4], 6);   //DA
				if(pframe[1]&2)  //From Ds==1
					rtw_secmicappend(&micdata, &pframe[16], 6);
				else
					rtw_secmicappend(&micdata, &pframe[10], 6);

			}

                    //if(pqospriv->qos_option==1)
                    if(pattrib->qos_en)
				priority[0]=(u8)pxmitframe->attrib.priority;

			
			rtw_secmicappend(&micdata, &priority[0], 4);
	
			payload=pframe;

			for(curfragnum=0;curfragnum<pattrib->nr_frags;curfragnum++){
				payload=(u8 *)RND4((SIZE_PTR)(payload));
				RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("===curfragnum=%d, pframe= 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,!!!\n",
					curfragnum,*payload, *(payload+1),*(payload+2),*(payload+3),*(payload+4),*(payload+5),*(payload+6),*(payload+7)));

				payload=payload+pattrib->hdrlen+pattrib->iv_len;
				RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("curfragnum=%d pattrib->hdrlen=%d pattrib->iv_len=%d",curfragnum,pattrib->hdrlen,pattrib->iv_len));
				if((curfragnum+1)==pattrib->nr_frags){
					length=pattrib->last_txcmdsz-pattrib->hdrlen-pattrib->iv_len-( (pattrib->bswenc) ? pattrib->icv_len : 0);
					rtw_secmicappend(&micdata, payload,length);
					payload=payload+length;
				}
				else{
					length=pxmitpriv->frag_len-pattrib->hdrlen-pattrib->iv_len-( (pattrib->bswenc) ? pattrib->icv_len : 0);
					rtw_secmicappend(&micdata, payload, length);
					payload=payload+length+pattrib->icv_len;
					RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("curfragnum=%d length=%d pattrib->icv_len=%d",curfragnum,length,pattrib->icv_len));
				}
			}
			rtw_secgetmic(&micdata,&(mic[0]));
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("xmitframe_addmic: before add mic code!!!\n"));
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("xmitframe_addmic: pattrib->last_txcmdsz=%d!!!\n",pattrib->last_txcmdsz));
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("xmitframe_addmic: mic[0]=0x%.2x ,mic[1]=0x%.2x ,mic[2]=0x%.2x ,mic[3]=0x%.2x \n\
  mic[4]=0x%.2x ,mic[5]=0x%.2x ,mic[6]=0x%.2x ,mic[7]=0x%.2x !!!!\n",
				mic[0],mic[1],mic[2],mic[3],mic[4],mic[5],mic[6],mic[7]));
			//add mic code  and add the mic code length in last_txcmdsz

			memcpy(payload, &(mic[0]),8);
			pattrib->last_txcmdsz+=8;
			
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("\n ========last pkt========\n"));
			payload=payload-pattrib->last_txcmdsz+8;
			for(curfragnum=0;curfragnum<pattrib->last_txcmdsz;curfragnum=curfragnum+8)
					RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,(" %.2x,  %.2x,  %.2x,  %.2x,  %.2x,  %.2x,  %.2x,  %.2x ",
					*(payload+curfragnum), *(payload+curfragnum+1), *(payload+curfragnum+2),*(payload+curfragnum+3),
					*(payload+curfragnum+4),*(payload+curfragnum+5),*(payload+curfragnum+6),*(payload+curfragnum+7)));
			}
/*
			else{
				RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("xmitframe_addmic: rtw_get_stainfo==NULL!!!\n"));
			}
*/		
	}
	
_func_exit_;

	return _SUCCESS;
}

static s32 xmitframe_swencrypt(_adapter *padapter, struct xmit_frame *pxmitframe){

	struct	pkt_attrib	 *pattrib = &pxmitframe->attrib;
	//struct 	security_priv	*psecuritypriv=&padapter->securitypriv;
	
_func_enter_;

	//if((psecuritypriv->sw_encrypt)||(pattrib->bswenc))	
	if(pattrib->bswenc)
	{
		//DBG_871X("start xmitframe_swencrypt\n");
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_alert_,("### xmitframe_swencrypt\n"));
		switch(pattrib->encrypt){
		case _WEP40_:
		case _WEP104_:
			rtw_wep_encrypt(padapter, (u8 *)pxmitframe);
			break;
		case _TKIP_:
			rtw_tkip_encrypt(padapter, (u8 *)pxmitframe);
			break;
		case _AES_:
			rtw_aes_encrypt(padapter, (u8 * )pxmitframe);
			break;
#ifdef CONFIG_WAPI_SUPPORT
		case _SMS4_:
			rtw_sms4_encrypt(padapter, (u8 * )pxmitframe);
#endif
		default:
				break;
		}

	} else {
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_notice_,("### xmitframe_hwencrypt\n"));
	}

_func_exit_;

	return _SUCCESS;
}

s32 rtw_make_wlanhdr (_adapter *padapter , u8 *hdr, struct pkt_attrib *pattrib)
{
	u16 *qc;

	struct rtw_ieee80211_hdr *pwlanhdr = (struct rtw_ieee80211_hdr *)hdr;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct qos_priv *pqospriv = &pmlmepriv->qospriv;
	u8 qos_option = false;
	sint res = _SUCCESS;
	u16 *fctrl = &pwlanhdr->frame_ctl;

	//struct sta_info *psta;

	//sint bmcst = IS_MCAST(pattrib->ra);

_func_enter_;

/*
	psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
	if(pattrib->psta != psta)
	{
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return;
	}

	if(psta==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}
*/

	memset(hdr, 0, WLANHDR_OFFSET);

	SetFrameSubType(fctrl, pattrib->subtype);

	if (pattrib->subtype & WIFI_DATA_TYPE)
	{
		if ((check_fwstate(pmlmepriv,  WIFI_STATION_STATE) == true)) {
			//to_ds = 1, fr_ds = 0;

			{
				// 1.Data transfer to AP
				// 2.Arp pkt will relayed by AP
				SetToDs(fctrl);							
				memcpy(pwlanhdr->addr1, get_bssid(pmlmepriv), ETH_ALEN);
				memcpy(pwlanhdr->addr2, pattrib->src, ETH_ALEN);
				memcpy(pwlanhdr->addr3, pattrib->dst, ETH_ALEN);
			} 

			if (pqospriv->qos_option)
				qos_option = true;

		}
		else if ((check_fwstate(pmlmepriv,  WIFI_AP_STATE) == true) ) {
			//to_ds = 0, fr_ds = 1;
			SetFrDs(fctrl);
			memcpy(pwlanhdr->addr1, pattrib->dst, ETH_ALEN);
			memcpy(pwlanhdr->addr2, get_bssid(pmlmepriv), ETH_ALEN);
			memcpy(pwlanhdr->addr3, pattrib->src, ETH_ALEN);

			if(pattrib->qos_en)
				qos_option = true;
		}
		else if ((check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true)) {
			memcpy(pwlanhdr->addr1, pattrib->dst, ETH_ALEN);
			memcpy(pwlanhdr->addr2, pattrib->src, ETH_ALEN);
			memcpy(pwlanhdr->addr3, get_bssid(pmlmepriv), ETH_ALEN);

			if(pattrib->qos_en)
				qos_option = true;
		}
		else {
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("fw_state:%x is not allowed to xmit frame\n", get_fwstate(pmlmepriv)));
			res = _FAIL;
			goto exit;
		}

		if(pattrib->mdata)
			SetMData(fctrl);

		if (pattrib->encrypt)
			SetPrivacy(fctrl);

		if (qos_option)
		{
			qc = (unsigned short *)(hdr + pattrib->hdrlen - 2);

			if (pattrib->priority)
				SetPriority(qc, pattrib->priority);

			SetEOSP(qc, pattrib->eosp);

			SetAckpolicy(qc, pattrib->ack_policy);
		}

		//TODO: fill HT Control Field

		//Update Seq Num will be handled by f/w
		{
			struct sta_info *psta;
			psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
			if(pattrib->psta != psta)
			{
				DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
				return _FAIL;
			}

			if(psta==NULL)
			{
				DBG_871X("%s, psta==NUL\n", __func__);
				return _FAIL;
			}

			if(!(psta->state &_FW_LINKED))
			{
				DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
				return _FAIL;
			}

		
			if(psta)
			{
				psta->sta_xmitpriv.txseq_tid[pattrib->priority]++;
				psta->sta_xmitpriv.txseq_tid[pattrib->priority] &= 0xFFF;
				pattrib->seqnum = psta->sta_xmitpriv.txseq_tid[pattrib->priority];

				SetSeqNum(hdr, pattrib->seqnum);

				//check if enable ampdu
				if(pattrib->ht_en && psta->htpriv.ampdu_enable)
				{
					if(psta->htpriv.agg_enable_bitmap & BIT(pattrib->priority))
						pattrib->ampdu_en = true;
				}

				//re-check if enable ampdu by BA_starting_seqctrl
				if(pattrib->ampdu_en == true)
				{					
					u16 tx_seq;

					tx_seq = psta->BA_starting_seqctrl[pattrib->priority & 0x0f];
		
					//check BA_starting_seqctrl
					if(SN_LESS(pattrib->seqnum, tx_seq))
					{
						//DBG_871X("tx ampdu seqnum(%d) < tx_seq(%d)\n", pattrib->seqnum, tx_seq);
						pattrib->ampdu_en = false;//AGG BK
					}
					else if(SN_EQUAL(pattrib->seqnum, tx_seq))
					{					
						psta->BA_starting_seqctrl[pattrib->priority & 0x0f] = (tx_seq+1)&0xfff;
					
						pattrib->ampdu_en = true;//AGG EN
					}
					else
					{
						//DBG_871X("tx ampdu over run\n");
						psta->BA_starting_seqctrl[pattrib->priority & 0x0f] = (pattrib->seqnum+1)&0xfff;
						pattrib->ampdu_en = true;//AGG EN
					}

				}
			}
		}
		
	}
	else
	{

	}

exit:

_func_exit_;

	return res;
}

s32 rtw_txframes_pending(_adapter *padapter)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	return ((_rtw_queue_empty(&pxmitpriv->be_pending) == false) || 
			 (_rtw_queue_empty(&pxmitpriv->bk_pending) == false) || 
			 (_rtw_queue_empty(&pxmitpriv->vi_pending) == false) ||
			 (_rtw_queue_empty(&pxmitpriv->vo_pending) == false));
}

s32 rtw_txframes_sta_ac_pending(_adapter *padapter, struct pkt_attrib *pattrib)
{	
	struct sta_info *psta;
	struct tx_servq *ptxservq;
	int priority = pattrib->priority;
/*
	if(pattrib->psta)
	{
		psta = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(&padapter->stapriv ,&pattrib->ra[0]);
	}	
*/
	psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
	if(pattrib->psta != psta)
	{
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return 0;
	}

	if(psta==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return 0;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return 0;
	}
	
	switch(priority) 
	{
			case 1:
			case 2:
				ptxservq = &(psta->sta_xmitpriv.bk_q);				
				break;
			case 4:
			case 5:
				ptxservq = &(psta->sta_xmitpriv.vi_q);				
				break;
			case 6:
			case 7:
				ptxservq = &(psta->sta_xmitpriv.vo_q);							
				break;
			case 0:
			case 3:
			default:
				ptxservq = &(psta->sta_xmitpriv.be_q);							
			break;
	
	}	

	return ptxservq->qcnt;	
}

/*
 * Calculate wlan 802.11 packet MAX size from pkt_attrib
 * This function doesn't consider fragment case
 */
u32 rtw_calculate_wlan_pkt_size_by_attribue(struct pkt_attrib *pattrib)
{
	u32	len = 0;

	len = pattrib->hdrlen + pattrib->iv_len; // WLAN Header and IV
	len += SNAP_SIZE + sizeof(u16); // LLC
	len += pattrib->pktlen;
	if (pattrib->encrypt == _TKIP_) len += 8; // MIC
	len += ((pattrib->bswenc) ? pattrib->icv_len : 0); // ICV

	return len;
}

/*

This sub-routine will perform all the following:

1. remove 802.3 header.
2. create wlan_header, based on the info in pxmitframe
3. append sta's iv/ext-iv
4. append LLC
5. move frag chunk from pframe to pxmitframe->mem
6. apply sw-encrypt, if necessary. 

*/
s32 rtw_xmitframe_coalesce(_adapter *padapter, _pkt *pkt, struct xmit_frame *pxmitframe)
{
	struct pkt_file pktfile;

	s32 frg_inx, frg_len, mpdu_len, llc_sz, mem_sz;

	SIZE_PTR addr;

	u8 *pframe, *mem_start;
	u8 hw_hdr_offset;

	//struct sta_info		*psta;
	//struct sta_priv		*pstapriv = &padapter->stapriv;
	//struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;

	u8 *pbuf_start;

	s32 bmcst = IS_MCAST(pattrib->ra);
	s32 res = _SUCCESS;

_func_enter_;

/*
	if (pattrib->psta)
	{
		psta = pattrib->psta;
	} else
	{	
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
	}

	if(psta==NULL)
        {
		
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}


	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}
*/
	if (pxmitframe->buf_addr == NULL){
		DBG_8192C("==> %s buf_addr==NULL \n",__FUNCTION__);
		return _FAIL;
	}

	pbuf_start = pxmitframe->buf_addr;
	
	hw_hdr_offset = TXDESC_OFFSET;
	mem_start = pbuf_start +	hw_hdr_offset;

	if (rtw_make_wlanhdr(padapter, mem_start, pattrib) == _FAIL) {
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("rtw_xmitframe_coalesce: rtw_make_wlanhdr fail; drop pkt\n"));
		DBG_8192C("rtw_xmitframe_coalesce: rtw_make_wlanhdr fail; drop pkt\n");
		res = _FAIL;
		goto exit;
	}

	_rtw_open_pktfile(pkt, &pktfile);
	_rtw_pktfile_read(&pktfile, NULL, pattrib->pkt_hdrlen);

	frg_inx = 0;
	frg_len = pxmitpriv->frag_len - 4;//2346-4 = 2342

	while (1)
	{
		llc_sz = 0;

		mpdu_len = frg_len;

		pframe = mem_start;

		SetMFrag(mem_start);

		pframe += pattrib->hdrlen;
		mpdu_len -= pattrib->hdrlen;

		//adding icv, if necessary...
		if (pattrib->iv_len)
		{
/*		
			//if (check_fwstate(pmlmepriv, WIFI_MP_STATE))
			//	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			//else
			//	psta = rtw_get_stainfo(pstapriv, pattrib->ra);

			if (psta != NULL)
			{
				switch(pattrib->encrypt)
				{
					case _WEP40_:
					case _WEP104_:
							WEP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);	
						break;
					case _TKIP_:			
						if(bmcst)
							TKIP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
						else
							TKIP_IV(pattrib->iv, psta->dot11txpn, 0);
						break;			
					case _AES_:
						if(bmcst)
							AES_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
						else
							AES_IV(pattrib->iv, psta->dot11txpn, 0);
						break;
#ifdef CONFIG_WAPI_SUPPORT
					case _SMS4_:
						rtw_wapi_get_iv(padapter,pattrib->ra,pattrib->iv);
						break;
#endif
				}
			}
*/
			memcpy(pframe, pattrib->iv, pattrib->iv_len);

			RT_TRACE(_module_rtl871x_xmit_c_, _drv_notice_,
				 ("rtw_xmitframe_coalesce: keyid=%d pattrib->iv[3]=%.2x pframe=%.2x %.2x %.2x %.2x\n",
				  padapter->securitypriv.dot11PrivacyKeyIndex, pattrib->iv[3], *pframe, *(pframe+1), *(pframe+2), *(pframe+3)));

			pframe += pattrib->iv_len;

			mpdu_len -= pattrib->iv_len;
		}

		if (frg_inx == 0) {
			llc_sz = rtw_put_snap(pframe, pattrib->ether_type);
			pframe += llc_sz;
			mpdu_len -= llc_sz;
		}

		if ((pattrib->icv_len >0) && (pattrib->bswenc)) {
			mpdu_len -= pattrib->icv_len;
		}


		if (bmcst) {
			// don't do fragment to broadcat/multicast packets
			mem_sz = _rtw_pktfile_read(&pktfile, pframe, pattrib->pktlen);
		} else {
			mem_sz = _rtw_pktfile_read(&pktfile, pframe, mpdu_len);
		}

		pframe += mem_sz;

		if ((pattrib->icv_len >0 )&& (pattrib->bswenc)) {
			memcpy(pframe, pattrib->icv, pattrib->icv_len); 
			pframe += pattrib->icv_len;
		}

		frg_inx++;

		if (bmcst || (rtw_endofpktfile(&pktfile) == true))
		{
			pattrib->nr_frags = frg_inx;

			pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->iv_len + ((pattrib->nr_frags==1)? llc_sz:0) + 
					((pattrib->bswenc) ? pattrib->icv_len : 0) + mem_sz;
			
			ClearMFrag(mem_start);

			break;
		} else {
			RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("%s: There're still something in packet!\n", __FUNCTION__));
		}

		addr = (SIZE_PTR)(pframe);

		mem_start = (unsigned char *)RND4(addr) + hw_hdr_offset;
		memcpy(mem_start, pbuf_start + hw_hdr_offset, pattrib->hdrlen);	
		
	}

	if (xmitframe_addmic(padapter, pxmitframe) == _FAIL)
	{
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("xmitframe_addmic(padapter, pxmitframe)==_FAIL\n"));
		DBG_8192C("xmitframe_addmic(padapter, pxmitframe)==_FAIL\n");
		res = _FAIL;
		goto exit;
	}

	xmitframe_swencrypt(padapter, pxmitframe);

	if(bmcst == false)
		update_attrib_vcs_info(padapter, pxmitframe);
	else
		pattrib->vcs_mode = NONE_VCS;

exit:	
	
_func_exit_;	

	return res;
}

/* Logical Link Control(LLC) SubNetwork Attachment Point(SNAP) header
 * IEEE LLC/SNAP header contains 8 octets
 * First 3 octets comprise the LLC portion
 * SNAP portion, 5 octets, is divided into two fields:
 *	Organizationally Unique Identifier(OUI), 3 octets,
 *	type, defined by that organization, 2 octets.
 */
s32 rtw_put_snap(u8 *data, u16 h_proto)
{
	struct ieee80211_snap_hdr *snap;
	u8 *oui;

_func_enter_;

	snap = (struct ieee80211_snap_hdr *)data;
	snap->dsap = 0xaa;
	snap->ssap = 0xaa;
	snap->ctrl = 0x03;

	if (h_proto == 0x8137 || h_proto == 0x80f3)
		oui = P802_1H_OUI;
	else
		oui = RFC1042_OUI;
	
	snap->oui[0] = oui[0];
	snap->oui[1] = oui[1];
	snap->oui[2] = oui[2];

	*(u16 *)(data + SNAP_SIZE) = htons(h_proto);

_func_exit_;

	return SNAP_SIZE + sizeof(u16);
}

void rtw_update_protection(_adapter *padapter, u8 *ie, uint ie_len)
{

	uint	protection;
	u8	*perp;
	sint	 erp_len;
	struct	xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct	registry_priv *pregistrypriv = &padapter->registrypriv;
	
_func_enter_;
	
	switch(pxmitpriv->vcs_setting)
	{
		case DISABLE_VCS:
			pxmitpriv->vcs = NONE_VCS;
			break;
	
		case ENABLE_VCS:
			break;
	
		case AUTO_VCS:
		default:
			perp = rtw_get_ie(ie, _ERPINFO_IE_, &erp_len, ie_len);
			if(perp == NULL)
			{
				pxmitpriv->vcs = NONE_VCS;
			}
			else
			{
				protection = (*(perp + 2)) & BIT(1);
				if (protection)
				{
					if(pregistrypriv->vcs_type == RTS_CTS)
						pxmitpriv->vcs = RTS_CTS;
					else
						pxmitpriv->vcs = CTS_TO_SELF;
				}
				else
					pxmitpriv->vcs = NONE_VCS;
			}

			break;			
	
	}

_func_exit_;

}

void rtw_count_tx_stats(PADAPTER padapter, struct xmit_frame *pxmitframe, int sz)
{
	struct sta_info *psta = NULL;
	struct stainfo_stats *pstats = NULL;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	u8	pkt_num = 1;

	if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
		pkt_num = pxmitframe->agg_num;

		pmlmepriv->LinkDetectInfo.NumTxOkInPeriod += pkt_num;

		pxmitpriv->tx_pkts += pkt_num;

		pxmitpriv->tx_bytes += sz;

		psta = pxmitframe->attrib.psta;
		if (psta)
		{
			pstats = &psta->sta_stats;

			pstats->tx_pkts += pkt_num;

			pstats->tx_bytes += sz;
		}		
	}
}

static struct xmit_buf *__rtw_alloc_cmd_xmitbuf(struct xmit_priv *pxmitpriv,
		enum cmdbuf_type buf_type)
{
	struct xmit_buf *pxmitbuf =  NULL;

_func_enter_;

	pxmitbuf = &pxmitpriv->pcmd_xmitbuf[buf_type];
	if (pxmitbuf !=  NULL) {
		pxmitbuf->priv_data = NULL;

		pxmitbuf->len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
		pxmitbuf->agg_num = 0;
		pxmitbuf->pg_num = 0;

		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}
	} else {
		DBG_871X("%s fail, no xmitbuf available !!!\n", __func__);
	}

_func_exit_;

	return pxmitbuf;
}

struct xmit_frame *__rtw_alloc_cmdxmitframe(struct xmit_priv *pxmitpriv,
		enum cmdbuf_type buf_type)
{
	struct xmit_frame		*pcmdframe;
	struct xmit_buf		*pxmitbuf;

	if ((pcmdframe = rtw_alloc_xmitframe(pxmitpriv)) == NULL)
	{
		DBG_871X("%s, alloc xmitframe fail\n", __FUNCTION__);
		return NULL;
	}

	if ((pxmitbuf = __rtw_alloc_cmd_xmitbuf(pxmitpriv, buf_type)) == NULL) {
		DBG_871X("%s, alloc xmitbuf fail\n", __FUNCTION__);
		rtw_free_xmitframe(pxmitpriv, pcmdframe);
		return NULL;
	}

	pcmdframe->frame_tag = MGNT_FRAMETAG;

	pcmdframe->pxmitbuf = pxmitbuf;

	pcmdframe->buf_addr = pxmitbuf->pbuf;

	pxmitbuf->priv_data = pcmdframe;

	return pcmdframe;

}

struct xmit_buf *rtw_alloc_xmitbuf_ext(struct xmit_priv *pxmitpriv)
{
	_irqL irqL;
	struct xmit_buf *pxmitbuf =  NULL;
	_list *plist, *phead;
	_queue *pfree_queue = &pxmitpriv->free_xmit_extbuf_queue;

_func_enter_;

	_enter_critical(&pfree_queue->lock, &irqL);

	if(_rtw_queue_empty(pfree_queue) == true) {
		pxmitbuf = NULL;
	} else {

		phead = get_list_head(pfree_queue);

		plist = get_next(phead);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		rtw_list_delete(&(pxmitbuf->list));
	}

	if (pxmitbuf !=  NULL)
	{
		pxmitpriv->free_xmit_extbuf_cnt--;
		#ifdef DBG_XMIT_BUF_EXT
		DBG_871X("DBG_XMIT_BUF_EXT ALLOC no=%d,  free_xmit_extbuf_cnt=%d\n",pxmitbuf->no, pxmitpriv->free_xmit_extbuf_cnt);
		#endif
		
	
		pxmitbuf->priv_data = NULL;

		pxmitbuf->len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
		pxmitbuf->agg_num = 1;

		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}

	}

	_exit_critical(&pfree_queue->lock, &irqL);

_func_exit_;

	return pxmitbuf;
}

s32 rtw_free_xmitbuf_ext(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	_irqL irqL;
	_queue *pfree_queue = &pxmitpriv->free_xmit_extbuf_queue;

_func_enter_;

	if(pxmitbuf==NULL)
	{
		return _FAIL;
	}

	_enter_critical(&pfree_queue->lock, &irqL);

	rtw_list_delete(&pxmitbuf->list);

	rtw_list_insert_tail(&(pxmitbuf->list), get_list_head(pfree_queue));
	pxmitpriv->free_xmit_extbuf_cnt++;
	#ifdef DBG_XMIT_BUF_EXT
	DBG_871X("DBG_XMIT_BUF_EXT FREE no=%d, free_xmit_extbuf_cnt=%d\n",pxmitbuf->no ,pxmitpriv->free_xmit_extbuf_cnt);
	#endif

	_exit_critical(&pfree_queue->lock, &irqL);

_func_exit_;

	return _SUCCESS;
} 

struct xmit_buf *rtw_alloc_xmitbuf(struct xmit_priv *pxmitpriv)
{
	_irqL irqL;
	struct xmit_buf *pxmitbuf =  NULL;
	_list *plist, *phead;
	_queue *pfree_xmitbuf_queue = &pxmitpriv->free_xmitbuf_queue;

_func_enter_;

	//DBG_871X("+rtw_alloc_xmitbuf\n");

	_enter_critical(&pfree_xmitbuf_queue->lock, &irqL);

	if(_rtw_queue_empty(pfree_xmitbuf_queue) == true) {
		pxmitbuf = NULL;
	} else {

		phead = get_list_head(pfree_xmitbuf_queue);

		plist = get_next(phead);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		rtw_list_delete(&(pxmitbuf->list));
	}

	if (pxmitbuf !=  NULL)
	{
		pxmitpriv->free_xmitbuf_cnt--;
		#ifdef DBG_XMIT_BUF
		DBG_871X("DBG_XMIT_BUF ALLOC no=%d,  free_xmitbuf_cnt=%d\n",pxmitbuf->no, pxmitpriv->free_xmitbuf_cnt);
		#endif
		//DBG_871X("alloc, free_xmitbuf_cnt=%d\n", pxmitpriv->free_xmitbuf_cnt);

		pxmitbuf->priv_data = NULL;

		pxmitbuf->len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
		pxmitbuf->agg_num = 0;
		pxmitbuf->pg_num = 0;

		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}
	}
	#ifdef DBG_XMIT_BUF
	else
	{
		DBG_871X("DBG_XMIT_BUF rtw_alloc_xmitbuf return NULL\n");
	}
	#endif

	_exit_critical(&pfree_xmitbuf_queue->lock, &irqL);

_func_exit_;

	return pxmitbuf;
}

s32 rtw_free_xmitbuf(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	_irqL irqL;
	_queue *pfree_xmitbuf_queue = &pxmitpriv->free_xmitbuf_queue;

_func_enter_;

	//DBG_871X("+rtw_free_xmitbuf\n");

	if(pxmitbuf==NULL)
	{
		return _FAIL;
	}

	if (pxmitbuf->sctx) {
		DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_FREE);
	}

	if(pxmitbuf->buf_tag == XMITBUF_CMD) {
	}
	else if(pxmitbuf->buf_tag == XMITBUF_MGNT) {
		rtw_free_xmitbuf_ext(pxmitpriv, pxmitbuf);
	}
	else
	{
		_enter_critical(&pfree_xmitbuf_queue->lock, &irqL);

		rtw_list_delete(&pxmitbuf->list);	

		rtw_list_insert_tail(&(pxmitbuf->list), get_list_head(pfree_xmitbuf_queue));

		pxmitpriv->free_xmitbuf_cnt++;
		//DBG_871X("FREE, free_xmitbuf_cnt=%d\n", pxmitpriv->free_xmitbuf_cnt);
		#ifdef DBG_XMIT_BUF
		DBG_871X("DBG_XMIT_BUF FREE no=%d, free_xmitbuf_cnt=%d\n",pxmitbuf->no ,pxmitpriv->free_xmitbuf_cnt);
		#endif
		_exit_critical(&pfree_xmitbuf_queue->lock, &irqL);
	}

_func_exit_;	 

	return _SUCCESS;	
} 

static void rtw_init_xmitframe(struct xmit_frame *pxframe)
{
	if (pxframe !=  NULL)//default value setting
	{
		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;

		memset(&pxframe->attrib, 0, sizeof(struct pkt_attrib));
		//pxframe->attrib.psta = NULL;

		pxframe->frame_tag = DATA_FRAMETAG;

		pxframe->pg_num = 1;
		pxframe->agg_num = 1;
		pxframe->ack_report = 0;
	}
}

/*
Calling context:
1. OS_TXENTRY
2. RXENTRY (rx_thread or RX_ISR/RX_CallBack)

If we turn on USE_RXTHREAD, then, no need for critical section.
Otherwise, we must use _enter/_exit critical to protect free_xmit_queue...

Must be very very cautious...

*/
struct xmit_frame *rtw_alloc_xmitframe(struct xmit_priv *pxmitpriv)//(_queue *pfree_xmit_queue)
{
	/*
		Please remember to use all the osdep_service api,
		and lock/unlock or _enter/_exit critical to protect 
		pfree_xmit_queue
	*/

	_irqL irqL;
	struct xmit_frame *pxframe = NULL;
	_list *plist, *phead;
	_queue *pfree_xmit_queue = &pxmitpriv->free_xmit_queue;

_func_enter_;

	_enter_critical_bh(&pfree_xmit_queue->lock, &irqL);

	if (_rtw_queue_empty(pfree_xmit_queue) == true) {
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_alloc_xmitframe:%d\n", pxmitpriv->free_xmitframe_cnt));
		pxframe =  NULL;
	} else {
		phead = get_list_head(pfree_xmit_queue);

		plist = get_next(phead);

		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		rtw_list_delete(&(pxframe->list));
		pxmitpriv->free_xmitframe_cnt--;
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_info_, ("rtw_alloc_xmitframe():free_xmitframe_cnt=%d\n", pxmitpriv->free_xmitframe_cnt));
	}

	_exit_critical_bh(&pfree_xmit_queue->lock, &irqL);

	rtw_init_xmitframe(pxframe);

_func_exit_;

	return pxframe;
}

struct xmit_frame *rtw_alloc_xmitframe_ext(struct xmit_priv *pxmitpriv)
{
	_irqL irqL;
	struct xmit_frame *pxframe = NULL;
	_list *plist, *phead;
	_queue *queue = &pxmitpriv->free_xframe_ext_queue;

_func_enter_;

	_enter_critical_bh(&queue->lock, &irqL);

	if (_rtw_queue_empty(queue) == true) {
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_alloc_xmitframe_ext:%d\n", pxmitpriv->free_xframe_ext_cnt));
		pxframe =  NULL;
	} else {
		phead = get_list_head(queue);
		plist = get_next(phead);
		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		rtw_list_delete(&(pxframe->list));
		pxmitpriv->free_xframe_ext_cnt--;
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_info_, ("rtw_alloc_xmitframe_ext():free_xmitframe_cnt=%d\n", pxmitpriv->free_xframe_ext_cnt));
	}

	_exit_critical_bh(&queue->lock, &irqL);

	rtw_init_xmitframe(pxframe);

_func_exit_;

	return pxframe;
}

struct xmit_frame *rtw_alloc_xmitframe_once(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame *pxframe = NULL;
	u8 *alloc_addr;

	alloc_addr = rtw_zmalloc(sizeof(struct xmit_frame) + 4);
	
	if (alloc_addr == NULL)
		goto exit;
		
	pxframe = (struct xmit_frame *)N_BYTE_ALIGMENT((SIZE_PTR)(alloc_addr), 4);
	pxframe->alloc_addr = alloc_addr;

	pxframe->padapter = pxmitpriv->adapter;
	pxframe->frame_tag = NULL_FRAMETAG;

	pxframe->pkt = NULL;

	pxframe->buf_addr = NULL;
	pxframe->pxmitbuf = NULL;

	rtw_init_xmitframe(pxframe);

	DBG_871X("################## %s ##################\n", __func__);

exit:
	return pxframe;
}

s32 rtw_free_xmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe)
{	
	_irqL irqL;
	_queue *queue = NULL;
	_adapter *padapter = pxmitpriv->adapter;
	_pkt *pndis_pkt = NULL;

_func_enter_;	

	if (pxmitframe == NULL) {
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("======rtw_free_xmitframe():pxmitframe==NULL!!!!!!!!!!\n"));
		goto exit;
	}

	if (pxmitframe->pkt){
		pndis_pkt = pxmitframe->pkt;
		pxmitframe->pkt = NULL;
	}

	if (pxmitframe->alloc_addr) {
		DBG_871X("################## %s with alloc_addr ##################\n", __func__);
		rtw_mfree(pxmitframe->alloc_addr, sizeof(struct xmit_frame) + 4);
		goto check_pkt_complete;
	}

	if (pxmitframe->ext_tag == 0)
		queue = &pxmitpriv->free_xmit_queue;
	else if(pxmitframe->ext_tag == 1)
		queue = &pxmitpriv->free_xframe_ext_queue;
	else
	{}

	_enter_critical_bh(&queue->lock, &irqL);

	rtw_list_delete(&pxmitframe->list);	
	rtw_list_insert_tail(&pxmitframe->list, get_list_head(queue));
	if (pxmitframe->ext_tag == 0) {
		pxmitpriv->free_xmitframe_cnt++;
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_debug_, ("rtw_free_xmitframe():free_xmitframe_cnt=%d\n", pxmitpriv->free_xmitframe_cnt));
	} else if(pxmitframe->ext_tag == 1) {
		pxmitpriv->free_xframe_ext_cnt++;
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_debug_, ("rtw_free_xmitframe():free_xframe_ext_cnt=%d\n", pxmitpriv->free_xframe_ext_cnt));
	} else {
	}

	_exit_critical_bh(&queue->lock, &irqL);

check_pkt_complete:

	if(pndis_pkt)
		rtw_os_pkt_complete(padapter, pndis_pkt);

exit:

_func_exit_;

	return _SUCCESS;
}

void rtw_free_xmitframe_queue(struct xmit_priv *pxmitpriv, _queue *pframequeue)
{
	_irqL irqL;
	_list	*plist, *phead;
	struct	xmit_frame 	*pxmitframe;

_func_enter_;	

	_enter_critical_bh(&(pframequeue->lock), &irqL);

	phead = get_list_head(pframequeue);
	plist = get_next(phead);
	
	while (rtw_end_of_queue_search(phead, plist) == false)
	{
			
		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		plist = get_next(plist); 
		
		rtw_free_xmitframe(pxmitpriv,pxmitframe);
			
	}
	_exit_critical_bh(&(pframequeue->lock), &irqL);

_func_exit_;
}

s32 rtw_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	DBG_COUNTER(padapter->tx_logs.core_tx_enqueue);
	if (rtw_xmit_classifier(padapter, pxmitframe) == _FAIL)
	{
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_,
			 ("rtw_xmitframe_enqueue: drop xmit pkt for classifier fail\n"));
//		pxmitframe->pkt = NULL;
		return _FAIL;
	}

	return _SUCCESS;
}

static struct xmit_frame *dequeue_one_xmitframe(struct xmit_priv *pxmitpriv, struct hw_xmit *phwxmit, struct tx_servq *ptxservq, _queue *pframe_queue)
{
	_list	*xmitframe_plist, *xmitframe_phead;
	struct	xmit_frame	*pxmitframe=NULL;

	xmitframe_phead = get_list_head(pframe_queue);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == false)
	{
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

		rtw_list_delete(&pxmitframe->list);

		ptxservq->qcnt--;

		//rtw_list_insert_tail(&pxmitframe->list, &phwxmit->pending);

		//ptxservq->qcnt--;

		break;		

		pxmitframe = NULL;

	}

	return pxmitframe;
}

struct xmit_frame* rtw_dequeue_xframe(struct xmit_priv *pxmitpriv, struct hw_xmit *phwxmit_i, sint entry)
{
	_irqL irqL0;
	_list *sta_plist, *sta_phead;
	struct hw_xmit *phwxmit;
	struct tx_servq *ptxservq = NULL;
	_queue *pframe_queue = NULL;
	struct xmit_frame *pxmitframe = NULL;
	_adapter *padapter = pxmitpriv->adapter;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	int i, inx[4];

_func_enter_;

	inx[0] = 0; inx[1] = 1; inx[2] = 2; inx[3] = 3;

	if(pregpriv->wifi_spec==1)
	{
		int j, tmp, acirp_cnt[4];

		for(j=0; j<4; j++)
			inx[j] = pxmitpriv->wmm_para_seq[j];
	}

	_enter_critical_bh(&pxmitpriv->lock, &irqL0);

	for(i = 0; i < entry; i++) 
	{
		phwxmit = phwxmit_i + inx[i];

		//_enter_critical_ex(&phwxmit->sta_queue->lock, &irqL0);

		sta_phead = get_list_head(phwxmit->sta_queue);
		sta_plist = get_next(sta_phead);

		while ((rtw_end_of_queue_search(sta_phead, sta_plist)) == false)
		{

			ptxservq= LIST_CONTAINOR(sta_plist, struct tx_servq, tx_pending);

			pframe_queue = &ptxservq->sta_pending;

			pxmitframe = dequeue_one_xmitframe(pxmitpriv, phwxmit, ptxservq, pframe_queue);

			if(pxmitframe)
			{
				phwxmit->accnt--;

				//Remove sta node when there is no pending packets.
				if(_rtw_queue_empty(pframe_queue)) //must be done after get_next and before break
					rtw_list_delete(&ptxservq->tx_pending);

				//_exit_critical_ex(&phwxmit->sta_queue->lock, &irqL0);

				goto exit;
			}

			sta_plist = get_next(sta_plist);

		}

		//_exit_critical_ex(&phwxmit->sta_queue->lock, &irqL0);

	}

exit:

	_exit_critical_bh(&pxmitpriv->lock, &irqL0);

_func_exit_;

	return pxmitframe;
}

struct tx_servq *rtw_get_sta_pending(_adapter *padapter, struct sta_info *psta, sint up, u8 *ac)
{
	struct tx_servq *ptxservq=NULL;
	
_func_enter_;	

	switch (up) 
	{
		case 1:
		case 2:
			ptxservq = &(psta->sta_xmitpriv.bk_q);
			*(ac) = 3;
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_get_sta_pending : BK \n"));
			break;

		case 4:
		case 5:
			ptxservq = &(psta->sta_xmitpriv.vi_q);
			*(ac) = 1;
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_get_sta_pending : VI\n"));
			break;

		case 6:
		case 7:
			ptxservq = &(psta->sta_xmitpriv.vo_q);
			*(ac) = 0;
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_get_sta_pending : VO \n"));			
			break;

		case 0:
		case 3:
		default:
			ptxservq = &(psta->sta_xmitpriv.be_q);
			*(ac) = 2;
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_get_sta_pending : BE \n"));				
		break;
		
	}

_func_exit_;

	return ptxservq;			
}

/*
 * Will enqueue pxmitframe to the proper queue,
 * and indicate it to xx_pending list.....
 */
s32 rtw_xmit_classifier(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	//_irqL irqL0;
	u8	ac_index;
	struct sta_info	*psta;
	struct tx_servq	*ptxservq;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct hw_xmit	*phwxmits =  padapter->xmitpriv.hwxmits;
	sint res = _SUCCESS;

_func_enter_;

	DBG_COUNTER(padapter->tx_logs.core_tx_enqueue_class);

/*
	if (pattrib->psta) {
		psta = pattrib->psta;		
	} else {
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta = rtw_get_stainfo(pstapriv, pattrib->ra);
	}
*/	

	psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
	if(pattrib->psta != psta)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_enqueue_class_err_sta);
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return _FAIL;
	}

	if (psta == NULL) {
		DBG_COUNTER(padapter->tx_logs.core_tx_enqueue_class_err_nosta);
		res = _FAIL;
		DBG_8192C("rtw_xmit_classifier: psta == NULL\n");
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("rtw_xmit_classifier: psta == NULL\n"));
		goto exit;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_enqueue_class_err_fwlink);
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}

	ptxservq = rtw_get_sta_pending(padapter, psta, pattrib->priority, (u8 *)(&ac_index));

	//_enter_critical(&pstapending->lock, &irqL0);

	if (rtw_is_list_empty(&ptxservq->tx_pending)) {
		rtw_list_insert_tail(&ptxservq->tx_pending, get_list_head(phwxmits[ac_index].sta_queue));
	}

	//_enter_critical(&ptxservq->sta_pending.lock, &irqL1);

	rtw_list_insert_tail(&pxmitframe->list, get_list_head(&ptxservq->sta_pending));
	ptxservq->qcnt++;
	phwxmits[ac_index].accnt++;

	//_exit_critical(&ptxservq->sta_pending.lock, &irqL1);

	//_exit_critical(&pstapending->lock, &irqL0);

exit:

_func_exit_;

	return res;
}

void rtw_alloc_hwxmits(_adapter *padapter)
{
	struct hw_xmit *hwxmits;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	pxmitpriv->hwxmit_entry = HWXMIT_ENTRY;

	pxmitpriv->hwxmits = NULL;

	pxmitpriv->hwxmits = (struct hw_xmit *)rtw_zmalloc(sizeof (struct hw_xmit) * pxmitpriv->hwxmit_entry);	

	if(pxmitpriv->hwxmits == NULL)
	{
		DBG_871X("alloc hwxmits fail!...\n");
		return;
	}
	
	hwxmits = pxmitpriv->hwxmits;

	if(pxmitpriv->hwxmit_entry == 5)
	{
		//pxmitpriv->bmc_txqueue.head = 0;
		//hwxmits[0] .phwtxqueue = &pxmitpriv->bmc_txqueue;
		hwxmits[0] .sta_queue = &pxmitpriv->bm_pending;
	
		//pxmitpriv->vo_txqueue.head = 0;
		//hwxmits[1] .phwtxqueue = &pxmitpriv->vo_txqueue;
		hwxmits[1] .sta_queue = &pxmitpriv->vo_pending;

		//pxmitpriv->vi_txqueue.head = 0;
		//hwxmits[2] .phwtxqueue = &pxmitpriv->vi_txqueue;
		hwxmits[2] .sta_queue = &pxmitpriv->vi_pending;
	
		//pxmitpriv->bk_txqueue.head = 0;
		//hwxmits[3] .phwtxqueue = &pxmitpriv->bk_txqueue;
		hwxmits[3] .sta_queue = &pxmitpriv->bk_pending;

      		//pxmitpriv->be_txqueue.head = 0;
		//hwxmits[4] .phwtxqueue = &pxmitpriv->be_txqueue;
		hwxmits[4] .sta_queue = &pxmitpriv->be_pending;
		
	}	
	else if(pxmitpriv->hwxmit_entry == 4)
	{

		//pxmitpriv->vo_txqueue.head = 0;
		//hwxmits[0] .phwtxqueue = &pxmitpriv->vo_txqueue;
		hwxmits[0] .sta_queue = &pxmitpriv->vo_pending;

		//pxmitpriv->vi_txqueue.head = 0;
		//hwxmits[1] .phwtxqueue = &pxmitpriv->vi_txqueue;
		hwxmits[1] .sta_queue = &pxmitpriv->vi_pending;

		//pxmitpriv->be_txqueue.head = 0;
		//hwxmits[2] .phwtxqueue = &pxmitpriv->be_txqueue;
		hwxmits[2] .sta_queue = &pxmitpriv->be_pending;
	
		//pxmitpriv->bk_txqueue.head = 0;
		//hwxmits[3] .phwtxqueue = &pxmitpriv->bk_txqueue;
		hwxmits[3] .sta_queue = &pxmitpriv->bk_pending;
	}
	else
	{
		

	}
	

}

void rtw_free_hwxmits(_adapter *padapter)
{
	struct hw_xmit *hwxmits;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	hwxmits = pxmitpriv->hwxmits;
	if(hwxmits)
		rtw_mfree((u8 *)hwxmits, (sizeof (struct hw_xmit) * pxmitpriv->hwxmit_entry));
}

void rtw_init_hwxmits(struct hw_xmit *phwxmit, sint entry)
{
	sint i;
_func_enter_;	
	for(i = 0; i < entry; i++, phwxmit++)
	{
		//spin_lock_init(&phwxmit->xmit_lock);
		//_rtw_init_listhead(&phwxmit->pending);		
		//phwxmit->txcmdcnt = 0;
		phwxmit->accnt = 0;
	}
_func_exit_;	
}

u32 rtw_get_ff_hwaddr(struct xmit_frame *pxmitframe)
{
	u32 addr;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;	
	
	switch(pattrib->qsel)
	{
		case 0:
		case 3:
			addr = BE_QUEUE_INX;
		 	break;
		case 1:
		case 2:
			addr = BK_QUEUE_INX;
			break;				
		case 4:
		case 5:
			addr = VI_QUEUE_INX;
			break;		
		case 6:
		case 7:
			addr = VO_QUEUE_INX;
			break;
		case 0x10:
			addr = BCN_QUEUE_INX;
			break;
		case 0x11://BC/MC in PS (HIQ)
			addr = HIGH_QUEUE_INX;
			break;
		case 0x12:
		default:
			addr = MGT_QUEUE_INX;
			break;		
			
	}

	return addr;

}

static void do_queue_select(_adapter	*padapter, struct pkt_attrib *pattrib)
{
	u8 qsel;
		
	qsel = pattrib->priority;
	RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("### do_queue_select priority=%d ,qsel = %d\n",pattrib->priority ,qsel));

	pattrib->qsel = qsel;
}

/*
 * The main transmit(tx) entry
 *
 * Return
 *	1	enqueue
 *	0	success, hardware will handle this xmit frame(packet)
 *	<0	fail
 */
s32 rtw_xmit(_adapter *padapter, _pkt **ppkt)
{
	static unsigned long start = 0;
	static u32 drop_cnt = 0;
#ifdef CONFIG_AP_MODE
	_irqL irqL0;
#endif
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct xmit_frame *pxmitframe = NULL;

	s32 res;

	DBG_COUNTER(padapter->tx_logs.core_tx);

	if (start == 0)
		start = jiffies;

	pxmitframe = rtw_alloc_xmitframe(pxmitpriv);

	if (jiffies_to_msecs(jiffies - start) > 2000) {
		if (drop_cnt)
			DBG_871X("DBG_TX_DROP_FRAME %s no more pxmitframe, drop_cnt:%u\n", __FUNCTION__, drop_cnt);
		start = jiffies;
		drop_cnt = 0;
	}

	if (pxmitframe == NULL) {
		drop_cnt ++;
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("rtw_xmit: no more pxmitframe\n"));
		DBG_COUNTER(padapter->tx_logs.core_tx_err_pxmitframe);
		return -1;
	}

	res = update_attrib(padapter, *ppkt, &pxmitframe->attrib);

#ifdef CONFIG_WAPI_SUPPORT
	if(pxmitframe->attrib.ether_type != 0x88B4)
	{
		if(rtw_wapi_drop_for_key_absent(padapter, pxmitframe->attrib.ra))
		{
			WAPI_TRACE(WAPI_RX,"drop for key absend when tx \n");
			res = _FAIL;
		}
	}
#endif
	if (res == _FAIL) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("rtw_xmit: update attrib fail\n"));
		#ifdef DBG_TX_DROP_FRAME
		DBG_871X("DBG_TX_DROP_FRAME %s update attrib fail\n", __FUNCTION__);
		#endif
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
		return -1;
	}
	pxmitframe->pkt = *ppkt;

	rtw_led_control(padapter, LED_CTL_TX);

	do_queue_select(padapter, &pxmitframe->attrib);

#ifdef CONFIG_AP_MODE
	_enter_critical_bh(&pxmitpriv->lock, &irqL0);
	if(xmitframe_enqueue_for_sleeping_sta(padapter, pxmitframe) == true)
	{
		_exit_critical_bh(&pxmitpriv->lock, &irqL0);
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue);
		return 1;		
	}
	_exit_critical_bh(&pxmitpriv->lock, &irqL0);
#endif

	//pre_xmitframe
	if (rtw_hal_xmit(padapter, pxmitframe) == false)
		return 1;

	return 0;
}

#define RTW_HIQ_FILTER_ALLOW_ALL 0
#define RTW_HIQ_FILTER_ALLOW_SPECIAL 1
#define RTW_HIQ_FILTER_DENY_ALL 2

inline bool xmitframe_hiq_filter(struct xmit_frame *xmitframe)
{
	bool allow = false;
	_adapter *adapter = xmitframe->padapter;
	struct registry_priv *registry = &adapter->registrypriv;

	if (registry->hiq_filter == RTW_HIQ_FILTER_ALLOW_SPECIAL) {
	
		struct pkt_attrib *attrib = &xmitframe->attrib;

		if (attrib->ether_type == 0x0806
			|| attrib->ether_type == 0x888e
			#ifdef CONFIG_WAPI_SUPPORT
			|| attrib->ether_type == 0x88B4
			#endif
			|| attrib->dhcp_pkt
		) {
			DBG_871X(FUNC_ADPT_FMT" ether_type:0x%04x%s\n", FUNC_ADPT_ARG(xmitframe->padapter)
				, attrib->ether_type, attrib->dhcp_pkt?" DHCP":"");
			allow = true;
		}
	}
	else if (registry->hiq_filter == RTW_HIQ_FILTER_ALLOW_ALL) {
		allow = true;
	}
	else if (registry->hiq_filter == RTW_HIQ_FILTER_DENY_ALL) {
	}
	else {
		rtw_warn_on(1);
	}

	return allow;
}

#if defined(CONFIG_AP_MODE)

sint xmitframe_enqueue_for_sleeping_sta(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	_irqL irqL;
	sint ret=false;
	struct sta_info *psta=NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	sint bmcst = IS_MCAST(pattrib->ra);
	bool update_tim = false;

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == false)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_warn_fwstate);
	    return ret;
	}
/*
	if(pattrib->psta)
	{
		psta = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(pstapriv, pattrib->ra);
	}
*/
	psta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
	if(pattrib->psta != psta)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_warn_sta);
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return false;
	}

	if(psta==NULL)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_warn_nosta);
		DBG_871X("%s, psta==NUL\n", __func__);
		return false;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_warn_link);
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return false;
	}

	if(pattrib->triggered==1)
	{
		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_warn_trigger);
		//DBG_871X("directly xmit pspoll_triggered packet\n");

		//pattrib->triggered=0;
		if (bmcst && xmitframe_hiq_filter(pxmitframe) == true)
			pattrib->qsel = 0x11;//HIQ

		return ret;
	}


	if(bmcst)
	{
		_enter_critical_bh(&psta->sleep_q.lock, &irqL);	
	
		if(pstapriv->sta_dz_bitmap)//if anyone sta is in ps mode
		{
			//pattrib->qsel = 0x11;//HIQ
			
			rtw_list_delete(&pxmitframe->list);
			
			//_enter_critical_bh(&psta->sleep_q.lock, &irqL);	
			
			rtw_list_insert_tail(&pxmitframe->list, get_list_head(&psta->sleep_q));
			
			psta->sleepq_len++;

			if (!(pstapriv->tim_bitmap & BIT(0)))
				update_tim = true;

			pstapriv->tim_bitmap |= BIT(0);//
			pstapriv->sta_dz_bitmap |= BIT(0);

			//DBG_871X("enqueue, sq_len=%d, tim=%x\n", psta->sleepq_len, pstapriv->tim_bitmap);

			if (update_tim == true) {
				update_beacon(padapter, _TIM_IE_, NULL, true);
			} else {
				chk_bmc_sleepq_cmd(padapter);
			}

			//_exit_critical_bh(&psta->sleep_q.lock, &irqL);				
			
			ret = true;			

			DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_mcast);
			
		}
		
		_exit_critical_bh(&psta->sleep_q.lock, &irqL);	
		
		return ret;
		
	}
	

	_enter_critical_bh(&psta->sleep_q.lock, &irqL);	
	
	if(psta->state&WIFI_SLEEP_STATE)
	{
		u8 wmmps_ac=0;
	
		if(pstapriv->sta_dz_bitmap&BIT(psta->aid))	
		{			
			rtw_list_delete(&pxmitframe->list);
		
			//_enter_critical_bh(&psta->sleep_q.lock, &irqL);	
			
			rtw_list_insert_tail(&pxmitframe->list, get_list_head(&psta->sleep_q));
			
			psta->sleepq_len++;

			switch(pattrib->priority)
			{
				case 1:
				case 2:
					wmmps_ac = psta->uapsd_bk&BIT(0);
					break;
				case 4:
				case 5:
					wmmps_ac = psta->uapsd_vi&BIT(0);
					break;
				case 6:
				case 7:
					wmmps_ac = psta->uapsd_vo&BIT(0);
					break;
				case 0:
				case 3:
				default:
					wmmps_ac = psta->uapsd_be&BIT(0);
					break;
			}

			if(wmmps_ac)
				psta->sleepq_ac_len++;

			if(((psta->has_legacy_ac) && (!wmmps_ac)) ||((!psta->has_legacy_ac)&&(wmmps_ac)))
			{
				if (!(pstapriv->tim_bitmap & BIT(psta->aid)))
					update_tim = true;

				pstapriv->tim_bitmap |= BIT(psta->aid);

				//DBG_871X("enqueue, sq_len=%d, tim=%x\n", psta->sleepq_len, pstapriv->tim_bitmap);

				if(update_tim == true)
				{
					//DBG_871X("sleepq_len==1, update BCNTIM\n");
					//upate BCN for TIM IE
					update_beacon(padapter, _TIM_IE_, NULL, true);
				}
			}

			//_exit_critical_bh(&psta->sleep_q.lock, &irqL);			

			//if(psta->sleepq_len > (NR_XMITFRAME>>3))
			//{
			//	wakeup_sta_to_xmit(padapter, psta);
			//}	

			ret = true;

			DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_ucast);
		}

	}

	_exit_critical_bh(&psta->sleep_q.lock, &irqL);	

	return ret;
	
}

static void dequeue_xmitframes_to_sleeping_queue(_adapter *padapter, struct sta_info *psta, _queue *pframequeue)
{
	sint ret;
	_list	*plist, *phead;
	u8	ac_index;
	struct tx_servq	*ptxservq;
	struct pkt_attrib	*pattrib;
	struct xmit_frame 	*pxmitframe;
	struct hw_xmit *phwxmits =  padapter->xmitpriv.hwxmits;
	
	phead = get_list_head(pframequeue);
	plist = get_next(phead);
	
	while (rtw_end_of_queue_search(phead, plist) == false)
	{			
		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		plist = get_next(plist);
		
		pattrib = &pxmitframe->attrib;

		pattrib->triggered = 0;
  		
		ret = xmitframe_enqueue_for_sleeping_sta(padapter, pxmitframe);	

		if(true == ret)
		{
			ptxservq = rtw_get_sta_pending(padapter, psta, pattrib->priority, (u8 *)(&ac_index));

			ptxservq->qcnt--;
			phwxmits[ac_index].accnt--;
		}
		else
		{
			//DBG_871X("xmitframe_enqueue_for_sleeping_sta return false\n");
		}
		
	}
	
}

void stop_sta_xmit(_adapter *padapter, struct sta_info *psta)
{	
	_irqL irqL0;	
	struct sta_info *psta_bmc;
	struct sta_xmit_priv *pstaxmitpriv;
	struct sta_priv *pstapriv = &padapter->stapriv;	
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;	
	
	pstaxmitpriv = &psta->sta_xmitpriv;

	//for BC/MC Frames
	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	
			
	_enter_critical_bh(&pxmitpriv->lock, &irqL0);

	psta->state |= WIFI_SLEEP_STATE;
	
	pstapriv->sta_dz_bitmap |= BIT(psta->aid);
	
	

	dequeue_xmitframes_to_sleeping_queue(padapter, psta, &pstaxmitpriv->vo_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->vo_q.tx_pending));


	dequeue_xmitframes_to_sleeping_queue(padapter, psta, &pstaxmitpriv->vi_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->vi_q.tx_pending));


	dequeue_xmitframes_to_sleeping_queue(padapter, psta, &pstaxmitpriv->be_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->be_q.tx_pending));
	

	dequeue_xmitframes_to_sleeping_queue(padapter, psta, &pstaxmitpriv->bk_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->bk_q.tx_pending));

	//for BC/MC Frames
	pstaxmitpriv = &psta_bmc->sta_xmitpriv;
	dequeue_xmitframes_to_sleeping_queue(padapter, psta_bmc, &pstaxmitpriv->be_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->be_q.tx_pending));
	
	_exit_critical_bh(&pxmitpriv->lock, &irqL0);
}	

void wakeup_sta_to_xmit(_adapter *padapter, struct sta_info *psta)
{	 
	_irqL irqL;	 
	u8 update_mask=0, wmmps_ac=0;
	struct sta_info *psta_bmc;
	_list	*xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe=NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	

	//_enter_critical_bh(&psta->sleep_q.lock, &irqL);
	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	xmitframe_phead = get_list_head(&psta->sleep_q);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == false)
	{
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

		rtw_list_delete(&pxmitframe->list);

		switch(pxmitframe->attrib.priority)
		{
			case 1:
			case 2:
				wmmps_ac = psta->uapsd_bk&BIT(1);
				break;
			case 4:
			case 5:
				wmmps_ac = psta->uapsd_vi&BIT(1);
				break;
			case 6:
			case 7:
				wmmps_ac = psta->uapsd_vo&BIT(1);
				break;
			case 0:
			case 3:
			default:
				wmmps_ac = psta->uapsd_be&BIT(1);
				break;
		}

		psta->sleepq_len--;
		if(psta->sleepq_len>0)
			pxmitframe->attrib.mdata = 1;
		else
			pxmitframe->attrib.mdata = 0;

		if(wmmps_ac)
		{
			psta->sleepq_ac_len--;
			if(psta->sleepq_ac_len>0)
			{
				pxmitframe->attrib.mdata = 1;
				pxmitframe->attrib.eosp = 0;
			}
			else
			{
				pxmitframe->attrib.mdata = 0;
				pxmitframe->attrib.eosp = 1;
			}
		}

		pxmitframe->attrib.triggered = 1;

/*
		_exit_critical_bh(&psta->sleep_q.lock, &irqL);
		if(rtw_hal_xmit(padapter, pxmitframe) == true)
		{
			rtw_os_xmit_complete(padapter, pxmitframe);
		}
		_enter_critical_bh(&psta->sleep_q.lock, &irqL);
*/
		rtw_hal_xmitframe_enqueue(padapter, pxmitframe);


	}

	if(psta->sleepq_len==0)
	{
		if (pstapriv->tim_bitmap & BIT(psta->aid)) {
			//DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap);
			//upate BCN for TIM IE
			//update_BCNTIM(padapter);
			update_mask = BIT(0);
		}

		pstapriv->tim_bitmap &= ~BIT(psta->aid);

		if(psta->state&WIFI_SLEEP_STATE)
			psta->state ^= WIFI_SLEEP_STATE;

		if(psta->state & WIFI_STA_ALIVE_CHK_STATE)
		{
			DBG_871X("%s alive check\n", __func__);
			psta->expire_to = pstapriv->expire_to;
			psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
		}

		pstapriv->sta_dz_bitmap &= ~BIT(psta->aid);
	}

	//for BC/MC Frames
	if(!psta_bmc)
		goto _exit;

	if((pstapriv->sta_dz_bitmap&0xfffe) == 0x0)//no any sta in ps mode
	{
		xmitframe_phead = get_list_head(&psta_bmc->sleep_q);
		xmitframe_plist = get_next(xmitframe_phead);

		while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == false)
		{
			pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

			xmitframe_plist = get_next(xmitframe_plist);

			rtw_list_delete(&pxmitframe->list);

			psta_bmc->sleepq_len--;
			if(psta_bmc->sleepq_len>0)
				pxmitframe->attrib.mdata = 1;
			else
				pxmitframe->attrib.mdata = 0;


			pxmitframe->attrib.triggered = 1;
/*
			_exit_critical_bh(&psta_bmc->sleep_q.lock, &irqL);
			if(rtw_hal_xmit(padapter, pxmitframe) == true)
			{
				rtw_os_xmit_complete(padapter, pxmitframe);
			}
			_enter_critical_bh(&psta_bmc->sleep_q.lock, &irqL);

*/
			rtw_hal_xmitframe_enqueue(padapter, pxmitframe);

		}

		if(psta_bmc->sleepq_len==0)
		{
			if (pstapriv->tim_bitmap & BIT(0)) {
				//DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap);
				//upate BCN for TIM IE
				//update_BCNTIM(padapter);
				update_mask |= BIT(1);
			}
			pstapriv->tim_bitmap &= ~BIT(0);
			pstapriv->sta_dz_bitmap &= ~BIT(0);
		}

	}	

_exit:

	//_exit_critical_bh(&psta_bmc->sleep_q.lock, &irqL);	
	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	if(update_mask)
	{
		//update_BCNTIM(padapter);
		//printk("%s => call update_beacon\n",__FUNCTION__);
		update_beacon(padapter, _TIM_IE_, NULL, true);
	}
	
}

void xmit_delivery_enabled_frames(_adapter *padapter, struct sta_info *psta)
{
	_irqL irqL;
	u8 wmmps_ac=0;
	_list	*xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe=NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;


	//_enter_critical_bh(&psta->sleep_q.lock, &irqL);
	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	xmitframe_phead = get_list_head(&psta->sleep_q);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == false)
	{			
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

		switch(pxmitframe->attrib.priority)
		{
			case 1:
			case 2:
				wmmps_ac = psta->uapsd_bk&BIT(1);
				break;
			case 4:
			case 5:
				wmmps_ac = psta->uapsd_vi&BIT(1);
				break;
			case 6:
			case 7:
				wmmps_ac = psta->uapsd_vo&BIT(1);
				break;
			case 0:
			case 3:
			default:
				wmmps_ac = psta->uapsd_be&BIT(1);
				break;	
		}
		
		if(!wmmps_ac)
			continue;

		rtw_list_delete(&pxmitframe->list);

		psta->sleepq_len--;
		psta->sleepq_ac_len--;

		if(psta->sleepq_ac_len>0)
		{
			pxmitframe->attrib.mdata = 1;
			pxmitframe->attrib.eosp = 0;
		}
		else
		{
			pxmitframe->attrib.mdata = 0;
			pxmitframe->attrib.eosp = 1;
		}

		pxmitframe->attrib.triggered = 1;
		rtw_hal_xmitframe_enqueue(padapter, pxmitframe);

		if((psta->sleepq_ac_len==0) && (!psta->has_legacy_ac) && (wmmps_ac))
		{
			pstapriv->tim_bitmap &= ~BIT(psta->aid);

			//DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap);
			//upate BCN for TIM IE
			//update_BCNTIM(padapter);
			update_beacon(padapter, _TIM_IE_, NULL, true);
			//update_mask = BIT(0);
		}
	
	}	

	//_exit_critical_bh(&psta->sleep_q.lock, &irqL);	
	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	return;
}

#endif

void enqueue_pending_xmitbuf(
	struct xmit_priv *pxmitpriv,
	struct xmit_buf *pxmitbuf)
{
	_irqL irql;
	_queue *pqueue;
	_adapter *pri_adapter = pxmitpriv->adapter;

	pqueue = &pxmitpriv->pending_xmitbuf_queue;

	_enter_critical_bh(&pqueue->lock, &irql);
	rtw_list_delete(&pxmitbuf->list);
	rtw_list_insert_tail(&pxmitbuf->list, get_list_head(pqueue));
	_exit_critical_bh(&pqueue->lock, &irql);

	up(&(pri_adapter->xmitpriv.xmit_sema));
}

void enqueue_pending_xmitbuf_to_head(
	struct xmit_priv *pxmitpriv,
	struct xmit_buf *pxmitbuf)
{
	_irqL irql;
	_queue *pqueue;
	_adapter *pri_adapter = pxmitpriv->adapter;

	pqueue = &pxmitpriv->pending_xmitbuf_queue;

	_enter_critical_bh(&pqueue->lock, &irql);
	rtw_list_delete(&pxmitbuf->list);
	rtw_list_insert_head(&pxmitbuf->list, get_list_head(pqueue));
	_exit_critical_bh(&pqueue->lock, &irql);
}

struct xmit_buf* dequeue_pending_xmitbuf(
	struct xmit_priv *pxmitpriv)
{
	_irqL irql;
	struct xmit_buf *pxmitbuf;
	_queue *pqueue;


	pxmitbuf = NULL;
	pqueue = &pxmitpriv->pending_xmitbuf_queue;

	_enter_critical_bh(&pqueue->lock, &irql);

	if (_rtw_queue_empty(pqueue) == false)
	{
		_list *plist, *phead;

		phead = get_list_head(pqueue);
		plist = get_next(phead);
		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);
		rtw_list_delete(&pxmitbuf->list);
	}

	_exit_critical_bh(&pqueue->lock, &irql);

	return pxmitbuf;
}

struct xmit_buf* dequeue_pending_xmitbuf_under_survey(
	struct xmit_priv *pxmitpriv)
{
	_irqL irql;
	struct xmit_buf *pxmitbuf;
	_queue *pqueue;


	pxmitbuf = NULL;
	pqueue = &pxmitpriv->pending_xmitbuf_queue;

	_enter_critical_bh(&pqueue->lock, &irql);

	if (_rtw_queue_empty(pqueue) == false)
	{
		_list *plist, *phead;
		u8 type;

		phead = get_list_head(pqueue);
		plist = phead;
		do {
			plist = get_next(plist);
				if (plist == phead) break;
			
			pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

			type = GetFrameSubType(pxmitbuf->pbuf + TXDESC_OFFSET);

			if ((type == WIFI_PROBEREQ) ||
				(type == WIFI_DATA_NULL) ||
				(type == WIFI_QOS_DATA_NULL))
			{
				rtw_list_delete(&pxmitbuf->list);
				break;
			}
			pxmitbuf = NULL;
		} while (1);
	}

	_exit_critical_bh(&pqueue->lock, &irql);

	return pxmitbuf;
}

sint check_pending_xmitbuf(
	struct xmit_priv *pxmitpriv)
{
	_irqL irql;
	_queue *pqueue;
	sint	ret = false;

	pqueue = &pxmitpriv->pending_xmitbuf_queue;

	_enter_critical_bh(&pqueue->lock, &irql);

	if(_rtw_queue_empty(pqueue) == false)
		ret = true;

	_exit_critical_bh(&pqueue->lock, &irql);

	return ret;
}

int rtw_xmit_thread(void * context)
{
	s32 err;
	PADAPTER padapter;


	err = _SUCCESS;
	padapter = (PADAPTER)context;

	thread_enter("RTW_XMIT_THREAD");

	do {
		err = rtw_hal_xmit_thread_handler(padapter);
		flush_signals_thread();
	} while (_SUCCESS == err);

	up(&padapter->xmitpriv.terminate_xmitthread_sema);

	thread_exit();
}

void rtw_sctx_init(struct submit_ctx *sctx, int timeout_ms)
{
	sctx->timeout_ms = timeout_ms;
	sctx->submit_time= jiffies;
	init_completion(&sctx->done);
	sctx->status = RTW_SCTX_SUBMITTED;
}

int rtw_sctx_wait(struct submit_ctx *sctx, const char *msg)
{
	int ret = _FAIL;
	unsigned long expire; 
	int status = 0;

	expire= sctx->timeout_ms ? msecs_to_jiffies(sctx->timeout_ms) : MAX_SCHEDULE_TIMEOUT;
	if (!wait_for_completion_timeout(&sctx->done, expire)) {
		/* timeout, do something?? */
		status = RTW_SCTX_DONE_TIMEOUT;
		DBG_871X("%s timeout: %s\n", __func__, msg);
	} else {
		status = sctx->status;
	}

	if (status == RTW_SCTX_DONE_SUCCESS) {
		ret = _SUCCESS;
	}

	return ret;
}

static bool rtw_sctx_chk_waring_status(int status)
{
	switch(status) {
	case RTW_SCTX_DONE_UNKNOWN:
	case RTW_SCTX_DONE_BUF_ALLOC:
	case RTW_SCTX_DONE_BUF_FREE:

	case RTW_SCTX_DONE_DRV_STOP:
	case RTW_SCTX_DONE_DEV_REMOVE:
		return true;
	default:
		return false;
	}
}

void rtw_sctx_done_err(struct submit_ctx **sctx, int status)
{
	if (*sctx) {
		if (rtw_sctx_chk_waring_status(status))
			DBG_871X("%s status:%d\n", __func__, status);
		(*sctx)->status = status;
		complete(&((*sctx)->done));
		*sctx = NULL;
	}
}

void rtw_sctx_done(struct submit_ctx **sctx)
{
	rtw_sctx_done_err(sctx, RTW_SCTX_DONE_SUCCESS);
}

int rtw_ack_tx_wait(struct xmit_priv *pxmitpriv, u32 timeout_ms)
{
	struct submit_ctx *pack_tx_ops = &pxmitpriv->ack_tx_ops;

	pack_tx_ops->submit_time = jiffies;
	pack_tx_ops->timeout_ms = timeout_ms;
	pack_tx_ops->status = RTW_SCTX_SUBMITTED;

	return rtw_sctx_wait(pack_tx_ops, __func__);
}

void rtw_ack_tx_done(struct xmit_priv *pxmitpriv, int status)
{
	struct submit_ctx *pack_tx_ops = &pxmitpriv->ack_tx_ops;
	
	if (pxmitpriv->ack_tx) {
		rtw_sctx_done_err(&pack_tx_ops, status);
	} else {
		DBG_871X("%s ack_tx not set\n", __func__);
	}
}
